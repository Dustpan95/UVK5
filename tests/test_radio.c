/* SPDX-License-Identifier: GPL-3.0-only
 * Host tests of actual firmware modules against a simulated BK4819 register file.
 * Does not model RF demodulation, audio quality, or MCU timing.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/aprs_rx.h"
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
#include "capture.h"
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
#ifdef ENABLE_TONE_SEARCH
bool SCANNER_IsScanning(void) { return scanning; }
#endif
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
    FONT_Init();
    for(unsigned i=0;i<128;i++) regs[i]=0x100+i;
    uint16_t before[128];memcpy(before,regs,sizeof(regs));
    gRxVfo->Modulation=MODULATION_FM;
    gRxVfo->pRX=&gRxVfo->freq_config_RX;gRxVfo->pRX->Frequency=14439000;
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
    gAPRS_BannerTicks=3;
    unsigned resume_beeps=beeps;
#ifdef ENABLE_APRS_SESSION
    uint32_t resume_age=gAPRS_SessionTicks;
#endif
    APRS_Resume();assert(regs[0x59]==0x1000 && !APRS_IsCapturing());
    assert(gAPRS_BannerTicks==3 && beeps==resume_beeps);
#ifdef ENABLE_APRS_SESSION
    assert(gAPRS_SessionTicks==resume_age);
#endif
    stopped=writes;APRS_Resume();assert(writes==stopped);
    APRS_Stop();gAPRS_Enabled=false;stopped=writes;
    APRS_Resume();assert(writes==stopped);
    gAPRS_Enabled=true;gAPRS_BannerTicks=0;
    APRS_Tick500ms();gCurrentFunction=FUNCTION_TRANSMIT;
    APRS_Interrupt(0x3002);assert(regs[0x59]==0);
    stopped=writes;APRS_Tick500ms();assert(writes==stopped);
    APRS_Resume();assert(writes==stopped);
    gCurrentFunction=FUNCTION_POWER_SAVE;assert(!APRS_CanListen());
    gCurrentFunction=FUNCTION_FOREGROUND;
    gScanStateDir=1;assert(!APRS_CanListen());gScanStateDir=0;
#ifdef ENABLE_TONE_SEARCH
    scanning=true;assert(!APRS_CanListen());scanning=false;
#else
    gScanStateDir=SCAN_FWD;assert(!APRS_CanListen());gScanStateDir=SCAN_OFF;
#endif
    gCssBackgroundScan=true;assert(!APRS_CanListen());gCssBackgroundScan=false;
    gEeprom.DUAL_WATCH=1;assert(!APRS_CanListen());gEeprom.DUAL_WATCH=0;
    gEeprom.CROSS_BAND_RX_TX=1;assert(!APRS_CanListen());gEeprom.CROSS_BAND_RX_TX=0;
#ifdef ENABLE_VFO_SPLIT
    gVfoSplit.mode=SPLIT_ALERT;assert(!APRS_CanListen());gVfoSplit.mode=SPLIT_OFF;
#endif
    gSerialConfigCountDown_500ms=1;assert(!APRS_CanListen());gSerialConfigCountDown_500ms=0;
    gRxVfo->Modulation=MODULATION_AM;assert(!APRS_CanListen());gRxVfo->Modulation=MODULATION_FM;
    puts("PASS RX register setup/restore, TX/scan/dual-watch/sleep/serial/mode exclusion");

    for (unsigned round=0;round<6;++round) {
        APRS_Tick500ms();
        assert(!APRS_IsCapturing());
        fifo=0;fifo_active=true;
        APRS_Interrupt(BK4819_REG_02_FSK_RX_SYNC);
        assert(APRS_IsCapturing());
        while(fifo<224) APRS_Interrupt(BK4819_REG_02_FSK_FIFO_ALMOST_FULL);
        APRS_Interrupt(BK4819_REG_02_FSK_RX_FINISHED);
        assert(!APRS_IsCapturing());
        fifo_active=false;
        assert(gAPRS_Log[0].kind==APRS_POSITION);
        assert(gAPRS_Log[0].latitude==49058333);
#ifdef ENABLE_APRS_MESSAGING
        assert(gAPRS_Log[0].frequency==14439000);
#endif
        assert(gAPRS_Count==(round<4 ? round+1 : 4));
    }
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    APRS_RxDiagnostics d;
    unsigned diagnostic_writes=writes;APRS_GetDiagnostics(&d);
    assert(writes==diagnostic_writes && d.ready && d.armed && !d.capturing);
    assert(d.sync==6 && d.fifo==168 && d.end==6 && d.good==6 && !d.bad);
    assert(d.lastBytes==240 && d.full==6 && d.frames==6 && d.crc==6 && d.flags>=6);
    assert(d.reg58==0x3fc3 && d.reg59==0x1000 && !memcmp(d.prefix,capture_fixture,8));
    uint8_t corrupt[sizeof(capture_fixture)];memcpy(corrupt,capture_fixture,sizeof(corrupt));corrupt[10]^=1;
    fifo_capture=corrupt;fifo=0;fifo_active=true;
    APRS_Interrupt(BK4819_REG_02_FSK_RX_SYNC);
    while(fifo<224) APRS_Interrupt(BK4819_REG_02_FSK_FIFO_ALMOST_FULL);
    APRS_Interrupt(BK4819_REG_02_FSK_RX_FINISHED);fifo_active=false;
    APRS_GetDiagnostics(&d);assert(d.good==6 && d.bad==1 && d.frames==7 && d.crc==6);
    fifo_capture=capture_fixture;fifo=0;fifo_active=true;
    APRS_Interrupt(BK4819_REG_02_FSK_RX_SYNC);
    APRS_Interrupt(BK4819_REG_02_FSK_FIFO_ALMOST_FULL);
    for(unsigned i=0;i<3;i++) APRS_Tick500ms();
    fifo_active=false;APRS_GetDiagnostics(&d);
    assert(d.timeout==1 && d.bad==2 && d.lastBytes==24 && d.armed && !d.capturing);
    APRS_Open();assert(gAPRS_DiagnosticPage==0);
    char diagnostic_path[256];
    for(unsigned page=0;page<3;page++) {
        assert(gAPRS_DiagnosticPage==page);
        uint8_t expected_title[128];
        if(page<2) {UI_DisplayClear();UI_PrintLine(page==0 ? "APRS RX DIAG D1":"APRS REG / RAW",0);memcpy(expected_title,gFrameBuffer[0],128);}
        diagnostic_writes=writes;APRS_Draw();assert(writes==diagnostic_writes);
        if(page<2) assert(!memcmp(expected_title,gFrameBuffer[0],128));
        snprintf(diagnostic_path,sizeof(diagnostic_path),"%s/diagnostics-%u.pbm",argv[1],page);image(diagnostic_path);
        assert(APRS_Key(KEY_MENU,true,false));
    }
    assert(!gAPRS_DiagnosticPage);
    assert(APRS_Key(KEY_EXIT,true,false) && !gAPRS_Browser);
    gSerialConfigCountDown_500ms=1;APRS_Tick500ms();APRS_GetDiagnostics(&d);assert(!d.ready && !d.armed);
    gSerialConfigCountDown_500ms=0;APRS_Resume();
    puts("PASS receive diagnostics: real FIFO/decoder counters, CRC rejection, timeout, live gates, read-only register snapshots and three UI pages");
#endif
    APRS_Stop();
    puts("PASS simulated FSK FIFO -> HDLC -> position -> bounded four-packet history");

    assert(gAPRS_AlertSound);
#ifdef ENABLE_APRS_SESSION
    assert(!gAPRS_Profile.callsign[0] && !gAPRS_InboxCount);
    assert(SESSION_SetCall("MYCALL"));
#endif
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
#ifdef ENABLE_APRS_SESSION
        assert(gAPRS_InboxCount==1); /* Repeated message ID is one inbox entry. */
