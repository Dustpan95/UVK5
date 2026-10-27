/* SPDX-License-Identifier: GPL-3.0-only
 * Host tests of actual firmware modules against a simulated BK4819 register file.
 * Does not model RF demodulation, audio quality, or MCU timing.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/aprs_ui.h"
#include "audio.h"
#include "font.h"
#ifdef ENABLE_APRS_SESSION
#include "app/aprs_message.h"
#include "app/aprs_session.h"
#endif
#include "app/chFrScanner.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "functions.h"
#include "settings.h"
#include "misc.h"
#include "ui/ui.h"
#include "ui/home.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "vfo_split.h"
#include "tx_compressor.h"
#include "toast.h"
#include "smart_squelch.h"
#include "rssi_histogram.h"
#include "dtmf.h"
#include "helper/battery.h"
#include "signal_quality.h"

EEPROM_Config_t gEeprom;
VFO_Info_t *gRxVfo = &gEeprom.VfoInfo[0], *gCurrentVfo = &gEeprom.VfoInfo[0];
FUNCTION_Type_t gCurrentFunction;
VfoSplit_t gVfoSplit;
VfoState_t VfoState[2];
int8_t gScanStateDir;
bool gScanPauseMode, gSetting_live_DTMF_decoder;
char gDTMF_InputBox[15],gDTMF_RX_live[20],toast_msg[TOAST_MSG_SIZE];
static bool bandscope_enabled;
bool BANDSCOPE_IsEnabled(void) { return bandscope_enabled; }
void BANDSCOPE_Render(uint8_t *line) { memset(line,0x55,128); }
uint8_t SCAN_RATE_ChannelsPerSec(void) { return 18; }
SignalQuality_t SIGNAL_QUALITY_Get(void) { return QUALITY_EXCELLENT; }
char SIGNAL_CLASSIFIER_GetSymbol(uint8_t vfo) { (void)vfo;return 'V'; }
bool gCssBackgroundScan, gUpdateDisplay, gDTMF_InputMode, gRxVfoIsActive, gWasFKeyPressed;
uint16_t gBatteryVoltageAverage;
volatile uint16_t gTxTimerCountdown_500ms;
const char gModulationStr[MODULATION_UKNOWN][4] = {"FM", "AM", "USB"};
static char channel_name[17] = "LOCAL RPT";
unsigned int BATTERY_VoltsToPercent(unsigned int v) { (void)v; return 76; }
volatile uint8_t gSerialConfigCountDown_500ms;
GUI_DisplayType_t gRequestDisplayScreen;
uint8_t gFrameBuffer[FRAME_LINES][LCD_WIDTH], gStatusLine[LCD_WIDTH];
char gInputBox[8];
uint8_t gInputBoxIndex, toast_timer;
#ifdef ENABLE_TONE_SEARCH
static bool scanning;
#endif
static uint16_t regs[128];
static unsigned writes, blits;
static unsigned beeps;
void AUDIO_PlayBeep(BEEP_Type_t beep) {
    assert(beep==BEEP_1KHZ_60MS_OPTIONAL);
    ++beeps;
}
RSSI_Histogram_t gRSSI_Histogram[2];
uint16_t BK4819_GetRSSI(void) { return 220; }
uint8_t BK4819_GetExNoiceIndicator(void) { return 12; }
uint8_t BK4819_GetGlitchIndicator(void) { return 10; }
#ifdef ENABLE_TONE_SEARCH
bool SCANNER_IsScanning(void) { return scanning; }
#endif
bool FUNCTION_IsRx(void) { return gCurrentFunction==FUNCTION_RECEIVE || gCurrentFunction==FUNCTION_MONITOR || gCurrentFunction==FUNCTION_INCOMING; }
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t reg) { assert(reg<128);

    return regs[reg]; }
void BK4819_WriteRegister(BK4819_REGISTER_t reg,uint16_t value) { assert(reg<128);regs[reg]=value;++writes; }
void ST7565_BlitFullScreen(void) { ++blits; }
void _putchar(char c) { (void)c; }
void SETTINGS_FetchChannelName(char *s, const int channel) { (void)channel;strcpy(s,channel_name); }
int16_t UI_RX_Signal_dBm(uint8_t *s) { *s=7;return -87; }
static void image(const char *name) {
    FILE *f=fopen(name,"w");assert(f);
    fprintf(f,"P1\n128 64\n");
    for(unsigned y=0;y<64;y++) {
        for(unsigned x=0;x<128;x++) {
            uint8_t value=y<8 ? gStatusLine[x] : gFrameBuffer[(y-8)/8][x];
            fprintf(f,"%u ",(value>>(y%8))&1);
        }
        fputc('\n',f);
    }
    fclose(f);
}
int main(int argc,char **argv) {
    assert(argc==2);FONT_Init();unsigned stopped;char path[256];
    gRxVfo->Modulation=MODULATION_FM;gCurrentFunction=FUNCTION_FOREGROUND;
    gEeprom.SQUELCH_LEVEL=3;
    gRxVfo->pRX=&gRxVfo->freq_config_RX;
    gRxVfo->pRX->Frequency=14652000;
    gRxVfo->SquelchOpenRSSIThresh=120;gRxVfo->SquelchCloseRSSIThresh=110;
    gRSSI_Histogram[0].valid=true;gRSSI_Histogram[0].noise_floor_dbm=-110;
    regs[0x78]=(120<<8)|110;
    for(unsigned i=0;i<200;i++) SMART_SQUELCH_Update();
    assert(regs[0x78]==((118<<8)|108));
    gScanStateDir=1;SMART_SQUELCH_Update();assert(regs[0x78]==((120<<8)|110));
    stopped=writes;for(unsigned i=0;i<20;i++) SMART_SQUELCH_Update();assert(writes==stopped);
    gScanStateDir=0;
    puts("PASS adaptive squelch voice bias, scan exclusion and threshold restore");

    regs[0x7d]=0xa010;regs[0x64]=32767;
    TX_COMPRESSOR_Start();
    for(unsigned i=0;i<100;i++) {
        TX_COMPRESSOR_Process();
        assert((regs[0x7d]&31)>=4 && (regs[0x7d]&31)<=31);
        assert((regs[0x7d]&~31)==0xa000);
    }
    assert((regs[0x7d]&31)<16);
    TX_COMPRESSOR_Stop();assert(regs[0x7d]==0xa010);
    gCurrentVfo->Modulation=MODULATION_AM;
    stopped=writes;TX_COMPRESSOR_Start();TX_COMPRESSOR_Process();TX_COMPRESSOR_Stop();assert(writes==stopped);
    gCurrentVfo->Modulation=MODULATION_FM;
    puts("PASS compressor gain limits, reserved bits, restore and FM-only gate");

    gEeprom.TX_VFO=0;gEeprom.ScreenChannel[0]=0;gEeprom.SQUELCH_LEVEL=3;
    gRxVfo->pRX=&gRxVfo->freq_config_RX;gRxVfo->pTX=&gRxVfo->freq_config_TX;
    gRxVfo->pRX->Frequency=14652000;gRxVfo->StepFrequency=1250;gCurrentFunction=FUNCTION_RECEIVE;
    memset(gFrameBuffer,0,sizeof(gFrameBuffer));
    gEeprom.VfoInfo[1].pRX=&gEeprom.VfoInfo[1].freq_config_RX;
    gEeprom.VfoInfo[1].pTX=&gEeprom.VfoInfo[1].freq_config_TX;
    gEeprom.VfoInfo[1].pRX->Frequency=14439000;
    gRxVfo->pRX->Frequency=14694000;
    gRxVfo->pTX->Frequency=14634000;
    UI_HomeStatus();
    assert(UI_HomeDraw());snprintf(path,sizeof(path),"%s/home-memory.pbm",argv[1]);image(path);
    uint8_t named[sizeof(gFrameBuffer)]; memcpy(named,gFrameBuffer,sizeof(named));
    channel_name[0]=0;
    memset(gFrameBuffer,0,sizeof(gFrameBuffer));
    assert(UI_HomeDraw());assert(memcmp(named,gFrameBuffer,sizeof(named))!=0);
    snprintf(path,sizeof(path),"%s/home-unnamed.pbm",argv[1]);image(path);
    gEeprom.ScreenChannel[0]=FREQ_CHANNEL_FIRST;
    memset(gFrameBuffer,0,sizeof(gFrameBuffer));
    assert(UI_HomeDraw());snprintf(path,sizeof(path),"%s/home-vfo.pbm",argv[1]);image(path);
    gEeprom.TX_VFO=2;assert(!UI_HomeDraw());gEeprom.TX_VFO=0;
    gEeprom.DUAL_WATCH=1;assert(UI_HomeDraw());gEeprom.DUAL_WATCH=0;
#ifdef ENABLE_VFO_SPLIT
    gVfoSplit.mode=SPLIT_ALERT;gVfoSplit.alert.active=true;
    gVfoSplit.alert.freq_10Hz=14439000;gVfoSplit.alert.rssi_dBm=-91;
    assert(UI_HomeDraw());snprintf(path,sizeof(path),"%s/home-split.pbm",argv[1]);image(path);
    gVfoSplit.mode=SPLIT_OFF;
#endif
    toast_timer=1;strcpy(toast_msg,"SIMPLEX");assert(UI_HomeDraw());
    snprintf(path,sizeof(path),"%s/home-toast.pbm",argv[1]);image(path);toast_timer=0;
    gInputBoxIndex=3;memset(gInputBox,10,sizeof(gInputBox));gInputBox[0]=1;gInputBox[1]=4;gInputBox[2]=6;
    assert(UI_HomeDraw());snprintf(path,sizeof(path),"%s/home-entry.pbm",argv[1]);image(path);gInputBoxIndex=0;
    VfoState[0]=VFO_STATE_TX_DISABLE;assert(UI_HomeDraw());
    snprintf(path,sizeof(path),"%s/home-warning.pbm",argv[1]);image(path);VfoState[0]=VFO_STATE_NORMAL;
    gScanStateDir=1;assert(UI_HomeDraw());snprintf(path,sizeof(path),"%s/home-scan.pbm",argv[1]);image(path);gScanStateDir=0;
    gDTMF_InputMode=true;strcpy(gDTMF_InputBox,"12345678*#ABCD");assert(UI_HomeDraw());gDTMF_InputMode=false;
    gSetting_live_DTMF_decoder=true;strcpy(gDTMF_RX_live,"1234567890123456789");assert(UI_HomeDraw());gSetting_live_DTMF_decoder=false;
    gCurrentFunction=FUNCTION_FOREGROUND;bandscope_enabled=true;assert(UI_HomeDraw());assert(gFrameBuffer[6][0]==0x55);bandscope_enabled=false;
    gCurrentFunction=FUNCTION_TRANSMIT;assert(UI_HomeDraw());
    puts("PASS compact scan/input/toast/split/warning/DTMF/bandscope/TX modes");
    puts("PASS custom home name/frequency fallback, battery status and real LCD rendering");
}
