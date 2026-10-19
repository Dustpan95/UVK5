/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include "experiments/bell202_symbols.h"
#include "app/aprs_message.h"
#include <string.h>
static void emit(bool high, void *context) { fputc(high?'1':'0',context); }
int main(int argc, char **argv) {
    uint8_t frame[330]; size_t n;
    if(argc==2 && !strcmp(argv[1],"--message")) {
        char text[69];n=fread(text,1,sizeof(text)-1,stdin);text[n]=0;
        n=MESSAGE_BuildFrame("N0CALL-15","MYCALL",text,"01",frame,sizeof(frame));
        if(!n) return 1;
    } else n=fread(frame,1,sizeof(frame),stdin);
    return BELL202_Encode(frame,n,emit,stdout)?0:1;
}
