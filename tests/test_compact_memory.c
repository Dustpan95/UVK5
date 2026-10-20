/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
void *FIRMWARE_memcpy(void *,const void *,size_t);
void *FIRMWARE_memmove(void *,const void *,size_t);
void *FIRMWARE_memset(void *,int,size_t);
int FIRMWARE_memcmp(const void *,const void *,size_t);
size_t FIRMWARE_strlen(const char *);
int FIRMWARE_strcmp(const char *,const char *);
char *FIRMWARE_strcpy(char *,const char *);
char *FIRMWARE_strncpy(char *,const char *,size_t);
static unsigned checks;
static int Sign(int n) {return (n>0)-(n<0);}
int main(void)
{
    unsigned char source[256],actual[256],expected[256];
    for(unsigned i=0;i<sizeof(source);i++) source[i]=(i*73u+19u)&255u;
    for(unsigned from=0;from<32;from++) for(unsigned to=0;to<32;to++)
        for(unsigned n=0;n<=128;n++) {
            memset(actual,0xa5,sizeof(actual));memset(expected,0xa5,sizeof(expected));
            assert(FIRMWARE_memcpy(actual+to,source+from,n)==actual+to);
            memcpy(expected+to,source+from,n);assert(!memcmp(actual,expected,sizeof(actual)));
            ++checks;
        }
    for(unsigned from=0;from<=64;from++) for(unsigned to=0;to<=64;to++)
        for(unsigned n=0;n<=64;n++) {
            memcpy(actual,source,sizeof(actual));memcpy(expected,source,sizeof(expected));
            assert(FIRMWARE_memmove(actual+to,actual+from,n)==actual+to);
            memmove(expected+to,expected+from,n);assert(!memcmp(actual,expected,sizeof(actual)));
            ++checks;
        }
    for(int value=-128;value<=255;value++) for(unsigned to=0;to<16;to++)
        for(unsigned n=0;n<=128;n++) {
            memcpy(actual,source,sizeof(actual));memcpy(expected,source,sizeof(expected));
            assert(FIRMWARE_memset(actual+to,value,n)==actual+to);
            memset(expected+to,value,n);assert(!memcmp(actual,expected,sizeof(actual)));
            ++checks;
        }
    for(unsigned n=0;n<=128;n++) for(unsigned mismatch=0;mismatch<=128;mismatch++) {
        memcpy(actual,source,sizeof(actual));actual[mismatch]^=0x80;
        assert(Sign(FIRMWARE_memcmp(actual,source,n))==Sign(memcmp(actual,source,n)));
        ++checks;
    }
    char text[130],other[130];
    for(unsigned length=0;length<=128;length++) {
        for(unsigned i=0;i<length;i++) text[i]=(char)(1+(i*79u)%255u);
        text[length]=0;assert(FIRMWARE_strlen(text)==strlen(text));
        for(unsigned to=0;to<8;to++) {
            memset(actual,0xa5,sizeof(actual));memset(expected,0xa5,sizeof(expected));
            assert(FIRMWARE_strcpy((char *)actual+to,text)==(char *)actual+to);
            strcpy((char *)expected+to,text);assert(!memcmp(actual,expected,sizeof(actual)));
            for(unsigned n=0;n<=144;n++) {
                memset(actual,0xa5,sizeof(actual));memset(expected,0xa5,sizeof(expected));
                assert(FIRMWARE_strncpy((char *)actual+to,text,n)==(char *)actual+to);
                strncpy((char *)expected+to,text,n);assert(!memcmp(actual,expected,sizeof(actual)));
                ++checks;
            }
        }
        for(unsigned i=0;i<=length;i++) {
            memcpy(other,text,length+1);
            other[i]=(char)((unsigned char)other[i]^0x80);other[length+1]=0;
            assert(Sign(FIRMWARE_strcmp(text,other))==Sign(strcmp(text,other)));
            ++checks;
        }
    }
    static unsigned char large_source[8224],large_actual[8224],large_expected[8224];
    const unsigned lengths[]={255,256,1023,1024,2047,2048,3883,4095,4096,8192};
    for(unsigned i=0;i<sizeof(large_source);i++) large_source[i]=(i*131u+37u)&255u;
    for(unsigned i=0;i<sizeof(lengths)/sizeof(*lengths);i++) for(unsigned offset=0;offset<32;offset++) {
        unsigned n=lengths[i];memset(large_actual,0,sizeof(large_actual));memset(large_expected,0,sizeof(large_expected));
        FIRMWARE_memcpy(large_actual+offset,large_source+offset,n);memcpy(large_expected+offset,large_source+offset,n);
        assert(!memcmp(large_actual,large_expected,sizeof(large_actual)));
        FIRMWARE_memmove(large_actual+offset+1,large_actual+offset,n);memmove(large_expected+offset+1,large_expected+offset,n);
        assert(!memcmp(large_actual,large_expected,sizeof(large_actual)));++checks;
    }
    printf("PASS compact memory/string routines: %u libc comparisons, overlaps, alignments, padding and large transfers\n",checks);
}
