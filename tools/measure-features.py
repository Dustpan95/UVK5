#!/usr/bin/env python3
"""Measure diagnostic feature variants without changing the real checkout.

Diagnostic builds use an enlarged FLASH region only in ignored copies, so an
oversize image can be measured. They must never be flashed or released. Module
stubs are counterfactual cost probes, not supported user-facing feature flags.
"""
from pathlib import Path
import argparse
import concurrent.futures
import hashlib
import json
import re
import shutil
import subprocess

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/feature-costs'
SOURCE=OUT/'source'
OUT.mkdir(parents=True,exist_ok=True)
if SOURCE.exists(): shutil.rmtree(SOURCE)
def ignore(directory,names):
    return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(ROOT,SOURCE,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(SOURCE.rglob('*')):
    if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
        fingerprint.update(str(p.relative_to(SOURCE)).encode());fingerprint.update(p.read_bytes())
variants=[('baseline',[],None)]
for label,flag in [
 ('messaging_editor','ENABLE_APRS_MESSAGING=1'),
 ('font_packing','ENABLE_FONT_PACKING=1'),('font_unpacked','ENABLE_FONT_PACKING=0'),
 ('spectrum','ENABLE_SPECTRUM=1'),('fm_broadcast','ENABLE_FMRADIO=1'),
 ('vox','ENABLE_VOX=1'),('voice_prompts','ENABLE_VOICE=1'),('noaa','ENABLE_NOAA=1'),
 ('dtmf_calling','ENABLE_DTMF_CALLING=1'),('mic_audio_bar','ENABLE_AUDIO_BAR=1'),
 ('bold_small_font','ENABLE_SMALL_BOLD=1'),('1750hz','ENABLE_TX1750=1'),
 ('without_am_fix','ENABLE_AM_FIX=0'),('without_adaptive_scan','ENABLE_ADAPTIVE_SCAN=0'),
 ('without_scan_ranges','ENABLE_SCAN_RANGES=0'),('without_flashlight','ENABLE_FLASHLIGHT=0')]:
    variants.append((label,[flag],None))
variants.append(('messaging_packed',['ENABLE_APRS_MESSAGING=1','ENABLE_FONT_PACKING=1'],None))
notice='/* Diagnostic stub in a complete isolated firmware copy; not release code. */\n'
stubs={
 'compressor':('tx_compressor.c','#include "tx_compressor.h"\nCompressorConfig_t gCompressorConfig;\nvoid TX_COMPRESSOR_Start(void){}\nvoid TX_COMPRESSOR_Process(void){}\nvoid TX_COMPRESSOR_Stop(void){}\n'),
 'adaptive_squelch':('smart_squelch.c','#include "smart_squelch.h"\nSmartSquelchState_t gSmartSquelch;\nvoid SMART_SQUELCH_Update(void){}\nint8_t SMART_SQUELCH_GetAdjustment(void){return 0;}\n'),
 'scanwatch':('scanwatch.c','#include "scanwatch.h"\nScanWatch_t gScanWatch;\nuint8_t gScanWatchArmed;\nvoid SCANWATCH_Enable(uint8_t v){(void)v;}\nvoid SCANWATCH_Disable(void){}\nbool SCANWATCH_OnScanStep(void){return false;}\nbool SCANWATCH_Update(bool signal){(void)signal;return false;}\n'),
 'vfo_split':('vfo_split.c','#include "vfo_split.h"\nVfoSplit_t gVfoSplit;\nvoid VFO_SPLIT_Process(void){}\n'),
 'about':('ui/vuurwerk_about.c','#include "ui/vuurwerk_about.h"\nvoid VUURWERK_ABOUT_Show(void){}\n')}
for label,payload in stubs.items():variants.append(('stub_'+label,[],payload))
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--profile',action='append',help='Measure only named profiles plus baseline')
args=parser.parse_args()
if args.profile:
    unknown=set(args.profile)-{v[0] for v in variants}
    if unknown: parser.error('Unknown profiles: '+', '.join(sorted(unknown)))
    variants=[v for v in variants if v[0]=='baseline' or v[0] in args.profile]
image='uvk6-build:verified'
subprocess.run(['docker','image','inspect',image],stdout=subprocess.DEVNULL,check=True)
def build(item):
    label,flags,stub=item
    work=OUT/label
    if work.exists():shutil.rmtree(work)
    shutil.copytree(SOURCE,work)
    p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
    if stub:
        relative,body=stub;p=work/relative
        old=p.read_text();end=old.find('*/')
        header=old[:end+2]+'\n' if end>=0 else ''
        archive=work/'diagnostic-originals'/relative
        archive.parent.mkdir(parents=True,exist_ok=True);archive.write_text(old)
        p.write_text(header+notice+body)
    command=['docker','run','--rm','--user','1000:1000','-v',str(work)+':/src','-w','/src',image,
             'bash','-c','set -e; make -j2 "$@"; arm-none-eabi-size -A uvk6-experimental','bash',*flags]
    result=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (work/'build.log').write_text(result.stdout)
    info={'profile':label,'flags':flags,'kind':'module-body diagnostic stub' if stub else 'build flag','exit_code':result.returncode}
    if result.returncode==0:
        info['flash_bytes']=(work/'uvk6-experimental.bin').stat().st_size
        sizes={}
        for line in result.stdout.splitlines():
            f=line.split()
            if len(f)==3 and f[0] in {'.data','.bss','._user_heap_stack'}:sizes[f[0]]=int(f[1])
        info['static_ram_bytes']=sizes.get('.data',0)+sizes.get('.bss',0)
        info['fits_60k']=info['flash_bytes']<=61440
    else:
        info['errors']=[line for line in result.stdout.splitlines() if 'error:' in line or 'undefined reference' in line][:6]
    print(json.dumps(info),flush=True)
    return info
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool: results=list(pool.map(build,variants))
base=results[0].get('flash_bytes')
for item in results:
    if 'flash_bytes' in item and base is not None:item['delta_flash_bytes']=item['flash_bytes']-base
(OUT/'results.json').write_text(json.dumps({'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1','diagnostic_flash_limit':131072,'real_flash_limit':61440,'profiles':results},indent=2)+'\n')
print('Saved',OUT/'results.json',flush=True)
