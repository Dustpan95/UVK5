#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
from pathlib import Path
import argparse, hashlib, json, os, re, subprocess
parser=argparse.ArgumentParser(description='Compare production RX paths against pinned TA1JS source; never accesses a radio.')
parser.add_argument('upstream',type=Path,help='Local git clone of bcanata/uv-k5-firmware-ta1js')
parser.add_argument('captures',nargs='*',type=Path,help='Optional private capture JSON files; raw/station data is never printed')
a=parser.parse_args()
root=Path(__file__).resolve().parents[1];u=a.upstream.resolve();out=root/'build/ta1js-comparison'
out.mkdir(parents=True,exist_ok=True)
pin='6320cdac7a2dce46f88027d220ff67e47eb0e3fb'
s=subprocess.check_output(['git','-C',str(u),'show',pin+':app/aprs_minimal.c'],text=True)
def fun(name):
 m=re.search(r'^(?:static )?(?:void|bool|uint16_t) '+name+r'\([^;]*?\)\s*\{',s,re.M);assert m,name
 start=m.start();brace=s.index('{',m.start());depth=1;i=brace+1
 while depth:
  depth+=(s[i]=='{')-(s[i]=='}');i+=1
 return s[start:i]
# Source functions remain verbatim. UI, configuration and TX callbacks are
# inert because this probe concerns passive modem RX/FCS only.
prefix='''#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "driver/bk4819.h"
#include "functions.h"
#define HDLC_BUF_SIZE 240u
#define APRS_RX_CAPTURE_BYTES 240u
#define APRS_RX_IRQ_MASK 0x3002u
static uint8_t gHdlcBuf[HDLC_BUF_SIZE],gRxFrame[80];
static bool gRxArmed,gRxCapturing;
static uint8_t gRxStuckTicks;
static uint16_t gRxCount;
static unsigned found;
static void APRS_ShowFrame(const uint8_t *frame,uint16_t len) {(void)frame;(void)len;found++;}
static void APRS_ExpireDisplay(void) {}
static void APRS_TxCooldown(void) {}
static bool APRS_Configured(void) {return false;}
static void APRS_SendAck(void) {}
static void APRS_TransmitNow(void) {}
static uint16_t gBeaconCountdown_500ms;
static bool gAPRS_AckPending;
static char gAPRS_AckTo[10];
static unsigned gAPRSState;
#define APRS_STATE_IDLE 0
'''
funcs=['APRS_CalculateCRC','AX25_CalculateFCS','APRS_RxArm','APRS_RxDrainFifo','APRS_DecodeCapture','APRS_HandleRxInterrupts','APRS_StopListening','APRS_Task']
# APRS_Task reads one EEPROM field even though the beacon countdown is zero.
prefix += '#define BK4819_REG_5E ((BK4819_REGISTER_t)0x5e)\n'
# Separate compilation unit uses the upstream EEPROM type; no reads occur in
# disabled TX paths, so replace the TX-only field reference by a private stub.
# Prefer preprocessor token alias to leave extracted bodies intact.
prefix += '#define gEeprom ta_eeprom\nstatic struct {uint8_t APRS_INTERVAL;} ta_eeprom;\n'
body=s[:s.index('*/')+2]+'\n'+prefix+'\n'.join(fun(n) for n in funcs)
body+='''
void TA_Arm(void) {found=0;APRS_RxArm();}
void TA_Irq(uint16_t bits) {APRS_HandleRxInterrupts(bits);}
void TA_Tick(void) {APRS_Task();}
unsigned TA_Found(void) {return found;}
unsigned TA_Decode(const uint8_t *data,uint16_t n) {memcpy(gHdlcBuf,data,n);gRxCount=n;found=0;return APRS_DecodeCapture();}
'''
(out/'upstream.c').write_text(body)
command=[os.environ.get('CC','gcc'),'-std=gnu11','-Wall','-Wextra','-Werror','-g',
 '-fsanitize=address,undefined','-fno-omit-frame-pointer','-ffunction-sections','-fdata-sections','-I',str(root),
 str(root/'tests/test_upstream_rx.c'),str(out/'upstream.c'),str(root/'app/aprs_rx.c'),
 str(root/'app/aprs_decode.c'),str(root/'app/aprs_position.c'),'-Wl,--gc-sections','-o',str(out/'probe')]
subprocess.run(command,check=True)
result=subprocess.check_output([str(out/'probe')],text=True)
print(result,end='')
report={'upstream_commit':pin,'source_sha256':hashlib.sha256(s.encode()).hexdigest(),
 'functions_sha256':{name:hashlib.sha256(fun(name).encode()).hexdigest() for name in funcs},
 'fixture_results':result.strip().splitlines(),'private_capture_results':[]}
for path in a.captures:
 data=json.loads(path.read_text())
 for capture in data['captures']:
  raw=bytes.fromhex(capture['raw_hex']);assert len(raw)==capture['length']<=240
  private=out/'private-capture.bin'
  try:
   private.write_bytes(raw)
   line=subprocess.check_output([str(out/'probe'),str(private)],text=True).strip().splitlines()[-1]
   print(line);report['private_capture_results'].append(line)
  finally:private.unlink(missing_ok=True)
(out/'comparison.json').write_text(json.dumps(report,indent=2)+'\n')
