/* Copyright 2024 UV-K5 Firmware Custom
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

/* Modified for UVK5: passive RX only, shared-radio arbitration and four
 * recent packets. BK4819 programming and capture flow adapted from TA1JS.
 * The optional transport receives decoded frames before mailbox filtering;
 * default RX does not dispatch data TX. No APRS EEPROM writes are introduced.
 */
#include "aprs_rx.h"
#ifdef ENABLE_APRS_TRANSPORT
#include "aprs_runtime.h"
#include "aprs_setup.h"
#endif
#ifdef ENABLE_APRS_SESSION
#include "aprs_message.h"
#include "aprs_session.h"
#endif
#include "audio.h"
#include "app/chFrScanner.h"
#include "app/scanner.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/ui.h"
#include "vfo_split.h"
#include <string.h>

bool gAPRS_Enabled, gAPRS_Browser;
bool gAPRS_AlertSound = true;
uint8_t gAPRS_BannerTicks;
char gAPRS_BannerSource[10], gAPRS_BannerText[18];
static bool alertPending;
APRS_Record gAPRS_Log[APRS_LOG_CAPACITY];
uint8_t gAPRS_Count, gAPRS_Selected;
static uint8_t capture[240];
static uint16_t captureCount;
static bool armed, capturing;
static uint8_t stuckTicks;
static const uint8_t registers[] = {0x70,0x71,0x72,0x58,0x5a,0x5c,0x5d,0x5e,0x3f};
static uint16_t saved[sizeof(registers)];
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
APRS_RxDiagnostics gAPRS_RxDiagnostics;
uint8_t gAPRS_DiagnosticPage;
/* Explicit little-endian wire format: 16-byte header, four 248-byte slots.
 * Each slot holds sequence:u32, length:u16, reason:u8, modem-mode:u8, raw[240]. */
static uint8_t captureDump[16+4*248],captureReason;
static uint32_t captureSequence;
static bool captureFrozen;
static uint8_t diagnosticMode;
/* BK4819 register manual: RX mode bits12:10, bandwidth bits3:1.
 * Gray-code order changes only one field between adjacent tests. */
static const uint16_t diagnosticReg58[]={0x3fc3,0x3fc9,0x33c9,0x33c3};
static void StoreLE(uint8_t *out,uint32_t value,unsigned bytes)
{
    for(unsigned i=0;i<bytes;i++) {out[i]=value;value>>=8;}
}
static void StoreDiagnosticCapture(void)
{
    if(captureFrozen) return;
    uint8_t *slot=captureDump+16+(captureSequence%4)*248;
    ++captureSequence;
    StoreLE(slot,captureSequence,4);StoreLE(slot+4,captureCount,2);
    slot[6]=captureReason;slot[7]=diagnosticMode;
    memset(slot+8,0,240);memcpy(slot+8,capture,captureCount);
}
uint8_t APRS_ReadDiagnosticCapture(uint16_t offset,uint8_t size,uint8_t *out)
{
    if(!out || offset>=sizeof(captureDump) || !size) return 0;
    if(size>64) size=64;
    if(size>sizeof(captureDump)-offset) size=sizeof(captureDump)-offset;
    if(!offset) {
        captureFrozen=true;
        memcpy(captureDump,"UVRX",4);captureDump[4]=2;
        captureDump[5]=captureSequence<4 ? captureSequence:4;
        captureDump[6]=captureSequence%4;captureDump[7]=1;
        StoreLE(captureDump+8,captureSequence,4);
        StoreLE(captureDump+12,240,2);StoreLE(captureDump+14,248,2);
    }
    if(!captureFrozen) return 0;
    memcpy(out,captureDump+offset,size);return size;
}
void APRS_GetDiagnostics(APRS_RxDiagnostics *out)
{
    *out=gAPRS_RxDiagnostics;
    out->ready=APRS_CanListen();out->armed=armed;out->capturing=capturing;
    out->bytes=captureCount;
    out->frozen=captureFrozen;out->mode=diagnosticMode;
    out->reg58=BK4819_ReadRegister(BK4819_REG_58);
    out->reg59=BK4819_ReadRegister(BK4819_REG_59);
    out->reg3f=BK4819_ReadRegister(BK4819_REG_3F);
}
#endif
#define APRS_RX_IRQ_MASK (BK4819_REG_02_FSK_RX_FINISHED | \
                          BK4819_REG_02_FSK_FIFO_ALMOST_FULL | \
                          BK4819_REG_02_FSK_RX_SYNC)

bool APRS_CanListen(void)
{
    return gAPRS_Enabled && !gSerialConfigCountDown_500ms && gRxVfo->Modulation == MODULATION_FM &&
        (gCurrentFunction == FUNCTION_FOREGROUND || FUNCTION_IsRx()) &&
        gScanStateDir == SCAN_OFF && !SCANNER_IsScanning() && !gCssBackgroundScan &&
        gEeprom.DUAL_WATCH == DUAL_WATCH_OFF &&
        gEeprom.CROSS_BAND_RX_TX == CROSS_BAND_OFF && !VFO_SPLIT_IsEnabled();
}

