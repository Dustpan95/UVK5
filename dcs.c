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

#include "dcs.h"

#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

/* Exact original lookup values are reconstructed before radio setup. */
#include "dcs_packed.inc"
uint16_t gToneOptions[154];
void DCS_Init(void)
{
    unsigned value=0;
    for(unsigned i=0;i<154;i++) {value+=gDcsDelta[i];gToneOptions[i]=value;}
}

static uint32_t DCS_CalculateGolay(uint32_t CodeWord)
{
	unsigned int i;
	uint32_t Word = CodeWord;
	for (i = 0; i < 12; i++)
	{
		Word <<= 1;
		if (Word & 0x1000)
			Word ^= 0x08EA;
	}
	return CodeWord | ((Word & 0x0FFE) << 11);
}

uint32_t DCS_GetGolayCodeWord(DCS_CodeType_t CodeType, uint8_t Option)
{
	uint32_t Code = DCS_CalculateGolay(DCS_Options[Option] + 0x800U);
	if (CodeType == CODE_TYPE_REVERSE_DIGITAL)
		Code ^= 0x7FFFFF;
	return Code;
}

uint8_t DCS_GetCdcssCode(uint32_t Code)
{
	unsigned int i;
	for (i = 0; i < 23; i++)
	{
		uint32_t Shift;

		if (((Code >> 9) & 0x7U) == 4)
		{
			unsigned int j;
			for (j = 0; j < ARRAY_SIZE(DCS_Options); j++)
				if (DCS_Options[j] == (Code & 0x1FF))
					if (DCS_GetGolayCodeWord(2, j) == Code)
						return j;
		}

		Shift = Code >> 1;
		if (Code & 1U)
			Shift |= 0x400000U;
		Code = Shift;
	}

	return 0xFF;
}

uint8_t DCS_GetCtcssCode(int Code)
{
	unsigned int i;
	uint8_t      Result = 0xFF;
	int          Smallest = ARRAY_SIZE(CTCSS_Options);

	for (i = 0; i < ARRAY_SIZE(CTCSS_Options); i++)
	{
		int Delta = Code - CTCSS_Options[i];
		if (Delta < 0)
			Delta = -(Code - CTCSS_Options[i]);
		if (Smallest > Delta)
		{
			Smallest = Delta;
			Result   = i;
		}
	}

	return Result;
}
