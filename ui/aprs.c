/* SPDX-License-Identifier: GPL-3.0-only */
#include "ui/display_text.h"
#include "app/aprs_ui.h"
#include "app/aprs_session.h"
#include "app/aprs_message.h"
#include "app/aprs_setup.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "ui/helper.h"
void SESSION_Draw(void)
{
    char line[22];UI_DisplayClear();UI_PrintLine(DISPLAY_TEXT("APRS TX"),0);
    static const char *const items[]={DISPLAY_TEXT("NEW MESSAGE"),DISPLAY_TEXT("MY CALLSIGN"),DISPLAY_TEXT("SETUP")};
    for(unsigned i=0;i<APRS_HUB_ITEMS;i++) {
        sprintf(line,DISPLAY_TEXT("%c%s"),i==gAPRS_HubSelected?'>':' ',items[i]);
        UI_PrintLine(line,i+1);
    }
    UI_DisplaySmallest(DISPLAY_TEXT("MENU:OPEN  EXIT:RADIO"),0,50,false,true);
    ST7565_BlitFullScreen();
}
void APRS_Draw(void)
{
    if(gAPRS_Message.page!=MESSAGE_CLOSED) MESSAGE_Draw();
    else if(gAPRS_Setup.page!=SETUP_CLOSED) SETUP_Draw();
    else SESSION_Draw();
}
