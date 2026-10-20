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

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--profile',action='append',help='Measure only named profiles plus baseline')
parser.add_argument('--output',default=None,help='Ignored build output directory name')
parser.add_argument('--transport',action='store_true',help='Measure against the actual complete APRS transport profile')
args=parser.parse_args()
ROOT=Path(__file__).resolve().parents[1]
if args.output and (not args.output.replace('-','').isalnum()):parser.error('Invalid output name')
OUT=ROOT/'build'/(args.output or ('transport-feature-costs' if args.transport else 'feature-costs'))
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
 ('session_receiver','ENABLE_APRS_SESSION=1'),
 ('without_uart','ENABLE_UART=0'),('without_wide_rx','ENABLE_WIDE_RX=0'),
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
if args.transport:
    variants=[v for v in variants if v[0]=='baseline' or v[0].startswith('without_')]
    variants.append(('flat_settings_menu',['ENABLE_CUSTOM_MENU_LAYOUT=0'],None))
    stubs.update({
      'bandscope':('bandscope.c','#include "bandscope.h"\nvoid BANDSCOPE_SetEnabled(bool x){(void)x;}\nbool BANDSCOPE_IsEnabled(void){return false;}\nvoid BANDSCOPE_Process(void){}\nvoid BANDSCOPE_Render(uint8_t *p){(void)p;}\n'),
      'gain_staging':('gain_staging.c','#include "gain_staging.h"\nGainStaging_t gGainStaging[2];\nvoid GAIN_STAGING_10ms(uint8_t v){(void)v;}\nint8_t GAIN_STAGING_GetGainDiff(uint8_t v){(void)v;return 0;}\n'),
      'signal_quality':('signal_quality.c','#include "signal_quality.h"\nRssiHistory_t gRssiHistory;\nvoid SIGNAL_QUALITY_Update(int16_t r,uint32_t f){(void)r;(void)f;}\nSignalQuality_t SIGNAL_QUALITY_Get(void){return QUALITY_GOOD;}\n'),
      'signal_classifier':('signal_classifier.c','#include "signal_classifier.h"\nSignalClassifier_t gSignalClassifier[2];\nvoid SIGNAL_CLASSIFIER_Update(uint8_t v,int16_t r,uint32_t f){(void)v;(void)r;(void)f;}\nSignalClass_t SIGNAL_CLASSIFIER_GetClass(uint8_t v){(void)v;return SIGNAL_CLASS_NORMAL;}\nchar SIGNAL_CLASSIFIER_GetSymbol(uint8_t v){(void)v;return 78;}\n'),
      'dual_watch_management':('dual_watch_mgmt.c','#include "dual_watch_mgmt.h"\nvoid DUAL_WATCH_MGMT_Init(void){}\nuint16_t DUAL_WATCH_MGMT_GetDwellTime(uint8_t v){(void)v;return 10;}\nvoid DUAL_WATCH_MGMT_ReportActivity(uint8_t v,uint16_t t){(void)v;(void)t;}\n'),
      'squelch_tail':('squelch_tail.c','#include "squelch_tail.h"\nSquelchTail_t gSquelchTail;\nvoid SQUELCH_TAIL_Process(void){}\n'),
      'ctcss_lead':('ctcss_lead.c','#include "ctcss_lead.h"\nvoid CTCSS_LEAD_Start(void){}\nvoid CTCSS_LEAD_Process(void){}\nvoid CTCSS_LEAD_Stop(void){}\n'),
      'backlight_fade':('backlight_fade.c','#include "backlight_fade.h"\nvoid BACKLIGHT_FADE_Tick500ms(void){}\nvoid BACKLIGHT_FADE_ArmDuringActivity(void){}\n'),
      'battery_sag_display':('battery_sag.c','#include "battery_sag.h"\nvoid BATTERY_SAG_Tick500ms(void){}\nuint16_t BATTERY_SAG_GetLast10mV(void){return 0;}\n'),
      'side_button_toasts':('side_toast.c','#include "side_toast.h"\nvoid VUURWERK_SideToast(enum ACTION_OPT_t o){(void)o;}\n'),
      'scan_rate_display':('scan_rate.c','#include "scan_rate.h"\nvoid SCAN_RATE_NoteStep(void){}\nvoid SCAN_RATE_Tick10ms(void){}\nvoid SCAN_RATE_Reset(void){}\nuint8_t SCAN_RATE_ChannelsPerSec(void){return 0;}\n'),
      'aprs_position_decoder':('app/aprs_position.c','#include "aprs_position.h"\nbool APRS_Position(const uint8_t *d,const uint8_t *i,uint16_t n,int32_t *a,int32_t *o){(void)d;(void)i;(void)n;(void)a;(void)o;return false;}\n'),
      'aprs_setup_display_only':('ui/aprs_setup.c','#include "app/aprs_setup.h"\nvoid SETUP_Draw(void){}\n'),
    })
