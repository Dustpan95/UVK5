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
        const char *const items[]={"PERSONAL INBOX","ALL TRAFFIC","STATIONS","NEW MESSAGE","MY CALLSIGN"};
        for(unsigned i=0;i<5;i++) {
            sprintf(line,"%c%s",i==gAPRS_HubSelected ? '>' : ' ',items[i]);
            UI_PrintStringSmallNormal(line,0,0,i+1);
        }
    } else if(gAPRS_View==APRS_STATIONS) {
        sprintf(line,"STATIONS %u HEARD",gAPRS_StationCount);UI_PrintStringSmallNormal(line,0,0,0);
        if(!gAPRS_StationCount) UI_PrintStringSmallNormal("No stations yet",0,0,2);
        const unsigned first=gAPRS_StationSelected/4*4;
        for(unsigned row=0;row<4 && first+row<gAPRS_StationCount;row++) {
            const APRS_Station *station=&gAPRS_Stations[first+row];
            sprintf(line,"%c%s",first+row==gAPRS_StationSelected ? '>' : ' ',station->callsign);
            UI_PrintStringSmallNormal(line,0,0,row+1);
            uint32_t age=(gAPRS_SessionTicks-station->heardTick)/2;
            if(age<3600) sprintf(line,"%02u:%02u",(unsigned)(age/60),(unsigned)(age%60));
            else if(age<86400) sprintf(line,"%uh",(unsigned)(age/3600));
            else sprintf(line,"%ud",(unsigned)(age/86400));
            UI_DisplaySmallest(line,100,8*(row+1),false,true);
        }
        footer="MENU:DETAIL  EXIT:APRS";
    } else if(gAPRS_View==APRS_STATION_DETAIL) {
        if(gAPRS_StationSelected<gAPRS_StationCount) {
            const APRS_Station *station=&gAPRS_Stations[gAPRS_StationSelected];
            UI_PrintString(station->callsign,0,128,0,8);
            if(station->hasPosition) {
                APRS_FormatCoordinate(line,station->latitude,'N','S');UI_PrintStringSmallNormal(line,0,0,3);
                APRS_FormatCoordinate(line,station->longitude,'E','W');UI_PrintStringSmallNormal(line,0,0,4);
            } else UI_PrintStringSmallNormal("No position yet",0,0,3);
            sprintf(line,"HEARD %us AGO",(unsigned)((gAPRS_SessionTicks-station->heardTick)/2));
            UI_DisplaySmallest(line,0,42,false,true);
        }
        footer="EXIT:STATIONS";
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
