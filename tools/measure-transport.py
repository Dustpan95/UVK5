#!/usr/bin/env python3
"""Compile the real transport profile in an oversized isolated copy for sizing.
The resulting diagnostic binary MUST NOT be flashed: production limit is 60 KiB.
No application source patches or runtime probes are inserted.
"""
import argparse,hashlib,json,os,re,shutil,subprocess
from pathlib import Path
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--name',default='transport-size')
parser.add_argument('flags',nargs='*')
args=parser.parse_args()
if not re.fullmatch(r'[a-z0-9-]+',args.name):parser.error('Invalid output name')
if any(not re.fullmatch(r'ENABLE_[A-Z0-9_]+=[01]',flag) for flag in args.flags):parser.error('Only boolean Make feature flags are accepted')
root=Path(__file__).resolve().parents[1];work=root/'build'/args.name
if work.exists():shutil.rmtree(work)
def ignore(directory,names):
    return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,work,ignore=ignore)
fingerprint=hashlib.sha256()
for source in sorted(work.rglob('*')):
    if source.is_file() and (source.suffix in {'.c','.h','.ld','.inc'} or source.name=='Makefile'):
        fingerprint.update(str(source.relative_to(work)).encode());fingerprint.update(source.read_bytes())
p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
build_command='set -e; make -j2 ENABLE_APRS_TRANSPORT=1 "$@"; arm-none-eabi-size -A uvk6-experimental; arm-none-eabi-nm -S --size-sort uvk6-experimental > symbols.txt'
compiler=shutil.which('arm-none-eabi-gcc')
native=compiler and os.environ.get('UVK6_USE_DOCKER')!='1' and subprocess.check_output([compiler,'-dumpfullversion'],text=True).strip()=='10.3.1'
command=['bash','-c',build_command,'bash',*args.flags] if native else [
    'docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
    '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c',build_command,'bash',*args.flags]
result=subprocess.run(command,cwd=work,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
(work/'build.log').write_text(result.stdout)
if result.returncode:raise SystemExit(result.stdout[-9000:])
size=(work/'uvk6-experimental.bin').stat().st_size
sections={}
for line in result.stdout.splitlines():
    fields=line.split()
    if len(fields)==3 and fields[0] in {'.data','.bss','._user_heap_stack'}:sections[fields[0]]=int(fields[1])
report={'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1',
    'static_ram_bytes':sections.get('.data',0)+sections.get('.bss',0),
    'reserved_stack_bytes':sections.get('._user_heap_stack',0),'flash_bytes':size,'flash_limit':61440,'over_by':max(0,size-61440),'diagnostic_only':True,'source_patches':'linker capacity only','profile':['ENABLE_APRS_TRANSPORT=1',*args.flags]}
(work/'results.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
