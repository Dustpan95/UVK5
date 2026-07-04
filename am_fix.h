
/* Copyright 2023 OneOfEleven
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

#ifndef AM_FIXH
#define AM_FIXH

#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_AM_FIX

	typedef struct
	{
		uint16_t reg_val;
		int8_t   gain_dB;
	} __attribute__((packed)) t_gain_table;

	extern const t_gain_table gain_table[];
	extern const uint8_t gain_table_size;

	/* VUURWERK v1.3.0 (F1/S2): gain_table[0] is the stock -7dB reference,
	 * NOT the bottom of the gain ladder; rungs 1..42 ascend from -93dB to
	 * 0dB. Code stepping off index 0 must enter the ladder at the rung
	 * adjacent to -7dB, otherwise a single "step" lands on
	 * gain_table[1] = -93dB. Rung 39 is -6dB, the nearest rung above the
	 * stock reference. Guarded by a _Static_assert in am_fix.c. */
	#define GAIN_TABLE_RUNG_ABOVE_STOCK 39

	void AM_fix_init(void);
	void AM_fix_reset(const unsigned vfo);
	void AM_fix_10ms(const unsigned vfo);
	#ifdef ENABLE_AM_FIX_SHOW_DATA
		void AM_fix_print_data(const unsigned vfo, char *s);
	#endif
	int8_t AM_fix_get_gain_diff();
	void AM_fix_enable(bool on);

#endif

#endif
