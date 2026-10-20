/* Original selected menu settings oracle; attribution retained below. */
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
#include "app/aprs_rx.h"
#include <string.h>

#if !defined(ENABLE_OVERLAY)
	#include "ARMCM0.h"
#include "dtmf.h"
#endif
#include "app/generic.h"
#include "app/menu.h"
#include "app/scanner.h"
#include "audio.h"
#include "board.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/eeprom.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "frequencies.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#if defined(ENABLE_OVERLAY)
	#include "sram-overlay.h"
#endif
#include "ui/inputbox.h"
#include "ui/menu.h"
#include "ui/ui.h"
#include "ui/vuurwerk_about.h"
#include "ui/vuurwerk_menu.h"
#include "scanwatch.h"
#include "app/chFrScanner.h"   // VUURWERK v1.3.0 (F10): gScanStateDir
#include "vfo_split.h"

#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

int REFERENCE_MENU_GetLimits(uint8_t menu_id, int32_t *pMin, int32_t *pMax)
{
	switch (menu_id)
	{
		case MENU_SQL:
			*pMin = 0;
			*pMax = 9;
			break;

		case MENU_STEP:
			*pMin = 0;
			*pMax = STEP_N_ELEM - 1;
			break;

		case MENU_ABR:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_BACKLIGHT) - 1;
			break;

		case MENU_ABR_MIN:
			*pMin = 0;
			*pMax = 9;
			break;

		case MENU_ABR_MAX:
			*pMin = 1;
			*pMax = 10;
			break;

		case MENU_F_LOCK:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_F_LOCK) - 1;
			break;

		case MENU_MDF:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_MDF) - 1;
			break;

		case MENU_TXP:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_TXP) - 1;
			break;

		case MENU_SFT_D:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_SFT_D) - 1;
			break;

		case MENU_TDR:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_RXMode) - 1;
			break;

		#ifdef ENABLE_VOICE
			case MENU_VOICE:
				*pMin = 0;
				*pMax = ARRAY_SIZE(gSubMenu_VOICE) - 1;
				break;
		#endif

		case MENU_SC_REV:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_SC_REV) - 1;
			break;

		case MENU_ROGER:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_ROGER) - 1;
			break;

		case MENU_PONMSG:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_PONMSG) - 1;
			break;

		case MENU_R_DCS:
		case MENU_T_DCS:
			*pMin = 0;
			*pMax = 208;
			//*pMax = (ARRAY_SIZE(DCS_Options) * 2);
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
			*pMin = 0;
			*pMax = ARRAY_SIZE(CTCSS_Options);
			break;

		case MENU_W_N:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_W_N) - 1;
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				*pMin = 0;
				*pMax = ARRAY_SIZE(gSubMenu_AL_MOD) - 1;
				break;
		#endif

		case MENU_RESET:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_RESET) - 1;
			break;

		case MENU_COMPAND:
		case MENU_ABR_ON_TX_RX:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_RX_TX) - 1;
			break;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
		#endif
		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
		#endif
		case MENU_BCL:
		case MENU_BEEP:
		case MENU_AUTOLK:
		case MENU_S_ADD1:
		case MENU_S_ADD2:
		case MENU_STE:
		case MENU_D_ST:
#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
#endif
		case MENU_D_LIVE_DEC:
		#ifdef ENABLE_NOAA
			case MENU_NOAA_S:
		#endif
		case MENU_350TX:
		case MENU_200TX:
		case MENU_500TX:
		case MENU_350EN:
		case MENU_SCREN:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_OFF_ON) - 1;
			break;

		case MENU_AM:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gModulationStr) - 1;
			break;

		case MENU_SCR:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_SCRAMBLER) - 1;
			break;

		case MENU_TOT:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_TOT) - 1;
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
		#endif
		case MENU_RP_STE:
			*pMin = 0;
			*pMax = 10;
			break;

		case MENU_MEM_CH:
		case MENU_1_CALL:
		case MENU_DEL_CH:
		case MENU_MEM_NAME:
			*pMin = 0;
			*pMax = MR_CHANNEL_LAST;
			break;

		case MENU_SLIST1:
		case MENU_SLIST2:
			*pMin = -1;
			*pMax = MR_CHANNEL_LAST;
			break;

		case MENU_SAVE:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_SAVE) - 1;
			break;

		case MENU_MIC:
			*pMin = 0;
			*pMax = 4;
			break;

		case MENU_S_LIST:
			*pMin = 0;
			*pMax = 2;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_D_RSP) - 1;
			break;
