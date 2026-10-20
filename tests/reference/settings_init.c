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

#include <string.h>

#ifdef ENABLE_FMRADIO
	#include "app/fm.h"
#endif
/* Frozen loader oracle from UVK5 3182e88; original notices retained. */
#include "dtmf.h"
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "misc.h"
#include "settings.h"
#include "ui/menu.h"

void REFERENCE_Settings_InitEEPROM(void)
{
	uint8_t Data[16] = {0};
	// 0E70..0E77
	EEPROM_ReadBuffer(0x0E70, Data, 8);
	gEeprom.CHAN_1_CALL          = IS_MR_CHANNEL(Data[0]) ? Data[0] : MR_CHANNEL_FIRST;
	gEeprom.SQUELCH_LEVEL        = (Data[1] < 10) ? Data[1] : 1;
	gEeprom.TX_TIMEOUT_TIMER     = (Data[2] < 11) ? Data[2] : 1;
	#ifdef ENABLE_NOAA
		gEeprom.NOAA_AUTO_SCAN   = (Data[3] <  2) ? Data[3] : false;
	#endif
	gEeprom.KEY_LOCK             = (Data[4] <  2) ? Data[4] : false;
	#ifdef ENABLE_VOX
		gEeprom.VOX_SWITCH       = (Data[5] <  2) ? Data[5] : false;
		gEeprom.VOX_LEVEL        = (Data[6] < 10) ? Data[6] : 1;
	#endif
	gEeprom.MIC_SENSITIVITY      = (Data[7] <  5) ? Data[7] : 4;

	// 0E78..0E7F
	EEPROM_ReadBuffer(0x0E78, Data, 8);
	gEeprom.BACKLIGHT_MAX 		  = (Data[0] & 0xF) <= 10 ? (Data[0] & 0xF) : 10;
	gEeprom.BACKLIGHT_MIN 		  = (Data[0] >> 4) < gEeprom.BACKLIGHT_MAX ? (Data[0] >> 4) : 0;
#ifdef ENABLE_BLMIN_TMP_OFF
	gEeprom.BACKLIGHT_MIN_STAT	  = BLMIN_STAT_ON;
#endif
	gEeprom.CHANNEL_DISPLAY_MODE  = (Data[1] < 4) ? Data[1] : MDF_FREQUENCY;    // 4 instead of 3 - extra display mode
	gEeprom.CROSS_BAND_RX_TX      = (Data[2] < 3) ? Data[2] : CROSS_BAND_OFF;
	gEeprom.BATTERY_SAVE          = (Data[3] < 5) ? Data[3] : 4;
	// VUURWERK v1.3.3: corrupt byte falls back to OFF, matching the
	// crossband line above. The stock CHAN_A fallback silently ENABLED
	// dual watch -- an autonomous mode -- on a scrambled config byte.
	// Valid stored values (0..2) are untouched.
	gEeprom.DUAL_WATCH            = (Data[4] < 3) ? Data[4] : DUAL_WATCH_OFF;
	gEeprom.BACKLIGHT_TIME        = (Data[5] < ARRAY_SIZE(gSubMenu_BACKLIGHT)) ? Data[5] : 3;
	gEeprom.TAIL_TONE_ELIMINATION = (Data[6] < 2) ? Data[6] : false;
	gEeprom.VFO_OPEN              = (Data[7] < 2) ? Data[7] : true;

	// 0E80..0E87
	EEPROM_ReadBuffer(0x0E80, Data, 8);
#ifdef ENABLE_NOAA
	gEeprom.ScreenChannel[0]   = IS_VALID_CHANNEL(Data[0]) ? Data[0] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gEeprom.ScreenChannel[1]   = IS_VALID_CHANNEL(Data[3]) ? Data[3] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
#else
	// VUURWERK v1.3.3: with NOAA compiled out, IS_VALID_CHANNEL still accepts
	// the NOAA range 207-216, but every NOAA guard downstream is #ifdef'd away
	// -- so a corrupt ScreenChannel byte in that range drives OOB reads AND an
	// OOB WRITE into gMR_ChannelAttributes[207..216] (settings.c UpdateChannel,
	// radio.c), which can corrupt adjacent scheduler globals. Reject the NOAA
	// range at load so every downstream channel index stays in [0, FREQ_CHANNEL_LAST].
	gEeprom.ScreenChannel[0]   = (Data[0] <= FREQ_CHANNEL_LAST) ? Data[0] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gEeprom.ScreenChannel[1]   = (Data[3] <= FREQ_CHANNEL_LAST) ? Data[3] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
#endif
	gEeprom.MrChannel[0]       = IS_MR_CHANNEL(Data[1])    ? Data[1] : MR_CHANNEL_FIRST;
	gEeprom.MrChannel[1]       = IS_MR_CHANNEL(Data[4])    ? Data[4] : MR_CHANNEL_FIRST;
	gEeprom.FreqChannel[0]     = IS_FREQ_CHANNEL(Data[2])  ? Data[2] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
	gEeprom.FreqChannel[1]     = IS_FREQ_CHANNEL(Data[5])  ? Data[5] : (FREQ_CHANNEL_FIRST + BAND6_400MHz);
#ifdef ENABLE_NOAA
	gEeprom.NoaaChannel[0] = IS_NOAA_CHANNEL(Data[6])  ? Data[6] : NOAA_CHANNEL_FIRST;
	gEeprom.NoaaChannel[1] = IS_NOAA_CHANNEL(Data[7])  ? Data[7] : NOAA_CHANNEL_FIRST;
#endif

#ifdef ENABLE_FMRADIO
	{	// 0E88..0E8F
		struct
		{
			uint16_t selFreq;
			uint8_t  selChn;
			uint8_t  isMrMode:1;
			uint8_t  band:2;
			//uint8_t  space:2;
		} __attribute__((packed)) fmCfg;
		EEPROM_ReadBuffer(0x0E88, &fmCfg, 4);

		gEeprom.FM_Band = fmCfg.band;
		//gEeprom.FM_Space = fmCfg.space;
		gEeprom.FM_SelectedFrequency = 
			(fmCfg.selFreq >= BK1080_GetFreqLoLimit(gEeprom.FM_Band) && fmCfg.selFreq <= BK1080_GetFreqHiLimit(gEeprom.FM_Band)) ? 
				fmCfg.selFreq : BK1080_GetFreqLoLimit(gEeprom.FM_Band);
			
		gEeprom.FM_SelectedChannel = fmCfg.selChn;
		gEeprom.FM_IsMrMode        = fmCfg.isMrMode;
	}

	// 0E40..0E67
	EEPROM_ReadBuffer(0x0E40, gFM_Channels, sizeof(gFM_Channels));
	FM_ConfigureChannelState();
#endif

	// 0E90..0E97
	EEPROM_ReadBuffer(0x0E90, Data, 8);
	gEeprom.BEEP_CONTROL                 = Data[0] & 1;
	gEeprom.KEY_M_LONG_PRESS_ACTION      = ((Data[0] >> 1) < ACTION_OPT_LEN) ? (Data[0] >> 1) : ACTION_OPT_NONE;
	gEeprom.KEY_1_SHORT_PRESS_ACTION     = (Data[1] < ACTION_OPT_LEN) ? Data[1] : ACTION_OPT_MONITOR;
	gEeprom.KEY_1_LONG_PRESS_ACTION      = (Data[2] < ACTION_OPT_LEN) ? Data[2] : ACTION_OPT_FLASHLIGHT;
	gEeprom.KEY_2_SHORT_PRESS_ACTION     = (Data[3] < ACTION_OPT_LEN) ? Data[3] : ACTION_OPT_SCAN;
	gEeprom.KEY_2_LONG_PRESS_ACTION      = (Data[4] < ACTION_OPT_LEN) ? Data[4] : ACTION_OPT_NONE;
	gEeprom.SCAN_RESUME_MODE             = (Data[5] < 3)              ? Data[5] : SCAN_RESUME_CO;
	gEeprom.AUTO_KEYPAD_LOCK             = (Data[6] < 2)              ? Data[6] : false;
	gEeprom.POWER_ON_DISPLAY_MODE        = (Data[7] < 4)              ? Data[7] : POWER_ON_DISPLAY_MODE_VOLTAGE;

	// 0E98..0E9F
	// VUURWERK v1.3.3 reclamation: the power-on password is only read back
	// under ENABLE_PWRON_PASSWORD (off in this build), so skip loading it when
	// the feature is compiled out. The 0xFF write in SaveSettings stays, so the
	// stored EEPROM bytes are unchanged for cross-firmware users.
#ifdef ENABLE_PWRON_PASSWORD
	EEPROM_ReadBuffer(0x0E98, Data, 8);
	memcpy(&gEeprom.POWER_ON_PASSWORD, Data, 4);
#endif

	// 0EA0..0EA7
	EEPROM_ReadBuffer(0x0EA0, Data, 8);
	#ifdef ENABLE_VOICE
	gEeprom.VOICE_PROMPT = (Data[0] < 3) ? Data[0] : VOICE_PROMPT_ENGLISH;
	#endif
	#ifdef ENABLE_RSSI_BAR
		if((Data[1] < 200 && Data[1] > 90) && (Data[2] < Data[1]-9 && Data[1] < 160  && Data[2] > 50)) {
			gEeprom.S0_LEVEL = Data[1];
			gEeprom.S9_LEVEL = Data[2];
		}
		else {
			gEeprom.S0_LEVEL = 130;
			gEeprom.S9_LEVEL = 76;
		}
	#endif

	// 0EA8..0EAF
	EEPROM_ReadBuffer(0x0EA8, Data, 8);
	#ifdef ENABLE_ALARM
		gEeprom.ALARM_MODE                 = (Data[0] <  2) ? Data[0] : true;
	#endif
	gEeprom.ROGER                          = (Data[1] <  3) ? Data[1] : ROGER_MODE_OFF;
	gEeprom.REPEATER_TAIL_TONE_ELIMINATION = (Data[2] < 11) ? Data[2] : 0;
	gEeprom.TX_VFO                         = (Data[3] <  2) ? Data[3] : 0;
	gEeprom.BATTERY_TYPE                   = (Data[4] < BATTERY_TYPE_UNKNOWN) ? Data[4] : BATTERY_TYPE_1600_MAH;

	// 0ED0..0ED7
	EEPROM_ReadBuffer(0x0ED0, Data, 8);
	gEeprom.DTMF_SIDE_TONE               = (Data[0] <   2) ? Data[0] : true;

#ifdef ENABLE_DTMF_CALLING
	gEeprom.DTMF_SEPARATE_CODE           = DTMF_ValidateCodes((char *)(Data + 1), 1) ? Data[1] : '*';
	gEeprom.DTMF_GROUP_CALL_CODE         = DTMF_ValidateCodes((char *)(Data + 2), 1) ? Data[2] : '#';
	gEeprom.DTMF_DECODE_RESPONSE         = (Data[3] < 4) ? Data[3] : 0;
	gEeprom.DTMF_auto_reset_time         = (Data[4] < 61 && Data[4] >= 5) ? Data[4] : 10;
#endif
	gEeprom.DTMF_PRELOAD_TIME            = (Data[5] < 101) ? Data[5] * 10 : 300;
	gEeprom.DTMF_FIRST_CODE_PERSIST_TIME = (Data[6] < 101) ? Data[6] * 10 : 100;
	gEeprom.DTMF_HASH_CODE_PERSIST_TIME  = (Data[7] < 101) ? Data[7] * 10 : 100;

	// 0ED8..0EDF
	EEPROM_ReadBuffer(0x0ED8, Data, 8);
	gEeprom.DTMF_CODE_PERSIST_TIME  = (Data[0] < 101) ? Data[0] * 10 : 100;
	gEeprom.DTMF_CODE_INTERVAL_TIME = (Data[1] < 101) ? Data[1] * 10 : 100;
#ifdef ENABLE_DTMF_CALLING
	gEeprom.PERMIT_REMOTE_KILL      = (Data[2] <   2) ? Data[2] : true;

	// 0EE0..0EE7

	EEPROM_ReadBuffer(0x0EE0, Data, sizeof(gEeprom.ANI_DTMF_ID));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gEeprom.ANI_DTMF_ID))) {
		memcpy(gEeprom.ANI_DTMF_ID, Data, sizeof(gEeprom.ANI_DTMF_ID));
	} else {
		strcpy(gEeprom.ANI_DTMF_ID, "123");
	}


	// 0EE8..0EEF
	EEPROM_ReadBuffer(0x0EE8, Data, sizeof(gEeprom.KILL_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gEeprom.KILL_CODE))) {
		memcpy(gEeprom.KILL_CODE, Data, sizeof(gEeprom.KILL_CODE));
	} else {
		strcpy(gEeprom.KILL_CODE, "ABCD9");
	}

	// 0EF0..0EF7
	EEPROM_ReadBuffer(0x0EF0, Data, sizeof(gEeprom.REVIVE_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gEeprom.REVIVE_CODE))) {
		memcpy(gEeprom.REVIVE_CODE, Data, sizeof(gEeprom.REVIVE_CODE));
	} else {
		strcpy(gEeprom.REVIVE_CODE, "9DCBA");
	}
