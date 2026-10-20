#!/usr/bin/env python3
"""Exercise both immutable-resource modes against the actual decoder."""
from pathlib import Path
import random
import struct
import subprocess
from huffman_lz import pack_huffman
rng=random.Random(253)
stream=bytearray()
for nibbles in (False,True):
    alphabet=256 if nibbles else 128
    patterns=[b'',b'A',b'AB',b'ABC',b'A'*19,b'AB'*513,
              bytes(range(alphabet))*33,bytes([0xff if nibbles else 0x7f])*100]
    for n in [2,3,7,8,9,16,17,18,19,63,64,65,255,256,257,1023,1024,1025,4095,4096,4097]:
        patterns += [bytes(rng.randrange(4) for _ in range(n)),bytes(rng.randrange(alphabet) for _ in range(n))]
    window=256 if nibbles else 1024
    prefix=bytes(rng.randrange(alphabet) for _ in range(window))
    patterns += [prefix+prefix[:18],prefix+b'X'+prefix[:18]]
    for _ in range(24):patterns.append(bytes(rng.randrange(alphabet) for _ in range(rng.randrange(128))))
    for raw in patterns:
        packed=pack_huffman(raw,nibbles)
        assert packed==pack_huffman(raw,nibbles), 'packing must be deterministic'
        stream+=struct.pack('<II',len(raw),len(packed))+raw+packed
root=Path(__file__).resolve().parents[1]
subprocess.run([str(root/'build/test_huffman_stream')],input=stream,check=True)
