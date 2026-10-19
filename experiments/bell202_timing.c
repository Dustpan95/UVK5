/* SPDX-License-Identifier: GPL-3.0-only
 * Experimental absolute-deadline Bell 202 scheduler. No radio/GPIO accesses.
 * Hardware still has to establish actual oscillator frequency, SPI latency,
 * phase continuity, FM deviation and independent on-air decoding.
 */
#include "bell202_timing.h"
enum { SYMBOL_CYCLES=40000, MAX_LATE=960, MAX_WRITE=8000, MAX_SYMBOLS=4096 };
typedef struct {
    const BELL202_Output *output;
    uint32_t next, writeCycles;
    unsigned symbols;
    BELL202_Result result;
} Timing;
static bool Wait(Timing *t, uint32_t deadline)
{
    const BELL202_Output *o=t->output;
    for(;;) {
        if(o->cancelled(o->context)) {t->result=BELL202_CANCELLED;return false;}
        const uint32_t delta=o->now(o->context)-deadline;
        if(!(delta&0x80000000u)) {
            if(delta>MAX_LATE) {t->result=BELL202_LATE;return false;}
            return true;
        }
    }
}
static void Symbol(bool high, void *context)
{
    Timing *t=context;
    if(t->result!=BELL202_SENT) return;
    if(t->symbols==MAX_SYMBOLS) {t->result=BELL202_TOO_LONG;return;}
    if(!Wait(t,t->next)) return;
    const BELL202_Output *o=t->output;
    uint32_t start=o->now(o->context);
    /* Account for work between polling and register access as well. */
    if(start-t->next>MAX_LATE) {t->result=BELL202_LATE;return;}
    o->tone(high,o->context);
    uint32_t duration=o->now(o->context)-start;
    if(duration>MAX_WRITE) {t->result=BELL202_WRITE_SLOW;return;}
    if(!t->symbols) t->writeCycles=duration;
    else if(duration>t->writeCycles+MAX_LATE || duration+MAX_LATE<t->writeCycles) {
        t->result=BELL202_WRITE_JITTER;return;
    }
    ++t->symbols;t->next+=SYMBOL_CYCLES;
}
BELL202_Result BELL202_Transmit(const uint8_t *frame, size_t length,
                                const BELL202_Output *o)
{
    if(!frame || length<16 || length>330 || !o || !o->now || !o->tone || !o->cancelled)
        return BELL202_INVALID;
    Timing t={.output=o,.next=o->now(o->context)+SYMBOL_CYCLES};
    if(!BELL202_Encode(frame,length,Symbol,&t)) return BELL202_INVALID;
    /* Hold the final tone for its full symbol, including initial write latency. */
    if(t.result==BELL202_SENT) Wait(&t,t.next+t.writeCycles);
    return t.result;
}
