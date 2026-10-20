/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

/* Modified for UVK5: user-approved custom home layout. The prior prototype
 * adapted F4HWN's channel-name/frequency presentation; that heritage and
 * original copyright notices are retained. See docs/design/DECISIONS.md.
 */
#include "ui/home.h"
#include "app/aprs_rx.h"
#include "app/chFrScanner.h"
#include "app/scanner.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "font.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "dtmf.h"
#include "toast.h"
#include "vfo_split.h"
#include "scan_rate.h"
#include "signal_quality.h"
#include "signal_classifier.h"
#include "bandscope.h"
#include <string.h>

/* Coordinates include the eight-pixel status line. */
static void Pixel(unsigned x, unsigned y, bool ink)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    uint8_t *p = y < 8 ? &gStatusLine[x] : &gFrameBuffer[y / 8 - 1][x];
    const uint8_t mask = 1u << (y & 7);
    if (ink) *p |= mask; else *p &= ~mask;
}

static void Rect(unsigned x, unsigned y, unsigned w, unsigned h, bool ink)
{
    for (unsigned row = y; row < y + h && row < LCD_HEIGHT; ++row)
        for (unsigned col = x; col < x + w && col < LCD_WIDTH; ++col)
            Pixel(col, row, ink);
}

/* Existing font tables: mini 3x5, small 6x7, normal 7x16. No new glyph set. */
static void Text(const char *s, unsigned x, unsigned y, unsigned size, bool inverse)
{
    const unsigned width = size == 0 ? 3 : size == 1 ? 6 : 7;
    for (; *s && x + width <= LCD_WIDTH; ++s, x += width + 1) {
        unsigned c = (uint8_t)*s;
        if (c < 33 || c > 126) continue;
        const uint8_t *glyph = size == 0 ? gFont3x5[c - 32] :
                               size == 1 ? gFontSmall[c - 33] : gFontBig[c - 33];
        const unsigned count = size == 2 ? 14 : width;
        for (unsigned i = 0; i < count; ++i)
            for (unsigned bit = 0; bit < 8; ++bit)
                if ((glyph[i] >> bit) & 1)
                    Pixel(x + i % width, y + i / width * 8 + bit, !inverse);
    }
}

static void Center(const char *s, unsigned y, unsigned size)
{
    const unsigned advance = size == 0 ? 4 : size == 1 ? 7 : 8;
    unsigned width = strlen(s) * advance;
    Text(s, width < LCD_WIDTH ? (LCD_WIDTH - width + 1) / 2 : 0, y, size, false);
}

static void Frequency(uint32_t f, unsigned y)
{
    char s[12];
    sprintf(s, "%3u.%05u", (unsigned)(f / 100000), (unsigned)(f % 100000));
    if (f >= 100000000) { Center(s, y, 2); return; }
    unsigned x = 23;
    for (unsigned n = 0; n < 7; ++n) {
        const unsigned c = s[n];
        if (c == '.') { Rect(x, y + 13, 3, 2, true); x += 3; continue; }
        if (c >= '0' && c <= '9') {
            const uint8_t *glyph = gFontBigDigits[c - '0'];
            for (unsigned i = 0; i < 20; ++i)
                for (unsigned bit = 0; bit < 8; ++bit)
                    if ((glyph[i] >> bit) & 1)
                        Pixel(x + 2 + i % 10, y + i / 10 * 8 + bit, true);
        }
        x += 13;
    }
    Text(s + 7, 109, y + 8, 1, false);
}

#ifdef ENABLE_APRS_TRANSPORT
#include "app/aprs_runtime.h"
static const VFO_Info_t *HomeVfo(unsigned index) {return RUNTIME_VoiceVfo(index);}
#else
static const VFO_Info_t *HomeVfo(unsigned index) {return &gEeprom.VfoInfo[index];}
#endif

static unsigned Primary(void)
{
    return gCurrentFunction==FUNCTION_TRANSMIT || gInputBoxIndex || gDTMF_InputMode || !gRxVfoIsActive ?
           gEeprom.TX_VFO : gEeprom.RX_VFO;
}

bool UI_HomeEligible(void)
{
    return gEeprom.TX_VFO<2 && gEeprom.RX_VFO<2;
}

static void Banner(const char *title, const char *body)
{
    Rect(0,47,128,17,false);Rect(0,47,128,1,true);
    Text(title,2,49,0,false);Text(body,2,57,1,false);
}

