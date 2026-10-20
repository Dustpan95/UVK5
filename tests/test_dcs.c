/* SPDX-License-Identifier: GPL-3.0-only */
#include "dcs.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern const uint16_t REFERENCE_CTCSS_Options[50],REFERENCE_DCS_Options[104];
uint32_t REFERENCE_DCS_GetGolayCodeWord(DCS_CodeType_t type,uint8_t option);
uint8_t REFERENCE_DCS_GetCdcssCode(uint32_t code),REFERENCE_DCS_GetCtcssCode(int code);
int main(void)
{
    DCS_Init();
    assert(!memcmp(CTCSS_Options,REFERENCE_CTCSS_Options,sizeof CTCSS_Options));
    assert(!memcmp(DCS_Options,REFERENCE_DCS_Options,sizeof DCS_Options));
    unsigned cases=0;
    for(unsigned i=0;i<104;i++) for(unsigned reverse=0;reverse<2;reverse++) {
        DCS_CodeType_t type=reverse ? CODE_TYPE_REVERSE_DIGITAL:CODE_TYPE_DIGITAL;
        uint32_t word=DCS_GetGolayCodeWord(type,i);
        assert(word==REFERENCE_DCS_GetGolayCodeWord(type,i));
        for(unsigned rotation=0;rotation<23;rotation++) {
            assert(DCS_GetCdcssCode(word)==REFERENCE_DCS_GetCdcssCode(word));
            word=(word>>1)|((word&1)<<22);++cases;
        }
    }
    for(int code=-100;code<=5000;code++) {assert(DCS_GetCtcssCode(code)==REFERENCE_DCS_GetCtcssCode(code));++cases;}
    DCS_Init();assert(!memcmp(DCS_Options,REFERENCE_DCS_Options,sizeof DCS_Options));
    printf("PASS exact tone tables and %u original Golay/rotation/nearest-tone comparisons\n",cases);
}
