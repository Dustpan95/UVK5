/* SPDX-License-Identifier: GPL-3.0-only
 * Host tests of actual firmware modules against a simulated BK4819 register file.
 * Does not model RF demodulation, audio quality, or MCU timing.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/aprs_rx.h"
#include "audio.h"
#ifdef ENABLE_APRS_MESSAGING
#include "app/aprs_message.h"
#endif
#include "app/chFrScanner.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "functions.h"
#include "settings.h"
#include "misc.h"
#include "ui/ui.h"
#include "ui/home.h"
#include "ui/main.h"
#include "vfo_split.h"
#include "tx_compressor.h"
#include "toast.h"
#include "smart_squelch.h"
#include "rssi_histogram.h"
#include "capture.h"
#include "dtmf.h"
#include "helper/battery.h"

EEPROM_Config_t gEeprom;
VFO_Info_t *gRxVfo = &gEeprom.VfoInfo[0], *gCurrentVfo = &gEeprom.VfoInfo[0];
FUNCTION_Type_t gCurrentFunction;
VfoSplit_t gVfoSplit;
VfoState_t VfoState[2];
int8_t gScanStateDir;
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
static bool scanning;
static uint16_t regs[128];
static unsigned writes, blits, fifo;
static bool fifo_active;
static const uint8_t *fifo_capture = capture_fixture;
static unsigned beeps;
void AUDIO_PlayBeep(BEEP_Type_t beep) {
    assert(beep==BEEP_1KHZ_60MS_OPTIONAL);
    ++beeps; APRS_Stop(); /* The real audio path releases the shared modem. */
}
RSSI_Histogram_t gRSSI_Histogram[2];
uint16_t BK4819_GetRSSI(void) { return 220; }
uint8_t BK4819_GetExNoiceIndicator(void) { return 12; }
uint8_t BK4819_GetGlitchIndicator(void) { return 10; }
bool SCANNER_IsScanning(void) { return scanning; }
bool FUNCTION_IsRx(void) { return gCurrentFunction==FUNCTION_RECEIVE || gCurrentFunction==FUNCTION_MONITOR || gCurrentFunction==FUNCTION_INCOMING; }
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t reg) { assert(reg<128);
    if (reg==0x5f && fifo_active) {
        assert(fifo+1<sizeof(capture_fixture));
        uint16_t value=fifo_capture[fifo] | fifo_capture[fifo+1]<<8;
        fifo+=2;return value;
    }
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
    assert(argc==2);
    for(unsigned i=0;i<128;i++) regs[i]=0x100+i;
    uint16_t before[128];memcpy(before,regs,sizeof(regs));
    gRxVfo->Modulation=MODULATION_FM;
    gCurrentFunction=FUNCTION_FOREGROUND;
    APRS_Tick500ms(); assert(writes==0);
    gAPRS_Enabled=true;
    assert(APRS_CanListen());
    APRS_Tick500ms();assert(regs[0x59]==0x1000);
    assert((regs[0x3f]&0x3002)==0x3002);
    assert(regs[0x71]==22714 && regs[0x72]==12389);
    APRS_Stop();assert(regs[0x59]==0);
    const uint8_t changed[]={0x70,0x71,0x72,0x58,0x5a,0x5c,0x5d,0x5e,0x3f};
    for(unsigned i=0;i<sizeof(changed);i++) assert(regs[changed[i]]==before[changed[i]]);
    unsigned stopped=writes;APRS_Stop();assert(writes==stopped);
    APRS_Tick500ms();gCurrentFunction=FUNCTION_TRANSMIT;
    APRS_Interrupt(0x3002);assert(regs[0x59]==0);
    stopped=writes;APRS_Tick500ms();assert(writes==stopped);
    gCurrentFunction=FUNCTION_POWER_SAVE;assert(!APRS_CanListen());
    gCurrentFunction=FUNCTION_FOREGROUND;
    gScanStateDir=1;assert(!APRS_CanListen());gScanStateDir=0;
    scanning=true;assert(!APRS_CanListen());scanning=false;
    gCssBackgroundScan=true;assert(!APRS_CanListen());gCssBackgroundScan=false;
    gEeprom.DUAL_WATCH=1;assert(!APRS_CanListen());gEeprom.DUAL_WATCH=0;
    gEeprom.CROSS_BAND_RX_TX=1;assert(!APRS_CanListen());gEeprom.CROSS_BAND_RX_TX=0;
    gVfoSplit.mode=SPLIT_ALERT;assert(!APRS_CanListen());gVfoSplit.mode=SPLIT_OFF;
    gSerialConfigCountDown_500ms=1;assert(!APRS_CanListen());gSerialConfigCountDown_500ms=0;
    gRxVfo->Modulation=MODULATION_AM;assert(!APRS_CanListen());gRxVfo->Modulation=MODULATION_FM;
    puts("PASS RX register setup/restore, TX/scan/dual-watch/sleep/serial/mode exclusion");

    for (unsigned round=0;round<6;++round) {
        APRS_Tick500ms();
        fifo=0;fifo_active=true;
        APRS_Interrupt(BK4819_REG_02_FSK_RX_SYNC);
        while(fifo<224) APRS_Interrupt(BK4819_REG_02_FSK_FIFO_ALMOST_FULL);
        APRS_Interrupt(BK4819_REG_02_FSK_RX_FINISHED);
        fifo_active=false;
        assert(gAPRS_Log[0].kind==APRS_POSITION);
        assert(gAPRS_Log[0].latitude==49058333);
        assert(gAPRS_Count==(round<4 ? round+1 : 4));
    }
    APRS_Stop();
    puts("PASS simulated FSK FIFO -> HDLC -> position -> bounded four-packet history");

    assert(gAPRS_AlertSound);
    fifo_capture=message_fixture;
    for(unsigned round=0;round<3;round++) {
        gAPRS_AlertSound=round!=1;
        APRS_Tick500ms();
        fifo=0;fifo_active=true;
        APRS_Interrupt(BK4819_REG_02_FSK_RX_SYNC);
        while(fifo<224) APRS_Interrupt(BK4819_REG_02_FSK_FIFO_ALMOST_FULL);
        APRS_Interrupt(BK4819_REG_02_FSK_RX_FINISHED);
        fifo_active=false;
        assert(gAPRS_Log[0].kind==APRS_MESSAGE);
        assert(gAPRS_BannerTicks==10 && !gAPRS_Browser);
        assert(!strcmp(gAPRS_BannerSource,"N0CALL-15"));
        assert(!strncmp(gAPRS_BannerText,"MEET AT TRAILHEAD",16));
        unsigned previous=beeps;
        if(round==2) assert(!APRS_Key(KEY_PTT,true,false));
        APRS_Tick500ms();
        assert(beeps==previous+(round==0));
        assert(regs[0x59]==0x1000); /* RX rearmed after shared-tone alert. */
        gAPRS_Enabled=false;
        for(unsigned i=0;i<10;i++) APRS_Tick500ms();
        assert(!gAPRS_BannerTicks);
        gAPRS_Enabled=true;
    }
    puts("PASS message banner, sound default/mute, PTT cancellation, expiry and RX rearm");

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

    APRS_Open();assert(gAPRS_Browser && gRequestDisplayScreen==DISPLAY_MAIN);
    gAPRS_Count=4;
    assert(APRS_Key(KEY_DOWN,true,false) && gAPRS_Selected==1);
    assert(APRS_Key(KEY_UP,true,false) && gAPRS_Selected==0);
    assert(APRS_Key(KEY_EXIT,true,false) && !gAPRS_Browser);
    APRS_Open();assert(!APRS_Key(KEY_PTT,true,false) && !gAPRS_Browser);
    APRS_Open();
    gAPRS_Log[0].kind=APRS_POSITION;
    strcpy(gAPRS_Log[0].source,"N0CALL-15");
    gAPRS_Log[0].latitude=49058333;gAPRS_Log[0].longitude=-72029166;
    APRS_Draw();assert(blits>0);
    char path[256];snprintf(path,sizeof(path),"%s/aprs-position.pbm",argv[1]);image(path);
    gAPRS_Log[0].kind=APRS_MESSAGE;strcpy(gAPRS_Log[0].destination,"DEST-12");
    strcpy(gAPRS_Log[0].text,"Hello from the APRS network. This entire message fits on the LCD.");
    APRS_Draw();snprintf(path,sizeof(path),"%s/aprs-message.pbm",argv[1]);image(path);
    memset(gAPRS_Log[0].text,'W',APRS_TEXT_MAX);gAPRS_Log[0].text[APRS_TEXT_MAX]=0;
    APRS_Draw();
#ifdef ENABLE_APRS_MESSAGING
    APRS_Open();
    assert(APRS_Key(KEY_MENU,true,false));
    assert(gAPRS_Message.page==MESSAGE_COMPOSE);
    assert(!strcmp(gAPRS_Message.recipient,"N0CALL-15"));
    assert(APRS_Key(KEY_2,true,false));
    assert(APRS_Key(KEY_2,true,true)); /* Held repeat must not cycle the letter. */
    assert(!strcmp(gAPRS_Message.text,"A"));
    APRS_Draw();snprintf(path,sizeof(path),"%s/message-compose.pbm",argv[1]);image(path);
    assert(!APRS_Key(KEY_PTT,true,false) && !gAPRS_Browser);
    assert(gAPRS_Message.page==MESSAGE_CLOSED && !strcmp(gAPRS_Message.text,"A"));
    MESSAGE_Open("N0CALL-7");strcpy(gAPRS_Message.text,"MEET AT TRAILHEAD AT 14:30");
    gAPRS_Message.page=MESSAGE_REVIEW;MESSAGE_Draw();
    snprintf(path,sizeof(path),"%s/message-review.pbm",argv[1]);image(path);
    MESSAGE_Key(KEY_MENU);MESSAGE_Draw();
    assert(gAPRS_Message.delivery==MESSAGE_TX_UNAVAILABLE);
    snprintf(path,sizeof(path),"%s/message-blocked.pbm",argv[1]);image(path);
    memset(gAPRS_Message.text,'W',67);gAPRS_Message.text[67]=0;
    for(unsigned page=MESSAGE_RECIPIENT;page<=MESSAGE_STATUS;page++) {
        gAPRS_Message.page=page;
        for(unsigned selected=0;selected<3;selected++) {
            gAPRS_Message.selected=selected; MESSAGE_Draw();
        }
    }
    gAPRS_Message.page=MESSAGE_CLOSED;
    puts("PASS integrated editor keys/held-repeat/PTT, draft preservation, candidate LCD bounds");
#endif
    gAPRS_Browser=false;
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
    gVfoSplit.mode=SPLIT_ALERT;assert(!UI_HomeDraw());gVfoSplit.mode=SPLIT_OFF;
    toast_timer=1;assert(!UI_HomeDraw());toast_timer=0;
    gInputBoxIndex=1;assert(!UI_HomeDraw());gInputBoxIndex=0;
    VfoState[0]=VFO_STATE_TX_DISABLE;assert(!UI_HomeDraw());
    puts("PASS browser navigation/PTT passthrough, real framebuffer rendering, UI fallback");
}
