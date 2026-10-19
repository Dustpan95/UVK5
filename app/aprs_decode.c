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


/* Modified for UVK5: receive-only parser, bounded message/position records,
 * strict AX.25 UI/PID validation and geographic bounds. Adapted from TA1JS
 * 6320cdac7a2dce46f88027d220ff67e47eb0e3fb. Coordinate decoding is
 * implemented separately in aprs_position.c; F4JTV parser source is excluded.
 */
#include "aprs_decode.h"
#include "aprs_position.h"
#include <string.h>
static uint16_t APRS_CalculateCRC(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0x8408;  // Reversed polynomial
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

uint16_t APRS_FCS(const uint8_t *data, uint16_t length)
{
    // AX.25/HDLC FCS: init 0xFFFF, poly 0x8408 (reflected), final XOR 0xFFFF.
    return (uint16_t)(~APRS_CalculateCRC(data, length));
}

static bool Address(const uint8_t *a, char out[10])
{
    unsigned n = 0;
    bool padding = false;
    for (unsigned i = 0; i < 6; ++i) {
        const char c = a[i] >> 1;
        if (a[i] & 1) return false;
        if (c == ' ') { padding = true; continue; }
        if (padding || !((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
            return false;
        out[n++] = c;
    }
    if (!n) return false;
    const uint8_t ssid = (a[6] >> 1) & 15;
    if (ssid) {
        out[n++] = '-';
        if (ssid >= 10) out[n++] = '1';
        out[n++] = '0' + ssid % 10;
    }
    out[n] = 0;
    return true;
}

static bool MessageId(const uint8_t *text, unsigned length, char out[6])
{
    if(!length || length>5) return false;
    for(unsigned i=0;i<length;i++) {
        uint8_t c=text[i];
        if(!((c>='0' && c<='9') || (c>='A' && c<='Z') || (c>='a' && c<='z'))) return false;
    }
    memcpy(out,text,length);out[length]=0;return true;
}

bool APRS_ParseFrame(const uint8_t *frame, uint16_t len, APRS_Record *out)
{
    if (!frame || !out || len < 19 || len > APRS_FRAME_MAX) return false;
    if (APRS_FCS(frame, len - 2) != (uint16_t)(frame[len-2] | frame[len-1] << 8))
        return false;
    uint16_t offset = 0;
    unsigned addresses = 0;
    char addr[10];
    do {
        if (offset + 7 > len - 2 || addresses == 10 || !Address(frame + offset, addr))
            return false;
        offset += 7;
        ++addresses;
    } while (!(frame[offset - 1] & 1));
    if (addresses < 2 || offset + 2 >= len - 2 || frame[offset] != 0x03 || frame[offset+1] != 0xf0)
        return false;
    memset(out, 0, sizeof(*out));
    Address(frame + 7, out->source);
    const uint8_t *p = frame + offset + 2;
    uint16_t n = len - 2 - offset - 2;
    if (n >= 11 && p[0] == ':' && p[10] == ':') {
        out->kind = APRS_MESSAGE;
        for (unsigned i = 0; i < 9; ++i)
            out->destination[i] = p[1+i] >= 32 && p[1+i] < 127 ? p[1+i] : '.';
        for (int i = 8; i >= 0 && out->destination[i] == ' '; --i) out->destination[i] = 0;
        p += 11; n -= 11;
        if(n>=4 && n<=8 && (!memcmp(p,"ack",3) || !memcmp(p,"rej",3)) &&
           MessageId(p+3,n-3,out->messageId)) {
            out->kind=p[0]=='a' ? APRS_ACK : APRS_REJECT;
            memcpy(out->text,p,n);return true;
        }
    } else {
        const uint8_t type = *p;
        const bool position = type == '!' || type == '=' || type == '@' || type == '/' ||
                              type == '`' || type == 0x27 || type == 0x1c || type == 0x1d;
        const bool have = position && APRS_Position(frame, p, n, &out->latitude, &out->longitude);
        if (position && !have) return false;
        if (have) {
            if (out->latitude < -90000000 || out->latitude > 90000000 ||
                out->longitude < -180000000 || out->longitude > 180000000) return false;
            out->kind = APRS_POSITION;
            return true;
        }
    }
    unsigned i = 0;
    for (; i < n && i < APRS_TEXT_MAX; ++i) {
        if (out->kind == APRS_MESSAGE && p[i] == '{') break;
        out->text[i] = p[i] >= 32 && p[i] < 127 ? p[i] : '.';
    }
    out->text[i] = 0;
    if(out->kind==APRS_MESSAGE && i<n && p[i]=='{')
        MessageId(p+i+1,n-i-1,out->messageId);
    return true;
}
bool APRS_DecodeBits(const uint8_t *capture, uint16_t count, APRS_Record *out)
{
    if (!capture || !out || count > 240) return false;
    uint8_t gRxFrame[APRS_FRAME_MAX];
    uint8_t  prev = 0, ones = 0;
    uint16_t nbits = 0xFFFF;  // not synced until the first flag

    memset(gRxFrame, 0, sizeof(gRxFrame));

    for (uint32_t i = 0; i < (uint32_t)count * 8u; i++) {
        const uint8_t level = (capture[i >> 3] >> (7u - (i & 7u))) & 1u;
        const uint8_t bit   = (level == prev) ? 1u : 0u;  // NRZI: no change = 1
        prev = level;

        if (bit) {
            if (ones < 7)
                ones++;
            if (ones >= 7) {          // 7+ ones: invalid, wait for the next flag
                nbits = 0xFFFF;
                continue;
            }
            if (nbits != 0xFFFF) {    // collect (closing flag bits stripped later)
                if (nbits < sizeof(gRxFrame) * 8u)
                    gRxFrame[nbits >> 3] |= (uint8_t)(1u << (nbits & 7u));  // LSB first
                nbits++;
            }
            continue;
        }

        // bit == 0
        if (ones == 5) {              // stuffed zero - drop it
            ones = 0;
            continue;
        }
        if (ones == 6) {              // flag 01111110 completed
            if (nbits != 0xFFFF && nbits >= 7u) {
                const uint16_t fb = nbits - 7u;  // strip the flag's leading 0111111
                if ((fb & 7u) == 0 && fb >= 17u * 8u && (fb >> 3) <= sizeof(gRxFrame)) {
                    const uint16_t len = fb >> 3;
                    if (APRS_ParseFrame(gRxFrame, len, out)) return true;
                }
            }
            memset(gRxFrame, 0, sizeof(gRxFrame));
            nbits = 0;
            ones  = 0;
            continue;
        }
        if (nbits != 0xFFFF) {
            if (nbits < sizeof(gRxFrame) * 8u)
                nbits++;              // 0 bit: buffer already zeroed
            else
                nbits = 0xFFFF;       // frame too long - resync
        }
        ones = 0;
    }
    return false;
}
