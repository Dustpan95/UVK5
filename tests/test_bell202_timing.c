/* SPDX-License-Identifier: GPL-3.0-only */
#include "experiments/bell202_timing.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef struct {
    uint32_t clock,first,last,latency,step;
    unsigned count,abortAt,jitterAt,lateAt,expectedCount;
    bool expected[4096];
} Fake;
static uint32_t Now(void *context)
{
    Fake *f=context;f->clock+=f->step;
    return f->clock;
}
static void Tone(bool high,void *context)
{
    Fake *f=context;
    assert(f->count<f->expectedCount && high==f->expected[f->count]);
    if(!f->count) f->first=f->clock;
    /* SPI work must not accumulate into the next symbol deadline. */
    assert(f->clock-f->first>=f->count*40000u-200u || !f->count);
    assert(f->clock-f->first<=f->count*40000u+200u);
    f->last=f->clock;
    ++f->count;f->clock+=f->latency;
    if(f->count==f->jitterAt) f->clock+=2000;
    if(f->count==f->lateAt) f->clock+=40000;
}
static bool Cancel(void *context)
{
    Fake *f=context;
    return f->abortAt && f->count==f->abortAt;
}
static void Expected(bool high,void *context)
{
    Fake *f=context;assert(f->expectedCount<4096);f->expected[f->expectedCount++]=high;
}
static Fake Init(const uint8_t *frame,size_t n)
{
    Fake f={.step=100,.latency=3600};
    assert(BELL202_Encode(frame,n,Expected,&f));return f;
}
int main(void)
{
    uint8_t frame[330];memset(frame,0xff,sizeof(frame));
    Fake f=Init(frame,sizeof(frame));
    BELL202_Output output={Now,Tone,Cancel,&f};
    assert(BELL202_Transmit(frame,sizeof(frame),&output)==BELL202_SENT);
    assert(f.count==f.expectedCount && f.count<4096);
    assert(f.clock-f.last>=40000u+f.latency);
    f=Init(frame,sizeof(frame));f.clock=UINT32_MAX-200000;
    assert(BELL202_Transmit(frame,sizeof(frame),&output)==BELL202_SENT);
    assert(f.count==f.expectedCount);
    f=Init(frame,16);f.abortAt=47;
    assert(BELL202_Transmit(frame,16,&output)==BELL202_CANCELLED && f.count==47);
    f=Init(frame,16);f.jitterAt=47;
    assert(BELL202_Transmit(frame,16,&output)==BELL202_WRITE_JITTER && f.count==47);
    f=Init(frame,16);f.latency=9000;
    assert(BELL202_Transmit(frame,16,&output)==BELL202_WRITE_SLOW && f.count==1);
    f=Init(frame,16);f.lateAt=47;
    assert(BELL202_Transmit(frame,16,&output)==BELL202_WRITE_SLOW && f.count==47);
    f=Init(frame,16);f.step=5000;
    assert(BELL202_Transmit(frame,16,&output)==BELL202_LATE && !f.count);
    assert(BELL202_Transmit(frame,15,&output)==BELL202_INVALID);
    assert(BELL202_Transmit(frame,16,0)==BELL202_INVALID);
    assert(BELL202_Transmit(0,16,&output)==BELL202_INVALID);
    puts("PASS absolute 1200-baud deadlines, longest stuffed frame, cycle wrap, cancellation and timing fault abort");
}
