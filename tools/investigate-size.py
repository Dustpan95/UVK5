#!/usr/bin/env python3
"""Compare compiler-size options with identical messaging features.

Only ignored diagnostic copies use a larger flash region. Never flash outputs.
This measures size, not behavioral correctness of alternate compiler options.
"""
import concurrent.futures
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/size-investigation';OUT.mkdir(parents=True,exist_ok=True)
SOURCE=OUT/'source'
if SOURCE.exists():shutil.rmtree(SOURCE)
def ignore(directory,names):
 return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(ROOT,SOURCE,ignore=ignore)
profiles={
 'unchanged':[],
 'no_inline_small':['-fno-inline-small-functions'],
 'no_inline_once':['-fno-inline-functions-called-once'],
 'no_inline_both':['-fno-inline-small-functions','-fno-inline-functions-called-once'],
 'inline_limit_3':['-finline-limit=3'],
 'allow_builtins':['ALLOW_BUILTINS'],
 'no_unwind':['-fno-unwind-tables','-fno-asynchronous-unwind-tables'],
 'no_switch_conversion':['-fno-tree-switch-conversion'],
 'no_caller_saves':['-fno-caller-saves'],
 'no_split_wide':['-fno-split-wide-types'],
 'no_ipa_cp':['-fno-ipa-cp'],
}
fingerprint=hashlib.sha256()
for p in sorted(SOURCE.rglob('*')):
 if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
  fingerprint.update(str(p.relative_to(SOURCE)).encode());fingerprint.update(p.read_bytes())
def build(item):
 name,flags=item;work=OUT/name
 if work.exists():shutil.rmtree(work)
 shutil.copytree(SOURCE,work)
 p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
 p=work/'Makefile';s=p.read_text()
 if 'ALLOW_BUILTINS' in flags:s=s.replace('-fno-builtin ','')
 flags=[f for f in flags if f!='ALLOW_BUILTINS']
 s=s.replace('CFLAGS = $(CFLAGS_MESSAGING)','CFLAGS = $(CFLAGS_MESSAGING) '+' '.join(flags))
 p.write_text(s)
 result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
  '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c',
  'set -e; make -j2 ENABLE_APRS_MESSAGING=1; arm-none-eabi-size -A uvk6-experimental; arm-none-eabi-nm -S --size-sort uvk6-experimental > symbols.txt'],
  stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 (work/'build.log').write_text(result.stdout)
 info={'profile':name,'flags':profiles[name],'exit_code':result.returncode}
 if result.returncode==0:info['flash_bytes']=(work/'uvk6-experimental.bin').stat().st_size
 else:info['errors']=[s for s in result.stdout.splitlines() if 'error:' in s or 'undefined reference' in s][:8]
 print(json.dumps(info),flush=True);return info
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
 results=list(pool.map(build,profiles.items()))
base=results[0].get('flash_bytes')
for r in results:
 if 'flash_bytes' in r:r['delta_bytes']=r['flash_bytes']-base
report={'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1','feature_flags':['ENABLE_APRS_MESSAGING=1'],
 'real_flash_limit':61440,'diagnostic_flash_limit':131072,'profiles':results}
(OUT/'results.json').write_text(json.dumps(report,indent=2)+'\n')
print('Saved',OUT/'results.json',flush=True)
