/* SPDX-License-Identifier: GPL-3.0-only */
#include "settings.h"
#include "misc.h"
#include "driver/eeprom.h"
#include "dtmf.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void REFERENCE_Settings_InitEEPROM(void);
static unsigned reads;
static unsigned char image[8192];
void EEPROM_ReadBuffer(uint16_t address,void *out,uint8_t size)
{
 assert((unsigned)address+size<=sizeof(image));
 memcpy(out,image+address,size);++reads;
}
void EEPROM_WriteBuffer(uint16_t address,const void *data) {(void)address;(void)data;assert(!"loader wrote EEPROM");}
static void Reset(void)
{
 memset(&gEeprom,0xa5,sizeof(gEeprom));memset(gMR_ChannelAttributes,0xa5,sizeof(gMR_ChannelAttributes));
 memset(gCustomAesKey,0xa5,sizeof(gCustomAesKey));
 bHasCustomAesKey=false;gSetting_F_LOCK=0xa5;
 gSetting_350TX=gSetting_200TX=gSetting_500TX=gSetting_350EN=gSetting_ScrambleEnable=false;
 gSetting_live_DTMF_decoder=false;gSetting_battery_text=0xa5;gSetting_backlight_on_tx_rx=0;
#ifdef ENABLE_AM_FIX
 gSetting_AM_fix=false;
#endif
#ifdef ENABLE_AUDIO_BAR
 gSetting_mic_bar=false;
#endif
#ifdef ENABLE_DTMF_CALLING
 gSetting_KILLED=false;
#endif
 reads=0;
}
static void Capture(unsigned char *out)
{
 memcpy(out,&gEeprom,sizeof(gEeprom));out+=sizeof(gEeprom);
 memcpy(out,gMR_ChannelAttributes,sizeof(gMR_ChannelAttributes));out+=sizeof(gMR_ChannelAttributes);
 memcpy(out,gCustomAesKey,sizeof(gCustomAesKey));out+=sizeof(gCustomAesKey);
 *out++=bHasCustomAesKey;*out++=gSetting_F_LOCK;
 *out++=gSetting_350TX;*out++=gSetting_200TX;*out++=gSetting_500TX;*out++=gSetting_350EN;
 *out++=gSetting_ScrambleEnable;*out++=gSetting_live_DTMF_decoder;*out++=gSetting_battery_text;
 *out++=gSetting_backlight_on_tx_rx;
#ifdef ENABLE_AM_FIX
 *out++=gSetting_AM_fix;
#endif
#ifdef ENABLE_AUDIO_BAR
 *out++=gSetting_mic_bar;
#endif
#ifdef ENABLE_DTMF_CALLING
 *out++=gSetting_KILLED;
#endif
}
static void Check(void)
{
 unsigned char actual[sizeof(gEeprom)+sizeof(gMR_ChannelAttributes)+sizeof(gCustomAesKey)+16]={0},expected[sizeof(actual)]={0};
 Reset();REFERENCE_Settings_InitEEPROM();Capture(expected);unsigned before=reads;
 Reset();SETTINGS_InitEEPROM();Capture(actual);assert(reads<before);
 assert(!memcmp(actual,expected,sizeof(actual)));
}
int main(void)
{
 unsigned cases=0;
 for(unsigned fill=0;fill<256;fill++) {memset(image,fill,sizeof(image));Check();++cases;}
 for(unsigned byte=0xe70;byte<0xf48;byte++) for(unsigned value=0;value<256;value++) {
  memset(image,0xff,sizeof(image));image[byte]=value;Check();++cases;
 }
 unsigned random=0x12345678;
 for(unsigned test=0;test<1024;test++) {
  for(unsigned i=0;i<sizeof(image);i++) {random=random*1664525u+1013904223u;image[i]=random>>24;}
  Check();++cases;
 }
 printf("PASS EEPROM loader: %u differential configurations, identical RAM settings, no EEPROM writes\n",cases);
}
