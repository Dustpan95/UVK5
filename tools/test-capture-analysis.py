#!/usr/bin/env python3
import importlib.util
from pathlib import Path
s=importlib.util.spec_from_file_location('analysis',Path(__file__).with_name('analyze-rx-capture.py'))
m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
b=Path('build/rx-diagnostics-tests/captures.bin').read_bytes();offset=48 if b[4]==3 else 24;raw=b[offset:offset+240]
for swap in [False,True]:
 changed=b''.join(raw[i:i+2][::-1] for i in range(0,len(raw),2)) if swap else raw
 result=m.analyze(changed,swap=swap)
 assert len(result['valid_fcs_frames'])==1
 assert result['valid_fcs_frames'][0]['aprs']['source']=='N0CALL-15'
 reverse=bytes(int(f'{x:08b}'[::-1],2) for x in changed)
 assert len(m.analyze(reverse,swap=swap,msb=False)['valid_fcs_frames'])==1
corrupt=bytearray(raw);corrupt[10]^=1
assert not m.analyze(corrupt)['valid_fcs_frames']
assert not m.analyze(b'\0'*240)['valid_fcs_frames']
assert m.fcs(b'123456789')==0x906e
print('PASS offline capture analysis: independent FCS vector, real fixture, both byte/bit orders, corrupted and empty-signal rejection')
