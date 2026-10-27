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

#include "gain_staging.h"
#include "am_fix.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "signal_classifier.h"
#include "app/chFrScanner.h"   // VUURWERK v1.2.8: gScanStateDir for the scan guard

GainStaging_t gGainStaging[2];

// Frequency change detection (per-VFO, like AM_fix)
static uint32_t last_frequency[2] = {0, 0};

// Target RSSI in raw BK4819 units: (dBm + 160) * 2
// FM: -75 dBm target (FM limiter has headroom, run higher gain)
#define FM_TARGET_RSSI  ((int16_t)((-75 + 160) * 2))

// v1.2.7: statified (zero external callers).
static void GAIN_STAGING_Reset(uint8_t vfo)
{
    if (vfo > 1) return;
    gGainStaging[vfo].table_index      = 0;  // Back to stock gain
    gGainStaging[vfo].table_index_prev = 0;
    gGainStaging[vfo].prev_rssi        = 0;
    gGainStaging[vfo].hold_counter     = 0;
}

void GAIN_STAGING_10ms(uint8_t vfo)
{
    if (vfo > 1) return;

    // === GUARD: FM mode only -- AM_fix owns REG_13 in AM/USB ===
    if (gRxVfo->Modulation != MODULATION_FM)
        return;


    // === VUURWERK v1.2.8: yield to the BK4819 hardware AGC during scan ===
    // The per-hop freq-change reset plus the release off-by-one would pin
    // REG_13 (the auto-AGC max-gain slot) to gain_table[1] = 0x0000 (-93 dB)
    // for most of each ~90 ms dwell, starving weak signals so the post-gain
    // RSSI never crosses the squelch threshold -- "goes deaf once I start
    // scanning". Restore the stock REG_13 baseline once and let the hardware
    // AGC own gain while scanning, matching stock egzumer scan behaviour.
    // Covers both the frequency-sweep and memory-channel scan paths.
    if (gScanStateDir != SCAN_OFF) {
        if (last_frequency[vfo] != 0) {     // run once on scan entry
            last_frequency[vfo] = 0;        // force a clean reset when scan ends
            GAIN_STAGING_Reset(vfo);
            BK4819_WriteRegister(BK4819_REG_13, gain_table[0].reg_val);
        }
        return;
    }

    // === GUARD: Only during FOREGROUND (idle) and RX -- skip TX and power save ===
    if (gCurrentFunction != FUNCTION_FOREGROUND && !FUNCTION_IsRx())
        return;

    // === FREQUENCY CHANGE DETECTION (like AM_fix) ===
    const uint32_t freq = gEeprom.VfoInfo[vfo].pRX->Frequency;
    if (freq != last_frequency[vfo]) {
        last_frequency[vfo] = freq;
        GAIN_STAGING_Reset(vfo);
        // Write stock gain immediately so the new channel starts clean
        BK4819_WriteRegister(BK4819_REG_13, gain_table[0].reg_val);
        return;  // Let next tick start fresh
    }

    // === READ RSSI ===
    const int16_t new_rssi = (int16_t)BK4819_GetRSSI();
    GainStaging_t *gs = &gGainStaging[vfo];

    // === VUURWERK v1.3.0 (F2): REG_13 refresh heartbeat ===
    // REG_13 is one physical register but this module gates writes on a
    // per-VFO index cache, so a dual-watch flip (RADIO_SetupAGC cache
    // skips FM->FM) or a spectrum-exit register edit leaves REG_13
    // carrying someone else's gain while we believe ours is applied.
    // Once a second, invalidate the cache so the write-gate below
    // re-asserts this VFO's gain. Sits after all guards: never fires
    // during TX, AM (am_fix owns REG_13), scan, or power save.
    static uint8_t hb_ticks;
    if (++hb_ticks >= 100) {
        hb_ticks = 0;
        gs->table_index_prev = 0xFF;   // out-of-range sentinel forces rewrite
    }

    // Average with previous reading (spike immunity)
    int16_t rssi = (gs->prev_rssi > 0) ? (gs->prev_rssi + new_rssi) / 2 : new_rssi;
    gs->prev_rssi = new_rssi;

    // === UPDATE HOLD COUNTER ===
    if (gs->hold_counter > 0)
        gs->hold_counter--;

    // === COMPUTE ERROR ===
    // diff_dB: positive = signal too strong, negative = too weak
    int16_t diff_dB = (rssi - FM_TARGET_RSSI) / 2;

    if (diff_dB > 0) {
        // Signal too strong -- DECREASE gain (fast attack)
        unsigned int index = gs->table_index;

        // VUURWERK v1.3.0 (F1): index 0 is the stock -7dB reference, not
        // the bottom rung. Enter the ladder at rung 39 (-6dB) before
        // stepping; without this, a small overshoot at stock gain slammed
        // the front end to gain_table[1] = -93dB and dropped the signal.
        if (index == 0) index = GAIN_TABLE_RUNG_ABOVE_STOCK;

        if (diff_dB >= 10) {
            // Large overshoot: jump aggressively
            const int16_t desired_gain_dB = (int16_t)gain_table[index].gain_dB - diff_dB + 8;
            while (index > 1)
                if (gain_table[--index].gain_dB <= desired_gain_dB)
                    break;
        } else {
            // Small overshoot: single step down
            if (index > 1)
                index--;
        }

        if (index < 1) index = 1;

        if (gs->table_index != index) {
            gs->table_index = index;
            // Adaptive hold time based on signal classification
            SignalClass_t sc = SIGNAL_CLASSIFIER_GetClass(vfo);
            if (sc == SIGNAL_CLASS_FAST)
                gs->hold_counter = 15;    // 150ms for FM/digital
            else if (sc == SIGNAL_CLASS_SLOW)
                gs->hold_counter = 50;    // 500ms for CW/carrier
            else
                gs->hold_counter = 30;    // 300ms default
        }
    }

    // 6dB hysteresis band: don't release hold until signal drops well below target
    if (diff_dB >= -6)
        gs->hold_counter = 30;

    if (gs->hold_counter == 0) {
        // Hold expired -- free to INCREASE gain (slow release)
        // This is the critical path: when squelch is closed and no signal,
        // gain ramps UP one step at a time until either:
        //   (a) a signal is found (squelch opens, then we pull down if needed)
        //   (b) we reach max gain (table_index = gain_table_size-1)
        // VUURWERK v1.3.0 (F1): releasing from index 0 (stock -7dB) must
        // step to rung 39 (-6dB), not to gain_table[1] = -93dB. The old
        // +1 caused a ~0.4s deaf dip (climb from -93dB at one rung per
        // tick) after every parked FM channel change or VFO flip.
        unsigned int index = (gs->table_index == 0)
            ? GAIN_TABLE_RUNG_ABOVE_STOCK : gs->table_index + 1u;
        if (index >= gain_table_size)
            index = gain_table_size - 1;
        gs->table_index = index;
    }

    // === APPLY TO REG_13 ===
    // Write only when the index has actually changed (minimize SPI bus traffic)
    if (gs->table_index != gs->table_index_prev) {
        BK4819_WriteRegister(BK4819_REG_13, gain_table[gs->table_index].reg_val);
        gs->table_index_prev = gs->table_index;
    }
}

int8_t GAIN_STAGING_GetGainDiff(uint8_t vfo)
{
    if (vfo > 1) return 0;

    // Same formula as AM_fix: original_dB - current_dB
    // Positive = gain was reduced, add back to RSSI for true signal strength
    return gain_table[0].gain_dB - gain_table[gGainStaging[vfo].table_index].gain_dB;
}
