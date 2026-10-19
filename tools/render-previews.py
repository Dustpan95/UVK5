#!/usr/bin/env python3
"""Convert host-test LCD framebuffer PBMs to PNGs without image dependencies."""
import binascii
import pathlib
import struct
import zlib

root = pathlib.Path(__file__).resolve().parents[1]
output = root / 'docs/screenshots'
output.mkdir(parents=True, exist_ok=True)

def chunk(kind, payload):
    return struct.pack('>I', len(payload)) + kind + payload + struct.pack('>I', binascii.crc32(kind + payload) & 0xffffffff)

for name in ('aprs-position', 'aprs-message', 'home-memory', 'home-unnamed', 'home-vfo', 'message-compose', 'message-review', 'message-blocked'):
    fields = (root / 'build' / (name + '.pbm')).read_text().split()
    assert fields[:3] == ['P1', '128', '64']
    pixels = [int(x) for x in fields[3:]]
    assert len(pixels) == 128 * 64
    scale = 4
    scanlines = bytearray()
    for y in range(64 * scale):
        scanlines.append(0)
        scanlines.extend(0 if pixels[(y // scale) * 128 + x // scale] else 255 for x in range(128 * scale))
    header = struct.pack('>IIBBBBB', 128 * scale, 64 * scale, 8, 0, 0, 0, 0)
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', header) + chunk(b'IDAT', zlib.compress(scanlines, 9)) + chunk(b'IEND', b'')
    (output / (name + '.png')).write_bytes(png)
print('Wrote host-rendered LCD previews to docs/screenshots/')
