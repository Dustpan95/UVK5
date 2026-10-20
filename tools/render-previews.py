#!/usr/bin/env python3
"""Convert host-test LCD framebuffer PBMs to PNGs without image dependencies."""
import binascii
import argparse
import pathlib
import struct
import zlib

root = pathlib.Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--setup',action='store_true',help='Render the actual experimental setup pages')
parser.add_argument('--receive-session',action='store_true',help='Render receive-session bench test captures')
args=parser.parse_args()
source=root/'build/setup-ui' if args.setup else root/'build/session-receiver' if args.receive_session else root/'build'
output = root / ('docs/screenshots/setup' if args.setup else 'docs/screenshots/receiver-bench' if args.receive_session else 'docs/screenshots')
output.mkdir(parents=True, exist_ok=True)

def chunk(kind, payload):
    return struct.pack('>I', len(payload)) + kind + payload + struct.pack('>I', binascii.crc32(kind + payload) & 0xffffffff)

names=('aprs-position', 'aprs-message', 'home-memory', 'home-unnamed', 'home-vfo', 'message-compose', 'message-review', 'message-blocked', 'personal-inbox', 'personal-message', 'personal-unconfigured', 'station-directory', 'station-detail', 'own-callsign')
pages=['overview','latitude','longitude','position-review','quick-messages','data-tx','frequency','audio-level','ack-wait','beacon-review']
files=[(f'setup-{i+1}',name) for i,name in enumerate(pages)] if args.setup else [(name,name) for name in names]
for input_name,name in files:
    if args.receive_session and name.startswith('message-'):continue
    fields = (source / (input_name + '.pbm')).read_text().split()
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
