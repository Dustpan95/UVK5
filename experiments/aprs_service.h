/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_SERVICE_H
#define UVK5_APRS_SERVICE_H
#include "app/aprs_tx.h"
typedef struct {uint32_t frequency,lifetimeTicks;bool enabled;} APRS_ServiceSettings;
bool SERVICE_Configure(const APRS_ServiceSettings *settings);
bool SERVICE_Enabled(void);
bool SERVICE_Message(const char *recipient,const char *text,const char *id,uint32_t now);
bool SERVICE_Beacon(uint32_t now);
void SERVICE_Process(const APRS_TX_Link *link);
#endif
