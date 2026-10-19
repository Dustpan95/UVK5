#!/usr/bin/env python3
"""Read-only consistency check for two original-UV-K6 raw EEPROM dumps.

This does not access serial ports, validate the MCU, interpret all calibration
values, or establish that recovery works. --chirp validates a known image trailer
and compares its 8192-byte EEPROM payload, preserving both input files.
"""
import argparse
import base64
import json
import hashlib
from pathlib import Path
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--chirp',action='store_true',help='Compare EEPROM payloads in two CHIRP UV-K5 images, excluding their metadata trailers')
parser.add_argument('first',type=Path)
parser.add_argument('second',type=Path)
args=parser.parse_args()
def payload(path):
 data=path.read_bytes()
 if not args.chirp:return data
 magic=b'\x00\xffchirp\xeeimg\x00\x01'
 if data[8192:8192+len(magic)]!=magic:parser.error('Expected a full 8192-byte EEPROM followed by CHIRP image metadata.')
 try:metadata=json.loads(base64.b64decode(data[8192+len(magic):],validate=True))
 except (ValueError,UnicodeError):parser.error('The CHIRP metadata trailer is invalid.')
 if not isinstance(metadata,dict) or metadata.get('vendor')!='Quansheng' or metadata.get('model') not in {'UV-K5','UV-K5 VUURWERK'}:
  parser.error('The metadata does not identify a supported Quansheng UV-K5 image format; this is not an MCU identity check.')
 return data[:8192]
try:
 first=payload(args.first);second=payload(args.second)
except OSError as error:parser.error(str(error))
if len(first)!=8192 or len(second)!=8192:parser.error('Both raw EEPROM dumps must contain exactly 8192 bytes, including calibration.')
if first!=second:parser.error('The dumps differ. Preserve both and investigate before treating either as a backup.')
calibration=first[0x1e00:0x2000]
if len(set(calibration))==1:parser.error('The calibration area is uniform; the read or radio data needs investigation.')
print('PASS two identical full-size EEPROM payloads; calibration area is nonuniform.')
print('EEPROM payload SHA-256:',hashlib.sha256(first).hexdigest())
print('Calibration SHA-256:',hashlib.sha256(calibration).hexdigest())
print('File consistency only: MCU identity, calibration validity and recovery remain unchecked.')
