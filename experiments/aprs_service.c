/* SPDX-License-Identifier: GPL-3.0-only
 * Outgoing session-only messages and manually requested static beacons.
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
        /* Identity changes stop any pending transmission. */
        TX_CancelAll();settings.enabled=false;
    }
    return settings.enabled;
}
bool SERVICE_Configure(const APRS_ServiceSettings *s)
{
    if(!s) return false;
    if(s->enabled && (!s->frequency || !Interval(s->lifetimeTicks) ||
       !MESSAGE_ValidCall(gAPRS_Profile.callsign))) return false;
    TX_CancelAll();settings=*s;
    if(s->enabled) strcpy(identity,gAPRS_Profile.callsign);
    return true;
}
bool SERVICE_Message(const char *recipient,const char *text,const char *id,uint32_t now)
{
    if(!SERVICE_Enabled()) return false;
    return TX_QueueMessage(settings.frequency,identity,recipient,text,id,
        (APRS_TX_Policy){3000,settings.lifetimeTicks,1},now);
}
bool SERVICE_Beacon(uint32_t now)
{
    return SERVICE_Enabled() && gAPRS_Profile.positionSet &&
        TX_QueueBeacon(settings.frequency,identity,gAPRS_Profile.latitude,
            gAPRS_Profile.longitude,settings.lifetimeTicks,now);
}
void SERVICE_Process(const APRS_TX_Link *link)
{
    if(SERVICE_Enabled()) TX_Process(link);
}
