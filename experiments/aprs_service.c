/* SPDX-License-Identifier: GPL-3.0-only
 * Approved automatic personal ACKs and three-attempt message policy.
 * Session RAM only. Runtime supplies explicit configuration and hardware binding;
 * no EEPROM writes or implicit frequency selection.
 */
#include "aprs_service.h"
#include "app/aprs_session.h"
#include <string.h>
static APRS_ServiceSettings settings;
static char identity[10];
static bool Interval(uint32_t n) {return n>=100 && n<=60000;}
bool SERVICE_Enabled(void)
{
    if(settings.enabled && strcmp(identity,gAPRS_Profile.callsign)) {
        /* An identity change cannot leave old automatic replies armed. */
        TX_CancelAll();settings.enabled=false;
    }
    return settings.enabled;
}
bool SERVICE_Configure(const APRS_ServiceSettings *s)
{
    if(!s) return false;
    if(s->enabled && (!s->frequency || !Interval(s->acknowledgmentTicks) ||
       !Interval(s->lifetimeTicks) || !Interval(s->controlLifetimeTicks) ||
       !MESSAGE_ValidCall(gAPRS_Profile.callsign))) return false;
    TX_CancelAll();settings=*s;
    if(s->enabled) strcpy(identity,gAPRS_Profile.callsign);
    return true;
}
bool SERVICE_Message(const char *recipient,const char *text,const char *id,uint32_t now)
{
    if(!SERVICE_Enabled()) return false;
    return TX_QueueMessage(settings.frequency,identity,recipient,text,id,
        (APRS_TX_Policy){settings.acknowledgmentTicks,settings.lifetimeTicks,3},now);
}
bool SERVICE_Beacon(uint32_t now)
{
    return SERVICE_Enabled() && gAPRS_Profile.positionSet &&
        TX_QueueBeacon(settings.frequency,identity,gAPRS_Profile.latitude,
            gAPRS_Profile.longitude,settings.lifetimeTicks,now);
}
APRS_ServiceEvent SERVICE_Receive(const APRS_Record *r,uint32_t now)
{
    const bool enabled=SERVICE_Enabled();
    if(!r) return APRS_SERVICE_IGNORED;
    if(r->kind==APRS_ACK || r->kind==APRS_REJECT) {
        /* Delivery evidence may arrive after stopping retries/disabling TX.
         * It never grants permission to transmit or acknowledges a control. */
        return r->frequency==gAPRS_TX.frequency &&
            TX_Acknowledge(r->source,r->destination,r->messageId,r->kind==APRS_REJECT) ?
            APRS_SERVICE_DELIVERY_UPDATED : APRS_SERVICE_IGNORED;
    }
    if(!enabled || r->frequency!=settings.frequency || !r->messageId[0] ||
       !SESSION_IsPersonal(r)) return APRS_SERVICE_IGNORED;
    return TX_QueueControl(settings.frequency,identity,r->source,r->messageId,false,
        settings.controlLifetimeTicks,now) ? APRS_SERVICE_ACK_QUEUED : APRS_SERVICE_ACK_REFUSED;
}
void SERVICE_Process(const APRS_TX_Link *link)
{
    if(SERVICE_Enabled()) TX_Process(link);
}
