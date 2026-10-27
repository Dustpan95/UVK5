/* SPDX-License-Identifier: GPL-3.0-only
 * Outgoing APRS UI only. No packet receiver, FIFO ownership or receive alerts. */
#include "aprs_ui.h"
#include "aprs_message.h"
#include "aprs_setup.h"
#include "aprs_session.h"
#include "misc.h"
#include "ui/ui.h"
bool gAPRS_Browser;
void APRS_Open(void)
{
    MESSAGE_Init();SESSION_Open();gAPRS_Browser=true;
    gRequestDisplayScreen=DISPLAY_MAIN;gUpdateDisplay=true;
}
void APRS_Tick500ms(void) {MESSAGE_Tick500ms();}
bool APRS_Key(KEY_Code_t key,bool pressed,bool held)
{
    if(!gAPRS_Browser) return false;
    if(key==KEY_PTT) {
        gAPRS_Browser=false;gAPRS_Message.page=MESSAGE_CLOSED;
        gAPRS_Setup.page=SETUP_CLOSED;gUpdateDisplay=true;return false;
    }
    if(!pressed || held) return true;
    if(gAPRS_Message.page!=MESSAGE_CLOSED) MESSAGE_Key(key);
    else if(gAPRS_Setup.page!=SETUP_CLOSED) SETUP_Key(key);
    else SESSION_Key(key);
    gUpdateDisplay=true;return true;
}
