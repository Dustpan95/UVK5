/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include "experiments/bell202_symbols.h"
#include "experiments/bell202_timing.h"
#include "app/aprs_message.h"
#include <string.h>
static void emit(bool high, void *context) { fputc(high?'1':'0',context); }
typedef struct {uint32_t clock,nextIRQ,random;} Trace;
static uint32_t now(void *context)
{
    Trace *t=context;t->clock+=64;
    if(t->clock>=t->nextIRQ) {t->clock+=300;t->nextIRQ+=480000;}
    return t->clock;
}
static bool cancelled(void *context) {(void)context;return false;}
static void tone(bool high,void *context)
{
    Trace *t=context;
    t->random=t->random*1664525u+1013904223u;
    /* 75 us SPI cost with +/- 6.7 us variation, plus simulated 10 ms IRQs. */
    t->clock+=3280+t->random%641;
    printf("%u,%u\n",t->clock,high ? 2200 : 1200);
}
int main(int argc, char **argv) {
    uint8_t frame[330]; size_t n;
    bool timed=argc>1 && !strcmp(argv[1],"--timed");
    if(timed) {--argc;++argv;}
    if(argc==2 && !strcmp(argv[1],"--message")) {
        char text[69];n=fread(text,1,sizeof(text)-1,stdin);text[n]=0;
        n=MESSAGE_BuildFrame("N0CALL-15","MYCALL",text,"01",frame,sizeof(frame));
        if(!n) return 1;
    } else if(argc==2 && !strcmp(argv[1],"--beacon")) {
        n=MESSAGE_BuildBeacon("N0CALL-15",49058333,-72029166,'/','-',frame,sizeof(frame));
        if(!n) return 1;
    } else if(argc==2 && (!strcmp(argv[1],"--ack") || !strcmp(argv[1],"--reject"))) {
        n=MESSAGE_BuildControl("N0CALL-15","MYCALL","aZ123",!strcmp(argv[1],"--reject"),frame,sizeof(frame));
        if(!n) return 1;
    } else n=fread(frame,1,sizeof(frame),stdin);
    if(!timed) return BELL202_Encode(frame,n,emit,stdout)?0:1;
    Trace trace={.nextIRQ=480000,.random=19};
    const BELL202_Output output={now,tone,cancelled,&trace};
    BELL202_Result result=BELL202_Transmit(frame,n,&output);
    printf("%u,0\n",trace.clock);
    return result==BELL202_SENT ? 0 : 1;
}