#endif
		case MENU_PTT_ID:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_PTT_ID) - 1;
			break;

		case MENU_BAT_TXT:
			*pMin = 0;
			*pMax = ARRAY_SIZE(gSubMenu_BAT_TXT) - 1;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_HOLD:
			*pMin = 5;
			*pMax = 60;
			break;
#endif
		case MENU_D_PRE:
			*pMin = 3;
			*pMax = 99;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			*pMin = 1;
			*pMax = 16;
			break;
#endif
		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				*pMin = -50;
				*pMax = +50;
				break;
		#endif

		case MENU_BATCAL:
			*pMin = 1600;
			*pMax = 2200;
			break;

		case MENU_BATTYP:
			*pMin = 0;
			*pMax = 1;
			break;

		case MENU_APRS_BEEP:
        case MENU_APRS_RX:
            *pMin = 0; *pMax = 1; break;
        case MENU_SCANWATCH:
			*pMin = 0;
			*pMax = gSubMenu_SCANWATCH_size - 1;
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			*pMin = 0;
			*pMax = gSubMenu_SIDEFUNCTIONS_size-1;
			break;

		default:
			return -1;
	}

	return 0;
}


void REFERENCE_MENU_AcceptSetting(void)
{
	int32_t        Min;
	int32_t        Max;
	FREQ_Config_t *pConfig = &gTxVfo->freq_config_RX;

	if (!REFERENCE_MENU_GetLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max))
	{
		if (gSubMenuSelection < Min) gSubMenuSelection = Min;
		else
		if (gSubMenuSelection > Max) gSubMenuSelection = Max;
	}

	switch (UI_MENU_GetCurrentMenuId())
	{
		default:
			return;

		case MENU_SQL:
			gEeprom.SQUELCH_LEVEL = gSubMenuSelection;
			gVfoConfigureMode     = VFO_CONFIGURE;
			break;

		case MENU_STEP:
			gTxVfo->STEP_SETTING = FREQUENCY_GetStepIdxFromSortedIdx(gSubMenuSelection);
			if (IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE))
			{
				gRequestSaveChannel = 1;
				return;
			}
			return;

		case MENU_TXP:
			gTxVfo->OUTPUT_POWER = gSubMenuSelection;
			gRequestSaveChannel = 1;
			return;

		case MENU_T_DCS:
			pConfig = &gTxVfo->freq_config_TX;

			// Fallthrough

		case MENU_R_DCS: {
			if (gSubMenuSelection == 0) {
				if (pConfig->CodeType == CODE_TYPE_CONTINUOUS_TONE) {
					return;
				}
				pConfig->Code = 0;
				pConfig->CodeType = CODE_TYPE_OFF;
			}
			else if (gSubMenuSelection < 105) {
				pConfig->CodeType = CODE_TYPE_DIGITAL;
				pConfig->Code = gSubMenuSelection - 1;
			}
			else {
				pConfig->CodeType = CODE_TYPE_REVERSE_DIGITAL;
				pConfig->Code = gSubMenuSelection - 105;
			}

			gRequestSaveChannel = 1;
			return;
		}
		case MENU_T_CTCS:
			pConfig = &gTxVfo->freq_config_TX;
			[[fallthrough]];
		case MENU_R_CTCS: {
			if (gSubMenuSelection == 0) {
				if (pConfig->CodeType != CODE_TYPE_CONTINUOUS_TONE) {
					return;
				}
				pConfig->Code     = 0;
				pConfig->CodeType = CODE_TYPE_OFF;
			}
			else {
				pConfig->Code     = gSubMenuSelection - 1;
				pConfig->CodeType = CODE_TYPE_CONTINUOUS_TONE;
			}

			gRequestSaveChannel = 1;
			return;
		}
		case MENU_SFT_D:
			gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION = gSubMenuSelection;
			gRequestSaveChannel                   = 1;
			return;

		case MENU_OFFSET:
			gTxVfo->TX_OFFSET_FREQUENCY = gSubMenuSelection;
			gRequestSaveChannel         = 1;
			return;

		case MENU_W_N:
			gTxVfo->CHANNEL_BANDWIDTH = gSubMenuSelection;
			gRequestSaveChannel       = 1;
			return;

		case MENU_SCR:
			gTxVfo->SCRAMBLING_TYPE = gSubMenuSelection;
			#if 0
				if (gSubMenuSelection > 0 && gSetting_ScrambleEnable)
					BK4819_EnableScramble(gSubMenuSelection - 1);
				else
					BK4819_DisableScramble();
			#endif
			gRequestSaveChannel     = 1;
			return;

		case MENU_BCL:
			gTxVfo->BUSY_CHANNEL_LOCK = gSubMenuSelection;
			gRequestSaveChannel       = 1;
			return;

		case MENU_MEM_CH:
			gTxVfo->CHANNEL_SAVE = gSubMenuSelection;
			#if 0
				gEeprom.MrChannel[0] = gSubMenuSelection;
			#else
				gEeprom.MrChannel[gEeprom.TX_VFO] = gSubMenuSelection;
			#endif
			gRequestSaveChannel = 2;
			gVfoConfigureMode   = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos      = true;
			return;

		case MENU_MEM_NAME:
			for (int i = 9; i >= 0; i--) {
				if (edit[i] != ' ' && edit[i] != '_' && edit[i] != 0x00 && edit[i] != 0xff)
					break;
				edit[i] = ' ';
			}

			SETTINGS_SaveChannelName(gSubMenuSelection, edit);
			return;

		case MENU_SAVE:
			gEeprom.BATTERY_SAVE = gSubMenuSelection;
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				gEeprom.VOX_SWITCH = gSubMenuSelection != 0;
				if (gEeprom.VOX_SWITCH)
					gEeprom.VOX_LEVEL = gSubMenuSelection - 1;
				SETTINGS_LoadCalibration();
				gFlagReconfigureVfos = true;
				gUpdateStatus        = true;
				break;
		#endif

		case MENU_ABR:
			gEeprom.BACKLIGHT_TIME = gSubMenuSelection;
			break;

		case MENU_ABR_MIN:
			gEeprom.BACKLIGHT_MIN = gSubMenuSelection;
			gEeprom.BACKLIGHT_MAX = MAX(gSubMenuSelection + 1 , gEeprom.BACKLIGHT_MAX);
			break;

		case MENU_ABR_MAX:
			gEeprom.BACKLIGHT_MAX = gSubMenuSelection;
			gEeprom.BACKLIGHT_MIN = MIN(gSubMenuSelection - 1, gEeprom.BACKLIGHT_MIN);
			break;

		case MENU_ABR_ON_TX_RX:
			gSetting_backlight_on_tx_rx = gSubMenuSelection;
			break;

		case MENU_TDR:
			gEeprom.DUAL_WATCH = (gEeprom.TX_VFO + 1) * (gSubMenuSelection & 1);
			gEeprom.CROSS_BAND_RX_TX = (gEeprom.TX_VFO + 1) * ((gSubMenuSelection & 2) > 0);

			gFlagReconfigureVfos = true;
			gUpdateStatus        = true;
			break;

		case MENU_BEEP:
			gEeprom.BEEP_CONTROL = gSubMenuSelection;
			break;

		case MENU_TOT:
			gEeprom.TX_TIMEOUT_TIMER = gSubMenuSelection;
			break;

		#ifdef ENABLE_VOICE
			case MENU_VOICE:
				gEeprom.VOICE_PROMPT = gSubMenuSelection;
				gUpdateStatus        = true;
				break;
		#endif

		case MENU_SC_REV:
			gEeprom.SCAN_RESUME_MODE = gSubMenuSelection;
			break;

		case MENU_MDF:
			gEeprom.CHANNEL_DISPLAY_MODE = gSubMenuSelection;
			break;

		case MENU_AUTOLK:
			gEeprom.AUTO_KEYPAD_LOCK = gSubMenuSelection;
			gKeyLockCountdown        = 30;
			break;

		case MENU_S_ADD1:
			gTxVfo->SCANLIST1_PARTICIPATION = gSubMenuSelection;
			SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			gVfoConfigureMode = VFO_CONFIGURE;
			gFlagResetVfos    = true;
			return;

		case MENU_S_ADD2:
			gTxVfo->SCANLIST2_PARTICIPATION = gSubMenuSelection;
			SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			gVfoConfigureMode = VFO_CONFIGURE;
			gFlagResetVfos    = true;
			return;

		case MENU_STE:
			gEeprom.TAIL_TONE_ELIMINATION = gSubMenuSelection;
			break;

		case MENU_RP_STE:
			gEeprom.REPEATER_TAIL_TONE_ELIMINATION = gSubMenuSelection;
			break;

		case MENU_MIC:
			gEeprom.MIC_SENSITIVITY = gSubMenuSelection;
			SETTINGS_LoadCalibration();
			gFlagReconfigureVfos = true;
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				gSetting_mic_bar = gSubMenuSelection;
				break;
		#endif

		case MENU_COMPAND:
			gTxVfo->Compander = gSubMenuSelection;
			SETTINGS_UpdateChannel(gTxVfo->CHANNEL_SAVE, gTxVfo, true);
			gVfoConfigureMode = VFO_CONFIGURE;
			gFlagResetVfos    = true;