#endif
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

#ifdef ENABLE_APRS_SESSION
    assert(SESSION_SetCall("OTHER"));
    APRS_Tick500ms();fifo=0;fifo_active=true;
    APRS_Interrupt(BK4819_REG_02_FSK_RX_SYNC);
    while(fifo<224) APRS_Interrupt(BK4819_REG_02_FSK_FIFO_ALMOST_FULL);
    APRS_Interrupt(BK4819_REG_02_FSK_RX_FINISHED);fifo_active=false;
    assert(!gAPRS_InboxCount && gAPRS_BannerTicks==10);
    unsigned previous_beeps=beeps;APRS_Tick500ms();assert(beeps==previous_beeps+1);
    assert(SESSION_SetCall("MYCALL"));
    puts("PASS other-station traffic still triggers banner/sound without entering personal inbox");
#endif

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
#ifdef ENABLE_APRS_SESSION
    assert(gAPRS_View==APRS_HUB);
    APRS_Key(KEY_DOWN,true,false);APRS_Key(KEY_MENU,true,false);
    assert(gAPRS_View==APRS_MONITOR);
#endif
    assert(APRS_Key(KEY_DOWN,true,false) && gAPRS_Selected==1);
    assert(APRS_Key(KEY_UP,true,false) && gAPRS_Selected==0);
