/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_SESSION_H
#define UVK5_APRS_SESSION_H
#include "aprs_decode.h"
#include "driver/keyboard.h"
#define APRS_INBOX_CAPACITY 4 /* Measured prototype capacity; not a release limit. */
typedef enum { APRS_HUB, APRS_PERSONAL, APRS_PERSONAL_DETAIL, APRS_MONITOR } APRS_View;
typedef struct {
    char callsign[10];
    int32_t latitude, longitude;
    bool positionSet;
} APRS_Profile;
extern APRS_Profile gAPRS_Profile;
extern APRS_Record gAPRS_Inbox[APRS_INBOX_CAPACITY];
extern uint8_t gAPRS_InboxCount, gAPRS_Unread, gAPRS_InboxSelected, gAPRS_HubSelected;
extern APRS_View gAPRS_View;
bool SESSION_SetCall(const char *call);
bool SESSION_SetPosition(int32_t latitude, int32_t longitude);
bool SESSION_SetQuick(unsigned slot, const char *text);
bool SESSION_IsPersonal(const APRS_Record *record);
void SESSION_Receive(const APRS_Record *record);
void SESSION_Open(void);
void SESSION_Key(KEY_Code_t key);
void SESSION_Draw(void);
#endif
