/* SPDX-License-Identifier: GPL-3.0-only */
#include "ui/display_text.h"
#include "app/aprs_session.h"
#include "app/aprs_rx.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
void SESSION_Draw(void)
{
    char line[22];
    UI_DisplayClear();
    const char *footer=DISPLAY_TEXT("MENU:OPEN  EXIT:RADIO");
    if(gAPRS_View==APRS_HUB) {
        UI_PrintLine(DISPLAY_TEXT("APRS"),0);
        static const char *const items[]={DISPLAY_TEXT("PERSONAL INBOX"),DISPLAY_TEXT("ALL TRAFFIC"),DISPLAY_TEXT("STATIONS"),
#ifndef ENABLE_APRS_CALLSIGN_ONLY
            DISPLAY_TEXT("NEW MESSAGE"),
#endif
            DISPLAY_TEXT("MY CALLSIGN")
#ifdef ENABLE_APRS_TRANSPORT
            ,DISPLAY_TEXT("SETUP")
#endif
        };
        #ifdef ENABLE_APRS_TRANSPORT
        const unsigned first=gAPRS_HubSelected/4*4;
#else
        const unsigned first=0;
#endif
        for(unsigned i=first;i<APRS_HUB_ITEMS && i<first+
#ifdef ENABLE_APRS_TRANSPORT
            4
#else
            APRS_HUB_ITEMS
#endif
            ;i++) {
            sprintf(line,DISPLAY_TEXT("%c%s"),i==gAPRS_HubSelected ? '>' : ' ',items[i]);
            UI_PrintLine(line,i-first+1);
        }
    } else if(gAPRS_View==APRS_STATIONS) {
        sprintf(line,DISPLAY_TEXT("STATIONS %u HEARD"),gAPRS_StationCount);UI_PrintLine(line,0);
        if(!gAPRS_StationCount) UI_PrintLine(DISPLAY_TEXT("No stations yet"),2);
        const unsigned first=gAPRS_StationSelected/4*4;
        for(unsigned row=0;row<4 && first+row<gAPRS_StationCount;row++) {
            const APRS_Station *station=&gAPRS_Stations[first+row];
            sprintf(line,DISPLAY_TEXT("%c%s"),first+row==gAPRS_StationSelected ? '>' : ' ',station->callsign);
            UI_PrintLine(line,row+1);
            uint32_t age=(gAPRS_SessionTicks-station->heardTick)/2;
            if(age<3600) sprintf(line,DISPLAY_TEXT("%02u:%02u"),(unsigned)(age/60),(unsigned)(age%60));
            else if(age<86400) sprintf(line,DISPLAY_TEXT("%uh"),(unsigned)(age/3600));
            else sprintf(line,DISPLAY_TEXT("%ud"),(unsigned)(age/86400));
            UI_DisplaySmallest(line,100,8*(row+1),false,true);
        }
        footer=DISPLAY_TEXT("MENU:DETAIL  EXIT:APRS");
    } else if(gAPRS_View==APRS_STATION_DETAIL) {
        if(gAPRS_StationSelected<gAPRS_StationCount) {
            const APRS_Station *station=&gAPRS_Stations[gAPRS_StationSelected];
            UI_PrintString(station->callsign,0,128,0,8);
            if(station->hasPosition) {
                APRS_FormatCoordinate(line,station->latitude,'N','S');UI_PrintLine(line,3);
                APRS_FormatCoordinate(line,station->longitude,'E','W');UI_PrintLine(line,4);
            } else UI_PrintLine(DISPLAY_TEXT("No position yet"),3);
            sprintf(line,DISPLAY_TEXT("HEARD %us AGO"),(unsigned)((gAPRS_SessionTicks-station->heardTick)/2));
            UI_DisplaySmallest(line,0,42,false,true);
        }
        footer=DISPLAY_TEXT("EXIT:STATIONS");
    } else if(gAPRS_View==APRS_PERSONAL_DETAIL) {
        UI_PrintLine(DISPLAY_TEXT("PERSONAL MESSAGE"),0);
        if(gAPRS_InboxSelected<gAPRS_InboxCount) APRS_DrawRecord(&gAPRS_Inbox[gAPRS_InboxSelected]);
#ifdef ENABLE_APRS_CALLSIGN_ONLY
        footer=DISPLAY_TEXT("EXIT:INBOX");
#else
        footer=DISPLAY_TEXT("MENU:REPLY  EXIT:INBOX");
#endif
    } else {
        sprintf(line,DISPLAY_TEXT("PERSONAL INBOX %u"),gAPRS_InboxCount);
        UI_PrintLine(line,0);
        if(!gAPRS_Profile.callsign[0]) {
            UI_PrintLine(DISPLAY_TEXT("SET YOUR CALLSIGN"),2);
            footer=DISPLAY_TEXT("MENU:SET CALL  EXIT:APRS");
        } else {
            if(!gAPRS_InboxCount) UI_PrintLine(DISPLAY_TEXT("No messages yet"),2);
            for(unsigned i=0;i<gAPRS_InboxCount;i++) {
                sprintf(line,DISPLAY_TEXT("%c%c%s"),i==gAPRS_InboxSelected ? '>' : ' ',
                        gAPRS_Unread&(1<<i) ? '*' : ' ',gAPRS_Inbox[i].source);
                UI_PrintLine(line,i+1);
            }
            sprintf(line,DISPLAY_TEXT("MY CALL %s"),gAPRS_Profile.callsign);UI_DisplaySmallest(line,0,42,false,true);
            footer=DISPLAY_TEXT("MENU:READ  EXIT:APRS");
        }
    }
    UI_DisplaySmallest(footer,0,50,false,true);
    ST7565_BlitFullScreen();
}