#ifdef ENABLE_APRS_SESSION
    assert(APRS_Key(KEY_EXIT,true,false) && gAPRS_View==APRS_HUB);
#endif
    assert(APRS_Key(KEY_EXIT,true,false) && !gAPRS_Browser);
    APRS_Open();assert(!APRS_Key(KEY_PTT,true,false) && !gAPRS_Browser);
    APRS_Open();
#ifdef ENABLE_APRS_SESSION
    gAPRS_View=APRS_MONITOR;
#endif
    gAPRS_Log[0].kind=APRS_POSITION;
    strcpy(gAPRS_Log[0].source,"N0CALL-15");
    gAPRS_Log[0].latitude=49058333;gAPRS_Log[0].longitude=-72029166;
    UI_HomeStatus();APRS_Draw();assert(blits>0);
    char path[256];snprintf(path,sizeof(path),"%s/aprs-position.pbm",argv[1]);image(path);
    gAPRS_Log[0].kind=APRS_MESSAGE;strcpy(gAPRS_Log[0].destination,"DEST-12");
    strcpy(gAPRS_Log[0].text,"Hello from the APRS network. This entire message fits on the LCD.");
    APRS_Draw();snprintf(path,sizeof(path),"%s/aprs-message.pbm",argv[1]);image(path);
    memset(gAPRS_Log[0].text,'W',APRS_TEXT_MAX);gAPRS_Log[0].text[APRS_TEXT_MAX]=0;
    APRS_Draw();
#ifdef ENABLE_APRS_SESSION
    APRS_Open();gAPRS_View=APRS_MONITOR;
#ifndef ENABLE_APRS_CALLSIGN_ONLY
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
#else
    APRS_Key(KEY_MENU,true,false);
    assert(gAPRS_Message.page==MESSAGE_CLOSED && APRS_HUB_ITEMS==4);
