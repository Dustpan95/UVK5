#!/usr/bin/env python3
"""Read-only consistency check for two original-UV-K6 raw EEPROM dumps.

This does not access serial ports, validate the MCU, interpret all calibration
values, or establish that recovery works. CHIRP metadata images are not raw dumps.
"""
import argparse
import hashlib
from pathlib import Path
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('first',type=Path)
parser.add_argument('second',type=Path)
args=parser.parse_args()
try:
 first=args.first.read_bytes();second=args.second.read_bytes()
except OSError as error:parser.error(str(error))
if len(first)!=8192 or len(second)!=8192:parser.error('Both raw EEPROM dumps must contain exactly 8192 bytes, including calibration.')
if first!=second:parser.error('The dumps differ. Preserve both and investigate before treating either as a backup.')
calibration=first[0x1e00:0x2000]
if len(set(calibration))==1:parser.error('The calibration area is uniform; the read or radio data needs investigation.')
print('PASS two identical full-size EEPROM files; calibration area is nonuniform.')
print('EEPROM SHA-256:',hashlib.sha256(first).hexdigest())
print('Calibration SHA-256:',hashlib.sha256(calibration).hexdigest())
print('File consistency only: MCU identity, calibration validity and recovery remain unchecked.')
