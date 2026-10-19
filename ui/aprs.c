/* SPDX-License-Identifier: GPL-3.0-only
 * UVK5 passive packet browser. No external screen or controller required.
 */
#include "app/aprs_rx.h"
#ifdef ENABLE_APRS_MESSAGING
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
    sprintf(out, "%lu.%05lu%c", (unsigned long)(abs / 1000000),
            (unsigned long)((abs % 1000000) / 10), value < 0 ? negative : positive);
}

void APRS_DrawRecord(const APRS_Record *r)
{
    char line[22];

    if (r->kind == APRS_MESSAGE || r->kind == APRS_ACK || r->kind == APRS_REJECT) {
        sprintf(line, "%s>%s", r->source, r->destination);
        UI_DisplaySmallest(line, 0, 8, false, true);
    } else UI_PrintString(r->source, 0, LCD_WIDTH, 1, 8);
    if (r->kind == APRS_POSITION) {
        APRS_FormatCoordinate(line, r->latitude, 'N', 'S');
        UI_PrintStringSmallNormal(line, 0, 0, 3);
        APRS_FormatCoordinate(line, r->longitude, 'E', 'W');
        UI_PrintStringSmallNormal(line, 0, 0, 4);
    } else {
        /* Four rows of 17 characters fit all 67 APRS message characters. */
        const unsigned len = strlen(r->text);
        for (unsigned row = 0; row < 4 && row * 17 < len; ++row) {
            unsigned n = len - row * 17;
            if (n > 17) n = 17;
            memcpy(line, r->text + row * 17, n);
            line[n] = 0;
            UI_PrintStringSmallNormal(line, 0, 0, row + 2);
        }
    }

}

void APRS_Draw(void)
{
#ifdef ENABLE_APRS_MESSAGING
    if (gAPRS_Message.page != MESSAGE_CLOSED) { MESSAGE_Draw(); return; }
    if (gAPRS_View != APRS_MONITOR) { SESSION_Draw(); return; }
#endif
    char line[22];
    UI_DisplayClear();
#ifdef ENABLE_APRS_MESSAGING
    sprintf(line, "MONITOR %u/%u", gAPRS_Count ? gAPRS_Selected+1 : 0, gAPRS_Count);
#else
    sprintf(line, "APRS %s %u/%u", !gAPRS_Enabled ? "OFF" : APRS_CanListen() ? "RX" : "PAUSE",
            gAPRS_Count ? gAPRS_Selected + 1 : 0, gAPRS_Count);
#endif
    UI_PrintStringSmallNormal(line, 0, 0, 0);
    if (!gAPRS_Count) {
        UI_PrintStringSmallNormal("No packets yet", 0, 0, 2);
        UI_PrintStringSmallNormal("FM, scan/DW off", 0, 0, 4);
    } else APRS_DrawRecord(&gAPRS_Log[gAPRS_Selected]);

#ifdef ENABLE_APRS_MESSAGING
    UI_DisplaySmallest("MENU:REPLY/NEW  EXIT:APRS", 0, 50, false, true);
#else
    UI_DisplaySmallest("UP/DN:PACKET  EXIT:RADIO", 0, 50, false, true);
#endif
    ST7565_BlitFullScreen();
}
