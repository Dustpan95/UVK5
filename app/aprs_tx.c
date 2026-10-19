/* SPDX-License-Identifier: GPL-3.0-only
 * One frozen session-only outbox. No radio, EEPROM or automatic send calls.
 */
#include "aprs_tx.h"
#include <string.h>
APRS_TX_Job gAPRS_TX;
static bool Active(void)
{
    return gAPRS_TX.state==APRS_TX_QUEUED || gAPRS_TX.state==APRS_TX_WAIT_ACK;
}
static bool Due(uint32_t now,uint32_t deadline) {return !((now-deadline)&0x80000000u);}
static bool Policy(APRS_TX_Policy p)
{
    return p.maxAttempts>=1 && p.maxAttempts<=3 &&
        p.acknowledgmentTicks>=100 && p.acknowledgmentTicks<=60000 &&
        p.lifetimeTicks>=100 && p.lifetimeTicks<=60000;
}
static bool Queue(const APRS_TX_Job *job,uint32_t now)
{
    if(Active() || !job->length || !Policy(job->policy)) return false;
    gAPRS_TX=*job;gAPRS_TX.state=APRS_TX_QUEUED;
    gAPRS_TX.nextTick=now;gAPRS_TX.expiresTick=now+job->policy.lifetimeTicks;
    return true;
}
bool TX_QueueMessage(const char *sender,const char *recipient,const char *text,
                     const char *id,APRS_TX_Policy policy,uint32_t now)
{
    APRS_TX_Job job={.policy=policy,.needsAcknowledgment=true};
    job.length=MESSAGE_BuildFrame(sender,recipient,text,id,job.frame,sizeof(job.frame));
    if(!job.length) return false;
    strcpy(job.sender,sender);strcpy(job.recipient,recipient);strcpy(job.id,id);
    return Queue(&job,now);
}
bool TX_QueueBeacon(const char *sender,int32_t latitude,int32_t longitude,
                    uint32_t lifetimeTicks,uint32_t now)
{
    APRS_TX_Job job={.policy={100,lifetimeTicks,1}};
    job.length=MESSAGE_BuildBeacon(sender,latitude,longitude,'/','-',job.frame,sizeof(job.frame));
    return Queue(&job,now);
}
bool TX_QueueControl(const char *sender,const char *recipient,const char *id,
                     bool rejected,uint32_t lifetimeTicks,uint32_t now)
{
    APRS_TX_Job job={.policy={100,lifetimeTicks,1}};
    job.length=MESSAGE_BuildControl(sender,recipient,id,rejected,job.frame,sizeof(job.frame));
    return Queue(&job,now);
}
void TX_Process(const APRS_TX_Link *link)
{
    APRS_TX_Job *j=&gAPRS_TX;
    if(!Active() || !link || !link->send || !link->now) return;
    const uint32_t now=link->now(link->context);
    if(Due(now,j->expiresTick)) {
        j->state=j->attempts ? APRS_TX_UNCONFIRMED : APRS_TX_EXPIRED;return;
    }
    if(!Due(now,j->nextTick)) return;
    if(j->attempts==j->policy.maxAttempts) {j->state=APRS_TX_UNCONFIRMED;return;}
    APRS_LinkResult result=link->send(j->frame,j->length,link->context);
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
    size_t an=strlen(a),bn=strlen(b);
    if(an>=2 && a[an-2]=='-' && a[an-1]=='0') an-=2;
    if(bn>=2 && b[bn-2]=='-' && b[bn-1]=='0') bn-=2;
    return an==bn && !memcmp(a,b,an);
}
bool TX_Acknowledge(const char *source,const char *addressee,const char *id,bool rejected)
{
    APRS_TX_Job *j=&gAPRS_TX;
    if(!Active() || !j->needsAcknowledgment || !j->attempts || !id ||
       !CallEqual(source,j->recipient) || !CallEqual(addressee,j->sender) || strcmp(id,j->id)) return false;
    j->state=rejected ? APRS_TX_REJECTED : APRS_TX_ACKED;return true;
}
void TX_Cancel(void) {if(Active()) gAPRS_TX.state=APRS_TX_CANCELLED;}
