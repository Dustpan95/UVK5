/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_SETUP_H
#define UVK5_APRS_SETUP_H
#include "driver/keyboard.h"
#include <stdbool.h>
#include <stdint.h>
typedef enum { SETUP_CLOSED,SETUP_HOME,SETUP_LATITUDE,SETUP_LONGITUDE,SETUP_POSITION_REVIEW,
    SETUP_QUICK,SETUP_DATA,SETUP_FREQUENCY,SETUP_AUDIO,SETUP_WAIT,SETUP_BEACON } APRS_SetupPage;
typedef struct {
    APRS_SetupPage page;
    uint8_t selected,length;
    char digits[9];
    int32_t latitude,longitude;
    bool negative,error;
} APRS_Setup;
extern APRS_Setup gAPRS_Setup;
/* DDMMmm / DDDMMmm, exact hundredths of a minute; no decimal degrees entry. */
bool SETUP_ParseDM(const char *digits,bool longitude,bool negative,int32_t *value);
void SETUP_Open(void);
void SETUP_Key(KEY_Code_t key);
void SETUP_Draw(void);
#endif