bool APRS_IsCapturing(void) {return armed && capturing;}

void APRS_Stop(void)
{
    if (!armed) return;
    BK4819_WriteRegister(BK4819_REG_59, 0);
    for (unsigned i = 0; i < sizeof(registers); ++i)
        BK4819_WriteRegister(registers[i], saved[i]);
    armed = capturing = false;
    captureCount = 0;
}

static void Arm(void)
{
    BK4819_WriteRegister(BK4819_REG_70, (1u << 15) | (1u << 7) | (96u << 0));
    BK4819_WriteRegister(BK4819_REG_71, 22714);   // 2200 Hz
    BK4819_WriteRegister(BK4819_REG_72, 12389);   // 1200 Hz / bit clock

    BK4819_WriteRegister(BK4819_REG_58,
        (1u << 13) |   // TX mode (unused while receiving)
        (7u << 10) |   // FSK RX mode: FFSK 1200/1800
        (3u <<  8) |   // FSK RX gain
        (3u <<  6) |   // FSK enable
        (1u <<  1) |   // FSK RX bandwidth FFSK 1200/1800
        (1u <<  0));   // FSK enable

#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    BK4819_WriteRegister(BK4819_REG_58,diagnosticReg58[diagnosticMode]);
#endif
    BK4819_WriteRegister(BK4819_REG_5A, 0xFEFE);  // NRZI'd flag-run pattern
    BK4819_WriteRegister(BK4819_REG_5C, 0xAA30);  // engine CRC off
    BK4819_WriteRegister(BK4819_REG_5D, (uint16_t)((240u - 1u) << 8));
    BK4819_WriteRegister(0x5e, (64u << 3) | (4u << 0));  // RX FIFO almost-full: 4 words

    const uint16_t mask = BK4819_ReadRegister(BK4819_REG_3F);
    BK4819_WriteRegister(BK4819_REG_3F, mask | APRS_RX_IRQ_MASK);

    BK4819_WriteRegister(BK4819_REG_59, 0x4000);  // clear RX FIFO (2-byte sync)
    BK4819_WriteRegister(BK4819_REG_59, 0x1000);  // FSK RX enable

    captureCount      = 0;
    capturing  = false;
    stuckTicks = 0;
    armed      = true;
}

void APRS_Resume(void)
{
    if(!APRS_CanListen() || armed) return;
    for(unsigned i=0;i<sizeof(registers);++i)
        saved[i]=BK4819_ReadRegister(registers[i]);
    Arm();
}


static void Drain(uint8_t words)
{
    while (words--) {
        uint16_t w = BK4819_ReadRegister(BK4819_REG_5F);
        if (captureCount < sizeof(capture)) capture[captureCount++] = w;
        if (captureCount < sizeof(capture)) capture[captureCount++] = w >> 8;
    }
}

static void Decode(void)
{
    APRS_Record record;
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    StoreDiagnosticCapture();
    gAPRS_RxDiagnostics.lastBytes=captureCount;
    memset(gAPRS_RxDiagnostics.prefix,0,sizeof(gAPRS_RxDiagnostics.prefix));
    memcpy(gAPRS_RxDiagnostics.prefix,capture,
           captureCount<sizeof(gAPRS_RxDiagnostics.prefix) ? captureCount:sizeof(gAPRS_RxDiagnostics.prefix));
    if(captureCount==sizeof(capture)) ++gAPRS_RxDiagnostics.full;
    if(!APRS_DecodeBits(capture,captureCount,&record)) {++gAPRS_RxDiagnostics.bad;return;}
    ++gAPRS_RxDiagnostics.good;
#else
    if (!APRS_DecodeBits(capture, captureCount, &record)) return;
#endif
#ifdef ENABLE_APRS_MESSAGING
    record.frequency=gRxVfo->pRX ? gRxVfo->pRX->Frequency : 0;
#endif
#ifdef ENABLE_APRS_SESSION
#ifdef ENABLE_APRS_TRANSPORT
    RUNTIME_Receive(&record);
#endif
    SESSION_Receive(&record);
#endif
    memmove(gAPRS_Log + 1, gAPRS_Log, (APRS_LOG_CAPACITY - 1) * sizeof(record));
    gAPRS_Log[0] = record;
    if (record.kind == APRS_MESSAGE) {
        memcpy(gAPRS_BannerSource, record.source, sizeof(gAPRS_BannerSource));
        memcpy(gAPRS_BannerText, record.text, sizeof(gAPRS_BannerText) - 1);
        gAPRS_BannerText[sizeof(gAPRS_BannerText) - 1] = 0;
        /* Initial five-second timing; exact timing remains a review setting. */
        gAPRS_BannerTicks = 10;
        alertPending = gAPRS_AlertSound;
    }
    if (gAPRS_Count < APRS_LOG_CAPACITY) ++gAPRS_Count;
    /* Keep an older selected entry stable when a new packet arrives. */
    if (gAPRS_Selected && gAPRS_Selected + 1 < gAPRS_Count) ++gAPRS_Selected;
    gUpdateDisplay = true;
}

