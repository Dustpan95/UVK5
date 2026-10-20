#!/usr/bin/env python3
"""Check compact arithmetic natively and ARM EABI returns on Cortex-M0 QEMU.

The generated ELF files are CPU test harnesses, never firmware or RF tests.
Both original libgcc and replacement entry points run the same ARM vectors.
"""
import json
import hashlib
import os
from pathlib import Path
import shutil
import subprocess

root = Path(__file__).resolve().parents[1]
out = root / 'build/compact-division'
out.mkdir(parents=True, exist_ok=True)
compiler = os.environ.get('CC', 'gcc')
flags = ['-std=c11', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
if os.environ.get('SANITIZE', '1') == '1':
    flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
subprocess.run([compiler, *flags, str(root / 'tests/test_compact_division.c'),
                str(root / 'helper/compact_division.c'), '-o', str(out / 'native')], check=True)
subprocess.run([str(out / 'native')], check=True)
boundary = [0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 63, 64, 127, 128, 255, 256,
            511, 512, 1023, 1024, 65535, 65536, 0x3fffffff, 0x40000000,
            0x7ffffffe, 0x7fffffff, 0x80000000, 0x80000001, 0xfffffffe, 0xffffffff]
pairs = [(n, d) for n in boundary for d in boundary]
random = 0x137e0cd5
for i in range(1024):
    random = (random * 1664525 + 1013904223) & 0xffffffff
    n = random
    random = (random * 1664525 + 1013904223) & 0xffffffff
    pairs.append((n, random if i % 64 else 0))
rows = []
for n, d in pairs:
    uq, ur = (divmod(n, d) if d else (0, n))
    sn = n if n < 0x80000000 else n - 0x100000000
    sd = d if d < 0x80000000 else d - 0x100000000
    sq = abs(sn) // abs(sd) if sd else 0
    if (sn < 0) != (sd < 0):
        sq = -sq
    sr = sn - sq * sd
    rows.append((n, d, uq, ur, sq & 0xffffffff, sr & 0xffffffff))
guest = '''#include <stdint.h>
typedef struct {uint32_t n,d,uq,ur,sq,sr;} Vector;
extern uint32_t __aeabi_uidiv(uint32_t,uint32_t),__udivsi3(uint32_t,uint32_t);
extern int32_t __aeabi_idiv(int32_t,int32_t),__divsi3(int32_t,int32_t);
extern uint64_t __aeabi_uidivmod(uint32_t,uint32_t),__aeabi_idivmod(int32_t,int32_t);
extern uint32_t ConsumerUDiv(uint32_t,uint32_t),ConsumerURem(uint32_t,uint32_t);
extern int32_t ConsumerSDiv(int32_t,int32_t),ConsumerSRem(int32_t,int32_t);
extern uint64_t ConsumerUBoth(uint32_t,uint32_t),ConsumerSBoth(int32_t,int32_t);
static const Vector vectors[]={
'''
guest += '\n'.join('    {' + ','.join(f'0x{x:08x}u' for x in row) + '},' for row in rows)
guest += '''
};
static void Write(const char *text) {
    register uint32_t operation __asm__("r0")=4;
    register const char *argument __asm__("r1")=text;
    __asm__ volatile("bkpt #0xab" : "+r"(operation) : "r"(argument) : "memory");
}
static __attribute__((noreturn)) void Exit(unsigned code) {
    uint32_t block[2];block[0]=0x20026;block[1]=code;
    register uint32_t operation __asm__("r0")=0x20;
    register uint32_t *argument __asm__("r1")=block;
    __asm__ volatile("bkpt #0xab" : "+r"(operation) : "r"(argument) : "memory");
    for(;;) {}
}
__attribute__((noreturn)) void Guest_Failure(unsigned index) {
    char hex[9];static const char digits[]="0123456789abcdef";
    for(unsigned i=0;i<8;i++) hex[i]=digits[(index>>((7-i)*4))&15];
    hex[8]=0;
    Write("FAIL Cortex-M0 division ABI vector ");Write(hex);Write("\\n");Exit(1);
}
void RunDivisionVectors(void) {
    for(unsigned i=0;i<sizeof(vectors)/sizeof(*vectors);i++) {
        const Vector *v=&vectors[i];uint64_t u=__aeabi_uidivmod(v->n,v->d);
        uint64_t s=__aeabi_idivmod(v->n,v->d);
        if((uint32_t)u!=v->uq || (uint32_t)(u>>32)!=v->ur ||
           __aeabi_uidiv(v->n,v->d)!=v->uq || __udivsi3(v->n,v->d)!=v->uq ||
           (uint32_t)s!=v->sq || (uint32_t)(s>>32)!=v->sr ||
           (uint32_t)__aeabi_idiv(v->n,v->d)!=v->sq || (uint32_t)__divsi3(v->n,v->d)!=v->sq)
            Guest_Failure(i);
        if(v->d) {
            u=ConsumerUBoth(v->n,v->d);
            if((uint32_t)u!=v->uq || (uint32_t)(u>>32)!=v->ur ||
               ConsumerUDiv(v->n,v->d)!=v->uq || ConsumerURem(v->n,v->d)!=v->ur)
                Guest_Failure(i);
            if(!(v->n==0x80000000u && v->d==0xffffffffu)) {
                s=ConsumerSBoth(v->n,v->d);
                if((uint32_t)s!=v->sq || (uint32_t)(s>>32)!=v->sr ||
                   (uint32_t)ConsumerSDiv(v->n,v->d)!=v->sq ||
                   (uint32_t)ConsumerSRem(v->n,v->d)!=v->sr) Guest_Failure(i);
            }
        }
    }
    Write("PASS Cortex-M0 division ABI and compiler-generated consumers\\n");Exit(0);
}
'''
(out / 'guest.c').write_text(guest)
(out / 'consumers.c').write_text('''#include <stdint.h>
uint32_t ConsumerUDiv(uint32_t n,uint32_t d) {return n/d;}
uint32_t ConsumerURem(uint32_t n,uint32_t d) {return n%d;}
int32_t ConsumerSDiv(int32_t n,int32_t d) {return n/d;}
int32_t ConsumerSRem(int32_t n,int32_t d) {return n%d;}
uint64_t ConsumerUBoth(uint32_t n,uint32_t d) {return ((uint64_t)(n%d)<<32)|(n/d);}
uint64_t ConsumerSBoth(int32_t n,int32_t d) {return ((uint64_t)(uint32_t)(n%d)<<32)|(uint32_t)(n/d);}
''')
(out / 'start.S').write_text('''.syntax unified
.cpu cortex-m0
.thumb
.section .vectors,"a",%progbits
.word 0x20004000
.word Reset_Handler
.word Fault_Handler
.word Fault_Handler
.section .text.start,"ax",%progbits
.global Reset_Handler
.type Reset_Handler,%function
.thumb_func
Reset_Handler:
    bl RunDivisionVectors
    b .
.thumb_func
Fault_Handler:
    movs r0,#127
    bl Guest_Failure
    b .
''')
(out / 'guest.ld').write_text('''ENTRY(Reset_Handler)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 256K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K }
SECTIONS {
 .text : {KEEP(*(.vectors)) *(.text*) *(.rodata*)} >FLASH
 .data : {*(.data*)} >RAM AT>FLASH
 .bss : {*(.bss*) *(COMMON)} >RAM
 /DISCARD/ : {*(.ARM.exidx*) *(.ARM.extab*) *(.comment*)}
}
''')
build = '''set -e
test "$(arm-none-eabi-gcc -dumpfullversion)" = 10.3.1
arm-none-eabi-gcc -std=gnu11 -Os -Wall -Wextra -Werror -mcpu=cortex-m0 -mthumb -fno-builtin -fno-lto -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -I . -nostdlib -Wl,-T,build/compact-division/guest.ld -Wl,--gc-sections -Wl,-z,noexecstack build/compact-division/start.S build/compact-division/guest.c build/compact-division/consumers.c "$@" -lgcc -o build/compact-division/current.elf
qemu-system-arm -M microbit -cpu cortex-m0 -nographic -semihosting-config enable=on,target=native -kernel build/compact-division/current.elf
'''
arm = shutil.which('arm-none-eabi-gcc')
native = arm and shutil.which('qemu-system-arm') and subprocess.check_output([arm, '-dumpfullversion'], text=True).strip() == '10.3.1'
for name, extra in [('original', []), ('compact', ['helper/compact_division.c', 'helper/compact_division_glue.S'])]:
    command = ['bash', '-c', build, 'bash', *extra] if native else [
        'docker', 'run', '--rm', '--user', f'{os.getuid()}:{os.getgid()}',
        '-v', str(root) + ':/src', '-w', '/src',
        os.environ.get('UVK6_BUILD_IMAGE', 'uvk6-build:jammy'), 'bash', '-c', build, 'bash', *extra]
    result = subprocess.run(command, cwd=root, text=True, capture_output=True, timeout=60)
    (out / (name + '.log')).write_text(result.stdout + result.stderr)
    if result.returncode or 'PASS Cortex-M0 division ABI' not in result.stdout + result.stderr:
        raise SystemExit(name + ' ARM test failed:\n' + result.stdout + result.stderr)
    shutil.copyfile(out / 'current.elf', out / (name + '.elf'))
    print('PASS', name, 'runtime helpers:', len(rows), 'Cortex-M0 vectors and generated / and % consumers')
(out / 'verification.json').write_text(json.dumps({
    'compiler': 'ARM GCC 10.3.1', 'cpu': 'QEMU Cortex-M0 / microbit CPU model',
    'vectors': len(rows), 'original_libgcc': 'passed', 'compact': 'passed',
    'source_sha256': {name: hashlib.sha256((root / name).read_bytes()).hexdigest()
                      for name in ['helper/compact_division.c', 'helper/compact_division.h',
                                   'helper/compact_division_glue.S', 'tests/test_compact_division.c']},
    'elf_sha256': {name: hashlib.sha256((out / (name + '.elf')).read_bytes()).hexdigest()
                   for name in ['original', 'compact']},
    'covers': ['all six ARM EABI/GCC entry names', 'r0 quotient and r1 remainder',
               'compiler-generated division/modulo consumers', 'signed boundaries',
               'existing divide-zero and INT_MIN/-1 runtime behavior'],
    'not_covered': ['radio peripherals', 'RF or cycle-accurate hardware timing'],
}, indent=2) + '\n')
