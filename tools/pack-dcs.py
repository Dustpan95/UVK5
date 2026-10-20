#!/usr/bin/env python3
"""Encode retained original tone tables as exact byte deltas."""
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parents[1];text=(root/'tests/reference/dcs.c').read_text();delta=[]
seed=0
for name in ['DCS','CTCSS']:
    body=re.search(r'const uint16_t REFERENCE_'+name+r'_Options\[\d+\] = \{(.*?)\};',text,re.S)[1]
    for token in re.findall(r'0x[0-9a-fA-F]+|\d+',body):
        value=int(token,0);delta.append(value-seed);seed=value
assert len(delta)==154 and all(0<=v<=255 for v in delta)
result='''/* Copyright 2023 Dual Tachyon. SPDX-License-Identifier: Apache-2.0
 * Exact deltas generated from the retained original tables in
 * tests/reference/dcs.c. No tone frequencies or codes are changed. */
static const uint8_t gDcsDelta[] = {
'''+ '\n'.join('    '+','.join(map(str,delta[i:i+16]))+',' for i in range(0,len(delta),16))+'\n};\n'
p=root/'dcs_packed.inc'
if '--check' in sys.argv:assert p.read_text()==result,'run tools/pack-dcs.py'
else:p.write_text(result)
print('PASS reproducible exact CTCSS/DCS deltas')
