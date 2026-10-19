#!/usr/bin/env python3
"""Measure retained message/beacon/HDLC/ACK helpers, without a radio driver.

Creates an oversized diagnostic copy under build/. Never flash this copy.
The probe only writes synthetic symbols to volatile RAM; no RF output exists.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parents[1];work=root/'build/protocol-size'
if work.exists():shutil.rmtree(work)
def ignore(directory,names):
 return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,work,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(work.rglob('*')):
 if p.is_file() and (p.suffix in {'.c','.h','.ld'} or p.name=='Makefile'):
  fingerprint.update(str(p.relative_to(work)).encode());fingerprint.update(p.read_bytes())
p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
def build():
 result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
 '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c',
 'set -e; make clean; make -j2 ENABLE_APRS_MESSAGING=1; arm-none-eabi-nm -S --size-sort uvk6-experimental > symbols.txt'],
 stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 if result.returncode:raise RuntimeError(result.stdout[-5000:])
 return (work/'uvk6-experimental.bin').stat().st_size,result.stdout
baseline,log=build();(work/'baseline.log').write_text(log)
print(json.dumps({'phase':'baseline','bytes':baseline}),flush=True)
(work/'app/protocol_probe.c').write_text('''/* Diagnostic RAM-only probe; never release or flash. */
#include "app/aprs_message.h"
#include "app/aprs_session.h"
#include "experiments/bell202_symbols.h"
volatile unsigned gProbeMode, gProbeTone, gProbeAccepted;
volatile int32_t gProbeLatitude, gProbeLongitude;
volatile char gProbeId[5],gProbeSource[9];
static void Symbol(bool high, void *context) { (void)context;gProbeTone=high?2200:1200; }
void APRS_ProtocolProbe(void) {
 uint8_t frame[160];size_t n;
 char id[6],source[10];
 for(unsigned i=0;i<5;i++) {id[i]=gProbeId[i];} id[5]=0;
 for(unsigned i=0;i<9;i++) {source[i]=gProbeSource[i];} source[9]=0;
 if(gProbeMode&1) n=MESSAGE_BuildBeacon(gAPRS_Profile.callsign,gProbeLatitude,gProbeLongitude,'/','-',frame,sizeof(frame));
 else n=MESSAGE_BuildFrame(gAPRS_Profile.callsign,gAPRS_Message.recipient,gAPRS_Message.text,id,frame,sizeof(frame));
 gProbeAccepted=BELL202_Encode(frame,n,Symbol,0);
 if(gProbeMode&2) gProbeAccepted=MESSAGE_MarkSent(gAPRS_Profile.callsign,id);
 if(gProbeMode&4) gProbeAccepted=MESSAGE_Acknowledge(source,gAPRS_Profile.callsign,id,gProbeMode&8);
}
''')
p=work/'Makefile';p.write_text(p.read_text().replace('OBJS += app/aprs_message.o','OBJS += app/protocol_probe.o experiments/bell202_symbols.o app/aprs_message.o'))
p=work/'app/aprs_message.c';p.write_text(p.read_text().replace('m->delivery=MESSAGE_TX_UNAVAILABLE;', 'extern void APRS_ProtocolProbe(void); APRS_ProtocolProbe(); m->delivery=MESSAGE_TX_UNAVAILABLE;'))
size,log=build();(work/'probe.log').write_text(log)
report={'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1',
 'real_flash_limit':61440,'diagnostic_flash_limit':131072,
 'messaging_candidate_bytes':baseline,'with_protocol_helpers_bytes':size,'increment_bytes':size-baseline,
 'not_included':['radio tone output and scheduling','TX limits/arbitration','busy-channel handling',
 'on-air ACK parsing and generation','retry service','position and saved-message editing UI','station directory']}
(work/'results.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
