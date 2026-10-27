/* SPDX-License-Identifier: GPL-3.0-only
 * Approved messaging flow; initial physical-key implementation for measurement.
 */
#include "ui/display_text.h"
#include "app/aprs_message.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
#include <string.h>
#ifdef ENABLE_APRS_TRANSPORT
#include "app/aprs_tx.h"
#endif
#ifndef ENABLE_APRS_CALLSIGN_ONLY
static void Body(const char *text, unsigned first)
{
    unsigned length=strlen(text);
    for(unsigned row=0;row<4 && row*17<length;row++) {
        char line[18]; unsigned n=length-row*17;
        if(n>17) n=17;
        memcpy(line,text+row*17,n);line[n]=0;
        UI_PrintLine(line,first+row);
    }
}
#endif
void MESSAGE_Draw(void)
{
    const APRS_Message *m=&gAPRS_Message;
#ifndef ENABLE_APRS_CALLSIGN_ONLY
    char line[22];
#endif
    UI_DisplayClear();
    const char *footer=DISPLAY_TEXT("MENU:NEXT  EXIT:BACK");
#ifdef ENABLE_APRS_CALLSIGN_ONLY
    {
        UI_PrintLine(DISPLAY_TEXT("MY CALLSIGN"),0);
        UI_PrintLine(m->editCall,2);
        UI_PrintLine(DISPLAY_TEXT("MULTI-TAP ABC/123"),3);
        UI_PrintLine(DISPLAY_TEXT("1:.-/  #:NEXT"),4);
        UI_PrintLine(DISPLAY_TEXT("*:DELETE"),5);
        footer=DISPLAY_TEXT("MENU:SAVE  EXIT:CANCEL");
    }
#else
    if(m->page==MESSAGE_RECIPIENT || m->page==MESSAGE_CALLSIGN) {
        UI_PrintLine(m->page==MESSAGE_CALLSIGN ? DISPLAY_TEXT("MY CALLSIGN") : DISPLAY_TEXT("RECIPIENT"),0);
        UI_PrintLine(m->page==MESSAGE_CALLSIGN ? m->editCall : m->recipient,2);
        UI_PrintLine(DISPLAY_TEXT("MULTI-TAP ABC/123"),3);
        UI_PrintLine(DISPLAY_TEXT("1:.-/  #:NEXT"),4);
        UI_PrintLine(DISPLAY_TEXT("*:DELETE"),5);
        if(m->page==MESSAGE_CALLSIGN) footer=DISPLAY_TEXT("MENU:SAVE  EXIT:CANCEL");
    }
    else if(m->page==MESSAGE_COMPOSE || m->page==MESSAGE_REVIEW
#ifdef ENABLE_APRS_TRANSPORT
        || m->page==MESSAGE_QUICK_EDIT
#endif
    ) {
        sprintf(line,DISPLAY_TEXT("%s %s"),m->page==MESSAGE_REVIEW ? DISPLAY_TEXT("SEND") : DISPLAY_TEXT("TO"),m->recipient);
        UI_PrintLine(line,0);
        const char *text=m->text;
#ifdef ENABLE_APRS_TRANSPORT
        if(m->page==MESSAGE_QUICK_EDIT) {text=MESSAGE_QuickEditText();UI_PrintLine(DISPLAY_TEXT("EDIT QUICK MESSAGE"),0);}
#endif
        Body(text,1);
        sprintf(line,DISPLAY_TEXT("%u/67%s"),(unsigned)strlen(text),m->tapTicks ? DISPLAY_TEXT(" ABC_") : DISPLAY_TEXT(" ABC"));
        UI_DisplaySmallest(line,0,42,false,true);
        footer=m->page==MESSAGE_REVIEW ? DISPLAY_TEXT("MENU:SEND  EXIT:EDIT") : DISPLAY_TEXT("MENU:OPTIONS  EXIT:BACK");
#ifdef ENABLE_APRS_TRANSPORT
        if(m->page==MESSAGE_QUICK_EDIT) footer=DISPLAY_TEXT("MENU:SAVE EXIT:CANCEL");
#endif
    } else if(m->page==MESSAGE_OPTIONS || m->page==MESSAGE_QUICK) {
        const bool quick=m->page==MESSAGE_QUICK;
        UI_PrintLine(quick ? DISPLAY_TEXT("QUICK MESSAGES") : DISPLAY_TEXT("MESSAGE OPTIONS"),0);
        static const char *const options[]={DISPLAY_TEXT("QUICK MESSAGE"),DISPLAY_TEXT("REVIEW AND SEND"),DISPLAY_TEXT("EDIT RECIPIENT")};
        for(unsigned i=0;i<3;i++) {
            sprintf(line,DISPLAY_TEXT("%c%.16s"),i==m->selected ? '>' : ' ',quick ? gAPRS_Quick[i] : options[i]);
            UI_PrintLine(line,2+i);
        }
        footer=DISPLAY_TEXT("MENU:SELECT  EXIT:BODY");
    } else {
#ifdef ENABLE_APRS_TRANSPORT
        static const char *const states[]={DISPLAY_TEXT("IDLE"),DISPLAY_TEXT("QUEUED"),DISPLAY_TEXT("TX SENT"),DISPLAY_TEXT("TX BLOCKED"),DISPLAY_TEXT("TX FAILED"),DISPLAY_TEXT("CANCELLED"),DISPLAY_TEXT("EXPIRED")};
        sprintf(line,gAPRS_TX.message ? DISPLAY_TEXT("TO %s") : DISPLAY_TEXT("STATIC BEACON"),gAPRS_TX.recipient);UI_PrintLine(line,0);
        UI_PrintLine(states[gAPRS_TX.state],2);
        if(gAPRS_TX.state==APRS_TX_SENT) UI_PrintLine(DISPLAY_TEXT("DELIVERY UNKNOWN"),3);
        sprintf(line,DISPLAY_TEXT("ATTEMPTS %u/%u"),gAPRS_TX.attempts,gAPRS_TX.policy.maxAttempts);UI_PrintLine(line,4);
        footer=gAPRS_TX.state==APRS_TX_QUEUED ? DISPLAY_TEXT("MENU:CANCEL EXIT:BACK") : DISPLAY_TEXT("EXIT:BACK");
#else
        sprintf(line,DISPLAY_TEXT("TO %s"),m->recipient);UI_PrintLine(line,0);
        const char *status=DISPLAY_TEXT("NOT SENT");
        UI_PrintLine(status,2);
        if(m->delivery==MESSAGE_TX_UNAVAILABLE)
            UI_PrintLine(DISPLAY_TEXT("TX UNAVAILABLE"),4);
        footer=DISPLAY_TEXT("EXIT:BACK");
#endif
    }
#endif
    if(m->error!=MESSAGE_OK) {
#ifdef ENABLE_APRS_CALLSIGN_ONLY
        const char *error=m->error==MESSAGE_BAD_CALL ? DISPLAY_TEXT("CHECK CALL/SSID") : DISPLAY_TEXT("CALLSIGN FULL");
#else
        const char *error=m->error==MESSAGE_BAD_CALL ? DISPLAY_TEXT("CHECK CALL/SSID") :
                          m->error==MESSAGE_EMPTY ? DISPLAY_TEXT("EMPTY MESSAGE") : m->error==MESSAGE_TX_BLOCKED ? DISPLAY_TEXT("CHECK TX SETUP / BUSY") : DISPLAY_TEXT("MESSAGE FULL");
#endif
        UI_DisplaySmallest(error,0,42,false,true);
    }
    UI_DisplaySmallest(footer,0,50,false,true);
    ST7565_BlitFullScreen();
}
