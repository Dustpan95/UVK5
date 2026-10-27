/* SPDX-License-Identifier: GPL-3.0-only
 * Session-only outgoing APRS profile. No received traffic or persistent writes. */
#include "aprs_session.h"
#include "aprs_message.h"
#include "aprs_ui.h"
#include "aprs_setup.h"
#include "misc.h"
#include <string.h>
APRS_Profile gAPRS_Profile;
uint8_t gAPRS_HubSelected;
bool SESSION_SetCall(const char *call)
{
    if(!MESSAGE_ValidCall(call)) return false;
    char canonical[10];strcpy(canonical,call);
    unsigned n=strlen(canonical);
    if(n>=2 && canonical[n-2]=='-' && canonical[n-1]=='0') canonical[n-2]=0;
    strcpy(gAPRS_Profile.callsign,canonical);return true;
}
bool SESSION_SetPosition(int32_t lat, int32_t lon)
{
    if(lat < -90000000 || lat > 90000000 || lon < -180000000 || lon > 180000000) return false;
    gAPRS_Profile.latitude=lat;gAPRS_Profile.longitude=lon;gAPRS_Profile.positionSet=true;
    return true;
}
bool SESSION_SetQuick(unsigned slot, const char *text)
{
    MESSAGE_Init();
    if(slot>=APRS_QUICK_COUNT || !text) return false;
    unsigned n=0;
    while(text[n] && n<=APRS_MESSAGE_MAX) {
        if(text[n]<' ' || text[n]>'}' || text[n]=='{' || text[n]=='|') return false;
        ++n;
    }
    if(n>APRS_MESSAGE_MAX) return false;
    memmove(gAPRS_Quick[slot],text,n+1);
    return true;
}

void SESSION_Open(void)
{
    gAPRS_HubSelected=0;gAPRS_Setup.page=SETUP_CLOSED;
    gAPRS_Message.page=MESSAGE_CLOSED;
}
void SESSION_Key(KEY_Code_t key)
{
    if(key==KEY_EXIT) gAPRS_Browser=false;
    else if(key==KEY_UP) gAPRS_HubSelected=(gAPRS_HubSelected+APRS_HUB_ITEMS-1)%APRS_HUB_ITEMS;
    else if(key==KEY_DOWN) gAPRS_HubSelected=(gAPRS_HubSelected+1)%APRS_HUB_ITEMS;
    else if(key==KEY_MENU) {
        if(gAPRS_HubSelected==0) MESSAGE_Open(0);
        else if(gAPRS_HubSelected==1) MESSAGE_EditCall();
        else SETUP_Open();
    }
    gUpdateDisplay=true;
}
