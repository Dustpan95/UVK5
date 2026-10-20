/* SPDX-License-Identifier: GPL-3.0-only
 * Separate-frequency APRS ownership/ACK-wait orchestration. Hardware operations
 * are injected; no default firmware build calls this model or retunes a radio.
 */
#include "aprs_channel.h"
static bool Due(uint32_t now,uint32_t deadline) {return !((now-deadline)&0x80000000u);}
void CHANNEL_Interrupt(APRS_Channel *c)
{
    if(!c || !c->leased) return;
    if(c->waiting && c->voiceTicks && gAPRS_TX.state==APRS_TX_WAIT_ACK &&
       gAPRS_TX.attempts<gAPRS_TX.policy.maxAttempts) {
        uint32_t resume=c->ops.now(c->ops.context)+c->voiceTicks;
        if(Due(resume,gAPRS_TX.nextTick)) gAPRS_TX.nextTick=resume;
    }
    c->leased=c->waiting=false;
    c->ops.restore(c->ops.context);
}
bool CHANNEL_Configure(APRS_Channel *c,const APRS_ChannelOps *ops)
{
    if(!c || !ops || !ops->now || !ops->enter || !ops->send || !ops->restore || !ops->interrupted) return false;
    CHANNEL_Interrupt(c);c->ops=*ops;c->configured=true;return true;
}
static uint32_t Now(void *context)
{
    APRS_Channel *c=context;return c->ops.now(c->ops.context);
}
static APRS_LinkResult Send(const uint8_t *frame,size_t length,uint32_t frequency,void *context)
{
    APRS_Channel *c=context;
    if(!c->leased) {
        APRS_ChannelAdmission admission=c->ops.enter(frequency,c->ops.context);
        switch(admission) {
        case APRS_CHANNEL_READY:c->leased=true;c->frequency=frequency;break;
        case APRS_CHANNEL_BUSY:return APRS_LINK_BUSY;
        case APRS_CHANNEL_DENIED:return APRS_LINK_DENIED;
        default:return APRS_LINK_FAULT;
        }
    }
    if(c->frequency!=frequency) {c->fault=true;return APRS_LINK_DENIED;}
    if(c->ops.interrupted(c->ops.context)) return APRS_LINK_BUSY;
    APRS_LinkResult result=c->ops.send(frame,length,frequency,c->ops.context);
    if(result==APRS_LINK_SENT && frame==gAPRS_TX.frame && gAPRS_TX.needsAcknowledgment) c->ownSent=true;
    if(result==APRS_LINK_FAULT || result==APRS_LINK_DENIED) c->fault=true;
    return result;
}
void CHANNEL_Process(APRS_Channel *c)
{
    if(!c || !c->configured) return;
    if(c->ops.interrupted(c->ops.context) || !SERVICE_Enabled()) {CHANNEL_Interrupt(c);return;}
    if(c->waiting && (gAPRS_TX.state!=APRS_TX_WAIT_ACK ||
       gAPRS_TX.frequency!=c->frequency || Due(Now(c),c->deadline))) {
        /* Restore voice before a future service call may acquire a retry. */
        CHANNEL_Interrupt(c);return;
    }
    c->ownSent=c->fault=false;
    const APRS_TX_Link link={Now,Send,c};SERVICE_Process(&link);
    if(c->ownSent) {c->waiting=true;c->deadline=gAPRS_TX.nextTick;}
    if(c->leased && (!c->waiting || c->fault || gAPRS_TX.state!=APRS_TX_WAIT_ACK)) CHANNEL_Interrupt(c);
}
