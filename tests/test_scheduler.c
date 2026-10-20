/* SPDX-License-Identifier: GPL-3.0-only
 * Include the real ISR to seed private clock state without firmware test hooks.
 */
#include "scheduler.c"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
EEPROM_Config_t gEeprom;
FUNCTION_Type_t gCurrentFunction;
int8_t gScanStateDir;
volatile uint16_t gPowerSave_10ms;
static unsigned checks;
static void Run(uint32_t seed,unsigned count)
{
    gGlobalSysTickCounter=seed;gTickModulo50=seed%50;
    gCurrentFunction=FUNCTION_FOREGROUND;gScanStateDir=SCAN_OFF;
    gTxTimerCountdown_500ms=3;gSerialConfigCountDown_500ms=5;
    for(unsigned i=0;i<count;i++) {
        uint32_t next=gGlobalSysTickCounter+1;
        bool half=next%50==0;
        uint16_t tx=gTxTimerCountdown_500ms;
        uint8_t serial=gSerialConfigCountDown_500ms;
        gNextTimeslice=gNextTimeslice_500ms=gNextTimeslice40ms=false;
        gTxTimeoutReached=false;
        SystickHandler();
        assert(SCHEDULER_GetTicks10ms()==next);
        assert(gNextTimeslice && gNextTimeslice_500ms==half);
        assert(gNextTimeslice40ms==((next&3)==0));
        assert(gTxTimerCountdown_500ms==(tx-(half && tx>0)));
        assert(gSerialConfigCountDown_500ms==(serial-(half && serial>0)));
        assert(gTxTimeoutReached==(half && tx==1));
        assert(gTickModulo50==next%50);
        ++checks;
    }
}
int main(void)
{
    Run(0,1000000);
    for(unsigned before=0;before<=100;before++) Run(UINT32_MAX-before,250);
    const uint32_t seed[]={49,50,99,100,65535,65536,0x7fffffff,0x80000000};
    for(unsigned i=0;i<sizeof(seed)/sizeof(*seed);i++) Run(seed[i],1000);
    printf("PASS actual SysTick ISR: %u modulo-equivalent ticks, epoch wrap and timeout cadence\n",checks);
}
