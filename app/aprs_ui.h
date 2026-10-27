/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_UI_H
#define UVK5_APRS_UI_H
#include <stdbool.h>
#include "driver/keyboard.h"
extern bool gAPRS_Browser;
void APRS_Open(void);
bool APRS_Key(KEY_Code_t key,bool pressed,bool held);
void APRS_Tick500ms(void);
void APRS_Draw(void);
#endif
