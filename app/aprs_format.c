/* SPDX-License-Identifier: GPL-3.0-only
 * Shared fixed-position text for the setup review and AX.25 position body.
 */
#include "app/aprs_format.h"
static void __attribute__((noinline)) FixedDigits(char *out,unsigned value,unsigned width)
{
    while(width) {out[--width]='0'+value%10u;value/=10u;}
}
unsigned __attribute__((noinline)) APRS_FormatDM(char *out, int32_t value, bool longitude, bool spaced)
{
    const uint32_t absolute=value<0 ? 0u-(uint32_t)value : (uint32_t)value;
    /* Round to the nearest hundredth minute, including degree carry. Callers
     * enforce the documented latitude/longitude bounds; arithmetic fits in 32 bits. */
    const uint32_t hundredths=(absolute*3u+250u)/500u;
    unsigned degrees=hundredths/6000u, minutes=hundredths%6000u;
    unsigned n=longitude ? 3:2;
    FixedDigits(out,degrees,n);
    if(spaced) out[n++]=' ';
    FixedDigits(out+n,minutes,4);
    out[n+4]=out[n+3];out[n+3]=out[n+2];out[n+2]='.';
    n+=5;
    out[n++]=longitude ? (value<0 ? 'W' : 'E') : (value<0 ? 'S' : 'N');
    out[n]=0;return n;
}
