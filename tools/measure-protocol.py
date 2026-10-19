#!/usr/bin/env python3
"""Measure protocol helpers and retained experimental radio transport.

Creates an oversized diagnostic copy under build/. Never execute or flash it.
The initial helper probe writes synthetic symbols to RAM. The last phase retains
actual hardware entry points for size measurement only.
"""
from pathlib import Path
import argparse
import hashlib
import json
import os
import shutil
import subprocess
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--transport-options',action='store_true',help='Also measure explicitly labeled optional feature combinations; never changes defaults')
args=parser.parse_args()
root=Path(__file__).resolve().parents[1];work=root/'build/protocol-size'
if work.exists():shutil.rmtree(work)
def ignore(directory,names):
 return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,work,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(work.rglob('*')):
 if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
  fingerprint.update(str(p.relative_to(work)).encode());fingerprint.update(p.read_bytes())
p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
def build(flags=()):
 result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
 '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c',
 'set -e; make clean; make -j2 ENABLE_APRS_MESSAGING=1 "$@"; arm-none-eabi-nm -S --size-sort uvk6-experimental > symbols.txt; arm-none-eabi-size -A uvk6-experimental','bash',*flags],
 stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 if result.returncode:raise RuntimeError(result.stdout[-5000:])
 return (work/'uvk6-experimental.bin').stat().st_size,result.stdout
def static_ram(log):
 sizes={}
 for line in log.splitlines():
  f=line.split()
  if len(f)==3 and f[0] in {'.data','.bss'}:sizes[f[0]]=int(f[1])
 return sum(sizes.values())
baseline,log=build();(work/'baseline.log').write_text(log)
baseline_ram=static_ram(log)
print(json.dumps({'phase':'baseline','bytes':baseline}),flush=True)
(work/'app/protocol_probe.c').write_text('''/* Diagnostic RAM-only probe; never release or flash. */
#include "app/aprs_message.h"
#include "app/aprs_session.h"
#include "experiments/bell202_symbols.h"
#include "experiments/bell202_timing.h"
volatile unsigned gProbeMode, gProbeTone, gProbeAccepted;
volatile int32_t gProbeLatitude, gProbeLongitude;
volatile uint32_t gProbeFrequency;
volatile char gProbeId[5],gProbeSource[9];
static void Symbol(bool high, void *context) { (void)context;gProbeTone=high?2200:1200; }
static uint32_t Now(void *context) { (void)context;static uint32_t cycle;return cycle+=100; }
static bool Cancelled(void *context) { (void)context;return gProbeMode&16; }
void APRS_ProtocolProbe(void) {
 uint8_t frame[160];size_t n;
 char id[6],source[10];
 for(unsigned i=0;i<5;i++) {id[i]=gProbeId[i];} id[5]=0;
 for(unsigned i=0;i<9;i++) {source[i]=gProbeSource[i];} source[9]=0;
 if(gProbeMode&8) n=MESSAGE_BuildControl(gAPRS_Profile.callsign,gAPRS_Message.recipient,id,gProbeMode&32,frame,sizeof(frame));
 else if(gProbeMode&1) n=MESSAGE_BuildBeacon(gAPRS_Profile.callsign,gProbeLatitude,gProbeLongitude,'/','-',frame,sizeof(frame));
 else n=MESSAGE_BuildFrame(gAPRS_Profile.callsign,gAPRS_Message.recipient,gAPRS_Message.text,id,frame,sizeof(frame));
 const BELL202_Output output={Now,Symbol,Cancelled,0};
 gProbeAccepted=BELL202_Transmit(frame,n,&output);
 if(gProbeMode&2) gProbeAccepted=MESSAGE_MarkSent(gAPRS_Profile.callsign,id);
 if(gProbeMode&4) gProbeAccepted=MESSAGE_Acknowledge(source,gAPRS_Profile.callsign,id,gProbeMode&8);
}
''')
p=work/'Makefile';p.write_text(p.read_text().replace('OBJS += app/aprs_message.o','OBJS += app/protocol_probe.o experiments/bell202_symbols.o experiments/bell202_timing.o app/aprs_message.o'))
p=work/'app/aprs_message.c';p.write_text(p.read_text().replace('m->delivery=MESSAGE_TX_UNAVAILABLE;', 'extern void APRS_ProtocolProbe(void); APRS_ProtocolProbe(); m->delivery=MESSAGE_TX_UNAVAILABLE;'))
size,log=build();(work/'probe.log').write_text(log)
report={'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1',
 'real_flash_limit':61440,'diagnostic_flash_limit':131072,
 'messaging_candidate_bytes':baseline,'messaging_static_ram_bytes':baseline_ram,'with_protocol_helpers_bytes':size,'increment_bytes':size-baseline,
 'not_included':['radio tone output and MCU clock backend','TX limits/arbitration','busy-channel handling',
 'automatic ACK dispatch','retry service','position and saved-message editing UI']}
