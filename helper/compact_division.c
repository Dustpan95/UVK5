/* SPDX-License-Identifier: GPL-3.0-only
 * Small shift/subtract division for the original Cortex-M0, which lacks UDIV.
 * Keep separate from LTO: compiler-generated runtime calls use the ARM EABI.
 */
#include "compact_division.h"
extern int __aeabi_idiv0(int value);
static uint64_t Result(uint32_t quotient,uint32_t remainder)
{
    return ((uint64_t)remainder<<32)|quotient;
}
uint64_t COMPACT_UDivMod(uint32_t numerator,uint32_t denominator)
{
    if(!denominator) return Result((uint32_t)__aeabi_idiv0(0),numerator);
    uint32_t bit=1,quotient=0;
    /* numerator>>1 bounds the last left shift, including UINT32_MAX/1. */
    while(denominator<=(numerator>>1)) {denominator<<=1;bit<<=1;}
    do {
        if(numerator>=denominator) {numerator-=denominator;quotient|=bit;}
        denominator>>=1;bit>>=1;
    } while(bit);
    return Result(quotient,numerator);
}
uint64_t COMPACT_SDivMod(int32_t numerator,int32_t denominator)
{
    if(!denominator) return Result((uint32_t)__aeabi_idiv0(0),(uint32_t)numerator);
    uint32_t n=(uint32_t)numerator,d=(uint32_t)denominator;
    if(numerator<0) n=0u-n;
    if(denominator<0) d=0u-d;
    uint64_t result=COMPACT_UDivMod(n,d);
    uint32_t quotient=(uint32_t)result,remainder=(uint32_t)(result>>32);
    if((numerator<0)!=(denominator<0)) quotient=0u-quotient;
    if(numerator<0) remainder=0u-remainder;
    /* Unsigned negation also preserves the old ARM INT_MIN/-1 wrap result. */
    return Result(quotient,remainder);
}