//			gRequestSaveChannel = 1;
			return;

		case MENU_1_CALL:
			gEeprom.CHAN_1_CALL = gSubMenuSelection;
			break;

		case MENU_S_LIST:
			gEeprom.SCAN_LIST_DEFAULT = gSubMenuSelection;
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				gEeprom.ALARM_MODE = gSubMenuSelection;
				break;
		#endif

		case MENU_D_ST:
			gEeprom.DTMF_SIDE_TONE = gSubMenuSelection;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			gEeprom.DTMF_DECODE_RESPONSE = gSubMenuSelection;
			break;

		case MENU_D_HOLD:
			gEeprom.DTMF_auto_reset_time = gSubMenuSelection;
			break;
#endif
		case MENU_D_PRE:
			gEeprom.DTMF_PRELOAD_TIME = gSubMenuSelection * 10;
			break;

		case MENU_PTT_ID:
			gTxVfo->DTMF_PTT_ID_TX_MODE = gSubMenuSelection;
			gRequestSaveChannel         = 1;
			return;

		case MENU_BAT_TXT:
			gSetting_battery_text = gSubMenuSelection;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			gTxVfo->DTMF_DECODING_ENABLE = gSubMenuSelection;
			DTMF_clear_RX();
			gRequestSaveChannel = 1;
			return;