(work/'results.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

# Separately retain the new frozen-outbox model, actual cycle counter and tone
# backend. This oversized diagnostic is measured, never executed on hardware.
p=work/'Makefile';p.write_text(p.read_text().replace('OBJS += app/protocol_probe.o',
 'OBJS += app/aprs_tx.o experiments/bell202_radio.o experiments/aprs_radio_owner.o app/protocol_probe.o'))
p=work/'app/protocol_probe.c';s=p.read_text()
s=s.replace('#include "experiments/bell202_timing.h"', '''#include "experiments/bell202_timing.h"
#include "experiments/bell202_radio.h"
#include "experiments/aprs_radio_owner.h"
#include "app/aprs_tx.h"
#include "app/aprs_rx.h"
extern uint32_t SCHEDULER_GetTicks10ms(void);
static uint32_t LinkNow(void *ctx) { (void)ctx;return SCHEDULER_GetTicks10ms(); }
static APRS_LinkResult SendRadio(const uint8_t *frame,size_t length,uint32_t frequency,void *ctx) {
 (void)ctx;
 const APRS_RadioConfig config={gProbeMode&128,gProbeMode&127};
 return APRS_RadioSend(frame,length,frequency,(void *)&config);
}''')
# The injected callbacks need the volatile input declaration above them.
s=s.replace('static APRS_LinkResult SendRadio(', 'extern volatile unsigned gProbeMode;\nstatic APRS_LinkResult SendRadio(')
s=s.replace(' gProbeAccepted=BELL202_Transmit(frame,n,&output);',''' gProbeAccepted=BELL202_Transmit(frame,n,&output);
 if(gProbeMode&64) {
  const uint32_t now=SCHEDULER_GetTicks10ms();
  if(gProbeMode&8) gProbeAccepted=TX_QueueControl(gProbeFrequency,gAPRS_Profile.callsign,gAPRS_Message.recipient,id,gProbeMode&32,1000,now);
  else if(gProbeMode&1) gProbeAccepted=TX_QueueBeacon(gProbeFrequency,gAPRS_Profile.callsign,gProbeLatitude,gProbeLongitude,1000,now);
  else gProbeAccepted=TX_QueueMessage(gProbeFrequency,gAPRS_Profile.callsign,gAPRS_Message.recipient,gAPRS_Message.text,id,(APRS_TX_Policy){1000,20000,3},now);
  const APRS_TX_Link link={LinkNow,SendRadio,0};TX_Process(&link);
  if(gProbeMode&2) gProbeAccepted=TX_Acknowledge(source,gAPRS_Profile.callsign,id,gProbeMode&4);
  if(gProbeMode&16) TX_CancelAll();
  if(gProbeMode&512) gProbeAccepted=TX_PendingControls();
  if((gProbeMode&1024) && gAPRS_Selected<gAPRS_Count && gAPRS_Selected<APRS_LOG_CAPACITY)
   gProbeFrequency=gAPRS_Log[gAPRS_Selected].frequency;
 }''')
p.write_text(s)
transport,log=build();(work/'transport.log').write_text(log)
report['with_tone_backend_and_outbox_bytes']=transport
report['transport_increment_bytes']=transport-size
report['transport_static_ram_bytes']=static_ram(log)
report['transport_still_excludes']=['normal UI send integration','automatic ACK dispatch',
 'position/quick-message configuration UI','relay path configuration','hardware carrier-sense/RF validation']
(work/'results.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

if args.transport_options:
 variants=[
  ('without_application_uart',['ENABLE_UART=0']),
  ('without_uart_am_fix',['ENABLE_UART=0','ENABLE_AM_FIX=0']),
  ('without_uart_am_fix_flashlight_scan_options',['ENABLE_UART=0','ENABLE_AM_FIX=0','ENABLE_FLASHLIGHT=0','ENABLE_SCAN_RANGES=0','ENABLE_ADAPTIVE_SCAN=0']),
 ]
 report['optional_transport_profiles']=[]
 for name,flags in variants:
  used,log=build(flags);(work/(name+'.log')).write_text(log)
  item={'profile':name,'flags':flags,'flash_bytes':used,'static_ram_bytes':static_ram(log),
        'delta_flash_bytes':used-transport,'over_flash_bytes':used-61440}
  report['optional_transport_profiles'].append(item);print(json.dumps(item),flush=True)
 (work/'results.json').write_text(json.dumps(report,indent=2)+'\n')

# Finally retain the session policy and channel orchestration. Entry/restore
# callbacks below only touch RAM: the actual channel hardware adapter is absent.
p=work/'Makefile';p.write_text(p.read_text().replace('OBJS += app/aprs_tx.o',
 'OBJS += experiments/aprs_service.o experiments/aprs_channel.o app/aprs_tx.o'))
p=work/'app/protocol_probe.c';s=p.read_text()
s=s.replace('#include "app/aprs_tx.h"', '#include "app/aprs_tx.h"\n#include "experiments/aprs_channel.h"')
s=s.replace('void APRS_ProtocolProbe(void) {', r'''static volatile uint32_t probeChannel,restores;
static APRS_ChannelAdmission EnterChannel(uint32_t frequency,void *ctx) {
 (void)ctx;probeChannel=frequency;
 return gProbeMode&4096 ? APRS_CHANNEL_BUSY : APRS_CHANNEL_READY;
}
static void RestoreChannel(void *ctx) {(void)ctx;++restores;}
static bool InterruptChannel(void *ctx) {(void)ctx;return gProbeMode&8192;}
void APRS_ProtocolProbe(void) {''')
s=s.replace(' const BELL202_Output output={Now,Symbol,Cancelled,0};', r''' if(gProbeMode&2048) {
  static APRS_Channel channel;
  const APRS_ChannelOps ops={LinkNow,EnterChannel,SendRadio,RestoreChannel,InterruptChannel,0};
  gProbeAccepted=CHANNEL_Configure(&channel,&ops);
  const APRS_ServiceSettings config={gProbeFrequency,APRS_ACK_DEFAULT_TICKS,60000,1000,gProbeMode&128};
  gProbeAccepted=SERVICE_Configure(&config);
  const uint32_t now=SCHEDULER_GetTicks10ms();
  if(gProbeMode&1) gProbeAccepted=SERVICE_Beacon(now);
  else gProbeAccepted=SERVICE_Message(gAPRS_Message.recipient,gAPRS_Message.text,id,now);
  if(gAPRS_Selected<gAPRS_Count && gAPRS_Selected<APRS_LOG_CAPACITY)
   gProbeAccepted=SERVICE_Receive(&gAPRS_Log[gAPRS_Selected],now);
  CHANNEL_Process(&channel);
  if(gProbeMode&16) CHANNEL_Interrupt(&channel);
 }
 const BELL202_Output output={Now,Symbol,Cancelled,0};''')
p.write_text(s)
service,log=build();(work/'service.log').write_text(log)
report['with_session_service_and_channel_model_bytes']=service
report['service_model_increment_bytes']=service-transport
report['service_model_static_ram_bytes']=static_ram(log)
report['service_model_still_excludes']=['normal UI/receive/foreground binding','actual separate-frequency hardware adapter',
 'position/quick-message configuration UI','unique outgoing ID allocation','relay path configuration','hardware carrier-sense/RF validation']
(work/'results.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