#endif

	// 0EF8..0F07
	EEPROM_ReadBuffer(0x0EF8, Data, sizeof(gEeprom.DTMF_UP_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gEeprom.DTMF_UP_CODE))) {
		memcpy(gEeprom.DTMF_UP_CODE, Data, sizeof(gEeprom.DTMF_UP_CODE));
	} else {
		strcpy(gEeprom.DTMF_UP_CODE, "12345");
	}

	// 0F08..0F17
	EEPROM_ReadBuffer(0x0F08, Data, sizeof(gEeprom.DTMF_DOWN_CODE));
	if (DTMF_ValidateCodes((char *)Data, sizeof(gEeprom.DTMF_DOWN_CODE))) {
		memcpy(gEeprom.DTMF_DOWN_CODE, Data, sizeof(gEeprom.DTMF_DOWN_CODE));
	} else {
		strcpy(gEeprom.DTMF_DOWN_CODE, "54321");
	}

	// 0F18..0F1F
	EEPROM_ReadBuffer(0x0F18, Data, 8);
	gEeprom.SCAN_LIST_DEFAULT = (Data[0] < 3) ? Data[0] : 0;  // we now have 'all' channel scan option
	for (unsigned int i = 0; i < 2; i++)
	{
		const unsigned int j = 1 + (i * 3);
		gEeprom.SCAN_LIST_ENABLED[i]     = (Data[j + 0] < 2) ? Data[j] : false;
		gEeprom.SCANLIST_PRIORITY_CH1[i] =  Data[j + 1];
		gEeprom.SCANLIST_PRIORITY_CH2[i] =  Data[j + 2];
	}

	// 0F40..0F47
	EEPROM_ReadBuffer(0x0F40, Data, 8);
	gSetting_F_LOCK            = (Data[0] < F_LOCK_LEN) ? Data[0] : F_LOCK_DEF;
	gSetting_350TX             = (Data[1] < 2) ? Data[1] : false;  // was true