#endif
    assert(!SESSION_SetCall("BAD-CALL") && !strcmp(gAPRS_Profile.callsign,"MYCALL"));
    unsigned monitor_count=gAPRS_Count;
    assert(SESSION_SetCall("NEWCAL-0") && !strcmp(gAPRS_Profile.callsign,"NEWCAL"));
    assert(!gAPRS_InboxCount && !gAPRS_Unread && gAPRS_Count==monitor_count);
    APRS_Record personal={.kind=APRS_MESSAGE};
    strcpy(personal.source,"N0CALL-7");strcpy(personal.destination,"NEWCAL-0");
    strcpy(personal.text,"PERSONAL MESSAGE");
    assert(SESSION_IsPersonal(&personal));
    for(unsigned i=0;i<6;i++) SESSION_Receive(&personal);
    assert(gAPRS_InboxCount==4 && gAPRS_Unread==15);
    APRS_Record unrelated=personal;strcpy(unrelated.destination,"OTHER-7");
    assert(!SESSION_IsPersonal(&unrelated));
    for(unsigned i=0;i<20;i++) SESSION_Receive(&unrelated);
    assert(gAPRS_InboxCount==4 && !strcmp(gAPRS_Inbox[0].text,"PERSONAL MESSAGE"));
    SESSION_Open();SESSION_Key(KEY_MENU);assert(gAPRS_View==APRS_PERSONAL);
    SESSION_Draw();snprintf(path,sizeof(path),"%s/personal-inbox.pbm",argv[1]);image(path);
    SESSION_Key(KEY_MENU);assert(gAPRS_View==APRS_PERSONAL_DETAIL && gAPRS_Unread==14);
    SESSION_Draw();snprintf(path,sizeof(path),"%s/personal-message.pbm",argv[1]);image(path);
    SESSION_Receive(&personal);assert(gAPRS_InboxSelected==1 && !(gAPRS_Unread&2));
    assert(gAPRS_View==APRS_PERSONAL_DETAIL);
    gAPRS_InboxSelected=3;SESSION_Receive(&personal);assert(gAPRS_View==APRS_PERSONAL);
    SESSION_Key(KEY_EXIT);assert(gAPRS_View==APRS_HUB);
    MESSAGE_EditCall();strcpy(gAPRS_Message.editCall,"BAD-99");MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.error==MESSAGE_BAD_CALL && !strcmp(gAPRS_Profile.callsign,"NEWCAL"));
    MESSAGE_Draw(); /* Actual error/callsign renderer in both receive/full profiles. */
    strcpy(gAPRS_Message.editCall,"OTHER-7");MESSAGE_Key(KEY_EXIT);
    assert(!strcmp(gAPRS_Profile.callsign,"NEWCAL")); /* Cancel doesn't reclassify. */
    MESSAGE_EditCall();strcpy(gAPRS_Message.editCall,"OTHER-7");MESSAGE_Key(KEY_MENU);
    assert(!strcmp(gAPRS_Profile.callsign,"OTHER-7") && !gAPRS_InboxCount);
    assert(gAPRS_Message.page==MESSAGE_CLOSED);
    assert(SESSION_SetPosition(-90000000,180000000) && gAPRS_Profile.positionSet);
    assert(!SESSION_SetPosition(90000001,0) && gAPRS_Profile.latitude==-90000000);
    assert(SESSION_SetQuick(0,"BACK AT BASE") && !strcmp(gAPRS_Quick[0],"BACK AT BASE"));
    assert(!SESSION_SetQuick(3,"BAD") && !SESSION_SetQuick(0,"BAD{TEXT"));
    assert(!strcmp(gAPRS_Quick[0],"BACK AT BASE"));
    char long_quick[69];memset(long_quick,'W',68);long_quick[68]=0;
    assert(!SESSION_SetQuick(0,long_quick));long_quick[67]=0;
    assert(SESSION_SetQuick(0,long_quick));
#ifndef ENABLE_APRS_CALLSIGN_ONLY
    gAPRS_Message.page=MESSAGE_QUICK;gAPRS_Message.selected=0;
    MESSAGE_Draw(); /* Full-length saved text must not overflow its list preview. */
    gAPRS_Message.text[0]=0;MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.page==MESSAGE_COMPOSE && strlen(gAPRS_Message.text)==67);
    gAPRS_Message.page=MESSAGE_CLOSED;
