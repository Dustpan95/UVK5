/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef TEST_RADIO_GPIO_H
#define TEST_RADIO_GPIO_H
#include <stdint.h>
typedef struct {uint32_t DATA;} TestRadioGPIO;
extern TestRadioGPIO testRadioGPIO;
#define GPIOC (&testRadioGPIO)
#endif