#endif

		case MENU_D_LIVE_DEC:
			gSetting_live_DTMF_decoder = gSubMenuSelection;
			gDTMF_RX_live_timeout = 0;
			memset(gDTMF_RX_live, 0, sizeof(gDTMF_RX_live));
			if (!gSetting_live_DTMF_decoder)
				BK4819_DisableDTMF();
			gFlagReconfigureVfos     = true;
			gUpdateStatus            = true;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			gDTMF_chosen_contact = gSubMenuSelection - 1;
			if (gIsDtmfContactValid)
			{
				GUI_SelectNextDisplay(DISPLAY_MAIN);
				gDTMF_InputMode       = true;
				gDTMF_InputBox_Index  = 3;
				memcpy(gDTMF_InputBox, gDTMF_ID, 4);
				gRequestDisplayScreen = DISPLAY_INVALID;
			}
			return;
#endif
		case MENU_PONMSG:
			gEeprom.POWER_ON_DISPLAY_MODE = gSubMenuSelection;
			break;

		case MENU_ROGER:
			gEeprom.ROGER = gSubMenuSelection;
			break;

		case MENU_AM:
			gTxVfo->Modulation     = gSubMenuSelection;
			gRequestSaveChannel = 1;
			return;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
				gSetting_AM_fix = gSubMenuSelection;
				gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
				gFlagResetVfos    = true;
				break;
		#endif

		#ifdef ENABLE_NOAA
			case MENU_NOAA_S:
				gEeprom.NOAA_AUTO_SCAN = gSubMenuSelection;
				gFlagReconfigureVfos   = true;
				break;
		#endif

		case MENU_DEL_CH:
			SETTINGS_UpdateChannel(gSubMenuSelection, NULL, false);
			gVfoConfigureMode = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos    = true;
			return;

		case MENU_RESET:
			SETTINGS_FactoryReset(gSubMenuSelection);
			return;

		case MENU_350TX:
			gSetting_350TX = gSubMenuSelection;
			break;

		case MENU_F_LOCK: {
			if(gSubMenuSelection == F_LOCK_NONE) { // select 10 times to enable
				gUnlockAllTxConfCnt++;
				if(gUnlockAllTxConfCnt < 10)
					return;
			}
			else
				gUnlockAllTxConfCnt = 0;

			gSetting_F_LOCK = gSubMenuSelection;
			break;
		}
		case MENU_200TX:
			gSetting_200TX = gSubMenuSelection;
			break;

		case MENU_500TX:
			gSetting_500TX = gSubMenuSelection;
			break;

		case MENU_350EN:
			gSetting_350EN       = gSubMenuSelection;
			gVfoConfigureMode    = VFO_CONFIGURE_RELOAD;
			gFlagResetVfos       = true;
			break;

		case MENU_SCREN:
			gSetting_ScrambleEnable = gSubMenuSelection;
			gFlagReconfigureVfos    = true;
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				writeXtalFreqCal(gSubMenuSelection, true);
				return;
		#endif

		case MENU_BATCAL:
		{																 // voltages are averages between discharge curves of 1600 and 2200 mAh
			// gBatteryCalibration[0] = (520ul * gSubMenuSelection) / 760;  // 5.20V empty, blinking above this value, reduced functionality below
			// gBatteryCalibration[1] = (689ul * gSubMenuSelection) / 760;  // 6.89V,  ~5%, 1 bars above this value
			// gBatteryCalibration[2] = (724ul * gSubMenuSelection) / 760;  // 7.24V, ~17%, 2 bars above this value
			gBatteryCalibration[3] =          gSubMenuSelection;         // 7.6V,  ~29%, 3 bars above this value
			// gBatteryCalibration[4] = (771ul * gSubMenuSelection) / 760;  // 7.71V, ~65%, 4 bars above this value
			// gBatteryCalibration[5] = 2300;
			SETTINGS_SaveBatteryCalibration(gBatteryCalibration);
			return;
		}

		case MENU_BATTYP:
			gEeprom.BATTERY_TYPE = gSubMenuSelection;
			break;

		case MENU_APRS_BEEP:
            gAPRS_AlertSound = gSubMenuSelection != 0;
            return;
        case MENU_APRS_RX:
            gAPRS_Enabled = gSubMenuSelection != 0;
            APRS_Stop();
            return; /* RAM-only: do not write EEPROM for APRS. */
        case MENU_SCANWATCH:
			if ((unsigned)gSubMenuSelection >= gSubMenu_SCANWATCH_size) return;
