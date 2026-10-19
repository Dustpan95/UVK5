/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_tx.h"
#include "app/aprs_decode.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint32_t tick,duration;
static unsigned sends;
static APRS_LinkResult result;
static uint8_t emitted[100];
static size_t emittedLength;
static uint32_t Now(void *context) {(void)context;return tick;}
static APRS_LinkResult Send(const uint8_t *frame,size_t n,void *context)
{
    (void)context;++sends;assert(n<=sizeof(emitted));
    memcpy(emitted,frame,n);emittedLength=n;tick+=duration;return result;
}
static const APRS_TX_Link link={Now,Send,0};
static const APRS_TX_Policy retry={1000,20000,3};
static void Init(uint32_t now)
{
    memset(&gAPRS_TX,0,sizeof(gAPRS_TX));sends=0;tick=now;duration=70;result=APRS_LINK_SENT;
}
static void Message(APRS_TX_Policy policy)
{
    assert(TX_QueueMessage("MYCALL-0","N0CALL-7","HELLO","123",policy,tick));
}
static void Parse(APRS_Kind kind)
{
    uint8_t packet[102];memcpy(packet,emitted,emittedLength);
    uint16_t fcs=APRS_FCS(packet,emittedLength);packet[emittedLength]=fcs;packet[emittedLength+1]=fcs>>8;
    APRS_Record record;assert(APRS_ParseFrame(packet,emittedLength+2,&record) && record.kind==kind);
}
int main(void)
{
    Init(100);TX_Process(&link);assert(!sends);
    char text[]="FROZEN TEXT";
    assert(TX_QueueMessage("MYCALL-0","N0CALL-7",text,"123",retry,tick));
    strcpy(text,"CHANGED");
    assert(!TX_Acknowledge("N0CALL-7","MYCALL","123",false));
    APRS_TX_Job old=gAPRS_TX;
    assert(!TX_QueueBeacon("MYCALL",0,0,1000,tick) && !memcmp(&old,&gAPRS_TX,sizeof(old)));
    TX_Process(&link);Parse(APRS_MESSAGE);
    assert(!memcmp(emitted+27,"FROZEN TEXT{123",15));
    assert(gAPRS_TX.state==APRS_TX_WAIT_ACK && gAPRS_TX.attempts==1);
    assert(gAPRS_TX.nextTick==tick+1000);
    assert(!TX_Acknowledge("N0CALL-8","MYCALL","123",false));
    assert(!TX_Acknowledge("N0CALL-7","MYCALL-1","123",false));
    assert(!TX_Acknowledge("N0CALL-7","MYCALL","124",false));
    assert(TX_Acknowledge("N0CALL-7","MYCALL","123",false));
    assert(gAPRS_TX.state==APRS_TX_ACKED);
    tick+=2000;TX_Process(&link);assert(sends==1);
    Init(UINT32_MAX-500);Message(retry);
    uint8_t frozen[100];memcpy(frozen,gAPRS_TX.frame,gAPRS_TX.length);
    for(unsigned attempt=1;attempt<=3;attempt++) {
        TX_Process(&link);assert(gAPRS_TX.attempts==attempt && sends==attempt);
        assert(!memcmp(frozen,emitted,emittedLength));
        tick=gAPRS_TX.nextTick-1;TX_Process(&link);assert(sends==attempt);
        ++tick;
    }
    TX_Process(&link);assert(sends==3 && gAPRS_TX.state==APRS_TX_UNCONFIRMED);
    Init(0);Message((APRS_TX_Policy){1000,20000,1});TX_Process(&link);
    tick=gAPRS_TX.nextTick;TX_Process(&link);
    assert(sends==1 && gAPRS_TX.state==APRS_TX_UNCONFIRMED);
    Init(0);Message(retry);result=APRS_LINK_BUSY;
    TX_Process(&link);assert(sends==1 && !gAPRS_TX.attempts);
    tick=gAPRS_TX.nextTick-1;TX_Process(&link);assert(sends==1);
    ++tick;result=APRS_LINK_SENT;TX_Process(&link);
    assert(sends==2 && gAPRS_TX.attempts==1);
    tick=gAPRS_TX.nextTick;result=APRS_LINK_BUSY;TX_Process(&link);
    assert(gAPRS_TX.attempts==1);
    assert(TX_Acknowledge("N0CALL-7","MYCALL","123",true));
    assert(gAPRS_TX.state==APRS_TX_REJECTED);
    Init(0);Message(retry);TX_Cancel();TX_Process(&link);
    assert(!sends && gAPRS_TX.state==APRS_TX_CANCELLED);
    Init(0);Message(retry);result=APRS_LINK_DENIED;TX_Process(&link);
    assert(!gAPRS_TX.attempts && gAPRS_TX.state==APRS_TX_BLOCKED);
    Init(0);Message(retry);result=APRS_LINK_FAULT;TX_Process(&link);
    assert(!gAPRS_TX.attempts && gAPRS_TX.state==APRS_TX_FAILED);
    Init(0);Message(retry);tick=20000;TX_Process(&link);
    assert(!sends && gAPRS_TX.state==APRS_TX_EXPIRED);
    Init(0);Message(retry);TX_Process(&link);tick=20000;TX_Process(&link);
    assert(sends==1 && gAPRS_TX.state==APRS_TX_UNCONFIRMED);
    Init(0);assert(TX_QueueBeacon("MYCALL",49058333,-72029166,1000,tick));
    TX_Process(&link);Parse(APRS_POSITION);
    assert(gAPRS_TX.state==APRS_TX_SENT && !TX_Acknowledge("MYCALL","MYCALL","123",false));
    assert(TX_QueueControl("MYCALL","N0CALL","a1",false,1000,tick));
    TX_Process(&link);Parse(APRS_ACK);assert(gAPRS_TX.state==APRS_TX_SENT);
    assert(!TX_QueueMessage("MYCALL","N0CALL","HELLO","1",(APRS_TX_Policy){1000,1000,0},tick));
    assert(!TX_QueueMessage("MYCALL","N0CALL","HELLO","1",(APRS_TX_Policy){1000,1000,4},tick));
    assert(!TX_QueueBeacon("MYCALL",90000001,0,1000,tick));
    puts("PASS frozen outbox, correlated ACK/reject, bounded one/three-attempt policy, busy deferral, cancel/error/expiry and time wrap");
    puts("PASS ACK timer starts after simulated radio handoff; message/beacon/control transport frames roundtrip");
}
