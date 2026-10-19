/* SPDX-License-Identifier: GPL-3.0-only
 * Minimal register/interrupt model for the actual SysTick backend host test.
 */
#ifndef TEST_CLOCK_ARMCM0_H
#define TEST_CLOCK_ARMCM0_H
#include <stdint.h>
typedef struct {volatile uint32_t LOAD,VAL,CTRL;} TestSysTick;
typedef struct {volatile uint32_t ICSR;} TestSCB;
extern TestSysTick testSysTick;
extern TestSCB testSCB;
extern uint32_t testPrimask;
#define SysTick (&testSysTick)
#define SCB (&testSCB)
#define SCB_ICSR_PENDSTSET_Msk (1u<<26)
#define SysTick_CTRL_ENABLE_Msk 1u
#define SysTick_CTRL_TICKINT_Msk 2u
static inline uint32_t __get_PRIMASK(void) {return testPrimask;}
static inline void __disable_irq(void) {testPrimask=1;}
static inline void __set_PRIMASK(uint32_t value) {testPrimask=value;}
static inline int SysTick_Config(uint32_t value) {SysTick->LOAD=value-1;SysTick->CTRL=3;return 0;}
#endif
