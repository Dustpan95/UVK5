/* SPDX-License-Identifier: GPL-3.0-only
 * Small, alignment-independent memory/string routines for the flash-limited
 * Cortex-M0 image. Compile without LTO to keep one shared implementation.
 */
#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__) && !defined(__clang__)
/* Do not turn a byte loop back into a call to the function being defined. */
#pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

void *memcpy(void *target,const void *source,size_t size)
{
    unsigned char *out=target;
    const unsigned char *in=source;
    for(size_t i=0;i<size;i++) out[i]=in[i];
    return target;
}

void *memmove(void *target,const void *source,size_t size)
{
    unsigned char *out=target;
    const unsigned char *in=source;
    if((uintptr_t)out>(uintptr_t)in) {
        while(size) {--size;out[size]=in[size];}
    } else {
        for(size_t i=0;i<size;i++) out[i]=in[i];
    }
    return target;
}

void *memset(void *target,int value,size_t size)
{
    unsigned char *out=target;
    for(size_t i=0;i<size;i++) out[i]=(unsigned char)value;
    return target;
}

int memcmp(const void *left,const void *right,size_t size)
{
    const unsigned char *a=left,*b=right;
    for(size_t i=0;i<size;i++) if(a[i]!=b[i]) return (int)a[i]-(int)b[i];
    return 0;
}

size_t strlen(const char *text)
{
    const char *end=text;
    while(*end) ++end;
    return (size_t)(end-text);
}

int strcmp(const char *left,const char *right)
{
    const unsigned char *a=(const unsigned char *)left,*b=(const unsigned char *)right;
    while(*a && *a==*b) {++a;++b;}
    return (int)*a-(int)*b;
}

char *strcpy(char *target,const char *source)
{
    char *out=target;
    do {*out=*source++;} while(*out++);
    return target;
}

char *strncpy(char *target,const char *source,size_t size)
{
    for(size_t i=0;i<size;i++) {
        target[i]=*source;
        if(*source) ++source;
    }
    return target;
}
