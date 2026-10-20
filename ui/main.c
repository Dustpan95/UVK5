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

#include "ui/display_text.h"
#include "app/aprs_rx.h"
#include "ui/home.h"
#include <string.h>
#include <stdlib.h>  // abs()

#include "app/chFrScanner.h"
#include "dtmf.h"
#ifdef ENABLE_AM_FIX
	#include "am_fix.h"
#endif
#include "bitmaps.h"
#include "board.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/ui.h"
#include "toast.h"
#include "signal_quality.h"
#include "signal_classifier.h"
#include "bandscope.h"
#include "gain_staging.h"
#include "app/scanner.h"
#include "frequencies.h"
#include "driver/uart.h"
#include "scan_rate.h"

center_line_t center_line = CENTER_LINE_NONE;

uint16_t gScreenChannelGuardTrips = 0;

const int8_t dBmCorrTable[7] = {
			-15, // band 1
			-25, // band 2
			-20, // band 3
			-4, // band 4
			-7, // band 5
			-6, // band 6
			 -1  // band 7
		};

const char *VfoStateStr[] = {
       [VFO_STATE_NORMAL]="",
       [VFO_STATE_BUSY]="BUSY",
       [VFO_STATE_BAT_LOW]="BAT LOW",
       [VFO_STATE_TX_DISABLE]="TX DISABLE",
       [VFO_STATE_TIMEOUT]="TIMEOUT",
       [VFO_STATE_ALARM]="ALARM",
       [VFO_STATE_VOLTAGE_HIGH]="VOLT HIGH"
};

// ***************************************************************************

#ifdef ENABLE_AUDIO_BAR

static void DrawLevelBar(uint8_t xpos, uint8_t line, uint8_t level)
{
	const char hollowBar[] = {
		0b01111111,
		0b01000001,
		0b01000001,
		0b01111111
	};

	uint8_t *p_line = gFrameBuffer[line];
	level = MIN(level, 13);

	for(uint8_t i = 0; i < level; i++) {
		if(i < 9) {
			for(uint8_t j = 0; j < 4; j++)
				p_line[xpos + i * 5 + j] = (~(0x7F >> (i+1))) & 0x7F;
		}
		else {
			memcpy(p_line + (xpos + i * 5), &hollowBar, ARRAY_SIZE(hollowBar));
		}
	}
}
#endif

#ifdef ENABLE_AUDIO_BAR

unsigned int sqrt16(unsigned int value)
{	// return square root of 'value'
	unsigned int shift = 16;         // number of bits supplied in 'value' .. 2 ~ 32
	unsigned int bit   = 1u << --shift;
	unsigned int sqrti = 0;
	while (bit)
	{
		const unsigned int temp = ((sqrti << 1) | bit) << shift--;
		if (value >= temp) {
			value -= temp;
			sqrti |= bit;
		}
		bit >>= 1;
	}
	return sqrti;
}

void UI_DisplayAudioBar(void)
{
	if (gSetting_mic_bar)
	{
		if(gLowBattery && !gLowBatteryConfirmed)
			return;

		const unsigned int line      = 6;

		if (gCurrentFunction != FUNCTION_TRANSMIT ||
			gScreenToDisplay != DISPLAY_MAIN
#ifdef ENABLE_DTMF_CALLING
			|| gDTMF_CallState != DTMF_CALL_STATE_NONE
#endif
			)
		{
			return;  // screen is in use
		}

#if defined(ENABLE_ALARM) || defined(ENABLE_TX1750)
		if (gAlarmState != ALARM_STATE_OFF)
			return;
#endif
		const unsigned int voice_amp  = BK4819_GetVoiceAmplitudeOut();  // 15:0

		// make non-linear to make more sensitive at low values
		const unsigned int level      = MIN(voice_amp * 8, 65535u);
		const unsigned int sqrt_level = MIN(sqrt16(level), 124u);
		uint8_t bars = 13 * sqrt_level / 124;

		uint8_t *p_line = gFrameBuffer[line];
		memset(p_line, 0, LCD_WIDTH);

		DrawLevelBar(62, line, bars);

		if (gCurrentFunction == FUNCTION_TRANSMIT)
			ST7565_BlitFullScreen();
	}
}
#endif


