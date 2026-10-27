/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_runtime.h"
#include "app/aprs_message.h"
#include "app/aprs_session.h"
#include "app/aprs_ui.h"
#include "experiments/aprs_hardware.h"
#include "settings.h"
#include "ui/ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
EEPROM_Config_t gEeprom;
GUI_DisplayType_t gScreenToDisplay=DISPLAY_MAIN,gRequestDisplayScreen=DISPLAY_INVALID;
bool gUpdateDisplay,gDTMF_InputMode,gPttIsPressed;
uint8_t gInputBoxIndex;
static uint32_t now;
static unsigned sends,restores;
static bool interrupt;
static APRS_Hardware *leased;
uint32_t SYSTICK_Cycles(void) {return 98765;}
uint32_t HARDWARE_Now(void *ctx) {(void)ctx;return now;}
int32_t RX_freq_check(uint32_t f) {return f==14439000 ? 0 : -1;}
int32_t TX_freq_check(uint32_t f) {return RX_freq_check(f);}
APRS_ChannelAdmission HARDWARE_Enter(uint32_t frequency,void *ctx)
{
    APRS_Hardware *h=ctx;assert(frequency==14439000 && !h->leased && h->radio.enabled);
    assert(h->radio.toneGain==25 && h->power==OUTPUT_POWER_LOW);
    h->index=0;h->voice=gEeprom.VfoInfo[0];h->leased=true;leased=h;return APRS_CHANNEL_READY;
}
APRS_LinkResult HARDWARE_Send(const uint8_t *frame,size_t n,uint32_t f,void *ctx)
{
    assert(frame && n>16 && f==14439000 && ctx==leased && leased->leased);
    ++sends;now+=50;return APRS_LINK_SENT;
}
void HARDWARE_Restore(void *ctx) {assert(ctx==leased && leased->leased);leased->leased=false;++restores;}
bool HARDWARE_Interrupted(void *ctx) {(void)ctx;return interrupt;}
int main(void)
{
    MESSAGE_Init();assert(!RUNTIME_Enabled() && !RUNTIME_Enable(true));
    assert(!RUNTIME_Configure(0,25));assert(!RUNTIME_Configure(14439000,128));
    assert(RUNTIME_Configure(14439000,25));assert(!RUNTIME_Enable(true));
    assert(SESSION_SetCall("MYCALL"));assert(RUNTIME_Enable(true));
    strcpy(gAPRS_Message.recipient,"N0CALL-7");strcpy(gAPRS_Message.text,"HELLO");
    assert(RUNTIME_SendDraft());
    char first[6];strcpy(first,gAPRS_TX.id);
    gScreenToDisplay=DISPLAY_MENU;RUNTIME_Process();assert(!sends);
    gScreenToDisplay=DISPLAY_MAIN;gAPRS_Browser=true;gAPRS_Message.page=MESSAGE_COMPOSE;
    RUNTIME_Process();assert(!sends);
    gAPRS_Message.page=MESSAGE_STATUS;RUNTIME_Process();
    assert(sends==1 && restores==1 && !RUNTIME_Leased() && gAPRS_TX.state==APRS_TX_SENT);
    for(unsigned i=0;i<1000;i++){now+=100;RUNTIME_Process();}
    assert(sends==1 && restores==1); /* No ACK listening, automatic replies or retries. */
    assert(RUNTIME_SendDraft() && strcmp(first,gAPRS_TX.id));RUNTIME_Cancel();
    assert(gAPRS_TX.state==APRS_TX_CANCELLED && RUNTIME_Enabled());
    assert(!RUNTIME_Beacon());assert(SESSION_SetPosition(0,0));assert(RUNTIME_Beacon());
    RUNTIME_Process();assert(sends==2 && restores==2 && !RUNTIME_Leased());
    assert(RUNTIME_SendDraft());assert(RUNTIME_Enable(false));
    assert(!RUNTIME_Enabled() && gAPRS_TX.state==APRS_TX_CANCELLED);
    assert(RUNTIME_Enable(true));assert(RUNTIME_SendDraft());gScreenToDisplay=DISPLAY_MENU;
    now+=60000;RUNTIME_Process();assert(gAPRS_TX.state==APRS_TX_EXPIRED);
    assert(RUNTIME_SendDraft());assert(SESSION_SetCall("NEWCAL"));RUNTIME_Process();
    assert(!RUNTIME_Enabled() && gAPRS_TX.state==APRS_TX_CANCELLED);
    APRS_Open();assert(gAPRS_Browser && APRS_HUB_ITEMS==3);
    unsigned before=sends;APRS_Key(KEY_MENU,true,true);assert(gAPRS_Message.page==MESSAGE_CLOSED);
    APRS_Key(KEY_MENU,true,false);assert(gAPRS_Message.page==MESSAGE_RECIPIENT);
    assert(!APRS_Key(KEY_PTT,true,false) && !gAPRS_Browser && sends==before);
    puts("PASS outgoing runtime: explicit enable, frozen IDs, UI exclusion, one unconfirmed handoff, immediate voice restore, no background sends, cancellation, expiry and PTT passthrough");
}
