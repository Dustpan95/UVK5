#!/usr/bin/env python3
"""Verify DP32G030 image layout, ELF memory budget, updater CRC and packing."""
import ast
import binascii
import hashlib
import json
import pathlib
import struct
import subprocess
import sys
from firmware_limits import FLASH_LIMIT, UPDATER_LIMIT, check_image_size

root = pathlib.Path(__file__).resolve().parents[1]
target = root / 'uvk6-experimental'
raw = target.with_suffix('.bin').read_bytes()
packed = target.with_suffix('.packed.bin').read_bytes()
check_image_size(len(raw))
sp, reset = struct.unpack_from('<II', raw)
assert sp == 0x20003ff0, f'unexpected DP32G030 initial stack: {sp:#x}'
assert reset & 1 and (reset & ~1) < len(raw), f'invalid Thumb reset vector: {reset:#x}'
assert len(packed) == len(raw) + 18, 'updater image length mismatch'
assert binascii.crc_hqx(packed[:-2], 0) == int.from_bytes(packed[-2:], 'little'), 'updater CRC mismatch'
# Read only the public obfuscation constant; do not execute the packer's CLI.
for node in ast.parse((root / 'fw-pack.py').read_text()).body:
    if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'OBFUSCATION' for t in node.targets):
        key = ast.literal_eval(node.value)
        break
else:
    raise AssertionError('upstream updater obfuscation key not found')
plain = bytes(value ^ key[i % len(key)] for i, value in enumerate(packed[:-2]))
assert plain[:8192] + plain[8208:] == raw, 'updater payload differs from raw firmware'
assert plain[8192:8208] == b'*VUURWERK rx-dev', 'unexpected experimental updater version'
attributes = subprocess.check_output(['arm-none-eabi-readelf', '-A', str(target)], text=True)
assert 'Tag_CPU_arch: v6S-M' in attributes and 'Tag_THUMB_ISA_use: Thumb-1' in attributes, 'wrong CPU architecture'
sections = subprocess.check_output(['arm-none-eabi-size', '-A', str(target)], text=True)
section_sizes = {}
for line in sections.splitlines():
    fields = line.split()
    if len(fields) == 3 and fields[0].startswith('.'):
        section_sizes[fields[0]] = int(fields[1])
# Include every flash LOAD segment (e.g. ARM exception index), not just .text.
programs = subprocess.check_output(['arm-none-eabi-readelf', '-lW', str(target)], text=True)
flash = 0
for line in programs.splitlines():
    fields = line.split()
    if fields and fields[0] == 'LOAD':
        physical, file_bytes = int(fields[3], 16), int(fields[4], 16)
        if physical < FLASH_LIMIT:
            flash = max(flash, physical + file_bytes)
static_ram = section_sizes.get('.data', 0) + section_sizes.get('.bss', 0)
reserved_stack = section_sizes.get('._user_heap_stack', 0)
assert flash == len(raw), 'raw length and ELF loadable sections differ'
assert reserved_stack >= 2048, 'linker stack reservation below 2 KiB'
assert static_ram + reserved_stack <= sp - 0x20000000, 'RAM reservation exceeds DP32G030 SRAM'
report = {
    'status': 'experimental; RF and physical hardware not tested',
    'hardware_target': 'original DP32G030 UV-K5 / UV-K6; not other MCU revisions',
    'compiler': subprocess.check_output(['arm-none-eabi-gcc', '-dumpfullversion'], text=True).strip(),
    'flash_bytes': flash, 'flash_limit_bytes': FLASH_LIMIT, 'flash_free_bytes': FLASH_LIMIT - flash,
    'updater_limit_bytes': UPDATER_LIMIT, 'updater_free_bytes': UPDATER_LIMIT - flash,
    'static_ram_bytes': static_ram, 'reserved_stack_bytes': reserved_stack,
    'ram_limit_bytes': 16384, 'initial_stack_pointer': hex(sp),
    'ram_after_static_bytes': sp - 0x20000000 - static_ram,
    'raw_sha256': hashlib.sha256(raw).hexdigest(),
    'packed_sha256': hashlib.sha256(packed).hexdigest(),
}
output = json.dumps(report, indent=2) + '\n'
(root / 'build').mkdir(exist_ok=True)
(root / 'build' / 'verification.json').write_text(output)
print(output, end='')
