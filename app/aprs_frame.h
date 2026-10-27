/* SPDX-License-Identifier: Apache-2.0 */
#ifndef UVK5_APRS_FRAME_H
#define UVK5_APRS_FRAME_H
#include <stdint.h>
#define APRS_FRAME_MAX 192
uint16_t APRS_FCS(const uint8_t *data,uint16_t length);
#endif
