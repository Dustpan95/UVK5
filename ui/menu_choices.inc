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

/* Original fixed-width choice tables: lossless resource-packer input. */

const char gSubMenu_TXP[][5] =
{
	"LOW",
	"MID",
	"HIGH"
};

const char gSubMenu_SFT_D[][4] =
{
	"OFF",
	"+",
	"-"
};

const char gSubMenu_W_N[][7] =
{
	"WIDE",
	"NARROW"
};

const char gSubMenu_OFF_ON[][4] =
{
	"OFF",
	"ON"
};

const char gSubMenu_SAVE[][4] =
{
	"OFF",
	"1:1",
	"1:2",
	"1:3",
	"1:4"
};

const char gSubMenu_TOT[][7] =
{
	"30 sec",
	"1 min",
	"2 min",
	"3 min",
	"4 min",
	"5 min",
	"6 min",
	"7 min",
	"8 min",
	"9 min",
	"15 min"
};

const char gSubMenu_SC_REV[][8] =
{
	"TIMEOUT",
	"CARRIER",
	"STOP"
};

const char gSubMenu_PONMSG[][8] =
{
	"FULL",
	"MESSAGE",
	"VOLTAGE",
	"NONE"
};

const char gSubMenu_ROGER[][6] =
{
	"OFF",
	"ROGER",
	"MDC"
};

const char gSubMenu_RESET[][4] =
{
	"VFO",
	"ALL"
};

const char gSubMenu_BACKLIGHT[][7] =
{
	"OFF",
	"5 sec",
	"10 sec",
	"20 sec",
	"1 min",
	"2 min",
	"4 min",
	"ON"
};

const char gSubMenu_RX_TX[][6] =
{
	"OFF",
	"TX",
	"RX",
	"TX/RX"
};

const char gSubMenu_BAT_TXT[][8] =
{
	"NONE",
	"VOLTAGE",
	"PERCENT"
};

const char gSubMenu_BATTYP[][9] =
{
	"1600mAh",
	"2200mAh"
};

const char gSubMenu_SCRAMBLER[][7] =
{
	"OFF",
	"2600Hz",
	"2700Hz",
	"2800Hz",
	"2900Hz",
	"3000Hz",
	"3100Hz",
	"3200Hz",
	"3300Hz",
	"3400Hz",
	"3500Hz"
};