void APRS_Interrupt(uint16_t bits)
{
    if (!armed) return;
    if (!APRS_CanListen()) { APRS_Stop(); return; }
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    gAPRS_RxDiagnostics.lastIrq=bits;
    if(bits & BK4819_REG_02_FSK_RX_SYNC) ++gAPRS_RxDiagnostics.sync;
    if(bits & BK4819_REG_02_FSK_FIFO_ALMOST_FULL) ++gAPRS_RxDiagnostics.fifo;
    if(bits & BK4819_REG_02_FSK_RX_FINISHED) ++gAPRS_RxDiagnostics.end;
#endif
    if (bits & BK4819_REG_02_FSK_RX_SYNC) {
        capturing = true;
        stuckTicks = 0;
        captureCount = 0;
    }
    if (bits & BK4819_REG_02_FSK_FIFO_ALMOST_FULL) {
        stuckTicks = 0;
        Drain(4);
    }
    if (bits & BK4819_REG_02_FSK_RX_FINISHED) {
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
        captureReason=1;
#endif
        Drain(8);
        Decode();
        Arm();
    }
}

void APRS_Tick500ms(void)
{
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    if(gAPRS_Browser) gUpdateDisplay=true;
#endif
#ifdef ENABLE_APRS_SESSION
    MESSAGE_Tick500ms();
    SESSION_Tick500ms();
#endif
    if (gAPRS_BannerTicks && --gAPRS_BannerTicks == 0) gUpdateDisplay = true;
    if (alertPending && gCurrentFunction == FUNCTION_FOREGROUND) {
        alertPending = false;
        if (gAPRS_AlertSound) AUDIO_PlayBeep(BEEP_1KHZ_60MS_OPTIONAL);
    }
    if (!APRS_CanListen()) { APRS_Stop(); return; }
    if (!armed) {
        for (unsigned i = 0; i < sizeof(registers); ++i)
            saved[i] = BK4819_ReadRegister(registers[i]);
        Arm();
    } else if (capturing && ++stuckTicks > 2) {
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
        ++gAPRS_RxDiagnostics.timeout;
        captureReason=2;
#endif
        Drain(8);
        Decode();
        Arm();
    }
}

void APRS_Open(void)
{
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    gAPRS_DiagnosticPage=0;
#endif
#ifdef ENABLE_APRS_SESSION
    MESSAGE_Init();
    SESSION_Open();
#endif
    gAPRS_Browser = true;
    gAPRS_Selected = 0;
    gRequestDisplayScreen = DISPLAY_MAIN;
    gUpdateDisplay = true;
}

bool APRS_Key(KEY_Code_t key, bool pressed, bool held)
{
    (void)held;
    if (key == KEY_PTT && pressed) {
        gAPRS_BannerTicks = 0;
        alertPending = false;
    }
    if (!gAPRS_Browser) return false;
    /* PTT must retain the stock press/release path; hide the browser first. */
    if (key == KEY_PTT) {
#ifdef ENABLE_APRS_SESSION
        gAPRS_Message.page = MESSAGE_CLOSED;
#ifdef ENABLE_APRS_TRANSPORT
        gAPRS_Setup.page=SETUP_CLOSED;
#endif
#endif
        gAPRS_Browser = false;
        gUpdateDisplay = true;
        return false;
    }
    if (!pressed) return true;
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    if(key==KEY_1 && !held) {
        captureFrozen=false;captureSequence=0;memset(captureDump,0,sizeof(captureDump));
        gUpdateDisplay=true;return true;
    }
    if(key==KEY_2 && !held) {
        /* Drop any partial capture before changing the demodulator. Preserve
         * completed captures (including frozen exports) and radio registers. */
        APRS_Stop();diagnosticMode=(diagnosticMode+1)%4;APRS_Resume();
        gUpdateDisplay=true;return true;
    }
    if(key==KEY_MENU && !held) {
        gAPRS_DiagnosticPage=(gAPRS_DiagnosticPage+1)%3;
        gUpdateDisplay=true;return true;
    }
#endif
#ifdef ENABLE_APRS_SESSION
    if (held) return true;
    if (gAPRS_Message.page != MESSAGE_CLOSED) {
        MESSAGE_Key(key);
        gUpdateDisplay = true;
        return true;
    }
#ifdef ENABLE_APRS_TRANSPORT
    if(gAPRS_Setup.page!=SETUP_CLOSED) SETUP_Key(key);
    else
#endif
    SESSION_Key(key);
    gUpdateDisplay = true;
    return true;
#endif
    if (key == KEY_EXIT) gAPRS_Browser = false;
    else if (key == KEY_UP && gAPRS_Selected) --gAPRS_Selected;
    else if (key == KEY_DOWN && gAPRS_Selected + 1 < gAPRS_Count) ++gAPRS_Selected;
    gUpdateDisplay = true;
    return true;
}
