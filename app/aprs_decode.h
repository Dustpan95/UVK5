/* SPDX-License-Identifier: Apache-2.0 */
#ifndef APRS_DECODE_H
#define APRS_DECODE_H
#include <stdint.h>
#include <stdbool.h>
#define APRS_FRAME_MAX 192
#define APRS_LOG_CAPACITY 4
#define APRS_TEXT_MAX 67
typedef enum { APRS_OTHER, APRS_POSITION, APRS_MESSAGE, APRS_ACK, APRS_REJECT } APRS_Kind;
typedef struct {
    char source[10];
    char destination[10];
    char text[APRS_TEXT_MAX + 1];
    char messageId[6];
    int32_t latitude, longitude; /* millionths of degrees */
    APRS_Kind kind;
} APRS_Record;
uint16_t APRS_FCS(const uint8_t *data, uint16_t length);
bool APRS_ParseFrame(const uint8_t *frame, uint16_t len, APRS_Record *out);
bool APRS_DecodeBits(const uint8_t *capture, uint16_t count, APRS_Record *out);
#endif
