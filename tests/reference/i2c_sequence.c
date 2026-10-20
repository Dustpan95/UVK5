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

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/systick.h"
#include "bsp/dp32g030/gpio.h"
void REFERENCE_I2C_Start(void)
{
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
	SYSTICK_DelayUs(1);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL);
	SYSTICK_DelayUs(1);
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
	SYSTICK_DelayUs(1);
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL);
	SYSTICK_DelayUs(1);
}

void REFERENCE_I2C_Stop(void)
{
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
	SYSTICK_DelayUs(1);
	GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL);
	SYSTICK_DelayUs(1);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SCL);
	SYSTICK_DelayUs(1);
	GPIO_SetBit(&GPIOA->DATA, GPIOA_PIN_I2C_SDA);
	SYSTICK_DelayUs(1);
}
