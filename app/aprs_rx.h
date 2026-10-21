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
/* Re-arm eligible receive immediately, without advancing timers or alerts. */
void APRS_Resume(void);
void APRS_Tick500ms(void);
void APRS_Interrupt(uint16_t bits);
void APRS_Open(void);
bool APRS_Key(KEY_Code_t key, bool pressed, bool held);
void APRS_Draw(void);
void APRS_FormatCoordinate(char *out,int32_t value,char positive,char negative);
void APRS_DrawRecord(const APRS_Record *record);
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
typedef struct {
    uint16_t sync, fifo, end, timeout, good, bad, lastBytes, full;
    uint16_t flags, frames, crc, lastIrq, bytes, reg58, reg59, reg3f, reg43, reg13, reg7e;
    uint8_t prefix[8], mode;
    bool ready, armed, capturing, frozen;
} APRS_RxDiagnostics;
extern APRS_RxDiagnostics gAPRS_RxDiagnostics;
extern uint8_t gAPRS_DiagnosticPage;
void APRS_GetDiagnostics(APRS_RxDiagnostics *out);
bool APRS_UsesReferenceGain(void);
/* Frozen diagnostic ring only; no EEPROM, RF or general memory access. */
uint8_t APRS_ReadDiagnosticCapture(uint16_t offset,uint8_t size,uint8_t *out);
#endif
#endif
