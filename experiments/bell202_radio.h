/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_BELL202_RADIO_H
#define UVK5_BELL202_RADIO_H
#include "bell202_timing.h"
/* Bench-only backend, deliberately absent from the normal firmware Makefile.
 * Never keys a transmitter or selects a frequency. Caller must acquire the
 * normal, permitted FM TX path, complete the PA soft-start and arrange cleanup
 * to receive on EVERY return. Voice PTT must not be held. TX subaudible code
 * and scrambling must be off. gain is an explicit uncalibrated bench parameter.
 * Leaves TX muted on return; caller must disable PA before resuming audio.
 * A SENT result means symbol emission completed, not that RF was decoded.
 */
BELL202_Result BELL202_Radio(const uint8_t *frame,size_t length,uint8_t gain);
#endif
