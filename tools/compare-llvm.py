#!/usr/bin/env python3
"""Diagnostic LLVM comparison, never flash the resulting oversized copies.

Requires uvk6-build:clang14 with signed Ubuntu clang-14/lld-14 packages on the
verified GCC10/newlib image. Production toolchain and Makefile stay unchanged.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parents[1];out=root/'build/llvm-comparison'
out.mkdir(parents=True,exist_ok=True)
def ignore(directory,names):
    return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
work=out/'source'
if work.exists():shutil.rmtree(work)
shutil.copytree(root,work,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(work.rglob('*')):
    if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
        fingerprint.update(str(p.relative_to(work)).encode());fingerprint.update(p.read_bytes())
p=work/'firmware.ld';s=p.read_text().replace('LENGTH = 60K','LENGTH = 128K')
# lld does not infer NOBITS for a location-counter-only heap/stack section.
# Explicit load address also accounts for LLVM's orphan .ARM.exidx section.
s=s.replace('flash_data_start = .;','flash_data_start = LOADADDR(.data);')
s=s.replace('.bss :','.bss (NOLOAD) :').replace('._user_heap_stack :','._user_heap_stack (NOLOAD) :')
p.write_text(s)
p=work/'Makefile';s=p.read_text()
s=s.replace('ENABLE_LTO := 0','ENABLE_LTO := 1')
s=s.replace('CC = clang --sysroot=/usr/arm-none-eabi --target=arm-none-eabi',
            'CC = clang-14 --sysroot=/usr/lib/arm-none-eabi --target=arm-none-eabi')
s=s.replace('-flto=auto','-flto=full')
s=s.replace('LD = arm-none-eabi-gcc','LD = clang-14 --target=arm-none-eabi -fuse-ld=lld-14 -flto=full -Oz')
s=s.replace('LDFLAGS += --specs=nano.specs','LDFLAGS += -nostdlib')
s=s.replace('LIBS =','LIBS = -Wl,--start-group /usr/lib/arm-none-eabi/newlib/thumb/v6-m/nofp/libc_nano.a /usr/lib/gcc/arm-none-eabi/10.3.1/thumb/v6-m/nofp/libgcc.a -Wl,--end-group')
p.write_text(s)
command=['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
         '-v',str(work)+':/src','-w','/src','uvk6-build:clang14','bash','-c',
         'set -e; clang-14 --version; make -j2 ENABLE_CLANG=1 ENABLE_APRS_MESSAGING=1; arm-none-eabi-readelf -A uvk6-experimental; arm-none-eabi-size -A uvk6-experimental']
result=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
(out/'build.log').write_text(result.stdout)
report={'source_fingerprint':fingerprint.hexdigest(),'compiler':result.stdout.splitlines()[0] if result.stdout else None,
        'exit_code':result.returncode,'real_flash_limit':61440,'diagnostic_flash_limit':131072}
if result.returncode==0:
    report['flash_bytes']=(work/'uvk6-experimental.bin').stat().st_size
    report['cpu_attributes']=result.stdout[result.stdout.index('Attribute Section:'):]
else:report['errors']=[line for line in result.stdout.splitlines() if 'error:' in line or 'undefined reference' in line][:12]
(out/'results.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
