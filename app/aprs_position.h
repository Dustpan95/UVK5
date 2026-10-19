/* SPDX-License-Identifier: Apache-2.0 */
#ifndef APRS_POSITION_H
#define APRS_POSITION_H
#include <stdbool.h>
#include <stdint.h>
bool APRS_Position(const uint8_t *destination, const uint8_t *info, uint16_t length,
                   int32_t *latitude, int32_t *longitude);
#endif
