/* SPDX-License-Identifier: GPL-3.0-only
 * RAM-only APRS profile and personal mailbox; no persistent storage accesses.
 */
#include "aprs_session.h"
#include "aprs_message.h"
#include "aprs_rx.h"
#include "misc.h"
#include <string.h>
APRS_Profile gAPRS_Profile;
APRS_Record gAPRS_Inbox[APRS_INBOX_CAPACITY];
uint8_t gAPRS_InboxCount, gAPRS_Unread, gAPRS_InboxSelected, gAPRS_HubSelected;
APRS_View gAPRS_View;
bool SESSION_SetCall(const char *call)
{
    if(!MESSAGE_ValidCall(call)) return false;
    char canonical[10];strcpy(canonical,call);
    unsigned n=strlen(canonical);
    if(n>=2 && canonical[n-2]=='-' && canonical[n-1]=='0') canonical[n-2]=0;
    if(strcmp(canonical,gAPRS_Profile.callsign)) {
        strcpy(gAPRS_Profile.callsign,canonical);
        /* Retain broad traffic; old-identity messages cease being personal. */
        memset(gAPRS_Inbox,0,sizeof(gAPRS_Inbox));
        gAPRS_InboxCount=gAPRS_Unread=gAPRS_InboxSelected=0;
        if(gAPRS_View==APRS_PERSONAL_DETAIL) gAPRS_View=APRS_PERSONAL;
    }
    return true;
}
bool SESSION_SetPosition(int32_t lat, int32_t lon)
{
    if(lat < -90000000 || lat > 90000000 || lon < -180000000 || lon > 180000000) return false;
    gAPRS_Profile.latitude=lat;gAPRS_Profile.longitude=lon;gAPRS_Profile.positionSet=true;
    return true;
}
bool SESSION_SetQuick(unsigned slot, const char *text)
{
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
bool SESSION_IsPersonal(const APRS_Record *r)
{
    if(!r || r->kind!=APRS_MESSAGE || !gAPRS_Profile.callsign[0] ||
       !MESSAGE_ValidCall(r->destination)) return false;
    size_t n=strlen(r->destination);
    if(n>=2 && r->destination[n-2]=='-' && r->destination[n-1]=='0') n-=2;
    return strlen(gAPRS_Profile.callsign)==n && !memcmp(r->destination,gAPRS_Profile.callsign,n);
}
void SESSION_Receive(const APRS_Record *r)
{
    if(!SESSION_IsPersonal(r)) return;
    memmove(gAPRS_Inbox+1,gAPRS_Inbox,(APRS_INBOX_CAPACITY-1)*sizeof(*r));
    gAPRS_Inbox[0]=*r;
    if(gAPRS_InboxCount<APRS_INBOX_CAPACITY) ++gAPRS_InboxCount;
    gAPRS_Unread=((gAPRS_Unread<<1)|1)&((1<<APRS_INBOX_CAPACITY)-1);
    if(gAPRS_View==APRS_PERSONAL_DETAIL) {
        if(gAPRS_InboxSelected+1<gAPRS_InboxCount) ++gAPRS_InboxSelected;
        else gAPRS_View=APRS_PERSONAL; /* Viewed entry was evicted; don't silently swap its body. */
    } else if(gAPRS_InboxSelected && gAPRS_InboxSelected+1<gAPRS_InboxCount) ++gAPRS_InboxSelected;
}
void SESSION_Open(void)
{
    gAPRS_View=APRS_HUB;gAPRS_HubSelected=0;
    gAPRS_Message.page=MESSAGE_CLOSED;
}
void SESSION_Key(KEY_Code_t key)
{
    if(gAPRS_View==APRS_HUB) {
        if(key==KEY_EXIT) gAPRS_Browser=false;
        else if(key==KEY_UP) gAPRS_HubSelected=(gAPRS_HubSelected+3)%4;
        else if(key==KEY_DOWN) gAPRS_HubSelected=(gAPRS_HubSelected+1)%4;
        else if(key==KEY_MENU) {
            if(gAPRS_HubSelected==0) { gAPRS_View=APRS_PERSONAL;gAPRS_InboxSelected=0; }
            else if(gAPRS_HubSelected==1) { gAPRS_View=APRS_MONITOR;gAPRS_Selected=0; }
            else if(gAPRS_HubSelected==2) MESSAGE_Open(0);
            else MESSAGE_EditCall();
        }
    } else if(gAPRS_View==APRS_PERSONAL) {
        if(key==KEY_EXIT) gAPRS_View=APRS_HUB;
        else if(key==KEY_UP && gAPRS_InboxSelected) --gAPRS_InboxSelected;
        else if(key==KEY_DOWN && gAPRS_InboxSelected+1<gAPRS_InboxCount) ++gAPRS_InboxSelected;
        else if(key==KEY_MENU) {
            if(!gAPRS_Profile.callsign[0]) MESSAGE_EditCall();
            else if(gAPRS_InboxCount) {
                gAPRS_View=APRS_PERSONAL_DETAIL;gAPRS_Unread&=~(1<<gAPRS_InboxSelected);
            }
        }
    } else if(gAPRS_View==APRS_PERSONAL_DETAIL) {
        if(key==KEY_EXIT) gAPRS_View=APRS_PERSONAL;
        else if(key==KEY_MENU && gAPRS_InboxSelected<gAPRS_InboxCount)
            MESSAGE_Open(gAPRS_Inbox[gAPRS_InboxSelected].source);
    } else {
        if(key==KEY_EXIT) gAPRS_View=APRS_HUB;
        else if(key==KEY_UP && gAPRS_Selected) --gAPRS_Selected;
        else if(key==KEY_DOWN && gAPRS_Selected+1<gAPRS_Count) ++gAPRS_Selected;
        else if(key==KEY_MENU) MESSAGE_Open(gAPRS_Count && gAPRS_Log[gAPRS_Selected].kind==APRS_MESSAGE ?
                                          gAPRS_Log[gAPRS_Selected].source : 0);
    }
    gUpdateDisplay=true;
}