#ifdef ENABLE_DTMF_CALLING
	gSetting_KILLED            = (Data[2] < 2) ? Data[2] : false;
#endif
	gSetting_200TX             = (Data[3] < 2) ? Data[3] : false;
	gSetting_500TX             = (Data[4] < 2) ? Data[4] : false;
	gSetting_350EN             = (Data[5] < 2) ? Data[5] : true;
	gSetting_ScrambleEnable    = (Data[6] < 2) ? Data[6] : true;
	//gSetting_TX_EN             = (Data[7] & (1u << 0)) ? true : false;
	gSetting_live_DTMF_decoder = !!(Data[7] & (1u << 1));
	gSetting_battery_text      = (((Data[7] >> 2) & 3u) <= 2) ? (Data[7] >> 2) & 3 : 2;
	#ifdef ENABLE_AUDIO_BAR
		gSetting_mic_bar       = !!(Data[7] & (1u << 4));
	#endif
	#ifdef ENABLE_AM_FIX
		gSetting_AM_fix        = !!(Data[7] & (1u << 5));
	#endif
	gSetting_backlight_on_tx_rx = (Data[7] >> 6) & 3u;

	if (!gEeprom.VFO_OPEN)
	{
		gEeprom.ScreenChannel[0] = gEeprom.MrChannel[0];
		gEeprom.ScreenChannel[1] = gEeprom.MrChannel[1];
	}

	// 0D60..0E27
	EEPROM_ReadBuffer(0x0D60, gMR_ChannelAttributes, sizeof(gMR_ChannelAttributes));
	for(uint16_t i = 0; i < sizeof(gMR_ChannelAttributes); i++) {
		ChannelAttributes_t *att = &gMR_ChannelAttributes[i];
		if(att->__val == 0xff){
			att->__val = 0;
			att->band = 0xf;
		}
	}

	// 0F30..0F3F
	EEPROM_ReadBuffer(0x0F30, gCustomAesKey, sizeof(gCustomAesKey));
	bHasCustomAesKey = false;
	for (unsigned int i = 0; i < ARRAY_SIZE(gCustomAesKey); i++)
	{
		if (gCustomAesKey[i] != 0xFFFFFFFFu)
		{
			bHasCustomAesKey = true;
			return;
		}
	}
}

