/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_SESSION_H
#define UVK5_APRS_SESSION_H
#include <stdint.h>
#include <stdbool.h>
#include "driver/keyboard.h"
#define APRS_HUB_ITEMS 3
typedef struct {
    char callsign[10];
    int32_t latitude,longitude;
    bool positionSet;
} APRS_Profile;
extern APRS_Profile gAPRS_Profile;
extern uint8_t gAPRS_HubSelected;
bool SESSION_SetCall(const char *call);
bool SESSION_SetPosition(int32_t latitude,int32_t longitude);
bool SESSION_SetQuick(unsigned slot,const char *text);
void SESSION_Open(void);
void SESSION_Key(KEY_Code_t key);
void SESSION_Draw(void);
#endif
