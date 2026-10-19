/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_BELL202_TIMING_H
#define UVK5_BELL202_TIMING_H
#include "bell202_symbols.h"
/* Experimental foreground scheduler for a 48 MHz clock. Not connected to RF.
 * now() must be a monotonically increasing wrapping 32-bit cycle counter.
 * tone() writes only the tone-frequency register; radio lifecycle is external.
 * cancelled() must include physical PTT and the normal TX timeout.
 * Callbacks run with interrupts enabled. No relative delay follows SPI writes.
 */
typedef struct {
    uint32_t (*now)(void *);
    void (*tone)(bool high, void *);
    bool (*cancelled)(void *);
    void *context;
} BELL202_Output;
typedef enum {
    BELL202_SENT, BELL202_INVALID, BELL202_CANCELLED, BELL202_LATE,
    BELL202_WRITE_SLOW, BELL202_WRITE_JITTER, BELL202_TOO_LONG
} BELL202_Result;
BELL202_Result BELL202_Transmit(const uint8_t *frame, size_t length,
                                const BELL202_Output *output);
#endif
