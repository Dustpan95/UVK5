/* SPDX-License-Identifier: GPL-3.0-only */
#include "experiments/aprs_service.h"
#include "app/aprs_session.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint32_t tick;
static unsigned sends;
static uint8_t wire[102];
static size_t wireLength;
static APRS_LinkResult result;
static const APRS_ServiceSettings config={14439000,APRS_ACK_DEFAULT_TICKS,20000,1000,true};
static uint32_t Now(void *context) {(void)context;return tick;}
static APRS_LinkResult Send(const uint8_t *frame,size_t n,uint32_t frequency,void *context)
{
    (void)context;assert(frequency==config.frequency && n<=100);
    memcpy(wire,frame,n);wireLength=n;++sends;tick+=50;return result;
}
static const APRS_TX_Link link={Now,Send,0};
static APRS_Record Parse(const uint8_t *frame,size_t n)
{
    uint8_t packet[102];assert(n<=100);memcpy(packet,frame,n);
    uint16_t crc=APRS_FCS(packet,n);packet[n]=crc;packet[n+1]=crc>>8;
    APRS_Record r;assert(APRS_ParseFrame(packet,n+2,&r));r.frequency=config.frequency;return r;
}
static APRS_Record Incoming(const char *recipient,const char *id)
{
    uint8_t frame[100];size_t n=MESSAGE_BuildFrame("N0CALL-7",recipient,"HELLO",id,frame,sizeof(frame));
    return Parse(frame,n);
}
static void Init(void)
{
    APRS_ServiceSettings off={0};assert(SERVICE_Configure(&off));
    memset(&gAPRS_TX,0,sizeof(gAPRS_TX));memset(&gAPRS_Profile,0,sizeof(gAPRS_Profile));
    assert(SESSION_SetCall("MYCALL"));tick=0;sends=0;result=APRS_LINK_SENT;
}
int main(void)
{
    Init();APRS_Record r=Incoming("MYCALL-0","123");
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_IGNORED);
    assert(!SERVICE_Message("N0CALL","HELLO","1",tick));SERVICE_Process(&link);assert(!sends);
    assert(SERVICE_Configure(&config));
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED && TX_PendingControls()==1);
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED && TX_PendingControls()==1);
    SERVICE_Process(&link);assert(sends==1 && !TX_PendingControls());
    APRS_Record ack=Parse(wire,wireLength);
    assert(ack.kind==APRS_ACK && !strcmp(ack.source,"MYCALL") &&
           !strcmp(ack.destination,"N0CALL-7") && !strcmp(ack.messageId,"123"));
    assert(SERVICE_Receive(&ack,tick)==APRS_SERVICE_IGNORED && !TX_PendingControls());
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED); /* ACK a later repeat too. */
    r=Incoming("OTHER","124");assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_IGNORED);
    r=Incoming("MYCALL","124");r.frequency++;
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_IGNORED);
    r.frequency=config.frequency;r.messageId[0]=0;
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_IGNORED);
    r=Incoming("MYCALL","124");assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED);
    r=Incoming("MYCALL","125");assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_REFUSED);
    assert(TX_PendingControls()==2);
    Init();assert(SERVICE_Configure(&config));
    assert(SERVICE_Message("N0CALL-7","FROZEN","123",tick));
    for(unsigned i=1;i<=3;++i) {
        SERVICE_Process(&link);assert(sends==i && gAPRS_TX.attempts==i);
        tick=gAPRS_TX.nextTick;
    }
    SERVICE_Process(&link);assert(sends==3 && gAPRS_TX.state==APRS_TX_UNCONFIRMED);
    uint8_t frame[35];size_t n=MESSAGE_BuildControl("N0CALL-7","MYCALL","123",false,frame,sizeof(frame));
    ack=Parse(frame,n);ack.frequency++;
    assert(SERVICE_Receive(&ack,tick)==APRS_SERVICE_IGNORED);
    ack.frequency=config.frequency;
    assert(SERVICE_Receive(&ack,tick)==APRS_SERVICE_DELIVERY_UPDATED && gAPRS_TX.state==APRS_TX_ACKED);
    Init();assert(SERVICE_Configure(&config));assert(!SERVICE_Beacon(tick));
    assert(SESSION_SetPosition(49058333,-72029166));assert(SERVICE_Beacon(tick));
    SERVICE_Process(&link);assert(Parse(wire,wireLength).kind==APRS_POSITION && sends==1);
    Init();assert(SERVICE_Configure(&config));assert(SERVICE_Message("N0CALL-7","HELLO","123",tick));
    SERVICE_Process(&link);r=Incoming("MYCALL","456");
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED);
    assert(SESSION_SetCall("NEWCAL"));SERVICE_Process(&link);
    assert(!SERVICE_Enabled() && !TX_PendingControls() && sends==1 && gAPRS_TX.state==APRS_TX_CANCELLED);
    Init();assert(SERVICE_Configure(&config));r=Incoming("MYCALL","123");
    assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED);
    APRS_ServiceSettings invalid=config;invalid.frequency=0;
    assert(!SERVICE_Configure(&invalid) && SERVICE_Enabled() && TX_PendingControls()==1);
    APRS_ServiceSettings off={0};assert(SERVICE_Configure(&off));SERVICE_Process(&link);
    assert(!SERVICE_Enabled() && !TX_PendingControls() && !sends);
    assert(!SERVICE_Configure(0));
    Init();assert(SERVICE_Configure(&config));assert(SERVICE_Message("N0CALL-7","HELLO","123",tick));
    SERVICE_Process(&link);assert(SERVICE_Configure(&off));
    n=MESSAGE_BuildControl("N0CALL-7","MYCALL","123",false,frame,sizeof(frame));ack=Parse(frame,n);
    assert(SERVICE_Receive(&ack,tick)==APRS_SERVICE_DELIVERY_UPDATED && !SERVICE_Enabled());
    SERVICE_Process(&link);assert(sends==1 && !TX_PendingControls());
    Init();assert(SERVICE_Configure(&config));assert(SERVICE_Message("N0CALL-7","HELLO","123",tick));
    SERVICE_Process(&link);APRS_TX_Job frozen=gAPRS_TX;
    r=Incoming("MYCALL","ABC");assert(SERVICE_Receive(&r,tick)==APRS_SERVICE_ACK_QUEUED);
    result=APRS_LINK_BUSY;SERVICE_Process(&link);
    assert(!memcmp(&frozen,&gAPRS_TX,sizeof(frozen)) && TX_PendingControls()==1);
    result=APRS_LINK_SENT;tick+=100;SERVICE_Process(&link);
    assert(Parse(wire,wireLength).kind==APRS_ACK && !memcmp(&frozen,&gAPRS_TX,sizeof(frozen)));
    puts("PASS explicit session TX enable, personal automatic ACK filtering/dedup/full queue, three total attempts, frequency-matched delivery, beacon validation and cancellation on disable/identity change");
}