static void Input(bool memory)
{
    char ascii[9],s[16];
    for(unsigned i=0;i<8;i++) ascii[i]=gInputBox[i]<10 ? '0'+gInputBox[i] : '-';
    ascii[8]=0;
    Rect(0,16,128,31,false);
    if(memory) sprintf(s,"CH-%.3s",ascii);
    else {
        const unsigned digits=HomeVfo(gEeprom.TX_VFO)->pRX->Frequency>=100000000 ? 4 : 3;
        sprintf(s,"%.*s.%s",digits,ascii,ascii+digits);
    }
    Center(s,20,2);Center(memory ? "CHANNEL ENTRY" : "FREQUENCY ENTRY",40,0);
    Rect(0,57,128,7,true);Text("EXIT:DELETE  HOLD EXIT:CANCEL",8,58,0,true);
}

static void Overlays(unsigned primary)
{
    char s[32],frequency[12];
    const VfoState_t state=VfoState[primary];
    static const char *const warnings[]={"","BUSY","BAT LOW","TX DISABLE","TIMEOUT","ALARM","VOLT HIGH"};
    if(state!=VFO_STATE_NORMAL) {
        Banner("RADIO ALERT",state<sizeof(warnings)/sizeof(*warnings) ? warnings[state] : "CHECK RADIO");
    } else if(gDTMF_InputMode) {
        Banner("DTMF ENTRY",gDTMF_InputBox);
    } else if(toast_timer) {
        Banner("RADIO",toast_msg);
    } else if(gVfoSplit.mode!=SPLIT_OFF) {
        const bool alert=gVfoSplit.alert.active;
        const uint32_t f=alert ? gVfoSplit.alert.freq_10Hz : gVfoSplit.current_freq_10Hz;
        Rect(0,47,128,17,false);Rect(0,47,128,1,true);
        if(alert) strcpy(s,"VFO B ACTIVITY");
        else sprintf(s,"B SCAN CH%03u",gVfoSplit.current_channel+1);
        Text(s,2,49,0,false);
        sprintf(frequency,"%3u.%05u",(unsigned)(f/100000),(unsigned)(f%100000));
        Text(frequency,2,57,1,false);
        if(alert) {sprintf(s,"%ddBm",gVfoSplit.alert.rssi_dBm);Text(s,86,58,0,false);}
    } else if(gScanStateDir!=SCAN_OFF || SCANNER_IsScanning()) {
        sprintf(s,"SCAN %s %uc/s",gScanPauseMode ? "HOLD" : gScanStateDir>0 ? "UP" : "DOWN",SCAN_RATE_ChannelsPerSec());
        Rect(0,47,128,17,false);Rect(0,47,128,1,true);Text(s,2,49,1,false);
        Text("MENU/EXIT:STOP  UP/DN:DIR",16,58,0,false);
#ifdef ENABLE_APRS_TRANSPORT
    } else if(RUNTIME_Leased()) {
        Banner("APRS ACTIVE","PTT:RETURN TO VOICE");
#endif
    } else if(gAPRS_BannerTicks) {
        sprintf(s,"MSG %s",gAPRS_BannerSource);Banner(s,gAPRS_BannerText);
    } else if(gSetting_live_DTMF_decoder && gDTMF_RX_live[0]) {
        unsigned n=strlen(gDTMF_RX_live);Banner("DTMF DECODE",gDTMF_RX_live+(n>17 ? n-17 : 0));
    } else if(BANDSCOPE_IsEnabled() && !FUNCTION_IsRx()) {
        memset(gFrameBuffer[6],0,128);BANDSCOPE_Render(gFrameBuffer[6]);
    }
}

