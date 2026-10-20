/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_format.h"
#include "ui/display_text.h"
#include "app/aprs_setup.h"
#include "app/aprs_runtime.h"
#include "app/aprs_session.h"
#include "app/aprs_message.h"
#include "app/aprs_rx.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
void __attribute__((noinline)) SETUP_Draw(void)
{
    const APRS_Setup *s=&gAPRS_Setup;char line[24];
    UI_DisplayClear();const char *footer=DISPLAY_TEXT("MENU:SELECT EXIT:BACK");
    if(s->page==SETUP_HOME || s->page==SETUP_QUICK || s->page==SETUP_DATA) {
        const char *const home[]={DISPLAY_TEXT("MY CALLSIGN"),DISPLAY_TEXT("STATIC POSITION"),DISPLAY_TEXT("QUICK MESSAGES"),DISPLAY_TEXT("DATA TX")};
        const char *const data[]={DISPLAY_TEXT("APRS FREQUENCY"),DISPLAY_TEXT("TX AUDIO LEVEL"),DISPLAY_TEXT("ACK WAIT"),DISPLAY_TEXT("DATA TX"),DISPLAY_TEXT("SEND BEACON")};
        unsigned count=s->page==SETUP_HOME ? 4 : s->page==SETUP_QUICK ? 3 : 5;
        UI_PrintStringSmallNormal(s->page==SETUP_HOME ? DISPLAY_TEXT("APRS SETUP SESSION") :
            s->page==SETUP_QUICK ? DISPLAY_TEXT("EDIT QUICK MSGS") : DISPLAY_TEXT("APRS DATA TX"),0,0,0);
        for(unsigned i=0;i<count;i++) {
            const char *label=s->page==SETUP_HOME ? home[i] : s->page==SETUP_QUICK ? gAPRS_Quick[i] : data[i];
            sprintf(line,DISPLAY_TEXT("%c%.16s"),i==s->selected ? '>' : ' ',label);
            if(s->page==SETUP_DATA && i==0) {
                if(gAPRS_Runtime.frequency) sprintf(line,DISPLAY_TEXT("%cFREQ %3u.%05u"),i==s->selected ? '>' : ' ',
                    (unsigned)(gAPRS_Runtime.frequency/100000),(unsigned)(gAPRS_Runtime.frequency%100000));
                else sprintf(line,DISPLAY_TEXT("%cFREQ NOT SET"),i==s->selected ? '>' : ' ');
            }
            if(s->page==SETUP_DATA && i==1) {
                if(gAPRS_Runtime.audioSet) sprintf(line,DISPLAY_TEXT("%cTX AUDIO %u"),i==s->selected ? '>' : ' ',gAPRS_Runtime.audioLevel);
                else sprintf(line,DISPLAY_TEXT("%cTX AUDIO NOT SET"),i==s->selected ? '>' : ' ');
            }
            if(s->page==SETUP_DATA && i==2) sprintf(line,DISPLAY_TEXT("%cACK WAIT %us"),i==s->selected ? '>' : ' ',gAPRS_Runtime.acknowledgmentSeconds);
            if(s->page==SETUP_DATA && i==3) sprintf(line,DISPLAY_TEXT("%cDATA TX %s"),i==s->selected ? '>' : ' ',RUNTIME_Enabled() ? DISPLAY_TEXT("ON") : DISPLAY_TEXT("OFF"));
            UI_PrintLine(line,i+1);
        }
    } else if(s->page==SETUP_POSITION_REVIEW || s->page==SETUP_BEACON) {
        bool beacon=s->page==SETUP_BEACON;
        UI_PrintLine(beacon ? DISPLAY_TEXT("REVIEW BEACON") : DISPLAY_TEXT("REVIEW POSITION"),0);
        APRS_FormatDM(line,beacon ? gAPRS_Profile.latitude : s->latitude,false,true);
        UI_PrintLine(line,2);
        APRS_FormatDM(line,beacon ? gAPRS_Profile.longitude : s->longitude,true,true);
        UI_PrintLine(line,3);
        UI_PrintLine(beacon ? (gAPRS_Profile.positionSet ? gAPRS_Profile.callsign : DISPLAY_TEXT("POSITION NOT SET")) : DISPLAY_TEXT("SAVE DOES NOT SEND"),4);
        footer=beacon ? DISPLAY_TEXT("MENU:SEND EXIT:CANCEL") : DISPLAY_TEXT("MENU:SAVE EXIT:CANCEL");
    } else {
        bool longitude=s->page==SETUP_LONGITUDE;
        bool position=longitude || s->page==SETUP_LATITUDE;
        const char *title=position ? (longitude ? DISPLAY_TEXT("LON DDD MM.mm") : DISPLAY_TEXT("LATITUDE DD MM.mm")) :
            s->page==SETUP_FREQUENCY ? DISPLAY_TEXT("APRS FREQ MHz") : s->page==SETUP_AUDIO ? DISPLAY_TEXT("TX AUDIO 0-127") : DISPLAY_TEXT("ACK WAIT SECONDS");
        UI_PrintLine(title,0);
        char formatted[13];unsigned out=0,width=longitude ? 3 : 2;
        if(position || s->page==SETUP_FREQUENCY) {
            unsigned digits=position ? width+4 : 8;
            for(unsigned i=0;i<digits;i++) {
                if(position && i==width) formatted[out++]=' ';
                if(i==(position ? width+2 : 3)) formatted[out++]='.';
                formatted[out++]=i<s->length ? s->digits[i] : '_';
            }
            if(position) formatted[out++]=longitude ? (s->negative ? 'W' : 'E') : (s->negative ? 'S' : 'N');
            formatted[out]=0;
        } else sprintf(formatted,DISPLAY_TEXT("%.8s"),s->digits);
        UI_PrintLine(formatted,2);
        UI_PrintLine(position ? DISPLAY_TEXT("UP/DOWN:HEMISPHERE") : DISPLAY_TEXT("SESSION ONLY"),4);
        UI_PrintLine(DISPLAY_TEXT("*:DELETE"),5);
        footer=DISPLAY_TEXT("MENU:NEXT EXIT:CANCEL");
    }
    if(s->error) UI_DisplaySmallest(DISPLAY_TEXT("CHECK INPUT / TX SETUP"),0,42,false,true);
    UI_DisplaySmallest(footer,0,50,false,true);ST7565_BlitFullScreen();
}
