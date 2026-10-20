/* Modified for UVK5: APRS receive / display integration; see docs/INTEGRATION.md. */
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

/* UVK5 integration: passive APRS hooks. */
// ---- Calling frequency lookup table ----
typedef struct {
	uint32_t band_start;    // band lower edge in Hz/10
	uint32_t band_end;      // band upper edge in Hz/10
	uint32_t call_freq;     // calling frequency in Hz/10
	uint16_t step_freq;     // step size in Hz/10
	uint8_t  modulation;    // 0=FM, 1=AM, 2=USB
} BandCallFreq_t;

static const BandCallFreq_t band_call_table[] = {
	{ 14400000, 14430000, 14420000, 100,   MODULATION_USB },  // 2m SSB (narrow, checked first)
	{ 14400000, 14800000, 14652000, 500,   MODULATION_FM  },  // 2m FM
	{ 43200000, 43300000, 43210000, 500,   MODULATION_USB },  // 70cm SSB (narrow, first)
	{ 42000000, 45000000, 44600000, 2500,  MODULATION_FM  },  // 70cm FM
	{ 15100000, 15400000, 15194000, 1125,  MODULATION_FM  },  // MURS 151.940 MHz
	{ 46200000, 46700000, 46256250, 1250,  MODULATION_FM  },  // FRS/GMRS 462.5625 MHz
	{  2690000,  2740000,  2718500, 1000,  MODULATION_FM  },  // CB 27.185 MHz
	{ 15600000, 16200000, 15680000, 2500,  MODULATION_FM  },  // Marine 156.800 MHz
	{ 11800000, 13600000, 12150000, 2500,  MODULATION_AM  },  // Airband 121.500 MHz
};
#define BAND_CALL_COUNT (sizeof(band_call_table) / sizeof(band_call_table[0]))

