/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_session.h"
#include "app/aprs_rx.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
void SESSION_Draw(void)
{
    char line[22];
    UI_DisplayClear();
    const char *footer="MENU:OPEN  EXIT:RADIO";
    if(gAPRS_View==APRS_HUB) {
        UI_PrintStringSmallNormal("APRS",0,0,0);
        const char *const items[]={"PERSONAL INBOX","ALL TRAFFIC","NEW MESSAGE","MY CALLSIGN"};
        for(unsigned i=0;i<4;i++) {
            sprintf(line,"%c%s",i==gAPRS_HubSelected ? '>' : ' ',items[i]);
            UI_PrintStringSmallNormal(line,0,0,i+1);
        }
    } else if(gAPRS_View==APRS_PERSONAL_DETAIL) {
        UI_PrintStringSmallNormal("PERSONAL MESSAGE",0,0,0);
        if(gAPRS_InboxSelected<gAPRS_InboxCount) APRS_DrawRecord(&gAPRS_Inbox[gAPRS_InboxSelected]);
        footer="MENU:REPLY  EXIT:INBOX";
    } else {
        sprintf(line,"PERSONAL INBOX %u",gAPRS_InboxCount);
        UI_PrintStringSmallNormal(line,0,0,0);
        if(!gAPRS_Profile.callsign[0]) {
            UI_PrintStringSmallNormal("SET YOUR CALLSIGN",0,0,2);
            footer="MENU:SET CALL  EXIT:APRS";
        } else {
            if(!gAPRS_InboxCount) UI_PrintStringSmallNormal("No messages yet",0,0,2);
            for(unsigned i=0;i<gAPRS_InboxCount;i++) {
                sprintf(line,"%c%c%s",i==gAPRS_InboxSelected ? '>' : ' ',
                        gAPRS_Unread&(1<<i) ? '*' : ' ',gAPRS_Inbox[i].source);
                UI_PrintStringSmallNormal(line,0,0,i+1);
            }
            sprintf(line,"MY CALL %s",gAPRS_Profile.callsign);UI_DisplaySmallest(line,0,42,false,true);
            footer="MENU:READ  EXIT:APRS";
        }
    }
    UI_DisplaySmallest(footer,0,50,false,true);
    ST7565_BlitFullScreen();
}
