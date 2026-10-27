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
/* Modified for UVK5: only the shared transmit-frame checksum remains. */
#include "aprs_frame.h"
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

