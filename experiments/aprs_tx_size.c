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

/* SIZE INVESTIGATION ONLY, NOT PART OF THE RELEASE PROFILE.
 * Minimal TA1JS manual beacon path. No LCD configuration, periodic beacons,
 * message composer, ACK/retry logic, or Bell 202 correction included.
 */
#include <string.h>
#include "app/aprs_decode.h"
#include "app/aprs_rx.h"
#include "app/app.h"
#include "driver/bk4819.h"
#include "driver/system.h"
#include "functions.h"
#include "radio.h"
#include "settings.h"
static void AX25_EncodeAddress(const char *callsign, uint8_t ssid, bool last, uint8_t out7[7])
{
    bool end = false;
    for (uint8_t i = 0; i < 6; i++) {
        char c = ' ';
        if (!end) {
            if (callsign[i] == 0)
                end = true;  // stop reading past the terminator
            else
                c = callsign[i];
        }
        out7[i] = (uint8_t)(c << 1);
    }
    out7[6] = (uint8_t)(0x60 | ((ssid & 0x0F) << 1) | (last ? 0x01 : 0x00));
}

// ---------------------------------------------------------------------------
// Bell 202 TX via the BK4819 hardware FSK engine.
//
// The modem clocks the bits out itself (no software bit timing), but it knows
// nothing about AX.25/HDLC, so the full on-air bitstream - flags, bit
// stuffing, NRZI - is prepared in software and pushed through the FSK FIFO
// as raw bytes (engine CRC and scrambler disabled).
//
// Register recipe: this repo's proven MDC1200 TX sequence + the Bell 202
// tone setup from gvJaime/uv-k5-firmware-aprs (tone1 forced to 2200 Hz on
// top of the FFSK 1200/1800 TX mode) + kamilsss655's deviation and
// TX-filter measurements.
// ---------------------------------------------------------------------------

#define HDLC_BUF_SIZE    240u  // TX: encoded bitstream; RX: raw capture (shared, never simultaneous)
#define HDLC_LEAD_FLAGS  32u   // ~213 ms of 0x7E preamble
#define HDLC_TAIL_FLAGS  3u

typedef struct {
    uint8_t *buf;
    uint16_t bits;
    uint8_t  ones;
    uint8_t  level;  // current NRZI line level
} hdlc_writer_t;

static void HDLC_PutBit(hdlc_writer_t *w, bool bit)
{
    if (w->bits >= HDLC_BUF_SIZE * 8u)
        return;
    if (!bit)
        w->level ^= 1;  // NRZI: 0 = transition, 1 = no change
    if (w->level)
        w->buf[w->bits >> 3] |= 0x80u >> (w->bits & 7u);  // modem shifts MSB first
    w->bits++;
}

static void HDLC_PutByte(hdlc_writer_t *w, uint8_t b, bool stuff)
{
    for (uint8_t i = 0; i < 8; i++) {  // AX.25 sends bits LSB first
        const bool bit = (b >> i) & 1u;
        HDLC_PutBit(w, bit);
        if (stuff && bit) {
            if (++w->ones == 5) {
                HDLC_PutBit(w, false);  // stuff a 0 after 5 consecutive 1s
                w->ones = 0;
            }
        } else {
            w->ones = 0;
        }
    }
}

static uint8_t gHdlcBuf[HDLC_BUF_SIZE];