#ifdef ENABLE_VFO_SPLIT
			if (gSubMenuSelection == 3) {
				// VFO Split mode
				gScanWatchArmed = 0;          // VUURWERK v1.3.0 (F10)
				SCANWATCH_Disable();
				gVfoSplit.mode = SPLIT_ALERT;
				gVfoSplit.speed = B_SPEED_NORMAL;
				gVfoSplit.hop_timer_ms = HOP_INTERVAL_NORMAL;
				gVfoSplit.current_channel = 0;
				gVfoSplit.scan_progress = 0;
				gVfoSplit.alert.active = false;
			} else {
				gVfoSplit.mode = SPLIT_OFF;
#else
            {
#endif
				// VUURWERK v1.3.0 (F10): persist the arming instead of
				// enabling the live state machine from the menu. Arming
				// while idle no longer leaves IsActive() true (which
				// drove the power-save wakeup path into DualwatchAlternate
				// with dual watch OFF); CHFRSCANNER_Start arms from
				// gScanWatchArmed on every scan launch, so the setting
				// survives scan stops instead of being one-shot.
				gScanWatchArmed = gSubMenuSelection;
				if (gSubMenuSelection == 0 || gScanStateDir == SCAN_OFF)
					SCANWATCH_Disable();
				else
					SCANWATCH_Enable(gSubMenuSelection - 1);
			}
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			{
				uint8_t * fun[]= {
					&gEeprom.KEY_1_SHORT_PRESS_ACTION,
					&gEeprom.KEY_1_LONG_PRESS_ACTION,
					&gEeprom.KEY_2_SHORT_PRESS_ACTION,
					&gEeprom.KEY_2_LONG_PRESS_ACTION,
					&gEeprom.KEY_M_LONG_PRESS_ACTION};
				*fun[UI_MENU_GetCurrentMenuId()-MENU_F1SHRT] = gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].id;
			}
			break;

	}

	gRequestSaveSettings = true;
}

