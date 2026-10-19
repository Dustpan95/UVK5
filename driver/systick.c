/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "ARMCM0.h"
#include "systick.h"
#include "../misc.h"

// 0x20000324
static uint32_t gTickMultiplier;

void SYSTICK_Init(void)
{
	SysTick_Config(480000);
	gTickMultiplier = 48;
}

uint32_t SYSTICK_Cycles(void)
{
	extern uint32_t SCHEDULER_GetTicks10ms(void);
	const uint32_t mask=__get_PRIMASK();
	__disable_irq();
	uint32_t epoch=SCHEDULER_GetTicks10ms();
	const uint32_t before=SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
	uint32_t value=SysTick->VAL;
	const uint32_t after=SCB->ICSR & SCB_ICSR_PENDSTSET_Msk;
	/* A reload between the first pending read and VAL requires a fresh VAL.
	 * A pending interrupt has not incremented the software epoch yet. This
	 * critical section is only a handful of register reads, far below 10 ms.
	 */
	if(after) {
		++epoch;
		if(!before) value=SysTick->VAL;
	}
	__set_PRIMASK(mask);
	return epoch*480000u + (479999u-value);
}

bool SYSTICK_CanCountCycles(void)
{
	const uint32_t required=SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
	return !__get_PRIMASK() && SysTick->LOAD==479999u &&
		(SysTick->CTRL & required)==required;
}

void SYSTICK_DelayUs(uint32_t Delay)
{
	const uint32_t ticks = Delay * gTickMultiplier;
	uint32_t elapsed_ticks = 0;
	uint32_t Start = SysTick->LOAD;
	uint32_t Previous = SysTick->VAL;
	do {
		uint32_t Current;

		do {
			Current = SysTick->VAL;
		} while (Current == Previous);

		uint32_t Delta = ((Current < Previous) ? - Current : Start - Current);

		elapsed_ticks += Delta + Previous;

		Previous = Current;
	} while (elapsed_ticks < ticks);
}
