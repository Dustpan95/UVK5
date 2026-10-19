/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_MESSAGE_H
#define UVK5_APRS_MESSAGE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "driver/keyboard.h"
#define APRS_QUICK_COUNT 3
#define APRS_MESSAGE_MAX 67
/* Capacities and multi-tap timing below are prototype values, not release limits. */
typedef enum {
    MESSAGE_CLOSED, MESSAGE_RECIPIENT, MESSAGE_COMPOSE, MESSAGE_OPTIONS,
    MESSAGE_QUICK, MESSAGE_REVIEW, MESSAGE_STATUS, MESSAGE_CALLSIGN
} MessagePage;
typedef enum {
    MESSAGE_DRAFT, MESSAGE_TX_UNAVAILABLE, MESSAGE_WAIT_ACK,
    MESSAGE_ACKED, MESSAGE_REJECTED
} MessageDelivery;
typedef enum { MESSAGE_OK, MESSAGE_BAD_CALL, MESSAGE_EMPTY, MESSAGE_FULL } MessageError;
typedef struct {
    char recipient[10], text[APRS_MESSAGE_MAX+1], sender[10], id[6], editCall[10];
    MessagePage page;
    MessageDelivery delivery;
    MessageError error;
    uint8_t selected, tapKey, tapIndex, tapTicks;
} APRS_Message;
extern APRS_Message gAPRS_Message;
extern char gAPRS_Quick[APRS_QUICK_COUNT][APRS_MESSAGE_MAX+1];
bool MESSAGE_ValidCall(const char *call);
void MESSAGE_EditCall(void);
void MESSAGE_Open(const char *replyTo);
void MESSAGE_Key(KEY_Code_t key);
void MESSAGE_Tick500ms(void);
/* An actual successful modem handoff must call this, never the Send button alone. */
bool MESSAGE_MarkSent(const char *sender, const char *id);
bool MESSAGE_Acknowledge(const char *source, const char *addressee,
                         const char *id, bool rejected);
/* Build an AX.25 UI body, without FCS. No relay path is assumed here. */
size_t MESSAGE_BuildFrame(const char *sender, const char *recipient,
                         const char *text, const char *id, uint8_t *out, size_t capacity);
size_t MESSAGE_BuildBeacon(const char *sender, int32_t latitude, int32_t longitude,
                           char table, char symbol, uint8_t *out, size_t capacity);
void MESSAGE_Draw(void);
#endif
