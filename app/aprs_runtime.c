/* SPDX-License-Identifier: GPL-3.0-only
 * Foreground binding for the session-only standalone APRS transmitter.
 */
#include "aprs_runtime.h"
#include "app/aprs_message.h"
#include "app/aprs_rx.h"
#include "experiments/aprs_hardware.h"
#include "driver/systick.h"
#include "misc.h"
#include "dtmf.h"
#include "ui/inputbox.h"
#include "frequencies.h"
#include "settings.h"
#include "ui/ui.h"
static APRS_Channel channel;
static APRS_Hardware hardware;
APRS_RuntimeSettings gAPRS_Runtime={.acknowledgmentSeconds=10};
static uint32_t idCursor,idCount;
static bool seeded;
void RUNTIME_Interrupt(void) {CHANNEL_Interrupt(&channel);}
bool RUNTIME_Leased(void) {return hardware.leased;}
const VFO_Info_t *RUNTIME_VoiceVfo(unsigned index)
{
    return hardware.leased && hardware.index==index ? &hardware.voice : &gEeprom.VfoInfo[index];
}
bool RUNTIME_Enabled(void) {return SERVICE_Enabled();}
bool RUNTIME_Enable(bool enabled)
{
    RUNTIME_Interrupt();
    APRS_ServiceSettings config={gAPRS_Runtime.frequency,gAPRS_Runtime.acknowledgmentSeconds*100u,
        60000,1000,enabled};
    if(enabled && (!gAPRS_Runtime.audioSet || RX_freq_check(gAPRS_Runtime.frequency) ||
       TX_freq_check(gAPRS_Runtime.frequency))) return false;
    if(!SERVICE_Configure(&config)) return false;
    hardware.radio=(APRS_RadioConfig){enabled,gAPRS_Runtime.audioLevel};
    static const APRS_ChannelOps ops={HARDWARE_Now,HARDWARE_Enter,HARDWARE_Send,HARDWARE_Restore,HARDWARE_Interrupted,&hardware};
    CHANNEL_Configure(&channel,&ops);channel.voiceTicks=3000;gUpdateDisplay=true;return true;
}
bool RUNTIME_Configure(uint32_t frequency,uint16_t seconds,uint8_t level)
{
    if(!frequency || RX_freq_check(frequency) || TX_freq_check(frequency) || !seconds || seconds>600 || level>127) return false;
    RUNTIME_Enable(false);gAPRS_Runtime=(APRS_RuntimeSettings){frequency,seconds,level,true};return true;
}
void RUNTIME_Process(void)
{
    const uint32_t now=HARDWARE_Now(0);TX_Tick(now);
    if(!hardware.leased && (gScreenToDisplay!=DISPLAY_MAIN || gRequestDisplayScreen!=DISPLAY_INVALID ||
       gInputBoxIndex || gDTMF_InputMode || gPttIsPressed ||
       (gAPRS_Browser && gAPRS_Message.page!=MESSAGE_CLOSED && gAPRS_Message.page!=MESSAGE_STATUS))) return;
    if(!hardware.leased && gEeprom.TX_VFO<2) hardware.power=gEeprom.VfoInfo[gEeprom.TX_VFO].OUTPUT_POWER;
    APRS_TX_State before=gAPRS_TX.state;CHANNEL_Process(&channel);
    if(before!=gAPRS_TX.state) gUpdateDisplay=true;
}
void RUNTIME_Receive(const APRS_Record *record)
{
    if(SERVICE_Receive(record,HARDWARE_Now(0))==APRS_SERVICE_DELIVERY_UPDATED) gUpdateDisplay=true;
}
bool RUNTIME_SendDraft(void)
{
    if(!RUNTIME_Enabled() || idCount==100000) return false;
    if(!seeded) {idCursor=(HARDWARE_Now(0)^SYSTICK_Cycles())%100000;seeded=true;}
    char id[6];uint32_t n=idCursor;id[5]=0;
    for(unsigned i=5;i>0;) {id[--i]='0'+n%10;n/=10;}
    if(!SERVICE_Message(gAPRS_Message.recipient,gAPRS_Message.text,id,HARDWARE_Now(0))) return false;
    ++idCount;if(++idCursor==100000) idCursor=0;
    gUpdateDisplay=true;return true;
}
bool RUNTIME_Beacon(void)
{
    bool queued=SERVICE_Beacon(HARDWARE_Now(0));if(queued) gUpdateDisplay=true;return queued;
}
void RUNTIME_Cancel(void) {TX_Cancel();RUNTIME_Interrupt();gUpdateDisplay=true;}