void REFERENCE_MENU_ShowCurrentSetting(void)
{
	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_SQL:
			gSubMenuSelection = gEeprom.SQUELCH_LEVEL;
			break;

		case MENU_STEP:
			gSubMenuSelection = FREQUENCY_GetSortedIdxFromStepIdx(gTxVfo->STEP_SETTING);
			break;

		case MENU_TXP:
			gSubMenuSelection = gTxVfo->OUTPUT_POWER;
			break;

		case MENU_RESET:
			gSubMenuSelection = 0;
			break;

		case MENU_R_DCS:
		case MENU_R_CTCS:
		{
			DCS_CodeType_t type = gTxVfo->freq_config_RX.CodeType;
			uint8_t code = gTxVfo->freq_config_RX.Code;
			int menuid = UI_MENU_GetCurrentMenuId();

#ifdef ENABLE_TONE_SEARCH
			if(gScanUseCssResult) {
				gScanUseCssResult = false;
				type = gScanCssResultType;
				code = gScanCssResultCode;
			}
#endif
			if((menuid==MENU_R_CTCS) ^ (type==CODE_TYPE_CONTINUOUS_TONE)) { //not the same type
				gSubMenuSelection = 0;
				break;
			}

			switch (type) {
				case CODE_TYPE_CONTINUOUS_TONE:
				case CODE_TYPE_DIGITAL:
					gSubMenuSelection = code + 1;
					break;
				case CODE_TYPE_REVERSE_DIGITAL:
					gSubMenuSelection = code + 105;
					break;
				default:
					gSubMenuSelection = 0;
					break;
			}
		break;
		}

		case MENU_T_DCS:
			switch (gTxVfo->freq_config_TX.CodeType)
			{
				case CODE_TYPE_DIGITAL:
					gSubMenuSelection = gTxVfo->freq_config_TX.Code + 1;
					break;
				case CODE_TYPE_REVERSE_DIGITAL:
					gSubMenuSelection = gTxVfo->freq_config_TX.Code + 105;
					break;
				default:
					gSubMenuSelection = 0;
					break;
			}
			break;

		case MENU_T_CTCS:
			gSubMenuSelection = (gTxVfo->freq_config_TX.CodeType == CODE_TYPE_CONTINUOUS_TONE) ? gTxVfo->freq_config_TX.Code + 1 : 0;
			break;

		case MENU_SFT_D:
			gSubMenuSelection = gTxVfo->TX_OFFSET_FREQUENCY_DIRECTION;
			break;

		case MENU_OFFSET:
			gSubMenuSelection = gTxVfo->TX_OFFSET_FREQUENCY;
			break;

		case MENU_W_N:
			gSubMenuSelection = gTxVfo->CHANNEL_BANDWIDTH;
			break;

		case MENU_SCR:
			gSubMenuSelection = gTxVfo->SCRAMBLING_TYPE;
			break;

		case MENU_BCL:
			gSubMenuSelection = gTxVfo->BUSY_CHANNEL_LOCK;
			break;

		case MENU_MEM_CH:
			#if 0
				gSubMenuSelection = gEeprom.MrChannel[0];
			#else
				gSubMenuSelection = gEeprom.MrChannel[gEeprom.TX_VFO];
			#endif
			break;

		case MENU_MEM_NAME:
			gSubMenuSelection = gEeprom.MrChannel[gEeprom.TX_VFO];
			break;

		case MENU_SAVE:
			gSubMenuSelection = gEeprom.BATTERY_SAVE;
			break;

#ifdef ENABLE_VOX
		case MENU_VOX:
			gSubMenuSelection = gEeprom.VOX_SWITCH ? gEeprom.VOX_LEVEL + 1 : 0;
			break;
#endif

		case MENU_ABR:
			gSubMenuSelection = gEeprom.BACKLIGHT_TIME;
			break;

		case MENU_ABR_MIN:
			gSubMenuSelection = gEeprom.BACKLIGHT_MIN;
			break;

		case MENU_ABR_MAX:
			gSubMenuSelection = gEeprom.BACKLIGHT_MAX;
			break;

		case MENU_ABR_ON_TX_RX:
			gSubMenuSelection = gSetting_backlight_on_tx_rx;
			break;

		case MENU_TDR:
			gSubMenuSelection = (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2;
			break;

		case MENU_BEEP:
			gSubMenuSelection = gEeprom.BEEP_CONTROL;
			break;

		case MENU_TOT:
			gSubMenuSelection = gEeprom.TX_TIMEOUT_TIMER;
			break;

#ifdef ENABLE_VOICE
		case MENU_VOICE:
			gSubMenuSelection = gEeprom.VOICE_PROMPT;
			break;
#endif

		case MENU_SC_REV:
			gSubMenuSelection = gEeprom.SCAN_RESUME_MODE;
			break;

		case MENU_MDF:
			gSubMenuSelection = gEeprom.CHANNEL_DISPLAY_MODE;
			break;

		case MENU_AUTOLK:
			gSubMenuSelection = gEeprom.AUTO_KEYPAD_LOCK;
			break;

		case MENU_S_ADD1:
			gSubMenuSelection = gTxVfo->SCANLIST1_PARTICIPATION;
			break;

		case MENU_S_ADD2:
			gSubMenuSelection = gTxVfo->SCANLIST2_PARTICIPATION;
			break;

		case MENU_STE:
			gSubMenuSelection = gEeprom.TAIL_TONE_ELIMINATION;
			break;

		case MENU_RP_STE:
			gSubMenuSelection = gEeprom.REPEATER_TAIL_TONE_ELIMINATION;
			break;

		case MENU_MIC:
			gSubMenuSelection = gEeprom.MIC_SENSITIVITY;
			break;

#ifdef ENABLE_AUDIO_BAR
		case MENU_MIC_BAR:
			gSubMenuSelection = gSetting_mic_bar;
			break;
#endif

		case MENU_COMPAND:
			gSubMenuSelection = gTxVfo->Compander;
			return;

		case MENU_1_CALL:
			gSubMenuSelection = gEeprom.CHAN_1_CALL;
			break;

		case MENU_S_LIST:
			gSubMenuSelection = gEeprom.SCAN_LIST_DEFAULT;
			break;

		case MENU_SLIST1:
			gSubMenuSelection = RADIO_FindNextChannel(0, 1, true, 0);
			break;

		case MENU_SLIST2:
			gSubMenuSelection = RADIO_FindNextChannel(0, 1, true, 1);
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				gSubMenuSelection = gEeprom.ALARM_MODE;
				break;
		#endif

		case MENU_D_ST:
			gSubMenuSelection = gEeprom.DTMF_SIDE_TONE;
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			gSubMenuSelection = gEeprom.DTMF_DECODE_RESPONSE;
			break;

		case MENU_D_HOLD:
			gSubMenuSelection = gEeprom.DTMF_auto_reset_time;
			break;
#endif
		case MENU_D_PRE:
			gSubMenuSelection = gEeprom.DTMF_PRELOAD_TIME / 10;
			break;

		case MENU_PTT_ID:
			gSubMenuSelection = gTxVfo->DTMF_PTT_ID_TX_MODE;
			break;

		case MENU_BAT_TXT:
			gSubMenuSelection = gSetting_battery_text;
			return;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
			gSubMenuSelection = gTxVfo->DTMF_DECODING_ENABLE;
			break;

		case MENU_D_LIST:
			gSubMenuSelection = gDTMF_chosen_contact + 1;
			break;
#endif
		case MENU_D_LIVE_DEC:
			gSubMenuSelection = gSetting_live_DTMF_decoder;
			break;

		case MENU_PONMSG:
			gSubMenuSelection = gEeprom.POWER_ON_DISPLAY_MODE;
			break;

		case MENU_ROGER:
			gSubMenuSelection = gEeprom.ROGER;
			break;

		case MENU_AM:
			gSubMenuSelection = gTxVfo->Modulation;
			break;

#ifdef ENABLE_AM_FIX
		case MENU_AM_FIX:
			gSubMenuSelection = gSetting_AM_fix;
			break;
#endif
		#ifdef ENABLE_NOAA
			case MENU_NOAA_S:
				gSubMenuSelection = gEeprom.NOAA_AUTO_SCAN;
				break;
		#endif

		case MENU_DEL_CH:
			#if 0
				gSubMenuSelection = RADIO_FindNextChannel(gEeprom.MrChannel[0], 1, false, 1);
			#else
				gSubMenuSelection = RADIO_FindNextChannel(gEeprom.MrChannel[gEeprom.TX_VFO], 1, false, 1);
			#endif
			break;

		case MENU_350TX:
			gSubMenuSelection = gSetting_350TX;
			break;

		case MENU_F_LOCK:
			gSubMenuSelection = gSetting_F_LOCK;
			break;

		case MENU_200TX:
			gSubMenuSelection = gSetting_200TX;
			break;

		case MENU_500TX:
			gSubMenuSelection = gSetting_500TX;
			break;

		case MENU_350EN:
			gSubMenuSelection = gSetting_350EN;
			break;

		case MENU_SCREN:
			gSubMenuSelection = gSetting_ScrambleEnable;
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				gSubMenuSelection = gEeprom.BK4819_XTAL_FREQ_LOW;
				break;
		#endif

		case MENU_BATCAL:
			gSubMenuSelection = gBatteryCalibration[3];
			break;

		case MENU_BATTYP:
			gSubMenuSelection = gEeprom.BATTERY_TYPE;
			break;

		case MENU_APRS_BEEP:
            gSubMenuSelection = gAPRS_AlertSound;
            break;
        case MENU_APRS_RX:
            gSubMenuSelection = gAPRS_Enabled;
            break;
        case MENU_SCANWATCH:
#ifdef ENABLE_VFO_SPLIT
			if (gVfoSplit.mode != SPLIT_OFF)
				gSubMenuSelection = 3;
			else
#endif
				// VUURWERK v1.3.0 (F10): read back the persistent arming,
				// not the live state machine -- the menu used to show OFF
				// whenever no scan was running even though the operator
				// had armed Scan+Watch.
				gSubMenuSelection = gScanWatchArmed;
			break;

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG: {
			uint8_t * fun[]= {
				&gEeprom.KEY_1_SHORT_PRESS_ACTION,
				&gEeprom.KEY_1_LONG_PRESS_ACTION,
				&gEeprom.KEY_2_SHORT_PRESS_ACTION,
				&gEeprom.KEY_2_LONG_PRESS_ACTION,
				&gEeprom.KEY_M_LONG_PRESS_ACTION};
			uint8_t id = *fun[UI_MENU_GetCurrentMenuId()-MENU_F1SHRT];

			for(int i = 0; i < gSubMenu_SIDEFUNCTIONS_size; i++) {
				if(gSubMenu_SIDEFUNCTIONS[i].id==id) {
					gSubMenuSelection = i;
					break;
				}

			}
			break;
		}

		default:
			return;
	}
}

