/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_setup.h"
#include "app/aprs_runtime.h"
#include "app/aprs_session.h"
#include "app/aprs_message.h"
#include "app/aprs_rx.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
static void __attribute__((noinline)) Position(char *out,int32_t value,bool longitude)
{
    uint32_t absolute=value<0 ? -value : value;
    uint32_t minutes=(absolute*3u+250u)/500u;
    sprintf(out,longitude ? "%03u %02u.%02u%c" : "%02u %02u.%02u%c",
        (unsigned)(minutes/6000),(unsigned)(minutes%6000/100),(unsigned)(minutes%100),
        longitude ? (value<0 ? 'W' : 'E') : (value<0 ? 'S' : 'N'));
}
void __attribute__((noinline)) SETUP_Draw(void)
{
    const APRS_Setup *s=&gAPRS_Setup;char line[24];
    UI_DisplayClear();const char *footer="MENU:SELECT EXIT:BACK";
    if(s->page==SETUP_HOME || s->page==SETUP_QUICK || s->page==SETUP_DATA) {
        const char *const home[]={"MY CALLSIGN","STATIC POSITION","QUICK MESSAGES","DATA TX"};
        const char *const data[]={"APRS FREQUENCY","TX AUDIO LEVEL","ACK WAIT","DATA TX","SEND BEACON"};
        unsigned count=s->page==SETUP_HOME ? 4 : s->page==SETUP_QUICK ? 3 : 5;
        UI_PrintStringSmallNormal(s->page==SETUP_HOME ? "APRS SETUP SESSION" :
            s->page==SETUP_QUICK ? "EDIT QUICK MSGS" : "APRS DATA TX",0,0,0);
        for(unsigned i=0;i<count;i++) {
            const char *label=s->page==SETUP_HOME ? home[i] : s->page==SETUP_QUICK ? gAPRS_Quick[i] : data[i];
            sprintf(line,"%c%.16s",i==s->selected ? '>' : ' ',label);
            if(s->page==SETUP_DATA && i==0) {
                if(gAPRS_Runtime.frequency) sprintf(line,"%cFREQ %3u.%05u",i==s->selected ? '>' : ' ',
                    (unsigned)(gAPRS_Runtime.frequency/100000),(unsigned)(gAPRS_Runtime.frequency%100000));
                else sprintf(line,"%cFREQ NOT SET",i==s->selected ? '>' : ' ');
            }
            if(s->page==SETUP_DATA && i==1) {
                if(gAPRS_Runtime.audioSet) sprintf(line,"%cTX AUDIO %u",i==s->selected ? '>' : ' ',gAPRS_Runtime.audioLevel);
                else sprintf(line,"%cTX AUDIO NOT SET",i==s->selected ? '>' : ' ');
            }
            if(s->page==SETUP_DATA && i==2) sprintf(line,"%cACK WAIT %us",i==s->selected ? '>' : ' ',gAPRS_Runtime.acknowledgmentSeconds);
            if(s->page==SETUP_DATA && i==3) sprintf(line,"%cDATA TX %s",i==s->selected ? '>' : ' ',RUNTIME_Enabled() ? "ON" : "OFF");
            UI_PrintStringSmallNormal(line,0,0,i+1);
        }
    } else if(s->page==SETUP_POSITION_REVIEW || s->page==SETUP_BEACON) {
        bool beacon=s->page==SETUP_BEACON;
        UI_PrintStringSmallNormal(beacon ? "REVIEW BEACON" : "REVIEW POSITION",0,0,0);
        Position(line,beacon ? gAPRS_Profile.latitude : s->latitude,false);
        UI_PrintStringSmallNormal(line,0,0,2);
        Position(line,beacon ? gAPRS_Profile.longitude : s->longitude,true);
        UI_PrintStringSmallNormal(line,0,0,3);
        UI_PrintStringSmallNormal(beacon ? (gAPRS_Profile.positionSet ? gAPRS_Profile.callsign : "POSITION NOT SET") : "SAVE DOES NOT SEND",0,0,4);
        footer=beacon ? "MENU:SEND EXIT:CANCEL" : "MENU:SAVE EXIT:CANCEL";
    } else {
        bool longitude=s->page==SETUP_LONGITUDE;
        bool position=longitude || s->page==SETUP_LATITUDE;
        const char *title=position ? (longitude ? "LON DDD MM.mm" : "LATITUDE DD MM.mm") :
            s->page==SETUP_FREQUENCY ? "APRS FREQ MHz" : s->page==SETUP_AUDIO ? "TX AUDIO 0-127" : "ACK WAIT SECONDS";
        UI_PrintStringSmallNormal(title,0,0,0);
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
        } else sprintf(formatted,"%.8s",s->digits);
        UI_PrintStringSmallNormal(formatted,0,0,2);
        UI_PrintStringSmallNormal(position ? "UP/DOWN:HEMISPHERE" : "SESSION ONLY",0,0,4);
        UI_PrintStringSmallNormal("*:DELETE",0,0,5);
        footer="MENU:NEXT EXIT:CANCEL";
    }
    if(s->error) UI_DisplaySmallest("CHECK INPUT / TX SETUP",0,42,false,true);
    UI_DisplaySmallest(footer,0,50,false,true);ST7565_BlitFullScreen();
}
