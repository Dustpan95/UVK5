/* SPDX-License-Identifier: Apache-2.0
 * UVK5 implementation of APRS 1.0.1 coordinate encodings (chapters 6, 9, 10).
 * Protocol reference: https://www.aprs.org/doc/APRS101.PDF
 * Written for this fork; no F4JTV position-parser source is included.
 * Position-ambiguity encodings are rejected rather than inventing precision.
 */
#include "aprs_position.h"
#include "aprs_scale.h"

static bool Decimal(const uint8_t *s, unsigned length, uint32_t *value)
{
    *value = 0;
    while (length--) {
        if (*s < '0' || *s > '9') return false;
        *value = *value * 10 + (*s++ - '0');
    }
    return true;
}

static bool Coordinate(const uint8_t *s, unsigned degreeDigits, unsigned limit,
                       uint8_t plus, uint8_t minus, int32_t *value)
{
    uint32_t degrees, minutes, fraction;
    if (!Decimal(s, degreeDigits, &degrees) ||
        !Decimal(s + degreeDigits, 2, &minutes) ||
        s[degreeDigits + 2] != '.' ||
        !Decimal(s + degreeDigits + 3, 2, &fraction)) return false;
    const uint8_t hemisphere = s[degreeDigits + 5];
    if (hemisphere != plus && hemisphere != minus) return false;
    if (degrees > limit || minutes >= 60 ||
        (degrees == limit && (minutes || fraction))) return false;
    const int32_t result = degrees * 1000000u + (minutes * 100u + fraction) * 10000u / 60u;
    *value = hemisphere == minus ? -result : result;
    return true;
}

static bool Base91(const uint8_t *s, uint32_t *value)
{
    *value = 0;
    for (unsigned i = 0; i < 4; ++i) {
        if (s[i] < 33 || s[i] > 123) return false;
        *value = *value * 91u + (s[i] - 33u);
    }
    return true;
}

static bool MicE(const uint8_t *destination, const uint8_t *info, uint16_t length,
                 int32_t *latitude, int32_t *longitude)
{
    if (length < 9) return false;
    uint32_t digits = 0;
    for (unsigned i = 0; i < 6; ++i) {
        const uint8_t c = destination[i] >> 1;
        unsigned digit;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'P' && c <= 'Y') digit = c - 'P';
        else if (i < 3 && c >= 'A' && c <= 'J') digit = c - 'A';
        else return false;
        digits = digits * 10u + digit;
    }
    const uint32_t degree = digits / 10000u, hundredths = digits % 10000u;
    if (degree > 90 || hundredths >= 6000 || (degree == 90 && hundredths)) return false;
    *latitude = degree * 1000000u + hundredths * 10000u / 60u;
    if ((destination[3] >> 1) < 'P') *latitude = -*latitude;

    if (info[1] < 28 || info[2] < 28 || info[3] < 28 || info[3] > 127) return false;
    unsigned lonDegrees = info[1] - 28u;
    if ((destination[4] >> 1) >= 'P') lonDegrees += 100;
    if (lonDegrees >= 180 && lonDegrees < 190) lonDegrees -= 80;
    else if (lonDegrees >= 190 && lonDegrees < 200) lonDegrees -= 190;
    unsigned minutes = info[2] - 28u;
    if (minutes >= 60) minutes -= 60;
    unsigned fraction = info[3] - 28u;
    if (lonDegrees >= 180 || minutes >= 60) return false;
    *longitude = lonDegrees * 1000000u + (minutes * 100u + fraction) * 10000u / 60u;
    if ((destination[5] >> 1) >= 'P') *longitude = -*longitude;
    return true;
}

bool APRS_Position(const uint8_t *destination, const uint8_t *info, uint16_t length,
                   int32_t *latitude, int32_t *longitude)
{
    if (!length) return false;
    const uint8_t type = *info++;
    --length;
    if (type == '`' || type == 0x27 || type == 0x1c || type == 0x1d)
        return MicE(destination, info - 1, length + 1, latitude, longitude);
    if (type != '!' && type != '=' && type != '@' && type != '/') return false;
    if (type == '@' || type == '/') {
        if (length < 7) return false;
        info += 7; length -= 7;
    }
    if (length < 1) return false;
    if (info[0] >= '0' && info[0] <= '9') {
        return length >= 19 &&
            Coordinate(info, 2, 90, 'N', 'S', latitude) &&
            Coordinate(info + 9, 3, 180, 'E', 'W', longitude);
    }
    /* Compressed position: table, yyyy, xxxx, symbol, csT. */
    uint32_t northing, easting;
    if (length < 13 || !Base91(info + 1, &northing) || !Base91(info + 5, &easting)) return false;
    if (northing > 180u * 380926u || easting > 360u * 190463u) return false;
    *latitude = 90000000 - (int32_t)(APRS_ScaleMillion(northing, 380926u));
    *longitude = -180000000 + (int32_t)(APRS_ScaleMillion(easting, 190463u));
    return true;
}
