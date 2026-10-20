#!/usr/bin/env python3
"""Compare actual F+9 lookup data/predicate against the retained original."""
from pathlib import Path
import os,re,subprocess
root=Path(__file__).resolve().parents[1]
source=(root/'app/main.c').read_text()
start=source.index('// ---- Calling frequency lookup table ----')
end=source.index('// ---- Save/restore state',start)
current=source[start:end]
original=(root/'tests/reference/band_call.c').read_text()
original=original[original.index('// ---- Calling frequency lookup table ----'):]
predicate=' '.join(re.search(r'if \((freq >= .*?band_end.*?)\) \{',source,re.S)[1].split())
# Original comparison uses full-resolution edges; do not normalize its inputs.
reference='freq >= reference[i].band_start && freq <= reference[i].band_end'
text='#include <stdint.h>\n#include <assert.h>\n#include <stdio.h>\n#include "radio.h"\n'
text+=original.replace('BandCallFreq_t','ReferenceBand').replace('band_call_table','reference').replace('BAND_CALL_COUNT','REFERENCE_COUNT')
text+=current
text+='''static int Old(uint32_t freq) {for(unsigned i=0;i<REFERENCE_COUNT;i++) if('''+reference+''') return i;return -1;}
static int Current(uint32_t freq) {for(unsigned i=0;i<BAND_CALL_COUNT;i++) if('''+predicate+''') return i;return -1;}
static unsigned checks;
static void Check(uint32_t freq) {assert(Old(freq)==Current(freq));++checks;}
int main(void) {
 assert(REFERENCE_COUNT==BAND_CALL_COUNT);
 for(unsigned i=0;i<BAND_CALL_COUNT;i++) {
  assert((uint32_t)band_call_table[i].band_start*1000u==reference[i].band_start);
  assert((uint32_t)band_call_table[i].band_end*1000u==reference[i].band_end);
  assert(band_call_table[i].call_freq==reference[i].call_freq);
  assert(band_call_table[i].step_freq==reference[i].step_freq);
  assert(band_call_table[i].modulation==reference[i].modulation);
  for(int edge=-2000;edge<=2000;edge++) {
   Check(reference[i].band_start+edge);Check(reference[i].band_end+edge);
  }
 }
 uint32_t random=0x49565123u;
 for(unsigned i=0;i<1000000;i++) {random=random*1664525u+1013904223u;Check(random);Check(random%130000001u);}
 Check(0);Check(UINT32_MAX);
 printf("PASS %u original calling-band lookup comparisons, exact inclusive edges, priority and unchanged channel parameters\\n",checks);
}
'''
out=root/'build/call-bands';out.mkdir(parents=True,exist_ok=True)
(out/'test.c').write_text(text)
cmd=[os.environ.get('CC','gcc'),'-std=c11','-O1','-Wall','-Wextra','-Werror','-I',str(root)]
if os.environ.get('SANITIZE','1')=='1':cmd+=['-fsanitize=address,undefined','-fno-omit-frame-pointer']
subprocess.run([*cmd,str(out/'test.c'),'-o',str(out/'test')],check=True)
subprocess.run([str(out/'test')],check=True)
