#!/usr/bin/env python3
"""Compare pinned local toolchains with identical sources and radio features.

Every output is an oversized diagnostic build; never flash these copies.
"""
from pathlib import Path
import concurrent.futures
import hashlib
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parents[1];out=root/'build/toolchain-comparison';out.mkdir(parents=True,exist_ok=True)
source=out/'source'
if source.exists():shutil.rmtree(source)
def ignore(directory,names):
 return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,source,ignore=ignore)
hash=hashlib.sha256()
for p in sorted(source.rglob('*')):
 if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
  hash.update(str(p.relative_to(source)).encode());hash.update(p.read_bytes())
profiles=[('gcc10_Os','uvk6-build:verified','Os'),('gcc13_Os','uvk6-build:gcc13','Os'),('gcc13_Oz','uvk6-build:gcc13','Oz')]
def build(profile):
 name,image,opt=profile;work=out/name
 if work.exists():shutil.rmtree(work)
 shutil.copytree(source,work)
 p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
 p=work/'Makefile';p.write_text(p.read_text().replace('-Os ',f'-{opt} '))
 result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
 '-v',str(work)+':/src','-w','/src',image,'bash','-c',
 'set -e; arm-none-eabi-gcc -dumpfullversion; make -j2 ENABLE_APRS_MESSAGING=1; arm-none-eabi-readelf -A uvk6-experimental; arm-none-eabi-size -A uvk6-experimental'],
 stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 (work/'build.log').write_text(result.stdout)
 info={'name':name,'image':image,'optimization':opt,'compiler':result.stdout.splitlines()[0] if result.stdout else None,'exit_code':result.returncode}
 if result.returncode==0:
  assert 'Tag_CPU_arch: v6S-M' in result.stdout and 'Tag_THUMB_ISA_use: Thumb-1' in result.stdout
  info['flash_bytes']=(work/'uvk6-experimental.bin').stat().st_size
  sizes={}
  for line in result.stdout.splitlines():
   fields=line.split()
   if len(fields)==3 and fields[0] in {'.data','.bss'}:sizes[fields[0]]=int(fields[1])
  info['static_ram_bytes']=sum(sizes.values())
 else:info['errors']=[line for line in result.stdout.splitlines() if 'error:' in line or 'undefined reference' in line][:8]
 print(json.dumps(info),flush=True);return info
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(build,profiles))
(out/'results.json').write_text(json.dumps({'source_fingerprint':hash.hexdigest(),'real_flash_limit':61440,'diagnostic_flash_limit':131072,'profiles':results},indent=2)+'\n')
