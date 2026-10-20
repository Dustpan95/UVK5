/* SPDX-License-Identifier: GPL-3.0-only
 * Shared fixed-position text for the setup review and AX.25 position body.
 */
#include "app/aprs_format.h"
unsigned __attribute__((noinline)) APRS_FormatDM(char *out, int32_t value, bool longitude, bool spaced)
{
    const uint32_t absolute=value<0 ? 0u-(uint32_t)value : (uint32_t)value;
    /* Round to the nearest hundredth minute, including degree carry. Callers
     * enforce the documented latitude/longitude bounds; arithmetic fits in 32 bits. */
    const uint32_t hundredths=(absolute*3u+250u)/500u;
    const unsigned degrees=hundredths/6000u, minutes=hundredths%6000u;
    unsigned n=0;
    if(longitude) out[n++]='0'+degrees/100u;
    out[n++]='0'+degrees/10u%10u;out[n++]='0'+degrees%10u;
    if(spaced) out[n++]=' ';
    out[n++]='0'+minutes/1000u;out[n++]='0'+minutes/100u%10u;
    out[n++]='.';out[n++]='0'+minutes/10u%10u;out[n++]='0'+minutes%10u;
    out[n++]=longitude ? (value<0 ? 'W' : 'E') : (value<0 ? 'S' : 'N');
    out[n]=0;return n;
}
