#!/usr/bin/env python3
"""Losslessly pack original bitmap fonts. Original font.c remains the source.

Requires host GCC; generated data and original copyright stay in the checkout.
--check verifies reproducibility without modifying generated source.
"""
from pathlib import Path
import argparse
import ctypes
import subprocess
root=Path(__file__).resolve().parents[1];out=root/'build/font-compression';out.mkdir(parents=True,exist_ok=True)
parser=argparse.ArgumentParser();parser.add_argument('--check',action='store_true');args=parser.parse_args()
from huffman_lz import pack_huffman
header='''/* Copyright 2023 Dual Tachyon. SPDX-License-Identifier: Apache-2.0
 * Losslessly generated from font.c by tools/pack-fonts.py. Do not edit.
 */
'''
parts=[]
for bold in (True,False):
 path=out/('original-bold.so' if bold else 'original-plain.so')
 subprocess.run(['gcc','-shared','-fPIC','-I',str(root),*(['-DENABLE_SMALL_BOLD'] if bold else []),str(root/'font.c'),'-o',str(path)],check=True)
 lib=ctypes.CDLL(str(path))
 names=[('gFontBig',1316),('gFontBigDigits',220),('gFont3x5',288),('gFontSmall',564)]
 if bold:names.append(('gFontSmallBold',564))
 data=b''.join(bytes((ctypes.c_uint8*n).in_dll(lib,name)) for name,n in names)
 compressed=pack_huffman(data,nibbles=True)
 parts.append(('bold' if bold else 'plain',len(data),len(compressed)))
 header+='#ifdef ENABLE_SMALL_BOLD\n' if bold else '#else\n'
 header+='static const uint8_t gPackedFonts[] = {\n'
 header+='\n'.join('    '+','.join(f'0x{x:02x}' for x in compressed[i:i+16])+',' for i in range(0,len(compressed),16))+'\n};\n'
header+='#endif\n'
target=root/'font_packed.inc'
if args.check:
 if not target.exists() or target.read_text()!=header:raise SystemExit('font_packed.inc is stale; run tools/pack-fonts.py')
else:target.write_text(header)
print('PASS reproducible font packing' if args.check else 'Generated font_packed.inc',parts)
