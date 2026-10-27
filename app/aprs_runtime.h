/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_RUNTIME_H
#define UVK5_APRS_RUNTIME_H
#include "radio.h"
typedef struct {uint32_t frequency;uint8_t audioLevel;bool audioSet;} APRS_RuntimeSettings;
extern APRS_RuntimeSettings gAPRS_Runtime;
bool RUNTIME_Configure(uint32_t frequency,uint8_t level);
bool RUNTIME_Enable(bool enabled);
bool RUNTIME_Enabled(void);
void RUNTIME_Process(void);
void RUNTIME_Interrupt(void);
bool RUNTIME_Leased(void);
const VFO_Info_t *RUNTIME_VoiceVfo(unsigned index);
bool RUNTIME_SendDraft(void);
bool RUNTIME_Beacon(void);
void RUNTIME_Cancel(void);
#endif
