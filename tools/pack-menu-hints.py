#!/usr/bin/env python3
"""Losslessly pack original hint text; --check verifies generated data."""
from pathlib import Path
import argparse
import ast
import re
from lzss import pack
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');args=parser.parse_args()
source=(root/'ui/menu_hints.inc').read_text();body=source[source.index('static const char sHintPool'):]
body=re.sub(r'//[^\n]*|/\*.*?\*/','',body,flags=re.S)
text=''.join(ast.literal_eval(s) for s in re.findall(r'"(?:\\.|[^"\\])*"',body))
raw=text.encode('ascii')+b'\0';data=pack(raw)
header='''/* Generated from ui/menu_hints.inc by tools/pack-menu-hints.py.
 * Original copyright (c) 2026 James Honiball (KC3TFZ); original notice and
 * complete-package license are retained in menu_hints.inc and LICENSE.
 */
'''
header+=f'enum {{ MENU_HINT_BYTES={len(raw)} }};\nstatic const uint8_t gPackedMenuHints[] = {{\n'
header+='\n'.join('    '+','.join(f'0x{x:02x}' for x in data[i:i+16])+',' for i in range(0,len(data),16))+'\n};\n'
target=root/'ui/menu_hints_packed.inc'
if args.check:
 if not target.exists() or target.read_text()!=header:raise SystemExit('Menu hint data stale; run tools/pack-menu-hints.py')
else:target.write_text(header)
print('PASS menu hint generation' if args.check else 'Generated menu hint data',{'raw':len(raw),'packed':len(data)})
