/* SPDX-License-Identifier: GPL-3.0-only
 * Shared register/FIFO fixture for verbatim upstream and local production RX. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app/aprs_rx.h"
#include "audio.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "settings.h"
#include "tests/capture.h"
EEPROM_Config_t gEeprom;
VFO_Info_t *gRxVfo=&gEeprom.VfoInfo[0];
FUNCTION_Type_t gCurrentFunction=FUNCTION_FOREGROUND;
int8_t gScanStateDir;
bool gCssBackgroundScan,gUpdateDisplay;
volatile uint8_t gSerialConfigCountDown_500ms;
bool FUNCTION_IsRx(void) {return gCurrentFunction==FUNCTION_RECEIVE;}
void AUDIO_PlayBeep(BEEP_Type_t beep) {(void)beep;}
static uint16_t regs[128],trace[512];
static unsigned writes,readpos;
static const uint8_t *input;
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t r) {
 if(r==0x5f) {assert(readpos+1<240);uint16_t w=input[readpos]|input[readpos+1]<<8;readpos+=2;return w;}
 return regs[r];
}
void BK4819_WriteRegister(BK4819_REGISTER_t r,uint16_t v) {assert(writes<256);trace[2*writes]=r;trace[2*writes+1]=v;writes++;regs[r]=v;}
void TA_Arm(void);void TA_Irq(uint16_t);void TA_Tick(void);unsigned TA_Found(void);unsigned TA_Decode(const uint8_t*,uint16_t);
int main(int argc,char **argv) {
 gRxVfo->Modulation=MODULATION_FM;gAPRS_Enabled=true;
 uint16_t savedtrace[512];
 TA_Arm();unsigned count=writes;memcpy(savedtrace,trace,sizeof(trace));
 memset(regs,0,sizeof(regs));writes=0;APRS_Resume();
 assert(writes==count&&!memcmp(savedtrace,trace,count*4));
 printf("identical arm writes: %u\n",count);
 for(unsigned implementation=0;implementation<2;implementation++) {
  for(unsigned timeout=0;timeout<2;timeout++) {
   input=capture_fixture;readpos=0;writes=0;gAPRS_Count=0;
   if(implementation) {TA_Arm();TA_Irq(2);} else {APRS_Stop();APRS_Resume();APRS_Interrupt(2);}
   unsigned limit=timeout?56:224;
   while(readpos<limit) {if(implementation) TA_Irq(0x1000);else APRS_Interrupt(0x1000);}
   if(timeout) for(unsigned i=0;i<3;i++) {if(implementation) TA_Tick();else APRS_Tick500ms();}
   else {if(implementation) TA_Irq(0x2000);else APRS_Interrupt(0x2000);}
   assert((implementation?TA_Found():gAPRS_Count)==1);
   printf("%s %s golden: valid, read %u bytes\n",implementation?"upstream":"UVK5",timeout?"stall":"finished",readpos);
  }
 }
 APRS_Record r;
 assert(TA_Decode(capture_fixture,240)&&APRS_DecodeBits(capture_fixture,240,&r));
 uint8_t corrupt[240];memcpy(corrupt,capture_fixture,240);corrupt[10]^=1;
 assert(!TA_Decode(corrupt,240)&&!APRS_DecodeBits(corrupt,240,&r));
 assert(!TA_Decode(capture_fixture,8)&&!APRS_DecodeBits(capture_fixture,8,&r));
 puts("both production decoders reject corrupted and truncated fixture");
 if(argc==2) {
  uint8_t buf[241];FILE *f=fopen(argv[1],"rb");assert(f);size_t n=fread(buf,1,sizeof(buf),f);assert(!ferror(f)&&n<=240);fclose(f);
  printf("capture: upstream=%u UVK5=%u bytes=%zu\n",TA_Decode(buf,n),APRS_DecodeBits(buf,n,&r),n);
 }
 return 0;
}
