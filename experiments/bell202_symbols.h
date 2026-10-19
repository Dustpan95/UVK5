/* SPDX-License-Identifier: GPL-3.0-only
 * UVK5 experimental protocol encoder; not linked into radio firmware.
 */
#ifndef UVK5_BELL202_SYMBOLS_H
#define UVK5_BELL202_SYMBOLS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Callback receives one 1200-baud symbol: false=1200 Hz, true=2200 Hz.
 * Hardware output must schedule absolute deadlines, not delay after SPI writes.
 * Input is an AX.25 frame without FCS. Caller validates addressing and TX policy.
 */
bool BELL202_Encode(const uint8_t *frame, size_t length,
                    void (*symbol)(bool, void *), void *context);
#endif
