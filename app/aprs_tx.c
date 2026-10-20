/* SPDX-License-Identifier: GPL-3.0-only
 * One frozen session-only outbox. No radio, EEPROM or automatic send calls.
 */
#include "aprs_tx.h"
#include <string.h>
APRS_TX_Job gAPRS_TX;
typedef struct {
    uint8_t frame[35],length;
    uint32_t frequency,nextTick,expiresTick;
} Control;
static Control controls[APRS_CONTROL_CAPACITY];
static uint8_t controlCount;
static bool Active(void)
{
    return gAPRS_TX.state==APRS_TX_QUEUED || gAPRS_TX.state==APRS_TX_WAIT_ACK;
}
static bool Due(uint32_t now,uint32_t deadline) {return !((now-deadline)&0x80000000u);}
static void PurgeControls(uint32_t now)
{
    unsigned kept=0;
    for(unsigned i=0;i<controlCount;i++) if(!Due(now,controls[i].expiresTick))
        controls[kept++]=controls[i];
    controlCount=kept;
}
uint8_t TX_PendingControls(void) {return controlCount;}
static bool Policy(APRS_TX_Policy p)
{
    return p.maxAttempts>=1 && p.maxAttempts<=3 &&
        p.acknowledgmentTicks>=100 && p.acknowledgmentTicks<=60000 &&
        p.lifetimeTicks>=100 && p.lifetimeTicks<=60000;
}
static bool Queue(const APRS_TX_Job *job,uint32_t now)
{
    if(Active() || !job->length || !job->frequency || !Policy(job->policy)) return false;
    gAPRS_TX=*job;gAPRS_TX.state=APRS_TX_QUEUED;
    gAPRS_TX.nextTick=now;gAPRS_TX.expiresTick=now+job->policy.lifetimeTicks;
    return true;
}
bool TX_QueueMessage(uint32_t frequency,const char *sender,const char *recipient,const char *text,
                     const char *id,APRS_TX_Policy policy,uint32_t now)
{
    APRS_TX_Job job={.policy=policy,.needsAcknowledgment=true,.frequency=frequency};
    job.length=MESSAGE_BuildFrame(sender,recipient,text,id,job.frame,sizeof(job.frame));
    if(!job.length) return false;
    strcpy(job.sender,sender);strcpy(job.recipient,recipient);strcpy(job.id,id);
    return Queue(&job,now);
}
bool TX_QueueBeacon(uint32_t frequency,const char *sender,int32_t latitude,int32_t longitude,
                    uint32_t lifetimeTicks,uint32_t now)
{
    APRS_TX_Job job={.policy={100,lifetimeTicks,1},.frequency=frequency};
    job.length=MESSAGE_BuildBeacon(sender,latitude,longitude,'/','-',job.frame,sizeof(job.frame));
    return Queue(&job,now);
}
bool TX_QueueControl(uint32_t frequency,const char *sender,const char *recipient,const char *id,
                     bool rejected,uint32_t lifetimeTicks,uint32_t now)
{
    Control c={.frequency=frequency,.nextTick=now,.expiresTick=now+lifetimeTicks};
    if(!frequency || lifetimeTicks<100 || lifetimeTicks>60000) return false;
    c.length=MESSAGE_BuildControl(sender,recipient,id,rejected,c.frame,sizeof(c.frame));
    if(!c.length) return false;
    PurgeControls(now);
    for(unsigned i=0;i<controlCount;i++) {
        const Control *old=&controls[i];
        /* Same peer, local identity and ID: deduplicate an identical reply,
         * refuse a contradictory ACK/reject while that reply is pending. */
        if(old->frequency==frequency && old->length==c.length &&
           !memcmp(old->frame,c.frame,27) && !memcmp(old->frame+30,c.frame+30,c.length-30))
            return !memcmp(old->frame+27,c.frame+27,3);
    }
    if(controlCount==APRS_CONTROL_CAPACITY) return false;
    controls[controlCount++]=c;return true; /* Queued, not yet transmitted. */
}
void TX_Tick(uint32_t now)
{
    APRS_TX_Job *j=&gAPRS_TX;
    if(Active() && Due(now,j->expiresTick)) {
        j->state=j->attempts ? APRS_TX_UNCONFIRMED : APRS_TX_EXPIRED;
    }
    if(Active() && j->attempts==j->policy.maxAttempts && Due(now,j->nextTick))
        j->state=APRS_TX_UNCONFIRMED;
    PurgeControls(now);
}
void TX_Process(const APRS_TX_Link *link)
{
    APRS_TX_Job *j=&gAPRS_TX;
    if(!link || !link->send || !link->now) return;
    const uint32_t now=link->now(link->context);TX_Tick(now);
    if(controlCount) {
        Control *c=&controls[0];
        if(!Due(now,c->nextTick)) return;
        APRS_LinkResult result=link->send(c->frame,c->length,c->frequency,link->context);
        if(result==APRS_LINK_BUSY) c->nextTick=link->now(link->context)+100;
        else {
            --controlCount;
            memmove(controls,controls+1,controlCount*sizeof(*controls));
        }
        return; /* At most one radio handoff in one foreground service call. */
    }
    if(!Active()) return;
    if(!Due(now,j->nextTick)) return;
    if(j->attempts==j->policy.maxAttempts) {j->state=APRS_TX_UNCONFIRMED;return;}
    APRS_LinkResult result=link->send(j->frame,j->length,j->frequency,link->context);
    if(result==APRS_LINK_BUSY) {j->nextTick=link->now(link->context)+100;return;}
    if(result!=APRS_LINK_SENT) {
        j->state=result==APRS_LINK_DENIED ? APRS_TX_BLOCKED : APRS_TX_FAILED;return;
    }
    ++j->attempts;
    j->state=j->needsAcknowledgment ? APRS_TX_WAIT_ACK : APRS_TX_SENT;
    /* A radio burst can block foreground for seconds. ACK time begins after
     * handoff completes, never from the timestamp before the transmission. */
    j->nextTick=link->now(link->context)+j->policy.acknowledgmentTicks;
}
static bool CallEqual(const char *a,const char *b)
{
    if(!MESSAGE_ValidCall(a) || !MESSAGE_ValidCall(b)) return false;
    return MESSAGE_CallsMatch(a,b);
}
bool TX_Acknowledge(const char *source,const char *addressee,const char *id,bool rejected)
{
    APRS_TX_Job *j=&gAPRS_TX;
    /* Stopping retries does not erase evidence of an earlier complete send.
     * A matching late ACK remains useful until this frozen job is replaced.
     * Do not overturn an ACK/rejection already received for that job.
     */
    if(j->state==APRS_TX_ACKED || j->state==APRS_TX_REJECTED ||
       !j->needsAcknowledgment || !j->attempts || !id ||
       !CallEqual(source,j->recipient) || !CallEqual(addressee,j->sender) || strcmp(id,j->id)) return false;
    j->state=rejected ? APRS_TX_REJECTED : APRS_TX_ACKED;return true;
}
void TX_Cancel(void) {if(Active()) gAPRS_TX.state=APRS_TX_CANCELLED;}
void TX_CancelAll(void) {TX_Cancel();controlCount=0;}
