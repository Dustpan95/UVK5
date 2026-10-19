/* SPDX-License-Identifier: GPL-3.0-only */
#include "packed.h"
void PACK_Expand(void *output, unsigned size, const uint8_t *in)
{
    uint8_t *p=output,*const end=p+size;
    while(p<end) {
        unsigned flags=*in++;
        for(unsigned bit=1;bit<256 && p<end;bit<<=1) {
            if(flags&bit) *p++=*in++;
            else {
                unsigned low=*in++, high=*in++;
                const unsigned distance=(low|((high&0xf0)<<4))+1;
                unsigned count=(high&15)+3;
                while(count--) {*p=*(p-distance);++p;}
            }
        }
    }
}
