/* Copyright (c) 2026 James Honiball (KC3TFZ)
 *
 * This file is part of VUURWERK and is dual-licensed:
 *   1. GPL v3 (when distributed as part of the VUURWERK firmware)
 *   2. Commercial license available from the author
 *
 * You may not extract, repackage, or redistribute this file
 * independently under any license other than GPL v3 as part
 * of the complete VUURWERK firmware, without written permission
 * from the author.
 *
 * Commercial licensing inquiries: jhoniball4@gmail.com
 */

#include "signal_classifier.h"

SignalClassifier_t gSignalClassifier[2];

// v1.2.7: Init removed. BSS zero-init gives prev_rssi=0,
// rise_time_ms=0, stable_count=0, classification=SIGNAL_CLASS_NOISE (=0).
// Worst case is one extra NOISE classification on the first signal
// post-boot before the algorithm self-corrects to FAST / NORMAL / SLOW.

void SIGNAL_CLASSIFIER_Update(uint8_t vfo, int16_t rssi_dbm, uint32_t frequency) {
    if (vfo > 1) return;

    SignalClassifier_t *sc = &gSignalClassifier[vfo];

    // VUURWERK v1.3.0 (F8): discard prior-channel state on frequency
    // change, mirroring the v1.2.7 resets in signal_quality.c and
    // smart_squelch.c. Without this the F/N/S/~ glyph and the
    // gain_staging hold-class carried the previous channel's
    // classification into the new channel's first ticks.
    static uint32_t last_frequency[2];
    if (frequency != last_frequency[vfo]) {
        last_frequency[vfo] = frequency;
        sc->prev_rssi      = 0;
        sc->rise_time_ms   = 0;
        sc->stable_count   = 0;
        sc->classification = SIGNAL_CLASS_NOISE;
    }

    // Check if signal is rising
    if (rssi_dbm > sc->prev_rssi + 3) {
        // Rising edge detected
        if (sc->rise_time_ms == 0) {
            // Start timing
            sc->rise_time_ms = 10; // First 10ms sample
        } else if (sc->rise_time_ms < 500) {
            sc->rise_time_ms += 10; // Increment by 10ms
        }
    } else if (rssi_dbm > sc->prev_rssi - 3 && rssi_dbm < sc->prev_rssi + 3) {
        // Stable signal
        if (sc->rise_time_ms > 0) {
            // Rise complete, classify
            if (sc->rise_time_ms < 50) {
                sc->classification = SIGNAL_CLASS_FAST;
            } else if (sc->rise_time_ms < 200) {
                sc->classification = SIGNAL_CLASS_NORMAL;
            } else {
                sc->classification = SIGNAL_CLASS_SLOW;
            }

            if (sc->stable_count < 255) {
                sc->stable_count++;
            }
        }
    } else {
        // Falling or unstable
        if (sc->stable_count < 3) {
            sc->classification = SIGNAL_CLASS_NOISE;
        }
        sc->rise_time_ms = 0;
        if (sc->stable_count > 0) {
            sc->stable_count--;
        }
    }

    sc->prev_rssi = rssi_dbm;
}

SignalClass_t SIGNAL_CLASSIFIER_GetClass(uint8_t vfo) {
    if (vfo > 1) return SIGNAL_CLASS_NOISE;
    return gSignalClassifier[vfo].classification;
}

char SIGNAL_CLASSIFIER_GetSymbol(uint8_t vfo) {
    if (vfo > 1) return '~';

    switch (gSignalClassifier[vfo].classification) {
        case SIGNAL_CLASS_FAST:   return 'F';
        case SIGNAL_CLASS_NORMAL: return 'N';
        case SIGNAL_CLASS_SLOW:   return 'S';
        default:                  return '~';
    }
}