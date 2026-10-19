/* SPDX-License-Identifier: GPL-3.0-only
 * Approved messaging flow; initial physical-key implementation for measurement.
 */
#include "app/aprs_message.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
#include <string.h>
static void Body(const char *text, unsigned first)
{
    unsigned length=strlen(text);
    for(unsigned row=0;row<4 && row*17<length;row++) {
        char line[18]; unsigned n=length-row*17;
        if(n>17) n=17;
        memcpy(line,text+row*17,n);line[n]=0;
        UI_PrintStringSmallNormal(line,0,0,first+row);
    }
}
void MESSAGE_Draw(void)
{
    const APRS_Message *m=&gAPRS_Message;
    char line[22];
    UI_DisplayClear();
    const char *footer="MENU:NEXT  EXIT:BACK";
    if(m->page==MESSAGE_RECIPIENT) {
        UI_PrintStringSmallNormal("RECIPIENT",0,0,0);
        UI_PrintStringSmallNormal(m->recipient,0,0,2);
        UI_PrintStringSmallNormal("MULTI-TAP ABC/123",0,0,3);
        UI_PrintStringSmallNormal("1:.-/  #:NEXT",0,0,4);
        UI_PrintStringSmallNormal("*:DELETE",0,0,5);
    } else if(m->page==MESSAGE_COMPOSE || m->page==MESSAGE_REVIEW) {
        sprintf(line,"%s %s",m->page==MESSAGE_REVIEW ? "SEND" : "TO",m->recipient);
        UI_PrintStringSmallNormal(line,0,0,0);
        Body(m->text,1);
        sprintf(line,"%u/67%s",(unsigned)strlen(m->text),m->tapTicks ? " ABC_" : " ABC");
        UI_DisplaySmallest(line,0,42,false,true);
        footer=m->page==MESSAGE_REVIEW ? "MENU:SEND  EXIT:EDIT" : "MENU:OPTIONS  EXIT:BACK";
    } else if(m->page==MESSAGE_OPTIONS || m->page==MESSAGE_QUICK) {
        const bool quick=m->page==MESSAGE_QUICK;
        UI_PrintStringSmallNormal(quick ? "QUICK MESSAGES" : "MESSAGE OPTIONS",0,0,0);
        const char *const options[]={"QUICK MESSAGE","REVIEW AND SEND","EDIT RECIPIENT"};
        for(unsigned i=0;i<3;i++) {
            sprintf(line,"%c%s",i==m->selected ? '>' : ' ',quick ? gAPRS_Quick[i] : options[i]);
            UI_PrintStringSmallNormal(line,0,0,2+i);
        }
        footer="MENU:SELECT  EXIT:BODY";
    } else {
        sprintf(line,"TO %s",m->recipient);UI_PrintStringSmallNormal(line,0,0,0);
        const char *status=m->delivery==MESSAGE_ACKED ? "ACK RECEIVED" :
            m->delivery==MESSAGE_WAIT_ACK ? "WAITING FOR ACK" :
            m->delivery==MESSAGE_REJECTED ? "MESSAGE REJECTED" : "NOT SENT";
        UI_PrintStringSmallNormal(status,0,0,2);
        if(m->delivery==MESSAGE_TX_UNAVAILABLE)
            UI_PrintStringSmallNormal("TX UNAVAILABLE",0,0,4);
        footer="EXIT:PACKETS";
    }
    if(m->error!=MESSAGE_OK) {
        const char *error=m->error==MESSAGE_BAD_CALL ? "CHECK CALL/SSID" :
                          m->error==MESSAGE_EMPTY ? "EMPTY MESSAGE" : "MESSAGE FULL";
        UI_DisplaySmallest(error,0,42,false,true);
    }
    UI_DisplaySmallest(footer,0,50,false,true);
    ST7565_BlitFullScreen();
}
