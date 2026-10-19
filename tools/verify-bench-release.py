#!/usr/bin/env python3
"""Rebuild the published RX bench binaries from their exact frozen source.

Uses a temporary source archive, never a Git worktree and never radio hardware.
The release manifest selects the source and every boolean build setting, so
ongoing development commits do not need to match an older published binary.
"""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import tempfile

root=Path(__file__).resolve().parents[1]
release=root/'release/rx-session-bench'
manifest=json.loads((release/'manifest.json').read_text())
commit=manifest['source_commit'];settings=manifest['make_settings']
assert re.fullmatch(r'[0-9a-f]{40}',commit) and manifest['source_tree_clean']
assert settings['ENABLE_APRS_SESSION']=='1' and settings['ENABLE_APRS_MESSAGING']=='0'
assert all(re.fullmatch(r'ENABLE_\w+',name) and value in ('0','1') for name,value in settings.items())
expected={
    'uvk6-experimental.bin':('uvk6-rx-session-bench.bin','raw_sha256'),
    'uvk6-experimental.packed.bin':('uvk6-rx-session-bench.packed.bin','packed_sha256'),
}
for name,key in expected.values():
    assert hashlib.sha256((release/name).read_bytes()).hexdigest()==manifest[key],name+' hash mismatch'
(root/'build').mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='bench-source-',dir=root/'build') as temporary:
    work=Path(temporary);archive=work/'source.tar'
    subprocess.run(['git','archive','--format=tar','--output='+str(archive),commit],cwd=root,check=True)
    subprocess.run(['tar','-xf',str(archive),'-C',str(work)],check=True)
    archive.unlink()
    args=[str(work/'tools/build.sh'),*(name+'='+value for name,value in settings.items())]
    result=subprocess.run(args,cwd=work,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    log=root/'build/receiver-bench-reproduction.log';log.write_text(result.stdout)
    if result.returncode:
        raise SystemExit('Frozen-source build failed; see '+str(log)+'\n'+result.stdout[-3000:])
    for target,(name,key) in expected.items():
        actual=(work/target).read_bytes()
        assert actual==(release/name).read_bytes(),name+' differs from frozen-source rebuild'
        print('PASS frozen-source byte-for-byte reproduction:',name,len(actual),manifest[key])
print('Reproduced exact published source:',commit)