static void APRS_TransmitUpstreamFFSK(const uint8_t *frame, uint16_t frame_len)
{
    uint16_t i;

    memset(gHdlcBuf, 0, sizeof(gHdlcBuf));
    hdlc_writer_t w = { gHdlcBuf, 0, 0, 1 };

    for (i = 0; i < HDLC_LEAD_FLAGS; i++)
        HDLC_PutByte(&w, 0x7E, false);
    for (i = 0; i < frame_len; i++)
        HDLC_PutByte(&w, frame[i], true);
    for (i = 0; i < HDLC_TAIL_FLAGS; i++)
        HDLC_PutByte(&w, 0x7E, false);

    uint16_t nbytes = (w.bits + 7u) / 8u;
    if (nbytes & 1u)
        nbytes++;  // FIFO takes 16-bit words

    // Key TX using the normal radio TX path (PA, frequency, timers).
    RADIO_PrepareTX();
    if (gCurrentFunction != FUNCTION_TRANSMIT) {
        return;
    }

    BK4819_SetAF(BK4819_AF_MUTE);

    const uint16_t css_val  = BK4819_ReadRegister(BK4819_REG_51);
    const uint16_t dev_val  = BK4819_ReadRegister(0x40);
    const uint16_t filt_val = BK4819_ReadRegister(BK4819_REG_2B);

    BK4819_WriteRegister(BK4819_REG_51, 0);                            // CTCSS/CDCSS off
    BK4819_WriteRegister(0x40, (dev_val & 0xF000u) | 1200u);  // tone deviation ~3 kHz
    // REG_2B<2> TX HPF300 off, <0> TX pre-emphasis off (Registers_V1.1 p.8-9;
    // 1 = disable). Measured 2026-07-25 with an RTL-SDR + direwolf: the AF TX
    // filter chain has no effect at all on the FFSK tone balance - neither
    // disabling TX LPF1 (<1>) nor re-enabling pre-emphasis moved the measured
    // mark/space ratio off 3/2. Do not chase the imbalance here.
    BK4819_WriteRegister(BK4819_REG_2B, (1u << 2) | (1u << 0));

    // REG_70: <15> tone1 enable, <14:8> tone1 gain, <7> tone2 enable, <6:0>
    // tone2 gain. Tone1's gain field stays 0 on purpose: REG_71 only uses
    // tone1 as the 2200 Hz *frequency* reference for the FSK engine, and
    // giving it amplitude lays a continuous 2200 Hz tone over the modulation -
    // measured on 2026-07-25, nothing decodes at all with the gain set. Only
    // tone2 (1200 Hz, the bit clock) carries amplitude.
    BK4819_WriteRegister(BK4819_REG_70,          // tone1 freq ref + tone2 gain 96
        (1u << 15) | (1u << 7) | (96u << 0));
    BK4819_WriteRegister(BK4819_REG_71, 22714);  // tone1: 2200 Hz (space)
    BK4819_WriteRegister(BK4819_REG_72, 12389);  // tone2: 1200 Hz (mark / bit clock)

    // FSK TX mode 001 = FFSK 1200/1800. Measured on air 2026-07-25 (RTL-SDR
    // capture, zero-crossing analysis): the tones really are 1200/1800 Hz, NOT
    // the 1200/2200 Bell 202 pair - REG_71 below has no effect on them. Mode
    // 011 (FFSK 1200/2400) was tried and is unusable: no 2400 Hz tone appears,
    // the output collapses to a near-constant 1200 Hz tone and nothing decodes
    // (neither direwolf nor a second UV-K5). 1200/1800 it is.
    BK4819_WriteRegister(BK4819_REG_58,
        (1u << 13) |   // FSK TX mode: FFSK 1200/1800
        (7u << 10) |   // FSK RX mode (unused while transmitting)
        (3u <<  8) |   // FSK RX gain
        (3u <<  6) |   // FSK enable (TRM REG_58<7:6> = 11; all Beken examples set this)
        (1u <<  1) |   // FSK RX bandwidth FFSK 1200/1800
        (1u <<  0));   // FSK enable

    BK4819_WriteRegister(BK4819_REG_5A, 0xAAAA);  // sync bytes double as extra
    BK4819_WriteRegister(BK4819_REG_5B, 0xAAAA);  // clock training before the flags
    BK4819_WriteRegister(BK4819_REG_5C, 0xAA30);  // engine CRC off
    BK4819_WriteRegister(BK4819_REG_5D, (uint16_t)((nbytes - 1u) << 8));

    BK4819_WriteRegister(BK4819_REG_59, 0x8068);  // clear TX FIFO; 4 sync bytes, 6 byte preamble
    BK4819_WriteRegister(BK4819_REG_59, 0x0068);

    for (i = 0; i < nbytes; i += 2)
        BK4819_WriteRegister(BK4819_REG_5F,
            (uint16_t)(gHdlcBuf[i + 1] << 8) | gHdlcBuf[i]);

    SYSTEM_DelayMs(20);

    BK4819_WriteRegister(BK4819_REG_59, 0x0868);  // FSK TX enable

    // 1200 baud -> 6.67 ms/byte; preamble(6) + sync(4) + payload, plus margin.
    SYSTEM_DelayMs(((10u + (uint32_t)nbytes) * 20u) / 3u + 100u);

    // Tear down the modem and unkey.
    BK4819_WriteRegister(BK4819_REG_59, 0x0068);
    BK4819_WriteRegister(BK4819_REG_70, 0x0000);
    BK4819_WriteRegister(BK4819_REG_58, 0x0000);
    BK4819_WriteRegister(0x40, dev_val);
    BK4819_WriteRegister(BK4819_REG_2B, filt_val);
    BK4819_WriteRegister(BK4819_REG_51, css_val);

    APP_EndTransmission();
    FUNCTION_Select(FUNCTION_FOREGROUND);
}


