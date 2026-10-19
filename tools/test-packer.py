#!/usr/bin/env python3
"""Exercise generated LZSS streams using the sanitizer-built firmware decoder."""
from pathlib import Path
import random
import struct
import subprocess
from lzss import pack
rng=random.Random(253)
patterns=[b'',b'A',b'AB',b'ABC',b'A'*19,b'AB'*513,bytes(range(256))*33]
for n in [3,7,8,9,17,18,19,63,64,65,255,256,257,4095,4096,4097]:
 patterns+=[bytes(rng.randrange(4) for _ in range(n)),bytes(rng.randrange(256) for _ in range(n))]
# A match first reusable beyond one full window, including its exact boundary.
prefix=bytes(rng.randrange(256) for _ in range(4096))
patterns += [prefix+prefix[:18],prefix+b'X'+prefix[:18]]
for _ in range(24):
 n=rng.randrange(128);patterns.append(bytes(rng.randrange(256) for _ in range(n)))
stream=bytearray()
for raw in patterns:
 packed=pack(raw)
 assert packed==pack(raw), 'packing must be deterministic'
 stream+=struct.pack('<II',len(raw),len(packed))+raw+packed
root=Path(__file__).resolve().parents[1]
subprocess.run([str(root/'build/test_packed_stream')],input=stream,check=True)
