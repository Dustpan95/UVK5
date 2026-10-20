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

/* Modified for UVK5: approved home status and percentage battery readout. */
#include "ui/display_text.h"
#include "ui/home.h"
#include <string.h>

#include "app/chFrScanner.h"
#ifdef ENABLE_FMRADIO
	#include "app/fm.h"
#endif
#include "app/scanner.h"
#include "bitmaps.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#include "ui/battery.h"
#include "ui/helper.h"
#include "ui/ui.h"
#include "ui/status.h"
#include "signal_quality.h"
#include "scanwatch.h"
#include "vfo_split.h"   // VUURWERK v1.3.1 (U1): split armed indicator

void UI_DisplayStatus()
{
	gUpdateStatus = false;
    if (gScreenToDisplay == DISPLAY_MAIN && UI_HomeEligible()) {
        UI_HomeStatus();
        ST7565_BlitStatusLine();
        return;
    }
	memset(gStatusLine, 0, sizeof(gStatusLine));

	uint8_t     *line = gStatusLine;
	unsigned int x    = 0;
	// **************

	// POWER-SAVE indicator
	if (gCurrentFunction == FUNCTION_TRANSMIT) {
		memcpy(line + x, BITMAP_TX, sizeof(BITMAP_TX));
	}
	else if (FUNCTION_IsRx()) {
		memcpy(line + x, BITMAP_RX, sizeof(BITMAP_RX));
	}
	else if (gCurrentFunction == FUNCTION_POWER_SAVE) {
		memcpy(line + x, BITMAP_POWERSAVE, sizeof(BITMAP_POWERSAVE));
	}
	x += 8;
	unsigned int x1 = x;

#ifdef ENABLE_NOAA
	if (gIsNoaaMode) { // NOASS SCAN indicator
		memcpy(line + x, BITMAP_NOAA, sizeof(BITMAP_NOAA));
		x1 = x + sizeof(BITMAP_NOAA);
	}
	x += sizeof(BITMAP_NOAA);
#endif

#ifdef ENABLE_DTMF_CALLING
	if (gSetting_KILLED) {
		memset(line + x, 0xFF, 10);
		x1 = x + 10;
	}
	else
#endif
#ifdef ENABLE_FMRADIO
	if (gFmRadioMode) { // FM indicator
		memcpy(line + x, BITMAP_FM, sizeof(BITMAP_FM));
		x1 = x + sizeof(BITMAP_FM);
	}
	else
#endif
	{ // SCAN indicator
		if (gScanStateDir != SCAN_OFF || SCANNER_IsScanning()) {
			char * s = DISPLAY_TEXT("");
			if (SCANWATCH_IsActive()) {
				s = DISPLAY_TEXT("S+W");  // Scan+Watch active
			}
			// === VUURWERK v1.2.7 CSS scan glyph ===
			else if (SCANNER_IsScanning()) {
				s = DISPLAY_TEXT("Cs");  // CSS (CTCSS/DCS) scan distinguished from channel scan
			}
			// === END VUURWERK v1.2.7 ===
			else if (IS_MR_CHANNEL(gNextMrChannel)) {
				switch(gEeprom.SCAN_LIST_DEFAULT) {
					case 0: s = DISPLAY_TEXT("1"); break;
					case 1: s = DISPLAY_TEXT("2"); break;
					case 2: s = DISPLAY_TEXT("*"); break;
				}
			}
			else {
				s = DISPLAY_TEXT("S");
			}
			UI_PrintStringSmallBufferNormal(s, line + x + 1);
			x1 = x + 10;
		}
	}
	x += 10;  // font character width

#ifdef ENABLE_VOICE
	// VOICE indicator
	if (gEeprom.VOICE_PROMPT != VOICE_PROMPT_OFF){
		memcpy(line + x, BITMAP_VoicePrompt, sizeof(BITMAP_VoicePrompt));
		x1 = x + sizeof(BITMAP_VoicePrompt);
	}
	x += sizeof(BITMAP_VoicePrompt);
#endif

	if(!SCANNER_IsScanning()) {
		uint8_t dw = (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2;
		if(dw == 1 || dw == 3) { // DWR - dual watch + respond
			if(gDualWatchActive)
				memcpy(line + x + (dw==1?0:2), BITMAP_TDR1, sizeof(BITMAP_TDR1) - (dw==1?0:5));
			else
				memcpy(line + x + 3, BITMAP_TDR2, sizeof(BITMAP_TDR2));
		}
		else if(dw == 2) { // XB - crossband
			memcpy(line + x + 2, BITMAP_XB, sizeof(BITMAP_XB));
		}
	}
	x += sizeof(BITMAP_TDR1) + 1;

	// === VUURWERK v1.3.1 (U1): VFO Split armed indicator ===
	// Split walks VFO B autonomously while parked; any mode that moves the
	// radio by itself must announce itself on screen. Slot x=35..50 is
	// unconditionally free in this build (VOX compiled out, lock floor 61).
	if (VFO_SPLIT_IsEnabled()) {
		UI_PrintStringSmallBufferNormal(DISPLAY_TEXT("SP"), line + x + 1);
		x1 = x + 16;
	}
	x += 16;

#ifdef ENABLE_VOX
	// VOX indicator
	if (gEeprom.VOX_SWITCH) {
		memcpy(line + x, BITMAP_VOX, sizeof(BITMAP_VOX));
		x1 = x + sizeof(BITMAP_VOX) + 1;
	}
	x += sizeof(BITMAP_VOX) + 1;
#endif

	x = MAX(x1, 61u);

	// KEY-LOCK indicator
	if (gEeprom.KEY_LOCK) {
		memcpy(line + x, BITMAP_KeyLock, sizeof(BITMAP_KeyLock));
		x += sizeof(BITMAP_KeyLock);
		x1 = x;
	}
	else if (gWasFKeyPressed) {
		memcpy(line + x, BITMAP_F_Key, sizeof(BITMAP_F_Key));
		x += sizeof(BITMAP_F_Key);
		x1 = x;
	}

	// Signal quality indicator "Q" + bars
	if (FUNCTION_IsRx()) {
		SignalQuality_t qlevel = SIGNAL_QUALITY_Get();
		// Draw "Q" letter (5px wide)
		const uint8_t q_glyph[] = {0x3E,0x41,0x49,0x21,0x5E};
		memcpy(line + x, q_glyph, 5);
		x += 6;
		// Draw 0-3 bars: 2px wide each, heights 2,4,6
		for (uint8_t b = 0; b < 3; b++) {
			if (b < (uint8_t)qlevel) {
				uint8_t h = (b + 1) * 2;
				uint8_t mask = (uint8_t)((1u << h) - 1) << (7 - h);
				line[x]     = mask;
				line[x + 1] = mask;
			}
			x += 3;
		}
		x1 = x;
	}

    /* Reserve the rightmost 28 pixels for 0..100%, including in menus. */
    memset(line + 100, 0, LCD_WIDTH - 100);
    char battery[5];
    sprintf(battery, DISPLAY_TEXT("%u%%"), BATTERY_VoltsToPercent(gBatteryVoltageAverage));
    UI_PrintStringSmallBufferNormal(battery, line + LCD_WIDTH - strlen(battery) * 7);

	// **************

	ST7565_BlitStatusLine();
}