void UI_HomeStatus(void)
{
    memset(gStatusLine, 0xff, sizeof(gStatusLine));
    const unsigned active = UI_HomeEligible() ? Primary() : 0;
    const VFO_Info_t *v = HomeVfo(active);
    const char *state = gCurrentFunction == FUNCTION_TRANSMIT ? "TX" :
                        FUNCTION_IsRx() ? "RX" : "";
    Text(state, 2, 1, 0, true);
    char s[16];
    const char power = v->OUTPUT_POWER == OUTPUT_POWER_HIGH ? 'H' :
                       v->OUTPUT_POWER == OUTPUT_POWER_MID ? 'M' : 'L';
    sprintf(s, "%s %c", v->Modulation<MODULATION_UKNOWN ? gModulationStr[v->Modulation] : "?", power);
    Text(s, 19, 1, 0, true);
    if (gAPRS_Enabled) Text("APRS", 47, 1, 0, true);
    if (gEeprom.KEY_LOCK) Text("LK", 75, 1, 0, true);
    else if (gWasFKeyPressed) Text("F", 75, 1, 0, true);
    else if (gAPRS_BannerTicks) Text("M", 75, 1, 0, true);
    sprintf(s, "%u%%", BATTERY_VoltsToPercent(gBatteryVoltageAverage));
    Text(s, 126 - (strlen(s) * 7 - 1), 0, 1, true);
}

bool UI_HomeDraw(void)
{
    if (!UI_HomeEligible()) return false;
    const bool tx = gCurrentFunction == FUNCTION_TRANSMIT;
    const unsigned primary = Primary();
    const VFO_Info_t *v = HomeVfo(primary);
    const uint32_t f = tx ? v->pTX->Frequency : v->pRX->Frequency;
    char s[32], name[17] = {0};
    const bool memory = IS_MR_CHANNEL(gEeprom.ScreenChannel[primary]);
    if (memory) SETTINGS_FetchChannelName(name, gEeprom.ScreenChannel[primary]);
    name[16] = 0;
    if (memory) sprintf(s, "%c  CH%03u", 'A' + primary, gEeprom.ScreenChannel[primary] + 1);
    else sprintf(s, "VFO %c", 'A' + primary);
    Text(s, 2, memory ? 9 : 10, 0, false);
    if (memory && name[0]) {
        Center(name, 16, 2);
        sprintf(s, "%3u.%05u", (unsigned)(f / 100000), (unsigned)(f % 100000));
        Center(s, 34, 1);
        const int32_t shift = (int32_t)v->pTX->Frequency - (int32_t)v->pRX->Frequency;
        const uint32_t delta = shift < 0 ? -shift : shift;
        sprintf(s, "SHIFT %c%u.%03u %s", shift < 0 ? '-' : '+',
                (unsigned)(delta / 100000), (unsigned)(delta % 100000 / 100),
                v->CHANNEL_BANDWIDTH ? "N" : "W");
        Center(s, 42, 0);
    } else {
        Frequency(f, 20);
        sprintf(s, "SQL%u %s %u.%ukHz", gEeprom.SQUELCH_LEVEL,
                v->CHANNEL_BANDWIDTH ? "N" : "W",
                v->StepFrequency / 100, v->StepFrequency % 100 / 10);
        Center(s, 39, 0);
    }
    if(FUNCTION_IsRx() && !gInputBoxIndex) {
        uint8_t level;int16_t dbm=UI_RX_Signal_dBm(&level);
        sprintf(s,"S%u %ddBm Q%u %c",level,dbm,SIGNAL_QUALITY_Get(),SIGNAL_CLASSIFIER_GetSymbol(primary));
        const unsigned y=memory && name[0] ? 42 : 39;
        Rect(0,y,128,7,false);Text(s,2,y,0,false);
        for(unsigned i=0;i<9;i++) if(level>i) Rect(100+i*3,y+1,2,4,true);
    }
    const unsigned secondary = primary ^ 1;
    const uint32_t other = HomeVfo(secondary)->pRX->Frequency;
    sprintf(s, "%c %3u.%05u", 'A' + secondary, (unsigned)(other / 100000), (unsigned)(other % 100000));
    Text(s, 4, 49, 1, false);
    if (tx) {
        Rect(0, memory && name[0] ? 42 : 38, 128, memory && name[0] ? 15 : 19, false);
        const unsigned elapsed = gEeprom.TX_TIMEOUT_TIMER ?
            (gEeprom.TX_TIMEOUT_TIMER * 120 - gTxTimerCountdown_500ms) / 2 : 0;
        sprintf(s, "TX %u:%02u", elapsed / 60, elapsed % 60);
        Center(s, 42, 1);
    } else {
        Rect(0, 57, 128, 7, true);
        Text("MENU   F+5:APRS   EXIT:A/B", 16, 58, 0, true);
        if(gInputBoxIndex) Input(memory);
        Overlays(primary);
    }

    return true;
}
