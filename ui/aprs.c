/* SPDX-License-Identifier: GPL-3.0-only
 * UVK5 passive packet browser. No external screen or controller required.
 */
#include "ui/display_text.h"
#include "app/aprs_rx.h"
#ifdef ENABLE_APRS_TRANSPORT
#include "app/aprs_setup.h"
#endif
#ifdef ENABLE_APRS_SESSION
#include "app/aprs_message.h"
#include "app/aprs_session.h"
#endif
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
#include <string.h>

void APRS_FormatCoordinate(char *out, int32_t value, char positive, char negative)
{
    const uint32_t abs = value < 0 ? (uint32_t)-value : (uint32_t)value;
    sprintf(out, DISPLAY_TEXT("%lu.%05lu%c"), (unsigned long)(abs / 1000000),
            (unsigned long)((abs % 1000000) / 10), value < 0 ? negative : positive);
}

void APRS_DrawRecord(const APRS_Record *r)
{
    char line[22];

    if (r->kind == APRS_MESSAGE || r->kind == APRS_ACK || r->kind == APRS_REJECT) {
        sprintf(line, DISPLAY_TEXT("%s>%s"), r->source, r->destination);
        UI_DisplaySmallest(line, 0, 8, false, true);
    } else UI_PrintString(r->source, 0, LCD_WIDTH, 1, 8);
    if (r->kind == APRS_POSITION) {
        APRS_FormatCoordinate(line, r->latitude, 'N', 'S');
        UI_PrintLine(line,3);
        APRS_FormatCoordinate(line, r->longitude, 'E', 'W');
        UI_PrintLine(line,4);
    } else {
        /* Four rows of 17 characters fit all 67 APRS message characters. */
        const unsigned len = strlen(r->text);
        for (unsigned row = 0; row < 4 && row * 17 < len; ++row) {
            unsigned n = len - row * 17;
            if (n > 17) n = 17;
            memcpy(line, r->text + row * 17, n);
            line[n] = 0;
            UI_PrintLine(line,row + 2);
        }
    }

}

void APRS_Draw(void)
{
#ifdef ENABLE_APRS_SESSION
    if (gAPRS_Message.page != MESSAGE_CLOSED) { MESSAGE_Draw(); return; }
#ifdef ENABLE_APRS_TRANSPORT
    if(gAPRS_Setup.page!=SETUP_CLOSED) {SETUP_Draw();return;}
#endif
    if (gAPRS_View != APRS_MONITOR) { SESSION_Draw(); return; }
#endif
    char line[22];
    UI_DisplayClear();
#ifdef ENABLE_APRS_SESSION
    sprintf(line, DISPLAY_TEXT("MONITOR %u/%u"), gAPRS_Count ? gAPRS_Selected+1 : 0, gAPRS_Count);
#else
    sprintf(line, DISPLAY_TEXT("APRS %s %u/%u"), !gAPRS_Enabled ? DISPLAY_TEXT("OFF") : APRS_CanListen() ? DISPLAY_TEXT("RX") : DISPLAY_TEXT("PAUSE"),
            gAPRS_Count ? gAPRS_Selected + 1 : 0, gAPRS_Count);
#endif
    UI_PrintLine(line,0);
    if (!gAPRS_Count) {
        UI_PrintLine(DISPLAY_TEXT("No packets yet"),2);
        UI_PrintLine(DISPLAY_TEXT("FM, scan/DW off"),4);
    } else APRS_DrawRecord(&gAPRS_Log[gAPRS_Selected]);

#if defined(ENABLE_APRS_SESSION) && !defined(ENABLE_APRS_CALLSIGN_ONLY)
    UI_DisplaySmallest(DISPLAY_TEXT("MENU:REPLY/NEW  EXIT:APRS"), 0, 50, false, true);
#elif defined(ENABLE_APRS_SESSION)
    UI_DisplaySmallest(DISPLAY_TEXT("UP/DN:PACKET  EXIT:APRS"), 0, 50, false, true);
#else
    UI_DisplaySmallest(DISPLAY_TEXT("UP/DN:PACKET  EXIT:RADIO"), 0, 50, false, true);
#endif
    ST7565_BlitFullScreen();
}
