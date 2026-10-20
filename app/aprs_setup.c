/* SPDX-License-Identifier: GPL-3.0-only
 * Session-only setup. Saving a position never queues a transmission.
 */
#include "aprs_setup.h"
/* Keep setup handlers shared under the pinned GCC LTO build. The measured
 * boundaries reduce flash without changing screens or input behavior. */
#include "aprs_session.h"
#include "aprs_message.h"
#include "aprs_runtime.h"
#include <string.h>
APRS_Setup gAPRS_Setup;
bool __attribute__((noinline)) SETUP_ParseDM(const char *digits,bool longitude,bool negative,int32_t *value)
{
    if(!digits || !value) return false;
    unsigned width=longitude ? 3 : 2, degrees=0,minutes=0;
    if(strlen(digits)!=width+4) return false;
    for(unsigned i=0;i<width+4;i++) {
        if(digits[i]<'0' || digits[i]>'9') return false;
        if(i<width) degrees=degrees*10+digits[i]-'0';
        else minutes=minutes*10+digits[i]-'0';
    }
    unsigned limit=longitude ? 180 : 90;
    if(degrees>limit || minutes>=6000 || (degrees==limit && minutes)) return false;
    int32_t result=degrees*1000000u+(minutes*500u+1u)/3u;
    *value=negative ? -result : result;return true;
}
static void Entry(APRS_SetupPage page)
{
    gAPRS_Setup.page=page;gAPRS_Setup.length=0;gAPRS_Setup.digits[0]=0;
    gAPRS_Setup.negative=false;gAPRS_Setup.error=false;
}
void SETUP_Open(void) {Entry(SETUP_HOME);gAPRS_Setup.selected=0;}
void __attribute__((noinline)) SETUP_Key(KEY_Code_t key)
{
    APRS_Setup *s=&gAPRS_Setup;s->error=false;
    if(s->page==SETUP_HOME || s->page==SETUP_QUICK || s->page==SETUP_DATA) {
        unsigned count=s->page==SETUP_HOME ? 4 : s->page==SETUP_QUICK ? 3 : 5;
        if(key==KEY_EXIT) {s->page=s->page==SETUP_HOME ? SETUP_CLOSED : SETUP_HOME;s->selected=0;}
        else if(key==KEY_UP) s->selected=(s->selected+count-1)%count;
        else if(key==KEY_DOWN) s->selected=(s->selected+1)%count;
        else if(key==KEY_MENU) {
            if(s->page==SETUP_HOME) {
                if(s->selected==0) MESSAGE_EditCall();
                else if(s->selected==1) Entry(SETUP_LATITUDE);
                else {s->page=s->selected==2 ? SETUP_QUICK : SETUP_DATA;s->selected=0;}
            } else if(s->page==SETUP_QUICK) MESSAGE_EditQuick(s->selected);
            else if(s->selected==0) Entry(SETUP_FREQUENCY);
            else if(s->selected==1) Entry(SETUP_AUDIO);
            else if(s->selected==2) Entry(SETUP_WAIT);
            else if(s->selected==3) s->error=!RUNTIME_Enable(!RUNTIME_Enabled());
            else {s->page=SETUP_BEACON;}
        }
        return;
    }
    if(key==KEY_EXIT) {s->page=SETUP_HOME;s->selected=0;return;}
    if(s->page==SETUP_POSITION_REVIEW) {
        if(key==KEY_MENU) {SESSION_SetPosition(s->latitude,s->longitude);s->page=SETUP_HOME;}
        return;
    }
    if(s->page==SETUP_BEACON) {
        if(key==KEY_MENU) {if(RUNTIME_Beacon()) {s->page=SETUP_CLOSED;gAPRS_Message.page=MESSAGE_STATUS;} else s->error=true;}
        return;
    }
    bool position=s->page==SETUP_LATITUDE || s->page==SETUP_LONGITUDE;
    if(position && (key==KEY_UP || key==KEY_DOWN)) {s->negative=!s->negative;return;}
    if(key==KEY_STAR) {if(s->length) s->digits[--s->length]=0;return;}
    unsigned limit=s->page==SETUP_LATITUDE ? 6 : s->page==SETUP_LONGITUDE ? 7 : s->page==SETUP_FREQUENCY ? 8 : 3;
    if(key<=KEY_9) {if(s->length<limit) {s->digits[s->length++]='0'+key;s->digits[s->length]=0;}return;}
    if(key!=KEY_MENU) return;
    if(position) {
        int32_t result;
        if(!SETUP_ParseDM(s->digits,s->page==SETUP_LONGITUDE,s->negative,&result)) {s->error=true;return;}
        if(s->page==SETUP_LATITUDE) {s->latitude=result;Entry(SETUP_LONGITUDE);}
        else {s->longitude=result;s->page=SETUP_POSITION_REVIEW;}
    } else {
        uint32_t n=0;for(unsigned i=0;i<s->length;i++) n=n*10+s->digits[i]-'0';
        if(!s->length || (s->page==SETUP_FREQUENCY && (s->length!=8 || RX_freq_check(n) || TX_freq_check(n))) ||
           (s->page==SETUP_AUDIO && n>127) || (s->page==SETUP_WAIT && (!n || n>600))) {s->error=true;return;}
        RUNTIME_Enable(false);
        if(s->page==SETUP_FREQUENCY) gAPRS_Runtime.frequency=n;
        else if(s->page==SETUP_AUDIO) {gAPRS_Runtime.audioLevel=n;gAPRS_Runtime.audioSet=true;}
        else gAPRS_Runtime.acknowledgmentSeconds=n;
        s->page=SETUP_DATA;s->selected=0;
    }
}
