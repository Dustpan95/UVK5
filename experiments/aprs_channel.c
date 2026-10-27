/* SPDX-License-Identifier: GPL-3.0-only
 * Separate-frequency outgoing APRS ownership. Hardware operations
 * are injected; the optional transport runtime binds the real radio adapter.
 */
#include "aprs_channel.h"
void CHANNEL_Interrupt(APRS_Channel *c)
{
    if(!c || !c->leased) return;
    c->leased=false;c->ops.restore(c->ops.context);
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
    if(result==APRS_LINK_FAULT || result==APRS_LINK_DENIED) c->fault=true;
    return result;
}
void CHANNEL_Process(APRS_Channel *c)
{
    if(!c || !c->configured) return;
    if(c->ops.interrupted(c->ops.context) || !SERVICE_Enabled()) {CHANNEL_Interrupt(c);return;}
    c->fault=false;
    const APRS_TX_Link link={Now,Send,c};SERVICE_Process(&link);
    /* No APRS receiver or ACK-listening lease: restore voice after each burst. */
    CHANNEL_Interrupt(c);
}