if args.transport:
    stubs['channel_frequency_scan']=('app/chFrScanner.c',
        '#include "app/chFrScanner.h"\nint8_t gScanStateDir;bool gScanKeepResult,gScanPauseMode;\n#ifdef ENABLE_SCAN_RANGES\nuint32_t gScanRangeStart,gScanRangeStop;\n#endif\nvoid CHFRSCANNER_Found(void){}\nvoid CHFRSCANNER_Stop(void){}\nvoid CHFRSCANNER_Start(bool b,int8_t d){(void)b;(void)d;}\nbool CHFRSCANNER_ContinueScanning(void){return false;}\n')
    stubs['tone_search']=('app/scanner.c',        '#include "app/scanner.h"\nDCS_CodeType_t gScanCssResultType;uint8_t gScanCssResultCode;SCAN_SaveState_t gScannerSaveState;uint8_t gScanChannel;uint32_t gScanFrequency;SCAN_CssState_t gScanCssState;uint8_t gScanProgressIndicator;bool gScanUseCssResult;\nvoid SCANNER_ProcessKeys(KEY_Code_t k,bool p,bool h){(void)k;(void)p;(void)h;}\nvoid SCANNER_Start(void){}\nvoid SCANNER_Stop(void){}\nvoid SCANNER_TimeSlice10ms(void){}\nvoid SCANNER_TimeSlice500ms(void){}\nbool SCANNER_IsScanning(void){return false;}\n')
    def replace_body(source,signature,body):
        start=source.index('{',source.index(signature));end=start+1;depth=1
        while depth:
            if source[end]=='{':depth+=1
            if source[end]=='}':depth-=1
            end+=1
        return source[:start]+'{'+body+'}'+source[end:]
    message=(SOURCE/'app/aprs_message.c').read_text()
    beacon=replace_body(message,'size_t MESSAGE_BuildBeacon(',        '(void)sender;(void)lat;(void)lon;(void)table;(void)symbol;(void)out;(void)capacity;return 0;')
    # The coordinate encoder is private to the beacon builder; remove its now
    # unused body as well so -Wunused-function is not mistaken for a cost result.
    start=beacon.index('static unsigned Position(');end=beacon.index('size_t MESSAGE_BuildBeacon(',start)
    beacon=beacon[:start]+beacon[end:]
    stubs['static_beacon_encoder_only']=('app/aprs_message.c',beacon)
    position=(SOURCE/'app/aprs_position.c').read_text()
    mice=replace_body(position,'static bool MicE(',        '(void)destination;(void)info;(void)length;(void)latitude;(void)longitude;return false;')
    stubs['mice_positions_only']=('app/aprs_position.c',mice)
    compressed=position[:position.index('    /* Compressed position:')]+'    return false;\n}\n'
    start=compressed.index('static bool Base91(');end=compressed.index('static bool MicE(',start)
    compressed=(compressed[:start]+compressed[end:]).replace('#include \"aprs_scale.h\"','')
    stubs['compressed_positions_only']=('app/aprs_position.c',compressed)
for label,payload in stubs.items():variants.append(('stub_'+label,[],payload))
if args.profile:
    unknown=set(args.profile)-{v[0] for v in variants}
    if unknown: parser.error('Unknown profiles: '+', '.join(sorted(unknown)))
    variants=[v for v in variants if v[0]=='baseline' or v[0] in args.profile]
image='uvk6-build:verified'
subprocess.run(['docker','image','inspect',image],stdout=subprocess.DEVNULL,check=True)
def build(item):
    label,flags,stub=item
    flags=(['ENABLE_APRS_TRANSPORT=1'] if args.transport else [])+flags
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
