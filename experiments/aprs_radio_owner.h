/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_RADIO_OWNER_H
#define UVK5_APRS_RADIO_OWNER_H
#include "app/aprs_tx.h"
typedef struct {bool enabled;uint8_t toneGain;} APRS_RadioConfig;
/* Bench-only transport callback. Not linked into normal firmware.
 * Enabled must be an explicit operator choice; zero-initialization disables it.
 * Requires stationary simplex FM, no codes/scrambler/PTT ID, completed receive
 * interrupt processing and closed squelch. This is conservative admission,
 * not a physically validated carrier-sense modem. Gain needs RF calibration.
 */
APRS_LinkResult APRS_RadioSend(const uint8_t *frame,size_t length,void *config);
#endif
