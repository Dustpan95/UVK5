#!/usr/bin/env python3
"""Measure selective noinline boundaries in isolated messaging builds.

No production source edits. Diagnostic enlarged-linker binaries must not flash.
"""
from pathlib import Path
import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import subprocess
root=Path(__file__).resolve().parents[1];out=root/'build/inlining';out.mkdir(parents=True,exist_ok=True)
source=out/'source'
if source.exists():shutil.rmtree(source)
def ignore(directory,names):
    return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,source,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(source.rglob('*')):
    if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
        fingerprint.update(str(p.relative_to(source)).encode());fingerprint.update(p.read_bytes())
groups={
    'baseline':{},
    'home_public':{'ui/home.c':['UI_HomeDraw','UI_HomeStatus']},
    'home_primitives':{'ui/home.c':['Text','Rect','Pixel','Center','Banner','Frequency']},
    'messaging':{'ui/aprs_message.c':['MESSAGE_Draw'],'ui/aprs_session.c':['SESSION_Draw'],
                 'app/aprs_message.c':['MESSAGE_Key'],'app/aprs_session.c':['SESSION_Key','SESSION_Receive']},
    'receiver':{'app/aprs_rx.c':['APRS_Tick500ms','APRS_Interrupt','APRS_CanListen','APRS_Key']},
    'menu':{'app/menu.c':['MENU_ShowCurrentSetting','MENU_AcceptSetting','MENU_Key_MENU','MENU_Key_UP_DOWN'],
            'ui/menu.c':['UI_DisplayMenu']},
}
def build(item):
    name,files=item;work=out/name
    if work.exists():shutil.rmtree(work)
    shutil.copytree(source,work)
    p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
    for filename,names in files.items():
        p=work/filename;s=p.read_text()
        for function in names:
            pattern=r'(?m)^((?:static\s+)?(?:void|bool)\s+)'+re.escape(function)+r'(\([^;{}]*\)\s*\{)'
            s,count=re.subn(pattern,lambda m:m[1]+'__attribute__((noinline)) '+function+m[2],s)
            if count!=1:raise ValueError((filename,function,count))
        p.write_text(s)
    result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
        '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c',
        'set -e; make -j2 ENABLE_APRS_MESSAGING=1; arm-none-eabi-size -A uvk6-experimental'],
        stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    (work/'build.log').write_text(result.stdout)
    report={'profile':name,'functions':files,'exit_code':result.returncode}
    if result.returncode==0:report['flash_bytes']=(work/'uvk6-experimental.bin').stat().st_size
    else:report['errors']=[line for line in result.stdout.splitlines() if 'error:' in line][:8]
    print(json.dumps(report),flush=True);return report
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:results=list(pool.map(build,groups.items()))
(out/'results.json').write_text(json.dumps({'source_fingerprint':fingerprint.hexdigest(),
    'real_flash_limit':61440,'diagnostic_flash_limit':131072,'profiles':results},indent=2)+'\n')
