#!/usr/bin/env python3
"""Attribute linked instructions to source, including LTO-inlined functions.

Builds only in an ignored diagnostic copy with enlarged flash. Never flash its
output. Debug information must leave the raw application bytes unchanged.
These are instruction locations, not removable feature costs.
"""
import collections
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
work = root / 'build/flash-attribution'
subprocess.run([sys.executable, str(root / 'tools/measure-transport.py'),
                '--name', 'flash-attribution'], check=True)
binary = work / 'uvk6-experimental.bin'
before = binary.read_bytes()
command = '''set -e
make clean >/dev/null
make -j2 ENABLE_APRS_TRANSPORT=1 DEBUG=1 >debug-build.log 2>&1
arm-none-eabi-objdump -dl uvk6-experimental >debug-disassembly.txt
arm-none-eabi-readelf -sW uvk6-experimental >debug-symbols.txt
'''
subprocess.run(['docker', 'run', '--rm', '--user', f'{os.getuid()}:{os.getgid()}',
                '-v', str(work) + ':/src', '-w', '/src',
                'uvk6-build:verified', 'bash', '-c', command], check=True)
if before != binary.read_bytes():
    raise SystemExit('Debug build changed application bytes; attribution rejected')

functions = []
for line in (work / 'debug-symbols.txt').read_text().splitlines():
    fields = line.split()
    if len(fields) >= 8 and fields[3] == 'FUNC' and fields[6] != 'UND':
        start = int(fields[1], 16) & ~1
        functions.append((start, start + int(fields[2])))
counts = collections.Counter()
main = collections.Counter()
source = '<unattributed>'
function = ''
seen = set()
for line in (work / 'debug-disassembly.txt').read_text().splitlines():
    match = re.match(r'^[0-9a-f]+ <(.+)>:', line)
    if match:
        source = '<unattributed>'
        function = match[1]
    match = re.match(r'^(/[^:]+):\d+', line)
    if match:
        filename = match[1]
        source = filename[5:] if filename.startswith('/src/') else '<toolchain>/' + Path(filename).name
    match = re.match(r'^\s*([0-9a-f]+):\s+([0-9a-f]{4}(?: [0-9a-f]{4})?)\s+([^\s]+)', line)
    if not match:
        continue
    address = int(match[1], 16)
    if match[3].startswith('.') or not any(start <= address < end for start, end in functions):
        continue
    size = len(match[2].replace(' ', '')) // 2
    addresses = set(range(address, address + size))
    if addresses & seen:
        raise SystemExit('Overlapping instructions; attribution rejected')
    seen.update(addresses)
    counts[source] += size
    if function == 'Main':
        main[source] += size
report = {
    'source_fingerprint': json.loads((work / 'results.json').read_text())['source_fingerprint'],
    'profile': 'ENABLE_APRS_TRANSPORT=1',
    'compiler': 'ARM GCC 10.3.1',
    'flash_bytes': len(before),
    'raw_sha256': hashlib.sha256(before).hexdigest(),
    'debug_build_byte_identical': True,
    'instruction_bytes': sum(counts.values()),
    'other_flash_bytes': len(before) - sum(counts.values()),
    'instruction_bytes_by_source': dict(counts.most_common()),
    'main_instruction_bytes_by_source': dict(main.most_common()),
    'limitations': 'Debug line attribution after LTO is approximate. Other flash includes constants, literal pools, vectors, initialized data and alignment. Counts are not standalone module sizes or savings from removal.',
    'diagnostic_only': True,
}
(work / 'attribution.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps(report, indent=2))
