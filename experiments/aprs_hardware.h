/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_HARDWARE_H
#define UVK5_APRS_HARDWARE_H
#include "aprs_channel.h"
#include "aprs_radio_owner.h"
#include "radio.h"
typedef struct {
    APRS_RadioConfig radio;
    uint8_t power,index;
    VFO_Info_t voice;
    VfoState_t voiceState;
    bool leased,rxActive;
} APRS_Hardware;
/* Actual original-MCU adapter. Normal firmware binding is still separate.
 * Foreground only. Caller zero-initializes state and explicitly supplies radio
 * enable, calibrated tone gain and power. The dispatcher must restore before
 * keys, serial traffic or any settings save and suppress unrelated tuning while
 * leased. No EEPROM write is performed by this adapter.
 */
APRS_ChannelAdmission HARDWARE_Enter(uint32_t frequency,void *context);
APRS_LinkResult HARDWARE_Send(const uint8_t *frame,size_t length,uint32_t frequency,void *context);
void HARDWARE_Restore(void *context);
bool HARDWARE_Interrupted(void *context);
uint32_t HARDWARE_Now(void *context);
#endif