// v1.3.3: shared RX signal readout so the centre RSSI bar and the bottom RX
// status line report the SAME calibrated figure (they used to differ -- the
// status line omitted the per-band dBm correction). Returns dBm; fills
// *s_level with the calibrated S-reading (0-9 across the S0..S9 EEPROM points).
int16_t UI_RX_Signal_dBm(uint8_t *s_level)
{
	const int16_t rssi_dBm =
		BK4819_GetRSSI_dBm()
#ifdef ENABLE_AM_FIX
		+ ((gSetting_AM_fix && gRxVfo->Modulation == MODULATION_AM) ? AM_fix_get_gain_diff() : 0)
#endif
		+ ((gRxVfo->Modulation == MODULATION_FM) ? GAIN_STAGING_GetGainDiff(gEeprom.RX_VFO) : 0)
		+ dBmCorrTable[gRxVfo->Band];
	const int16_t s0_dBm = -gEeprom.S0_LEVEL;
	const int     s0_9   = gEeprom.S0_LEVEL - gEeprom.S9_LEVEL;
	const int denominator=s0_9*100/9;
	*s_level = denominator>0 ? MIN(MAX((int32_t)(rssi_dBm-s0_dBm)*100/denominator,0),9) : 0;
	return rssi_dBm;
}

#ifdef ENABLE_AGC_SHOW_DATA
void UI_MAIN_PrintAGC(bool now)
{
	char buf[20];
	memset(gFrameBuffer[3], 0, 128);
	union {
		struct {
			uint16_t _ : 5;
			uint16_t agcSigStrength : 7;
			int16_t gainIdx : 3;
			uint16_t agcEnab : 1;
		};
    	uint16_t __raw;
	} reg7e;
	reg7e.__raw = BK4819_ReadRegister(0x7E);
	uint8_t gainAddr = reg7e.gainIdx < 0 ? 0x14 : 0x10 + reg7e.gainIdx;
	union {
		struct {
			uint16_t pga:3;
			uint16_t mixer:2;
			uint16_t lna:3;
			uint16_t lnaS:2;
		};
		uint16_t __raw;
	} agcGainReg;
	agcGainReg.__raw = BK4819_ReadRegister(gainAddr);
	int8_t lnaShortTab[] = {-28, -24, -19, 0};
	int8_t lnaTab[] = {-24, -19, -14, -9, -6, -4, -2, 0};
	int8_t mixerTab[] = {-8, -6, -3, 0};
	int8_t pgaTab[] = {-33, -27, -21, -15, -9, -6, -3, 0};
	int16_t agcGain = lnaShortTab[agcGainReg.lnaS] + lnaTab[agcGainReg.lna] + mixerTab[agcGainReg.mixer] + pgaTab[agcGainReg.pga];

	sprintf(buf, DISPLAY_TEXT("%d%2d %2d %2d %3d"), reg7e.agcEnab, reg7e.gainIdx, -agcGain, reg7e.agcSigStrength, BK4819_GetRSSI());
	UI_PrintStringSmallNormal(buf, 2, 0, 3);
	if(now)
		ST7565_BlitLine(3);
}
#endif

void UI_MAIN_TimeSlice500ms(void)
{
    if(gScreenToDisplay==DISPLAY_MAIN) gUpdateDisplay=true;
}

// ***************************************************************************

void UI_DisplayMain(void)
{
	center_line = CENTER_LINE_NONE;

	// clear the screen
	UI_DisplayClear();

	if(gLowBattery && !gLowBatteryConfirmed) {
		UI_DisplayPopup(DISPLAY_TEXT("LOW BATTERY"));
		ST7565_BlitFullScreen();
		return;
	}

	if (gEeprom.KEY_LOCK && gKeypadLocked > 0)
	{	// tell user how to unlock the keyboard
		UI_PrintString(DISPLAY_TEXT("Long press #"), 0, LCD_WIDTH, 1, 8);
		UI_PrintString(DISPLAY_TEXT("to unlock"),    0, LCD_WIDTH, 3, 8);
		ST7565_BlitFullScreen();
		return;
	}

	if (gAPRS_Browser) { APRS_Draw(); return; }
    if (UI_HomeDraw()) { ST7565_BlitFullScreen(); return; }

    /* Invalid indices must not index VFO arrays. Normal modes, scan, input,
     * split activity and temporary messages share ui/home.c rendering. */
    UI_DisplayPopup(DISPLAY_TEXT("VFO ERROR"));
    ST7565_BlitFullScreen();
}
