/* SPDX-License-Identifier: GPL-3.0-only */
#include "ARMCM0.h"
#include "driver/systick.h"
#include <assert.h>
#include <stdio.h>
TestSysTick testSysTick;
TestSCB testSCB;
uint32_t testPrimask;
static uint32_t epoch;
uint32_t SCHEDULER_GetTicks10ms(void) {assert(testPrimask==1);return epoch;}
int main(void)
{
    SYSTICK_Init();assert(SysTick->LOAD==479999 && SYSTICK_CanCountCycles());
    testPrimask=1;assert(!SYSTICK_CanCountCycles());testPrimask=0;
    SysTick->CTRL=1;assert(!SYSTICK_CanCountCycles());SysTick->CTRL=3;
    SysTick->LOAD=479998;assert(!SYSTICK_CanCountCycles());SysTick->LOAD=479999;
    epoch=7;SysTick->VAL=479999;
    assert(SYSTICK_Cycles()==7u*480000u && !testPrimask);
    SysTick->VAL=0;
    assert(SYSTICK_Cycles()==8u*480000u-1u && !testPrimask);
    /* Reloaded hardware whose interrupt has not updated the software epoch. */
    SCB->ICSR=SCB_ICSR_PENDSTSET_Msk;SysTick->VAL=479900;
    assert(SYSTICK_Cycles()==8u*480000u+99u && !testPrimask);
    SCB->ICSR=0;++epoch;
    assert(SYSTICK_Cycles()==8u*480000u+99u);
    testPrimask=1;epoch=UINT32_MAX;SysTick->VAL=0;
    uint32_t expected=epoch*480000u+479999u;
    assert(SYSTICK_Cycles()==expected && testPrimask==1);
    SCB->ICSR=SCB_ICSR_PENDSTSET_Msk;SysTick->VAL=479999;
    assert(SYSTICK_Cycles()==0 && testPrimask==1);
    puts("PASS actual SysTick backend epoch/pending reload, cycle/epoch wrap and interrupt-mask restoration");
}
