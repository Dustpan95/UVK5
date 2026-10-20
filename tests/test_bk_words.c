/* SPDX-License-Identifier: GPL-3.0-only
 * Linux virtual RAM stands in for the GPIOC register page. No real hardware.
 * Compares pin state at every programmed delay, not real CPU/RF timing.
 */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "driver/bk4819.h"
#include "driver/systick.h"
#include "bsp/dp32g030/gpio.h"
void REFERENCE_BK4819_WriteU8(uint8_t value);
void REFERENCE_BK4819_WriteU16(uint16_t value);
static uint32_t trace[64];static unsigned count;
void SYSTICK_DelayUs(uint32_t delay)
{
    assert(delay==1 && count<64);trace[count++]=GPIOC->DATA;
}
int main(void)
{
    void *page=mmap((void *)GPIOC_BASE_ADDR,4096,PROT_READ|PROT_WRITE,
                    MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    assert(page==(void *)GPIOC_BASE_ADDR);
    unsigned cases=0;
    for(unsigned width=8;width<=16;width+=8) for(unsigned seed=0;seed<2;seed++) {
        const uint32_t initial=seed ? 0x55555555u:0xaaaaaaaau;
        for(unsigned value=0;value<(1u<<width);value++) {
            uint32_t expected[64];GPIOC->DATA=initial;GPIOC->DIR=0x12345678;count=0;
            if(width==8) REFERENCE_BK4819_WriteU8(value);else REFERENCE_BK4819_WriteU16(value);
            assert(count==width*3);memcpy(expected,trace,sizeof expected);uint32_t final=GPIOC->DATA;
            GPIOC->DATA=initial;count=0;
            if(width==8) BK4819_WriteU8(value);else BK4819_WriteU16(value);
            assert(count==width*3 && !memcmp(expected,trace,count*sizeof *trace));
            assert(GPIOC->DATA==final && GPIOC->DIR==0x12345678);++cases;
        }
    }
    assert(!munmap(page,4096));
    printf("PASS %u real BK4819 byte/word pin-state and programmed-delay traces match original driver\n",cases);
}
