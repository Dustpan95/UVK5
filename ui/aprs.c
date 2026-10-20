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

#ifdef ENABLE_APRS_RX_DIAGNOSTICS
#include "radio.h"
/* Diagnostic-only literals stay outside DISPLAY_TEXT so they do not enlarge
 * the complete release profile's immutable resource bundle. */
static void DrawDiagnostics(void)
{
    APRS_RxDiagnostics d;char line[48];
    APRS_GetDiagnostics(&d);UI_DisplayClear();
    if(gAPRS_DiagnosticPage==0) {
        UI_PrintLine("APRS RX DIAG D3",0);
        sprintf(line,"EN%u RD%u AR%u CP%u",gAPRS_Enabled,d.ready,d.armed,d.capturing);UI_PrintLine(line,1);
        sprintf(line,"S%u F%u E%u",d.sync,d.fifo,d.end);UI_DisplaySmallest(line,0,16,false,true);
        sprintf(line,"%c %s B%u L%u",'A'+d.mode,d.frozen?"HELD":"COLLECT",d.bytes,d.lastBytes);UI_DisplaySmallest(line,0,24,false,true);
        sprintf(line,"OK%u BAD%u TO%u",d.good,d.bad,d.timeout);UI_DisplaySmallest(line,0,32,false,true);
        sprintf(line,"FL%u FR%u CRC%u",d.flags,d.frames,d.crc);UI_DisplaySmallest(line,0,40,false,true);
        UI_PrintLine("1:CLR 2:MODE MENU",6);
    } else {
        UI_PrintLine("APRS REG / RAW",0);
        sprintf(line,"58=%05u 59=%05u",d.reg58,d.reg59);UI_PrintLine(line,1);
        sprintf(line,"3F%05u I%05u",d.reg3f,d.lastIrq);UI_DisplaySmallest(line,0,16,false,true);
        sprintf(line,"%u.%05u MHz",(unsigned)(gRxVfo->pRX->Frequency/100000u),
                (unsigned)(gRxVfo->pRX->Frequency%100000u));UI_DisplaySmallest(line,0,24,false,true);
        for(unsigned row=0;row<2;row++) {
            static const char hex[]="0123456789ABCDEF";
            memcpy(line,"RAW ",4);
            for(unsigned i=0;i<4;i++) {
                unsigned value=d.prefix[row*4+i];line[4+i*2]=hex[value>>4];line[5+i*2]=hex[value&15];
            }
            line[12]=0;UI_PrintLine(line,row+4);
        }
        UI_PrintLine("1:CLR 2:MODE MENU",6);
    }
    ST7565_BlitFullScreen();
}
#endif

void APRS_FormatCoordinate(char *out, int32_t value, char positive, char negative)
{
    const uint32_t abs = value < 0 ? 0u-(uint32_t)value : (uint32_t)value;
    unsigned n=UI_FormatFrequency(out,abs/10u,false);
    out[n]=value<0 ? negative:positive;out[n+1]=0;
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
#ifdef ENABLE_APRS_RX_DIAGNOSTICS
    if(gAPRS_DiagnosticPage<2) {DrawDiagnostics();return;}
#endif
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
