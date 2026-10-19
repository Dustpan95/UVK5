#!/usr/bin/env python3
"""Measure model-bound APRS service paths without the older parallel probes.

Only an ignored isolated copy is changed. Its linker is oversized and its
channel entry/restore are RAM models, not a hardware adapter. Never execute,
flash or distribute the resulting diagnostic image as usable firmware.
"""
from pathlib import Path
import hashlib
import json
import os
import shutil
import subprocess
root=Path(__file__).resolve().parents[1];work=root/'build/service-size'
if work.exists():shutil.rmtree(work)
def ignore(directory,names):
 return [n for n in names if n in {'.git','build','__pycache__'} or n.endswith(('.o','.d','.map','.pyc')) or n.startswith('uvk6-experimental')]
shutil.copytree(root,work,ignore=ignore)
fingerprint=hashlib.sha256()
for p in sorted(work.rglob('*')):
 if p.is_file() and (p.suffix in {'.c','.h','.ld','.inc'} or p.name=='Makefile'):
  fingerprint.update(str(p.relative_to(work)).encode());fingerprint.update(p.read_bytes())
p=work/'firmware.ld';p.write_text(p.read_text().replace('LENGTH = 60K','LENGTH = 128K'))
p=work/'Makefile';s=p.read_text().replace('OBJS += app/aprs_message.o',
 'OBJS += app/aprs_runtime_probe.o app/aprs_tx.o experiments/aprs_service.o experiments/aprs_channel.o experiments/aprs_radio_owner.o experiments/bell202_radio.o experiments/bell202_timing.o experiments/bell202_symbols.o app/aprs_message.o');p.write_text(s)
(work/'app/aprs_runtime_probe.c').write_text(r'''/* DIAGNOSTIC ONLY: never execute or flash. Entry/restore simulate RAM state. */
#include "app/aprs_message.h"
#include "app/aprs_session.h"
#include "experiments/aprs_channel.h"
#include "experiments/aprs_radio_owner.h"
extern uint32_t SCHEDULER_GetTicks10ms(void);
volatile uint32_t probeFrequency,probeVersion,probeBeacon,probeChannel,probeRestores;
volatile int32_t probeLatitude,probeLongitude;
volatile uint8_t probeToneGain,probeAdmission;
volatile bool probeEnabled,probeInterrupted,probePosition;
volatile char probeId[5];
static uint32_t Now(void *ctx) {(void)ctx;return SCHEDULER_GetTicks10ms();}
static APRS_ChannelAdmission Enter(uint32_t frequency,void *ctx) {
 (void)ctx;probeChannel=frequency;return (APRS_ChannelAdmission)probeAdmission;
}
static void Restore(void *ctx) {(void)ctx;++probeRestores;}
static bool Interrupted(void *ctx) {(void)ctx;return probeInterrupted;}
static APRS_LinkResult Send(const uint8_t *frame,size_t n,uint32_t frequency,void *ctx) {
 (void)ctx;const APRS_RadioConfig config={probeEnabled,probeToneGain};
 return APRS_RadioSend(frame,n,frequency,(void *)&config);
}
bool APRS_RuntimeSendDraft(void) {
 char id[6];for(unsigned i=0;i<5;++i) id[i]=probeId[i];id[5]=0;
 return SERVICE_Message(gAPRS_Message.recipient,gAPRS_Message.text,id,Now(0));
}
void APRS_RuntimeProbe(void) {
 static APRS_Channel channel;static uint32_t seen,beacon;
 if(probeVersion!=seen) {
  const APRS_ServiceSettings config={probeFrequency,APRS_ACK_DEFAULT_TICKS,60000,1000,probeEnabled};
  if(SERVICE_Configure(&config)) {
   seen=probeVersion;
   const APRS_ChannelOps ops={Now,Enter,Send,Restore,Interrupted,0};CHANNEL_Configure(&channel,&ops);
   if(probePosition) SESSION_SetPosition(probeLatitude,probeLongitude);
  }
 }
 if(probeBeacon!=beacon) {beacon=probeBeacon;SERVICE_Beacon(Now(0));}
 CHANNEL_Process(&channel);
 if(gAPRS_TX.state==APRS_TX_ACKED) gAPRS_Message.delivery=MESSAGE_ACKED;
 if(gAPRS_TX.state==APRS_TX_REJECTED) gAPRS_Message.delivery=MESSAGE_REJECTED;
}
''')
p=work/'app/app.c';s=p.read_text();needle='void APP_TimeSlice10ms(void)\n{';assert needle in s
p.write_text(s.replace(needle,needle+'\n    extern void APRS_RuntimeProbe(void); APRS_RuntimeProbe();',1))
p=work/'app/aprs_rx.c';s=p.read_text().replace('#include "aprs_rx.h"','#include "aprs_rx.h"\n#include "experiments/aprs_service.h"\nextern uint32_t SCHEDULER_GetTicks10ms(void);')
needle='record.frequency=gRxVfo->pRX ? gRxVfo->pRX->Frequency : 0;';assert needle in s
p.write_text(s.replace(needle,needle+'\n    SERVICE_Receive(&record,SCHEDULER_GetTicks10ms());',1))
p=work/'app/aprs_message.c';s=p.read_text();needle='m->delivery=MESSAGE_TX_UNAVAILABLE;';assert needle in s
p.write_text(s.replace(needle,'extern bool APRS_RuntimeSendDraft(void); m->delivery=APRS_RuntimeSendDraft() ? MESSAGE_WAIT_ACK : MESSAGE_TX_UNAVAILABLE;',1))
p=work/'app/aprs_session.c';s=p.read_text();needle='MESSAGE_Acknowledge(r->source,r->destination,r->messageId,r->kind==APRS_REJECT);';assert needle in s
p.write_text(s.replace(needle,'/* SERVICE_Receive handles control correlation in this diagnostic copy. */',1))
result=subprocess.run(['docker','run','--rm','--user',f'{os.getuid()}:{os.getgid()}',
 '-v',str(work)+':/src','-w','/src','uvk6-build:verified','bash','-c',
 'set -e; make -j2 ENABLE_APRS_MESSAGING=1; arm-none-eabi-size -A uvk6-experimental; arm-none-eabi-nm -S --size-sort uvk6-experimental > symbols.txt'],
 stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
(work/'build.log').write_text(result.stdout)
if result.returncode:raise RuntimeError(result.stdout[-4000:])
sizes={}
for line in result.stdout.splitlines():
 f=line.split()
 if len(f)==3 and f[0] in {'.data','.bss'}:sizes[f[0]]=int(f[1])
used=(work/'uvk6-experimental.bin').stat().st_size
report={'source_fingerprint':fingerprint.hexdigest(),'compiler':'ARM GCC 10.3.1',
 'real_flash_limit':61440,'diagnostic_flash_limit':131072,'model_bound_service_bytes':used,
 'over_flash_bytes':used-61440,'static_ram_bytes':sum(sizes.values()),
 'scope':'Diagnostic UI/RX/foreground hooks and RAM channel model; not operational firmware',
 'still_excludes':['actual separate-frequency hardware adapter','position/quick-message editing screens',
 'unique outgoing ID allocation','complete delivery-state UI','relay paths','hardware carrier-sense/RF validation']}
(work/'results.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
