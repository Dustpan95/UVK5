#!/usr/bin/env python3
"""Losslessly pack DISPLAY_TEXT literals; preserve readable UI source labels.

--check verifies generated files. --test also exercises every compiled literal
offset and expanded byte with sanitizers, including constant initializers.
"""
import argparse
import ast
import json
import os
from pathlib import Path
import re
import subprocess
from huffman_lz import pack_huffman

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true')
parser.add_argument('--test', action='store_true')
args = parser.parse_args()
values = set()
tokens = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
for source in sorted((root / 'ui').glob('*.c')):
    text = tokens.sub(lambda m: ' ' if m[0].startswith('/') else m[0], source.read_text())
    for literal in re.findall(r'DISPLAY_TEXT(?:_OFFSET)?\(\s*("(?:\\.|[^"\\])*")\s*\)', text):
        value = ast.literal_eval(literal)
        value.encode('ascii')
        values.add(value)
if not values:
    raise SystemExit('No display text found')
# Preserve complete C strings and suffix aliases, grouping similar-length
# labels before compression. All literal offsets are regenerated and tested.
roots=[value for value in values if not any(other!=value and other.endswith(value) for other in values)]
raw=bytearray()
for value in sorted(roots,key=lambda value:(-len(value)//4,value)):
    raw.extend(value.encode('ascii')+b'\0')
offsets={value:raw.find(value.encode('ascii')+b'\0') for value in values}
assert all(offset>=0 for offset in offsets.values())
text_raw = bytes(raw)
hints_source = (root / 'ui/menu_hints.inc').read_text()
hints_source = hints_source[hints_source.index('static const char sHintPool'):]
hints_source = tokens.sub(lambda m: ' ' if m[0].startswith('/') else m[0], hints_source)
hints = (''.join(ast.literal_eval(s) for s in re.findall(r'"(?:\\.|[^"\\])*"', hints_source)) + '\0').encode('ascii')
choices_source = tokens.sub(lambda m: ' ' if m[0].startswith('/') else m[0],
                            (root / 'ui/menu_choices.inc').read_text())
choices = []
for match in re.finditer(r'const char (gSubMenu_\w+)\[\]\[(\d+)\]\s*=\s*\{(.*?)\};', choices_source, re.S):
    name, width = match[1], int(match[2])
    rows = [ast.literal_eval(s).encode('ascii') + b'\0'
            for s in re.findall(r'"(?:\\.|[^"\\])*"', match[3])]
    if not rows or any(len(row) > width for row in rows):
        raise SystemExit('Invalid choice table: ' + name)
    data = b''.join(row.ljust(width, b'\0') for row in rows)
    choices.append((name, len(rows), width, data))
raw = text_raw + b''.join(data for _, _, _, data in choices) + hints
packed = pack_huffman(raw)
notice = '''/* SPDX-License-Identifier: GPL-3.0-only
 * Generated losslessly from project UI text by tools/pack-display-text.py.
 * Original copyright/license notices remain in the source UI files and CREDITS.
 */
'''
header = notice + '''#ifndef UVK5_DISPLAY_TEXT_H
#define UVK5_DISPLAY_TEXT_H
#include <stdint.h>
'''
header += f'enum {{ DISPLAY_TEXT_BYTES={len(text_raw)}, DISPLAY_HINT_BYTES={len(hints)}, DISPLAY_RESOURCE_BYTES={len(raw)} }};\n'
header += 'typedef struct {\n    char text[DISPLAY_TEXT_BYTES];\n'
for name, count, width, _ in choices:
    header += f'    char menu_{name[9:]}[{count}][{width}];\n'
header += '    char hints[DISPLAY_HINT_BYTES];\n} DisplayResources;\nextern DisplayResources gDisplayResources;\n'
header += '#define gDisplayText (gDisplayResources.text)\nvoid DISPLAY_TEXT_Init(void);\n'
for name, count, width, _ in choices:
    header += f'#define {name} (*(const char (*)[{count}][{width}])gDisplayResources.menu_{name[9:]})\n'
# Explicit compiler builtins fold these literal comparisons even with
# -fno-builtin. RAM addresses remain C constant initializers for pointer tables.
# An unregistered literal makes the array bound negative, failing compilation.
header += '#define DISPLAY_TEXT_OFFSET(s) ( \\\n'
for value in sorted(values):
    header += f'    __builtin_strcmp((s),{json.dumps(value)})==0 ? {offsets[value]} : \\\n'
header += '    -1)\n'
header += '''#define DISPLAY_TEXT(s) (gDisplayText + DISPLAY_TEXT_OFFSET(s) + \\
    0*sizeof(char[DISPLAY_TEXT_OFFSET(s)>=0 ? 1 : -1]))
#endif
'''
data_source = notice + 'static const uint8_t gPackedDisplayText[] = {\n'
data_source += '\n'.join('    ' + ','.join(f'0x{x:02x}' for x in packed[i:i+16]) + ','
                         for i in range(0, len(packed), 16)) + '\n};\n'
for filename, data in [('ui/display_text.h', header), ('ui/display_text_packed.inc', data_source)]:
    target = root / filename
    if args.check or args.test:
        if not target.exists() or target.read_text() != data:
            raise SystemExit(filename + ' stale; run python3 tools/pack-display-text.py')
    else:
        target.write_text(data)
print('PASS display text generation' if args.check or args.test else 'Generated display text',
      {'strings': len(values), 'choice_tables': len(choices), 'raw': len(raw), 'packed': len(packed)})
if args.test:
    out = root / 'build/display-text'
    out.mkdir(parents=True, exist_ok=True)
    test = '''#include "ui/display_text.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const struct {const char *actual,*expected;unsigned offset;} cases[]={
'''
    for value in sorted(values):
        text = json.dumps(value)
        test += f'    {{DISPLAY_TEXT({text}),{text},{offsets[value]}}},\n'
    test += '};\nstatic const unsigned char expected[]={' + ','.join(str(x) for x in raw) + '};\n'
    test += '#include "ui/menu_hints.inc"\n'
    for name, _, _, _ in choices:
        test += '#undef ' + name + '\n'
    test += '#include "ui/menu_choices.inc"\n'
    test += '''int main(void) {
    DISPLAY_TEXT_Init();
    assert(sizeof(expected)==sizeof(gDisplayResources));
    assert(!memcmp(expected,&gDisplayResources,sizeof(expected)));
    assert(!memcmp(sHintPool,gDisplayResources.hints,sizeof(sHintPool)));
    for(unsigned i=0;i<sizeof(cases)/sizeof(*cases);i++) {
        assert(cases[i].actual==gDisplayText+cases[i].offset);
        assert(!strcmp(cases[i].actual,cases[i].expected));
    }
'''
    for name, _, _, _ in choices:
        test += f'    assert(sizeof({name})==sizeof(gDisplayResources.menu_{name[9:]}));\n'
        test += f'    assert(!memcmp({name},gDisplayResources.menu_{name[9:]},sizeof({name})));\n'
    test += '    puts("PASS every display label, hint, choice-table byte and compiled literal offset");\n}\n'
    (out / 'test.c').write_text(test)
    compiler = os.environ.get('CC', 'gcc')
    flags = ['-std=c11', '-Wall', '-Wextra', '-Werror', '-fno-builtin', '-I', str(root)]
    if os.environ.get('SANITIZE', '1') == '1':
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    subprocess.run([compiler, *flags, str(out / 'test.c'), str(root / 'ui/display_text.c'),
                    str(root / 'helper/packed.c'), str(root / 'helper/packed_huffman.c'), '-o', str(out / 'test')], check=True)
    subprocess.run([str(out / 'test')], check=True)
    unknown = '#include "ui/display_text.h"\nconst char *bad=DISPLAY_TEXT("UVK5_UNREGISTERED_LITERAL");\n'
    result = subprocess.run([compiler, *flags, '-x', 'c', '-c', '-', '-o', str(out / 'unknown.o')],
                            input=unknown, text=True, capture_output=True)
    if result.returncode == 0 or 'negative' not in result.stderr:
        raise SystemExit('Unregistered-literal compile rejection not established')
    print('PASS unknown display literal rejected at compile time')
