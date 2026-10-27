/* SPDX-License-Identifier: GPL-3.0-only
 * One frozen session-only outbox. No radio, EEPROM or automatic send calls.
 */
#include "aprs_tx.h"
#include <string.h>
APRS_TX_Job gAPRS_TX;
static bool Active(void) {return gAPRS_TX.state==APRS_TX_QUEUED;}
static bool Due(uint32_t now,uint32_t deadline) {return !((now-deadline)&0x80000000u);}
static bool Policy(APRS_TX_Policy p)
{
    return p.maxAttempts>=1 && p.maxAttempts<=3 &&
        p.retryTicks>=100 && p.retryTicks<=60000 &&
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
    APRS_TX_Job job={.policy=policy,.message=true,.frequency=frequency};
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

void TX_Tick(uint32_t now)
{
    if(Active() && Due(now,gAPRS_TX.expiresTick))
        gAPRS_TX.state=gAPRS_TX.attempts ? APRS_TX_SENT : APRS_TX_EXPIRED;
}
void TX_Process(const APRS_TX_Link *link)
{
    if(!link || !link->send || !link->now) return;
    APRS_TX_Job *j=&gAPRS_TX;
    uint32_t now=link->now(link->context);TX_Tick(now);
    if(!Active() || !Due(now,j->nextTick)) return;
    APRS_LinkResult result=link->send(j->frame,j->length,j->frequency,link->context);
    if(result==APRS_LINK_BUSY) {j->nextTick=link->now(link->context)+100;return;}
    if(result!=APRS_LINK_SENT) {j->state=result==APRS_LINK_DENIED ? APRS_TX_BLOCKED : APRS_TX_FAILED;return;}
    ++j->attempts;
    j->state=j->attempts<j->policy.maxAttempts ? APRS_TX_QUEUED : APRS_TX_SENT;
    j->nextTick=link->now(link->context)+j->policy.retryTicks;
}
void TX_Cancel(void) {if(Active()) gAPRS_TX.state=APRS_TX_CANCELLED;}
void TX_CancelAll(void) {TX_Cancel();}
