/* SPDX-License-Identifier: Apache-2.0
 * Exact bounded integer scaling for APRS compressed coordinates.
 */
#ifndef UVK5_APRS_SCALE_H
#define UVK5_APRS_SCALE_H
#include <stdint.h>
/* Validated caller bounds: n <= 68,566,680, d is 190463 or 380926.
 * Computes floor(n*1000000/d) without a 64-bit runtime division helper.
 * During the fraction loop, r < 3*d, so every intermediate fits uint32_t.
 */
static uint32_t APRS_ScaleMillion(uint32_t n, uint32_t d)
{
    const uint32_t whole=n/d, remainder=n%d;
    uint32_t fraction=0, r=0;
    for(uint32_t bit=1u<<19;bit;bit>>=1) {
        fraction<<=1;
        r<<=1;
        if(1000000u&bit) r+=remainder;
        if(r>=d) {r-=d;++fraction;}
        if(r>=d) {r-=d;++fraction;}
    }
    return whole*1000000u+fraction;
}
#endif
