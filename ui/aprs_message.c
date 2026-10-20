/* SPDX-License-Identifier: GPL-3.0-only
 * Approved messaging flow; initial physical-key implementation for measurement.
 */
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
    const char *footer="MENU:NEXT  EXIT:BACK";
#ifdef ENABLE_APRS_CALLSIGN_ONLY
    {
        UI_PrintLine("MY CALLSIGN",0);
        UI_PrintLine(m->editCall,2);
        UI_PrintLine("MULTI-TAP ABC/123",3);
        UI_PrintLine("1:.-/  #:NEXT",4);
        UI_PrintLine("*:DELETE",5);
        footer="MENU:SAVE  EXIT:CANCEL";
    }
#else
    if(m->page==MESSAGE_RECIPIENT || m->page==MESSAGE_CALLSIGN) {
        UI_PrintLine(m->page==MESSAGE_CALLSIGN ? "MY CALLSIGN" : "RECIPIENT",0);
        UI_PrintLine(m->page==MESSAGE_CALLSIGN ? m->editCall : m->recipient,2);
        UI_PrintLine("MULTI-TAP ABC/123",3);
        UI_PrintLine("1:.-/  #:NEXT",4);
        UI_PrintLine("*:DELETE",5);
        if(m->page==MESSAGE_CALLSIGN) footer="MENU:SAVE  EXIT:CANCEL";
    }
    else if(m->page==MESSAGE_COMPOSE || m->page==MESSAGE_REVIEW
#ifdef ENABLE_APRS_TRANSPORT
        || m->page==MESSAGE_QUICK_EDIT
#endif
    ) {
        sprintf(line,"%s %s",m->page==MESSAGE_REVIEW ? "SEND" : "TO",m->recipient);
        UI_PrintLine(line,0);
        const char *text=m->text;
#ifdef ENABLE_APRS_TRANSPORT
        if(m->page==MESSAGE_QUICK_EDIT) {text=MESSAGE_QuickEditText();UI_PrintLine("EDIT QUICK MESSAGE",0);}
#endif
        Body(text,1);
        sprintf(line,"%u/67%s",(unsigned)strlen(text),m->tapTicks ? " ABC_" : " ABC");
        UI_DisplaySmallest(line,0,42,false,true);
        footer=m->page==MESSAGE_REVIEW ? "MENU:SEND  EXIT:EDIT" : "MENU:OPTIONS  EXIT:BACK";
#ifdef ENABLE_APRS_TRANSPORT
        if(m->page==MESSAGE_QUICK_EDIT) footer="MENU:SAVE EXIT:CANCEL";
#endif
    } else if(m->page==MESSAGE_OPTIONS || m->page==MESSAGE_QUICK) {
        const bool quick=m->page==MESSAGE_QUICK;
        UI_PrintLine(quick ? "QUICK MESSAGES" : "MESSAGE OPTIONS",0);
        const char *const options[]={"QUICK MESSAGE","REVIEW AND SEND","EDIT RECIPIENT"};
        for(unsigned i=0;i<3;i++) {
            sprintf(line,"%c%.16s",i==m->selected ? '>' : ' ',quick ? gAPRS_Quick[i] : options[i]);
            UI_PrintLine(line,2+i);
        }
        footer="MENU:SELECT  EXIT:BODY";
    } else {
#ifdef ENABLE_APRS_TRANSPORT
        const char *const states[]={"IDLE","QUEUED","WAITING FOR ACK","BEACON SENT","ACK RECEIVED",
            "MESSAGE REJECTED","UNCONFIRMED","TX BLOCKED","TX FAILED","CANCELLED","EXPIRED"};
        sprintf(line,gAPRS_TX.needsAcknowledgment ? "TO %s" : "STATIC BEACON",gAPRS_TX.recipient);UI_PrintLine(line,0);
        UI_PrintLine(states[gAPRS_TX.state],2);
        sprintf(line,"ATTEMPTS %u/%u",gAPRS_TX.attempts,gAPRS_TX.policy.maxAttempts);UI_PrintLine(line,4);
        footer="MENU:CANCEL EXIT:BACK";
#else
        sprintf(line,"TO %s",m->recipient);UI_PrintLine(line,0);
        const char *status=m->delivery==MESSAGE_ACKED ? "ACK RECEIVED" :
            m->delivery==MESSAGE_WAIT_ACK ? "WAITING FOR ACK" :
            m->delivery==MESSAGE_REJECTED ? "MESSAGE REJECTED" : "NOT SENT";
        UI_PrintLine(status,2);
        if(m->delivery==MESSAGE_TX_UNAVAILABLE)
            UI_PrintLine("TX UNAVAILABLE",4);
        footer="EXIT:PACKETS";
#endif
    }
#endif
    if(m->error!=MESSAGE_OK) {
#ifdef ENABLE_APRS_CALLSIGN_ONLY
        const char *error=m->error==MESSAGE_BAD_CALL ? "CHECK CALL/SSID" : "CALLSIGN FULL";
#else
        const char *error=m->error==MESSAGE_BAD_CALL ? "CHECK CALL/SSID" :
                          m->error==MESSAGE_EMPTY ? "EMPTY MESSAGE" : m->error==MESSAGE_TX_BLOCKED ? "CHECK TX SETUP / BUSY" : "MESSAGE FULL";
#endif
        UI_DisplaySmallest(error,0,42,false,true);
    }
    UI_DisplaySmallest(footer,0,50,false,true);
    ST7565_BlitFullScreen();
}
