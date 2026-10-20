/* SPDX-License-Identifier: GPL-3.0-only
 * Virtual RAM replaces the GPIOA page; no hardware or EEPROM is accessed. */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "driver/i2c.h"
#include "driver/systick.h"
#include "bsp/dp32g030/gpio.h"
void REFERENCE_I2C_Start(void),REFERENCE_I2C_Stop(void);
static uint32_t trace[4];static unsigned count;
void SYSTICK_DelayUs(uint32_t delay) {assert(delay==1 && count<4);trace[count++]=GPIOA->DATA;}
int main(void)
{
    void *page=mmap((void *)GPIOA_BASE_ADDR,4096,PROT_READ|PROT_WRITE,
                    MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    assert(page==(void *)GPIOA_BASE_ADDR);
    unsigned cases=0;
    for(unsigned stop=0;stop<2;stop++) for(unsigned low=0;low<65536;low++) {
        uint32_t initial=0xa5a50000u|low,expected[4];
        GPIOA->DATA=initial;GPIOA->DIR=0x12345678;count=0;
        if(stop) REFERENCE_I2C_Stop();else REFERENCE_I2C_Start();
        assert(count==4);memcpy(expected,trace,sizeof trace);uint32_t final=GPIOA->DATA;
        GPIOA->DATA=initial;count=0;
        if(stop) I2C_Stop();else I2C_Start();
        assert(count==4 && !memcmp(expected,trace,sizeof trace));
        assert(GPIOA->DATA==final && GPIOA->DIR==0x12345678);++cases;
    }
    assert(!munmap(page,4096));
    printf("PASS %u real I2C START/STOP pin-state and programmed-delay traces match original driver\n",cases);
}