static char callsign[7];
static char comment[32];
static uint8_t ssid;
static int32_t latitude, longitude;
static uint16_t APRS_BuildHeader(uint8_t *frame)
{
    // <tocall>,WIDE1-1,WIDE2-1 path, source callsign/SSID from the menu settings.
    const char *src = callsign;
    if (src[0] <= ' ')
        src = "N0CALL";

    uint16_t idx = 0;
    AX25_EncodeAddress("APZUVK", 0, false, &frame[idx]); idx += 7;
    AX25_EncodeAddress(src, ssid & 0x0F, false, &frame[idx]); idx += 7;
    AX25_EncodeAddress("WIDE1", 1, false, &frame[idx]); idx += 7;
    AX25_EncodeAddress("WIDE2", 1, true,  &frame[idx]); idx += 7;
    frame[idx++] = 0x03; // Control: UI frame
    frame[idx++] = 0xF0; // PID: no layer 3
    return idx;
}

static void APRS_TxFrame(uint8_t *frame, uint16_t idx)
{
    const uint16_t fcs = APRS_FCS(frame, idx);
    frame[idx++] = (uint8_t)(fcs & 0xFF);
    frame[idx++] = (uint8_t)((fcs >> 8) & 0xFF);
    APRS_TransmitUpstreamFFSK(frame, idx);
}

static uint16_t APRS_PosDigits(uint8_t *out, uint32_t v, uint8_t deg3)
{
    // micro-degrees -> "DDMM.mm" / "DDDMM.mm"
    const uint32_t deg    = v / 1000000u;
    const uint32_t min100 = ((v % 1000000u) * 6u) / 1000u;
    uint16_t n = 0;
    if (deg3)
        out[n++] = (uint8_t)('0' + (deg / 100u) % 10u);
    out[n++] = (uint8_t)('0' + (deg / 10u) % 10u);
    out[n++] = (uint8_t)('0' + deg % 10u);
    out[n++] = (uint8_t)('0' + (min100 / 1000u) % 10u);
    out[n++] = (uint8_t)('0' + (min100 / 100u) % 10u);
    out[n++] = '.';
    out[n++] = (uint8_t)('0' + (min100 / 10u) % 10u);
    out[n++] = (uint8_t)('0' + min100 % 10u);
    return n;
}

static void APRS_TransmitBeaconCoords(int32_t lat_udeg, int32_t lon_udeg)
{
    // Build "!DDMM.mmN/DDDMM.mmE>comment" for the given coordinates.
    uint8_t frame[7 * 4 + 2 + 21 + 31 + 2];
    uint16_t idx = APRS_BuildHeader(frame);
    int32_t v;
    char hemi;

    frame[idx++] = '!';
    v = lat_udeg;
    hemi = 'N';
    if (v < 0) { v = -v; hemi = 'S'; }
    idx += APRS_PosDigits(&frame[idx], (uint32_t)v, 0);
    frame[idx++] = (uint8_t)hemi;
    frame[idx++] = '/';
    v = lon_udeg;
    hemi = 'E';
    if (v < 0) { v = -v; hemi = 'W'; }
    idx += APRS_PosDigits(&frame[idx], (uint32_t)v, 1);
    frame[idx++] = (uint8_t)hemi;
    frame[idx++] = '>';
    for (uint8_t i = 0; i < 31 && comment[i]; i++)
        frame[idx++] = (uint8_t)comment[i];

    APRS_TxFrame(frame, idx);
}

static void APRS_TransmitBeacon(void)
{
    // Position and comment come from the menu settings (Loc / Cmnt).
    APRS_TransmitBeaconCoords(latitude, longitude);
}


void APRS_TxTrial(void)
{
    /* Volatile input prevents LTO from discarding the unconfigured prototype.
     * It is not an operator-usable settings mechanism. N0CALL is refused.
     */
    extern volatile unsigned gAPRS_TrialInput;
    for (unsigned i=0;i<6;i++) callsign[i]=(char)gAPRS_TrialInput;
    if (!callsign[0] || !strcmp(callsign,"N0CALL")) return;
    extern volatile int32_t gAPRS_TrialLatitude, gAPRS_TrialLongitude;
    latitude = gAPRS_TrialLatitude;
    longitude = gAPRS_TrialLongitude;
    ssid = gAPRS_TrialInput & 15;
    if (latitude < -90000000 || latitude > 90000000 || longitude < -180000000 || longitude > 180000000) return;
    APRS_Stop();
    APRS_TransmitBeacon();
}
volatile unsigned gAPRS_TrialInput;
volatile int32_t gAPRS_TrialLatitude, gAPRS_TrialLongitude;
