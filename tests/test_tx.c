/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_tx.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint32_t tick;
static unsigned sends;
static APRS_LinkResult result=APRS_LINK_SENT;
static uint8_t wire[100];
static uint32_t Now(void *c) {(void)c;return tick;}
static APRS_LinkResult Send(const uint8_t *f,size_t n,uint32_t frequency,void *c)
{(void)c;assert(frequency==14439000 && n<=sizeof(wire));memcpy(wire,f,n);++sends;tick+=70;return result;}
static const APRS_TX_Link link={Now,Send,0};
int main(void)
{
    APRS_TX_Policy once={3000,60000,1};
    char body[]="FROZEN";
    assert(!TX_QueueMessage(0,"MYCALL","PEER",body,"1",once,tick));
    assert(TX_QueueMessage(14439000,"MYCALL","PEER",body,"1",once,tick));
    strcpy(body,"EDITED");assert(!TX_QueueBeacon(14439000,"MYCALL",0,0,60000,tick));
    result=APRS_LINK_BUSY;TX_Process(&link);assert(sends==1 && !gAPRS_TX.attempts);
    assert(gAPRS_TX.nextTick==tick+100);tick=gAPRS_TX.nextTick;
    result=APRS_LINK_SENT;TX_Process(&link);assert(sends==2 && gAPRS_TX.attempts==1 && gAPRS_TX.state==APRS_TX_SENT);
    assert(!memcmp(wire+27,"FROZEN{1",8));
    for(unsigned i=0;i<1000;i++){tick+=100;TX_Process(&link);}assert(sends==2);
    assert(TX_QueueBeacon(14439000,"MYCALL",0,0,60000,tick));TX_Process(&link);
    assert(!gAPRS_TX.message && gAPRS_TX.state==APRS_TX_SENT && wire[16]=='!');
    assert(TX_QueueMessage(14439000,"MYCALL","PEER","CANCEL","2",once,tick));
    TX_CancelAll();TX_Process(&link);assert(gAPRS_TX.state==APRS_TX_CANCELLED && sends==3);
    tick=0xfffffff0u;assert(TX_QueueBeacon(14439000,"MYCALL",0,0,100,tick));
    tick+=100;TX_Tick(tick);assert(gAPRS_TX.state==APRS_TX_EXPIRED);
    assert(TX_QueueMessage(14439000,"MYCALL","PEER","DENIED","3",once,tick));
    result=APRS_LINK_DENIED;TX_Process(&link);assert(gAPRS_TX.state==APRS_TX_BLOCKED && !gAPRS_TX.attempts);
    assert(TX_QueueBeacon(14439000,"MYCALL",0,0,1000,tick));
    result=APRS_LINK_FAULT;TX_Process(&link);assert(gAPRS_TX.state==APRS_TX_FAILED);
    puts("PASS TX-only queue: frozen bytes/frequency, busy deferral, one completed send, no repeats, explicit beacon, cancellation, wraparound expiry and denied/fault handling");
}