#endif
    memset(&gAPRS_Profile,0,sizeof(gAPRS_Profile)); /* Simulate reboot BSS clearing. */
    memset(gAPRS_Inbox,0,sizeof(gAPRS_Inbox));gAPRS_InboxCount=gAPRS_Unread=0;
    SESSION_Open();SESSION_Key(KEY_MENU);SESSION_Draw();
    snprintf(path,sizeof(path),"%s/personal-unconfigured.pbm",argv[1]);image(path);
    assert(!SESSION_IsPersonal(&personal));
    SESSION_Key(KEY_MENU);assert(gAPRS_Message.page==MESSAGE_CALLSIGN);
    MESSAGE_Key(KEY_EXIT);
    APRS_Open();APRS_Key(KEY_MENU,true,false);APRS_Key(KEY_MENU,true,false);
    assert(gAPRS_Message.page==MESSAGE_CALLSIGN);
    const KEY_Code_t call_keys[]={KEY_6,KEY_F,KEY_9,KEY_9,KEY_9,KEY_F,
        KEY_2,KEY_2,KEY_2,KEY_F,KEY_2,KEY_F,KEY_5,KEY_5,KEY_5,KEY_F,KEY_5,KEY_5,KEY_5};
    unsigned config_writes=writes;
    for(unsigned i=0;i<sizeof(call_keys)/sizeof(*call_keys);i++) APRS_Key(call_keys[i],true,false);
    APRS_Draw();snprintf(path,sizeof(path),"%s/own-callsign.pbm",argv[1]);image(path);
    APRS_Key(KEY_MENU,true,false);
    assert(!strcmp(gAPRS_Profile.callsign,"MYCALL") && gAPRS_Message.page==MESSAGE_CLOSED);
    assert(writes==config_writes);

    APRS_Record station_packet={.kind=APRS_POSITION,.latitude=49058333,.longitude=-72029166};
    strcpy(station_packet.source,"N0STN");
    SESSION_Receive(&station_packet);unsigned station_count=gAPRS_StationCount;
    assert(gAPRS_Stations[0].hasPosition && !strcmp(gAPRS_Stations[0].callsign,"N0STN"));
    SESSION_Tick500ms();station_packet.kind=APRS_OTHER;
    SESSION_Receive(&station_packet);assert(gAPRS_StationCount==station_count && gAPRS_Stations[0].hasPosition);
    assert(gAPRS_Stations[0].latitude==49058333 && gAPRS_Stations[0].heardTick==gAPRS_SessionTicks);
    gAPRS_View=APRS_STATION_DETAIL;gAPRS_StationSelected=0;
    UI_HomeStatus();SESSION_Draw();snprintf(path,sizeof(path),"%s/station-detail.pbm",argv[1]);image(path);
    strcpy(station_packet.source,"N1STN");SESSION_Receive(&station_packet);
    assert(gAPRS_StationSelected==1 && !strcmp(gAPRS_Stations[gAPRS_StationSelected].callsign,"N0STN"));
    for(unsigned i=2;i<11;i++) {sprintf(station_packet.source,"N%uSTN",i);SESSION_Receive(&station_packet);}
    assert(gAPRS_StationCount==8 && gAPRS_View==APRS_STATIONS);
    gAPRS_StationSelected=7;SESSION_Draw();snprintf(path,sizeof(path),"%s/station-directory.pbm",argv[1]);image(path);
    SESSION_Key(KEY_DOWN);assert(gAPRS_StationSelected==7);
    SESSION_Key(KEY_MENU);assert(gAPRS_View==APRS_STATION_DETAIL);
    SESSION_Key(KEY_EXIT);assert(gAPRS_View==APRS_STATIONS);
#ifndef ENABLE_APRS_CALLSIGN_ONLY
    strcpy(gAPRS_Message.recipient,"N0ACK");strcpy(gAPRS_Message.text,"PING");
    assert(MESSAGE_MarkSent("MYCALL","01"));
    APRS_Record ack={.kind=APRS_ACK};strcpy(ack.source,"N0WRNG");strcpy(ack.destination,"MYCALL");strcpy(ack.messageId,"01");
    unsigned before_ack=gAPRS_InboxCount;
    SESSION_Receive(&ack);assert(gAPRS_Message.delivery==MESSAGE_WAIT_ACK);
    strcpy(ack.source,"N0ACK");SESSION_Receive(&ack);
    assert(gAPRS_Message.delivery==MESSAGE_ACKED && gAPRS_InboxCount==before_ack);
    gAPRS_Message.page=MESSAGE_CLOSED;
    puts("PASS received ACK routing/correlation without inbox pollution");
#endif
    puts("PASS station deduplication, retained positions, last-heard age, detail stability and bounded eviction");
    puts("PASS session profile validation, personal filtering/unread/eviction, monitor isolation, callsign editing");
    gAPRS_Message.page=MESSAGE_CLOSED;
#ifndef ENABLE_APRS_CALLSIGN_ONLY
    puts("PASS integrated editor keys/held-repeat/PTT, draft preservation, candidate LCD bounds");
#else
    APRS_Open();gAPRS_HubSelected=APRS_HUB_ITEMS-1;APRS_Key(KEY_MENU,true,false);
    assert(gAPRS_Message.page==MESSAGE_CALLSIGN);
    assert(!APRS_Key(KEY_PTT,true,false) && !gAPRS_Browser && gAPRS_Message.page==MESSAGE_CLOSED);
    puts("PASS receive-session hub, callsign editor/PTT and monitor without unfinished compose controls");
#endif
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
    puts("PASS browser navigation/PTT passthrough, real framebuffer rendering, UI fallback");
}
