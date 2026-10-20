/* SPDX-License-Identifier: GPL-3.0-only */
#include "helper/compact_division.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
static int handlerValue;
int __aeabi_idiv0(int value) {assert(value==0);return handlerValue;}
static unsigned checks;
static void Check(uint32_t numerator,uint32_t denominator)
{
    uint64_t actual=COMPACT_UDivMod(numerator,denominator);
    uint32_t q=denominator ? numerator/denominator : (uint32_t)handlerValue;
    uint32_t r=denominator ? numerator%denominator : numerator;
    assert((uint32_t)actual==q && (uint32_t)(actual>>32)==r);
    int32_t n=(int32_t)numerator,d=(int32_t)denominator;
    actual=COMPACT_SDivMod(n,d);
    if(d) {
        /* int64_t oracle avoids C's INT32_MIN/-1 overflow. */
        int64_t quotient=(int64_t)n/d;
        q=(uint32_t)quotient;r=(uint32_t)((int64_t)n-quotient*d);
    } else {q=(uint32_t)handlerValue;r=numerator;}
    assert((uint32_t)actual==q && (uint32_t)(actual>>32)==r);
    checks+=2;
}
int main(void)
{
    uint32_t boundary[]={0,1,2,3,7,8,15,16,31,32,63,64,127,128,255,256,511,512,
        1023,1024,65535,65536,0x3fffffff,0x40000000,0x7ffffffe,0x7fffffff,
        0x80000000,0x80000001,0xfffffffe,0xffffffff};
    for(unsigned i=0;i<sizeof(boundary)/sizeof(*boundary);i++)
        for(unsigned j=0;j<sizeof(boundary)/sizeof(*boundary);j++) Check(boundary[i],boundary[j]);
    for(unsigned bit=0;bit<32;bit++) for(unsigned n=0;n<65536;n++) {
        Check(n,1u<<bit);Check(0u-n,1u<<bit);
    }
    uint32_t random=0x83e9bffe;
    for(unsigned i=0;i<1000000;i++) {
        random=random*1664525u+1013904223u;uint32_t n=random;
        random=random*1664525u+1013904223u;uint32_t d=(i&1023) ? random : 0;
        Check(n,d);
    }
    handlerValue=-123;
    for(unsigned i=0;i<sizeof(boundary)/sizeof(*boundary);i++) Check(boundary[i],0);
    printf("PASS compact division: %u unsigned/signed oracle checks, boundaries and divide-zero hook\n",checks);
}
