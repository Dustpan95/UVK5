#!/usr/bin/env python3
"""Execute the actual immutable-resource decoder on a Cortex-M0 CPU model.
This tests decoding/guards, not radio peripherals or physical hardware.
"""
import hashlib,json,os,re,runpy,shutil,subprocess,sys
from pathlib import Path
root=Path(__file__).resolve().parents[1];out=root/'build/huffman-cpu';out.mkdir(parents=True,exist_ok=True)
saved=sys.argv;sys.argv=['pack-display-text.py','--check'];data=runpy.run_path(str(root/'tools/pack-display-text.py'));sys.argv=saved
fonts=(root/'font_packed.inc').read_text()
blobs=[('font-bold',fonts.split('#else')[0],(root/'build/font-compression/original-bold.bin').read_bytes()),
       ('font-plain',fonts.split('#else')[1],(root/'build/font-compression/original-plain.bin').read_bytes()),
       ('display',(root/'ui/display_text_packed.inc').read_text(),data['raw'])]
source='#include <stdint.h>\n#include "helper/packed.h"\nstatic uint8_t output[4352];\n'
checks=[];hashes={}
for index,(name,text,raw) in enumerate(blobs):
    packed=bytes(int(v,16) for v in re.findall(r'0x([0-9a-fA-F]{2})',text))
    source+=f'static const uint8_t packed_{index}[]={{'+','.join(map(str,packed))+'};\n'
    digest=2166136261
    for b in raw:digest=((digest^b)*16777619)&0xffffffff
    checks.append(f'    Check(packed_{index},{len(raw)},0x{digest:08x}u);')
    hashes[name]={'raw_sha256':hashlib.sha256(raw).hexdigest(),'packed_sha256':hashlib.sha256(packed).hexdigest()}
source+='''static void Write(const char *text) {
    register uint32_t operation __asm__("r0")=4;
    register const char *argument __asm__("r1")=text;
    __asm__ volatile("bkpt #0xab" : "+r"(operation) : "r"(argument) : "memory");
}
static __attribute__((noreturn)) void Exit(unsigned code) {
    uint32_t block[2]={0x20026,code};
    register uint32_t operation __asm__("r0")=0x20;
    register uint32_t *argument __asm__("r1")=block;
    __asm__ volatile("bkpt #0xab" : "+r"(operation) : "r"(argument) : "memory");
    for(;;) {}
}
__attribute__((noreturn)) void Failure(void) {Write("FAIL Cortex-M0 resource decoder\\n");Exit(1);}
static void Check(const uint8_t *packed,unsigned size,uint32_t expected) {
    for(unsigned i=0;i<sizeof(output);i++) output[i]=0xa5;
    PACK_HuffmanExpand(output+16,size,packed);
    uint32_t digest=2166136261u;
    for(unsigned i=0;i<size;i++) digest=(digest^output[16+i])*16777619u;
    if(digest!=expected) Failure();
    for(unsigned i=0;i<16;i++) if(output[i]!=0xa5 || output[16+size+i]!=0xa5) Failure();
}
void RunCodec(void) {
'''+ '\n'.join(checks)+'''
    Write("PASS Cortex-M0 exact font/text expansion and output guards\\n");Exit(0);
}
'''
(out/'guest.c').write_text(source)
(out/'start.S').write_text('''.syntax unified
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
    bl RunCodec
    b .
.thumb_func
Fault_Handler:
    bl Failure
    b .
''')
(out/'guest.ld').write_text('''ENTRY(Reset_Handler)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 256K
         RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K }
SECTIONS {
 .text : {KEEP(*(.vectors)) *(.text*) *(.rodata*)} >FLASH
 .data : {*(.data*)} >RAM AT>FLASH
 .bss : {*(.bss*) *(COMMON)} >RAM
 /DISCARD/ : {*(.ARM.exidx*) *(.ARM.extab*) *(.comment*)}
}
''')
command='''set -e
test "$(arm-none-eabi-gcc -dumpfullversion)" = 10.3.1
arm-none-eabi-gcc -std=gnu11 -Os -Wall -Wextra -Werror -mcpu=cortex-m0 -mthumb -fno-builtin -fno-lto -ffunction-sections -fdata-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -I . -nostdlib -Wl,-T,build/huffman-cpu/guest.ld -Wl,--gc-sections -Wl,-z,noexecstack build/huffman-cpu/start.S build/huffman-cpu/guest.c helper/packed_huffman.c helper/compact_memory.c -lgcc -o build/huffman-cpu/guest.elf
qemu-system-arm -M microbit -cpu cortex-m0 -nographic -semihosting-config enable=on,target=native -kernel build/huffman-cpu/guest.elf
'''
native=shutil.which('arm-none-eabi-gcc') and shutil.which('qemu-system-arm') and subprocess.check_output(['arm-none-eabi-gcc','-dumpfullversion'],text=True).strip()=='10.3.1'
args=['bash','-c',command] if native else ['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}','-v',str(root)+':/src','-w','/src',os.environ.get('UVK6_BUILD_IMAGE','uvk6-build:verified'),'bash','-c',command]
subprocess.run(args,cwd=root,check=True,timeout=30)
report={'compiler':'ARM GCC 10.3.1','cpu':'QEMU microbit Cortex-M0, 16 KiB SRAM','status':'passed','resources':hashes,'decoder_sha256':hashlib.sha256((root/'helper/packed_huffman.c').read_bytes()).hexdigest(),'elf_sha256':hashlib.sha256((out/'guest.elf').read_bytes()).hexdigest(),'limitations':'CPU/decoder execution only; no radio peripherals, physical hardware or RF validation.'}
(out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
