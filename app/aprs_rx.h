/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef APRS_RX_H
#define APRS_RX_H
#include "aprs_decode.h"
#include "driver/keyboard.h"
extern bool gAPRS_Enabled, gAPRS_Browser, gAPRS_AlertSound;
extern uint8_t gAPRS_BannerTicks;
extern char gAPRS_BannerSource[10], gAPRS_BannerText[18];
extern APRS_Record gAPRS_Log[APRS_LOG_CAPACITY];
extern uint8_t gAPRS_Count, gAPRS_Selected;
bool APRS_CanListen(void);
bool APRS_IsCapturing(void);
void APRS_Stop(void);
void APRS_Tick500ms(void);
void APRS_Interrupt(uint16_t bits);
void APRS_Open(void);
bool APRS_Key(KEY_Code_t key, bool pressed, bool held);
void APRS_Draw(void);
void APRS_FormatCoordinate(char *out,int32_t value,char positive,char negative);
void APRS_DrawRecord(const APRS_Record *record);
#endif
