/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_format.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned cases;
static void Compare(int32_t value,bool longitude,bool spaced)
{
    char original[24],actual[24];
    uint32_t absolute=value<0 ? (uint32_t)-value : (uint32_t)value;
    uint32_t hundredths=(absolute*3u+250u)/500u;
    unsigned degree=hundredths/6000u,minute=hundredths%6000u;
    char hemisphere=longitude ? (value<0 ? 'W':'E') : (value<0 ? 'S':'N');
    snprintf(original,sizeof original,longitude ? "%03u%s%02u.%02u%c" : "%02u%s%02u.%02u%c",
             degree,spaced ? " ":"",minute/100u,minute%100u,hemisphere);
    memset(actual,0x55,sizeof actual);
    unsigned n=APRS_FormatDM(actual,value,longitude,spaced);
    assert(n==strlen(original) && !strcmp(actual,original));
    assert(actual[n+1]==0x55); ++cases;
}
int main(void)
{
    /* Sweep both complete geographic ranges at sub-hundredth-minute spacing.
     * Add rounding boundaries and limits explicitly. The oracle uses the prior
     * independent printf formatting path for both UI and on-air forms. */
    for(unsigned longitude=0;longitude<2;longitude++) {
        const int32_t limit=longitude ? 180000000:90000000;
        for(int32_t value=-limit;value<=limit;value+=499)
            for(unsigned spaced=0;spaced<2;spaced++) Compare(value,longitude,spaced);
        const int32_t edges[]={-limit,-limit+1,-167,-166,-84,-83,-1,0,1,83,84,166,167,limit-1,limit};
        for(unsigned i=0;i<sizeof edges/sizeof *edges;i++)
            for(unsigned spaced=0;spaced<2;spaced++) Compare(edges[i],longitude,spaced);
    }
    printf("PASS %u shared position formatter comparisons, lengths and output bounds\n",cases);
}
