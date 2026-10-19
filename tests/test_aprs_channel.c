/* SPDX-License-Identifier: GPL-3.0-only */
#include "experiments/aprs_channel.h"
#include "app/aprs_session.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static const APRS_ServiceSettings settings={14439000,APRS_ACK_DEFAULT_TICKS,60000,1000,true};
static uint32_t now,frequency;
static unsigned enters,sends,restores;
static bool interrupted,interruptOnEnter;
static APRS_ChannelAdmission admission;
static APRS_LinkResult result;
static APRS_Channel channel;
static uint32_t Now(void *ctx) {(void)ctx;return now;}
static APRS_ChannelAdmission Enter(uint32_t f,void *ctx)
{
    (void)ctx;assert(f==settings.frequency && frequency==14652000);++enters;
    if(admission==APRS_CHANNEL_READY) frequency=f;
    if(interruptOnEnter) interrupted=true;
    return admission;
}
static APRS_LinkResult Send(const uint8_t *frame,size_t n,uint32_t f,void *ctx)
{
    (void)ctx;assert(frame && n>=16 && f==settings.frequency && frequency==f && !interrupted);
    ++sends;now+=50;return result;
}
static void Restore(void *ctx)
{
    (void)ctx;assert(frequency==settings.frequency && !channel.leased);
    ++restores;frequency=14652000;
}
static bool Interrupted(void *ctx) {(void)ctx;return interrupted;}
static const APRS_ChannelOps ops={Now,Enter,Send,Restore,Interrupted,0};
static void Init(uint32_t start)
{
    CHANNEL_Interrupt(&channel);memset(&channel,0,sizeof(channel));
    APRS_ServiceSettings off={0};assert(SERVICE_Configure(&off));memset(&gAPRS_TX,0,sizeof(gAPRS_TX));
    memset(&gAPRS_Profile,0,sizeof(gAPRS_Profile));assert(SESSION_SetCall("MYCALL"));
    assert(SERVICE_Configure(&settings));assert(CHANNEL_Configure(&channel,&ops));
    now=start;frequency=14652000;enters=sends=restores=0;interrupted=interruptOnEnter=false;
    admission=APRS_CHANNEL_READY;result=APRS_LINK_SENT;
}
static void Message(void) {assert(SERVICE_Message("N0CALL-7","HELLO","123",now));}
int main(void)
{
    Init(0);Message();CHANNEL_Process(&channel);
    assert(enters==1 && sends==1 && !restores && channel.waiting && frequency==settings.frequency);
    assert(channel.deadline==50+1000 && gAPRS_TX.nextTick==channel.deadline);
    now=channel.deadline-1;CHANNEL_Process(&channel);assert(!restores && sends==1);
    assert(TX_Acknowledge("N0CALL-7","MYCALL","123",false));CHANNEL_Process(&channel);
    assert(restores==1 && frequency==14652000 && !channel.waiting);
    CHANNEL_Process(&channel);assert(enters==1 && sends==1 && restores==1);
    Init(UINT32_MAX-500);Message();
    for(unsigned i=1;i<=3;++i) {
        CHANNEL_Process(&channel);assert(sends==i && channel.waiting);
        now=channel.deadline;CHANNEL_Process(&channel);
        assert(restores==i && frequency==14652000 && !channel.leased && sends==i);
    }
    CHANNEL_Process(&channel);assert(sends==3 && gAPRS_TX.state==APRS_TX_UNCONFIRMED);
    Init(0);Message();CHANNEL_Process(&channel);now=100;interrupted=true;
    uint32_t deadline=gAPRS_TX.nextTick;CHANNEL_Process(&channel);
    assert(restores==1 && sends==1 && frequency==14652000 && gAPRS_TX.nextTick==deadline);
    CHANNEL_Process(&channel);assert(restores==1 && enters==1);
    interrupted=false;CHANNEL_Process(&channel);assert(enters==1); /* No early retry. */
    Init(0);Message();admission=APRS_CHANNEL_BUSY;CHANNEL_Process(&channel);
    assert(!sends && !restores && !channel.leased && frequency==14652000);
    now=gAPRS_TX.nextTick;admission=APRS_CHANNEL_DENIED;CHANNEL_Process(&channel);
    assert(!sends && gAPRS_TX.state==APRS_TX_BLOCKED && !restores);
    Init(0);Message();result=APRS_LINK_FAULT;CHANNEL_Process(&channel);
    assert(sends==1 && restores==1 && !channel.leased && gAPRS_TX.state==APRS_TX_FAILED);
    Init(0);Message();interruptOnEnter=true;CHANNEL_Process(&channel);
    assert(enters==1 && !sends && restores==1 && frequency==14652000);
    Init(0);assert(SESSION_SetPosition(0,0));assert(SERVICE_Beacon(now));CHANNEL_Process(&channel);
    assert(sends==1 && restores==1 && !channel.waiting && gAPRS_TX.state==APRS_TX_SENT);
    Init(0);Message();CHANNEL_Process(&channel);
    APRS_ServiceSettings off={0};assert(SERVICE_Configure(&off));CHANNEL_Process(&channel);
    assert(restores==1 && !channel.leased && gAPRS_TX.state==APRS_TX_CANCELLED);
    Init(0);Message();CHANNEL_Process(&channel);assert(CHANNEL_Configure(&channel,&ops));
    assert(restores==1 && !channel.leased);
    assert(!CHANNEL_Configure(&channel,0));CHANNEL_Interrupt(&channel);assert(restores==1);
    Init(0);Message();CHANNEL_Process(&channel);deadline=channel.deadline;
    assert(TX_QueueControl(settings.frequency,"MYCALL","OTHER","ABC",false,1000,now));
    CHANNEL_Process(&channel);
    assert(sends==2 && enters==1 && !restores && channel.waiting && channel.deadline==deadline);
    assert(TX_QueueControl(settings.frequency,"MYCALL","OTHER","ABD",false,1000,now));
    result=APRS_LINK_FAULT;CHANNEL_Process(&channel);
    assert(restores==1 && !channel.leased && gAPRS_TX.state==APRS_TX_WAIT_ACK && gAPRS_TX.nextTick==deadline);
    Init(0);APRS_ServiceSettings shortLife=settings;shortLife.lifetimeTicks=100;
    assert(SERVICE_Configure(&shortLife));Message();CHANNEL_Process(&channel);
    now=100;CHANNEL_Process(&channel);
    assert(restores==1 && !channel.leased && gAPRS_TX.state==APRS_TX_UNCONFIRMED);
    Init(0);Message();CHANNEL_Process(&channel);
    APRS_ChannelOps invalid=ops;invalid.restore=0;
    assert(!CHANNEL_Configure(&channel,&invalid) && channel.leased && !restores);
    CHANNEL_Interrupt(&channel);assert(restores==1);
    puts("PASS separate-channel model: tune before send, 10-second ACK deadline after handoff, early ACK/PTT/fault/disable restoration, three attempts and wrap-safe waits");
}
