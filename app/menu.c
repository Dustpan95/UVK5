/* Modified for UVK5: outgoing APRS integration; receive removed; see docs/INTEGRATION.md. */
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

/* UVK5 integration: outgoing APRS controls; no APRS receiver. */
#include <string.h>
#include <stddef.h>

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

uint8_t gUnlockAllTxConfCnt;

#ifdef ENABLE_F_CAL_MENU
	void writeXtalFreqCal(const int32_t value, const bool update_eeprom)
	{
		BK4819_WriteRegister(BK4819_REG_3B, 22656 + value);

		if (update_eeprom)
		{
			struct
			{
				int16_t  BK4819_XtalFreqLow;
				uint16_t EEPROM_1F8A;
				uint16_t EEPROM_1F8C;
				uint8_t  VOLUME_GAIN;
				uint8_t  DAC_GAIN;
			} __attribute__((packed)) misc;

			gEeprom.BK4819_XTAL_FREQ_LOW = value;

			// radio 1 .. 04 00 46 00 50 00 2C 0E
			// radio 2 .. 05 00 46 00 50 00 2C 0E
			//
			EEPROM_ReadBuffer(0x1F88, &misc, 8);
			misc.BK4819_XtalFreqLow = value;
			EEPROM_WriteBuffer(0x1F88, &misc);
		}
	}
#endif

#ifdef ENABLE_TONE_SEARCH
void MENU_StartCssScan(void)
{
	SCANNER_Start();   // VUURWERK v1.3.0 (A1)
	gUpdateStatus = true;
	gCssBackgroundScan = true;

	gRequestDisplayScreen = DISPLAY_MENU;
}

void MENU_CssScanFound(void)
{
	if(gScanCssResultType == CODE_TYPE_DIGITAL || gScanCssResultType == CODE_TYPE_REVERSE_DIGITAL) {
		gMenuCursor = UI_MENU_GetMenuIdx(MENU_R_DCS);
	}
	else if(gScanCssResultType == CODE_TYPE_CONTINUOUS_TONE) {
		gMenuCursor = UI_MENU_GetMenuIdx(MENU_R_CTCS);
	}

	// Set up category navigation to TONE category
	gMenuShowCategoryPicker = false;
	gSelectedCategory = MCAT_TONE;
	VUURWERK_MENU_SelectCategory(MCAT_TONE);

	// Find the correct position within the category for gMenuCursor
	for (uint8_t i = 0; i < gCategoryItemCount; i++) {
		if (gCategoryItems[i] == gMenuCursor) {
			gCategoryItemIdx = i;
			break;
		}
	}

	MENU_ShowCurrentSetting();

	gUpdateStatus = true;
	gUpdateDisplay = true;
}

#endif

void MENU_StopCssScan(void)
{
	gCssBackgroundScan = false;

#ifdef ENABLE_VOICE
	gAnotherVoiceID       = VOICE_ID_SCANNING_STOP;
#endif
	gUpdateDisplay = true;
	gUpdateStatus = true;
}

static uint8_t *const menu_side_fields[]= {
					&gEeprom.KEY_1_SHORT_PRESS_ACTION,
					&gEeprom.KEY_1_LONG_PRESS_ACTION,
					&gEeprom.KEY_2_SHORT_PRESS_ACTION,
					&gEeprom.KEY_2_LONG_PRESS_ACTION,
					&gEeprom.KEY_M_LONG_PRESS_ACTION};

/* One-byte menu fields share the same storage and save behavior. */
enum {
    FIELD_VFO=0x8000, FIELD_SQL=0x4000, FIELD_GLOBAL=0x2000,
    FIELD_RELOAD=0x1000, FIELD_RECONFIGURE=0x0800, FIELD_OFFSET_MASK=0x07ff
};
typedef struct { uint16_t offset; uint8_t id, maximum; } MenuField;
_Static_assert(sizeof(EEPROM_Config_t)<=FIELD_OFFSET_MASK && sizeof(VFO_Info_t)<=FIELD_OFFSET_MASK,"menu field offset range");
static const MenuField menu_fields[] = {
    {offsetof(EEPROM_Config_t,SQUELCH_LEVEL) | FIELD_SQL, MENU_SQL, 9},
    {offsetof(EEPROM_Config_t,BATTERY_SAVE), MENU_SAVE, ARRAY_SIZE(gSubMenu_SAVE)-1},
    {offsetof(EEPROM_Config_t,BACKLIGHT_TIME), MENU_ABR, ARRAY_SIZE(gSubMenu_BACKLIGHT)-1},
    {offsetof(EEPROM_Config_t,BEEP_CONTROL), MENU_BEEP, 1},
    {offsetof(EEPROM_Config_t,TX_TIMEOUT_TIMER), MENU_TOT, ARRAY_SIZE(gSubMenu_TOT)-1},
    {offsetof(EEPROM_Config_t,SCAN_RESUME_MODE), MENU_SC_REV, ARRAY_SIZE(gSubMenu_SC_REV)-1},
    {offsetof(EEPROM_Config_t,CHANNEL_DISPLAY_MODE), MENU_MDF, ARRAY_SIZE(gSubMenu_MDF)-1},
    {offsetof(EEPROM_Config_t,TAIL_TONE_ELIMINATION), MENU_STE, 1},
    {offsetof(EEPROM_Config_t,REPEATER_TAIL_TONE_ELIMINATION), MENU_RP_STE, 10},
    {offsetof(EEPROM_Config_t,CHAN_1_CALL), MENU_1_CALL, MR_CHANNEL_LAST},
    {offsetof(EEPROM_Config_t,SCAN_LIST_DEFAULT), MENU_S_LIST, 2},
    {offsetof(EEPROM_Config_t,DTMF_SIDE_TONE), MENU_D_ST, 1},
    {offsetof(EEPROM_Config_t,POWER_ON_DISPLAY_MODE), MENU_PONMSG, ARRAY_SIZE(gSubMenu_PONMSG)-1},
    {offsetof(EEPROM_Config_t,ROGER), MENU_ROGER, ARRAY_SIZE(gSubMenu_ROGER)-1},
    {offsetof(EEPROM_Config_t,BATTERY_TYPE), MENU_BATTYP, 1},
    {offsetof(VFO_Info_t,OUTPUT_POWER) | FIELD_VFO, MENU_TXP, ARRAY_SIZE(gSubMenu_TXP)-1},
    {offsetof(VFO_Info_t,TX_OFFSET_FREQUENCY_DIRECTION) | FIELD_VFO, MENU_SFT_D, ARRAY_SIZE(gSubMenu_SFT_D)-1},
    {offsetof(VFO_Info_t,CHANNEL_BANDWIDTH) | FIELD_VFO, MENU_W_N, ARRAY_SIZE(gSubMenu_W_N)-1},
    {offsetof(VFO_Info_t,SCRAMBLING_TYPE) | FIELD_VFO, MENU_SCR, ARRAY_SIZE(gSubMenu_SCRAMBLER)-1},
    {offsetof(VFO_Info_t,BUSY_CHANNEL_LOCK) | FIELD_VFO, MENU_BCL, 1},
    {offsetof(VFO_Info_t,DTMF_PTT_ID_TX_MODE) | FIELD_VFO, MENU_PTT_ID, ARRAY_SIZE(gSubMenu_PTT_ID)-1},
    {offsetof(VFO_Info_t,Modulation) | FIELD_VFO, MENU_AM, ARRAY_SIZE(gModulationStr)-1},
    {0 | FIELD_GLOBAL, MENU_350TX, 1},
    {1 | FIELD_GLOBAL, MENU_200TX, 1},
    {2 | FIELD_GLOBAL, MENU_500TX, 1},
    {3 | FIELD_GLOBAL, MENU_ABR_ON_TX_RX, ARRAY_SIZE(gSubMenu_RX_TX)-1},
    {4 | FIELD_GLOBAL, MENU_BAT_TXT, ARRAY_SIZE(gSubMenu_BAT_TXT)-1},
    {5 | (FIELD_GLOBAL | FIELD_RELOAD), MENU_350EN, 1},
    {6 | (FIELD_GLOBAL | FIELD_RECONFIGURE), MENU_SCREN, 1},

#ifdef ENABLE_AM_FIX
    {7 | (FIELD_GLOBAL | FIELD_RELOAD), MENU_AM_FIX, 1},
#endif

};
static const MenuField *FindMenuField(uint8_t id)
{
    for(unsigned i=0;i<ARRAY_SIZE(menu_fields);i++) if(menu_fields[i].id==id) return &menu_fields[i];
    return NULL;
}
static uint8_t *const global_menu_fields[]={
    (uint8_t *)&gSetting_350TX,
    (uint8_t *)&gSetting_200TX,
    (uint8_t *)&gSetting_500TX,
    (uint8_t *)&gSetting_backlight_on_tx_rx,
    (uint8_t *)&gSetting_battery_text,
    (uint8_t *)&gSetting_350EN,
    (uint8_t *)&gSetting_ScrambleEnable,
#ifdef ENABLE_AM_FIX
    (uint8_t *)&gSetting_AM_fix,
#endif
};
static uint8_t *MenuFieldAddress(const MenuField *field)
{
    if(field->offset & FIELD_GLOBAL) return global_menu_fields[field->offset & FIELD_OFFSET_MASK];
    uint8_t *base=field->offset & FIELD_VFO ? (uint8_t *)gTxVfo : (uint8_t *)&gEeprom;
    return base+(field->offset & FIELD_OFFSET_MASK);
}
_Static_assert(sizeof(((EEPROM_Config_t *)0)->SQUELCH_LEVEL)==1,"compact menu field ABI: SQUELCH_LEVEL");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->BATTERY_SAVE)==1,"compact menu field ABI: BATTERY_SAVE");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->BACKLIGHT_TIME)==1,"compact menu field ABI: BACKLIGHT_TIME");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->BEEP_CONTROL)==1,"compact menu field ABI: BEEP_CONTROL");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->TX_TIMEOUT_TIMER)==1,"compact menu field ABI: TX_TIMEOUT_TIMER");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->SCAN_RESUME_MODE)==1,"compact menu field ABI: SCAN_RESUME_MODE");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->CHANNEL_DISPLAY_MODE)==1,"compact menu field ABI: CHANNEL_DISPLAY_MODE");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->TAIL_TONE_ELIMINATION)==1,"compact menu field ABI: TAIL_TONE_ELIMINATION");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->REPEATER_TAIL_TONE_ELIMINATION)==1,"compact menu field ABI: REPEATER_TAIL_TONE_ELIMINATION");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->CHAN_1_CALL)==1,"compact menu field ABI: CHAN_1_CALL");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->SCAN_LIST_DEFAULT)==1,"compact menu field ABI: SCAN_LIST_DEFAULT");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->DTMF_SIDE_TONE)==1,"compact menu field ABI: DTMF_SIDE_TONE");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->POWER_ON_DISPLAY_MODE)==1,"compact menu field ABI: POWER_ON_DISPLAY_MODE");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->ROGER)==1,"compact menu field ABI: ROGER");
_Static_assert(sizeof(((EEPROM_Config_t *)0)->BATTERY_TYPE)==1,"compact menu field ABI: BATTERY_TYPE");
_Static_assert(sizeof(((VFO_Info_t *)0)->OUTPUT_POWER)==1,"compact menu field ABI: OUTPUT_POWER");
_Static_assert(sizeof(((VFO_Info_t *)0)->TX_OFFSET_FREQUENCY_DIRECTION)==1,"compact menu field ABI: TX_OFFSET_FREQUENCY_DIRECTION");
_Static_assert(sizeof(((VFO_Info_t *)0)->CHANNEL_BANDWIDTH)==1,"compact menu field ABI: CHANNEL_BANDWIDTH");
_Static_assert(sizeof(((VFO_Info_t *)0)->SCRAMBLING_TYPE)==1,"compact menu field ABI: SCRAMBLING_TYPE");
_Static_assert(sizeof(((VFO_Info_t *)0)->BUSY_CHANNEL_LOCK)==1,"compact menu field ABI: BUSY_CHANNEL_LOCK");
_Static_assert(sizeof(((VFO_Info_t *)0)->DTMF_PTT_ID_TX_MODE)==1,"compact menu field ABI: DTMF_PTT_ID_TX_MODE");
_Static_assert(sizeof(((VFO_Info_t *)0)->Modulation)==1,"compact menu field ABI: Modulation");

#define MENU_LIMITS(minimum,maximum) (((uint32_t)(uint16_t)(minimum)<<16)|(uint16_t)(maximum))
static uint32_t PackedLimits(uint8_t menu_id)
{
    const MenuField *field=FindMenuField(menu_id);
    if(field) return field->maximum;
    switch (menu_id)
	{

		case MENU_STEP:
			return MENU_LIMITS(0, STEP_N_ELEM - 1);

		case MENU_ABR_MIN:
			return MENU_LIMITS(0, 9);

		case MENU_ABR_MAX:
			return MENU_LIMITS(1, 10);

		case MENU_F_LOCK:
			return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_F_LOCK) - 1);

		case MENU_TDR:
			return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_RXMode) - 1);

		#ifdef ENABLE_VOICE
			case MENU_VOICE:
				return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_VOICE) - 1);
		#endif

		case MENU_R_DCS:
		case MENU_T_DCS:
			return MENU_LIMITS(0, 208);

		case MENU_R_CTCS:
		case MENU_T_CTCS:
			return MENU_LIMITS(0, ARRAY_SIZE(CTCSS_Options));

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_AL_MOD) - 1);
		#endif

		case MENU_RESET:
			return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_RESET) - 1);

		case MENU_COMPAND:

			return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_RX_TX) - 1);

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
		#endif

		case MENU_AUTOLK:
		case MENU_S_ADD1:
		case MENU_S_ADD2:

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_DCD:
#endif
		case MENU_D_LIVE_DEC:
		#ifdef ENABLE_NOAA
			case MENU_NOAA_S:
		#endif

			return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_OFF_ON) - 1);

		#ifdef ENABLE_VOX
			case MENU_VOX:
		#endif

			return MENU_LIMITS(0, 10);

		case MENU_MEM_CH:

		case MENU_DEL_CH:
		case MENU_MEM_NAME:
			return MENU_LIMITS(0, MR_CHANNEL_LAST);

		case MENU_SLIST1:
		case MENU_SLIST2:
			return MENU_LIMITS(-1, MR_CHANNEL_LAST);

		case MENU_MIC:
			return MENU_LIMITS(0, 4);

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			return MENU_LIMITS(0, ARRAY_SIZE(gSubMenu_D_RSP) - 1);
#endif

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_HOLD:
			return MENU_LIMITS(5, 60);
#endif
		case MENU_D_PRE:
			return MENU_LIMITS(3, 99);

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			return MENU_LIMITS(1, 16);
#endif
		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				return MENU_LIMITS(-50, +50);
		#endif

		case MENU_BATCAL:
			return MENU_LIMITS(1600, 2200);


        case MENU_SCANWATCH:
			return MENU_LIMITS(0, gSubMenu_SCANWATCH_size - 1);

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			return MENU_LIMITS(0, gSubMenu_SIDEFUNCTIONS_size-1);

		default:
			return UINT32_MAX;
	}

	return UINT32_MAX;
}

static inline __attribute__((always_inline)) int UnpackLimits(uint8_t id,int32_t *minimum,int32_t *maximum)
{
    uint32_t range=PackedLimits(id);
    if(range==UINT32_MAX) return -1;
    *minimum=(int16_t)(range>>16);*maximum=range&0xffff;return 0;
}
int MENU_GetLimits(uint8_t id,int32_t *minimum,int32_t *maximum)
{
    return UnpackLimits(id,minimum,maximum);
}

void MENU_AcceptSetting(void)
{
	int32_t        Min;
	int32_t        Max;
	FREQ_Config_t *pConfig = &gTxVfo->freq_config_RX;

	if (!UnpackLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max))
	{
		if (gSubMenuSelection < Min) gSubMenuSelection = Min;
		else
		if (gSubMenuSelection > Max) gSubMenuSelection = Max;
	}

    const MenuField *field=FindMenuField(UI_MENU_GetCurrentMenuId());
    if(field) {
        *MenuFieldAddress(field)=gSubMenuSelection;
        if(field->offset & FIELD_VFO) gRequestSaveChannel=1;
        else {
            if(field->offset & FIELD_SQL) gVfoConfigureMode=VFO_CONFIGURE;
            if(field->offset & FIELD_RELOAD) {gVfoConfigureMode=VFO_CONFIGURE_RELOAD;gFlagResetVfos=true;}
            if(field->offset & FIELD_RECONFIGURE) gFlagReconfigureVfos=true;
            gRequestSaveSettings=true;
        }
        return;
    }
    switch (UI_MENU_GetCurrentMenuId())
	{
		default:
			return;

		case MENU_STEP:
			gTxVfo->STEP_SETTING = FREQUENCY_GetStepIdxFromSortedIdx(gSubMenuSelection);
			if (IS_FREQ_CHANNEL(gTxVfo->CHANNEL_SAVE))
			{
				gRequestSaveChannel = 1;
				return;
			}
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

		case MENU_OFFSET:
			gTxVfo->TX_OFFSET_FREQUENCY = gSubMenuSelection;
			gRequestSaveChannel         = 1;
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

		case MENU_ABR_MIN:
			gEeprom.BACKLIGHT_MIN = gSubMenuSelection;
			gEeprom.BACKLIGHT_MAX = MAX(gSubMenuSelection + 1 , gEeprom.BACKLIGHT_MAX);
			break;

		case MENU_ABR_MAX:
			gEeprom.BACKLIGHT_MAX = gSubMenuSelection;
			gEeprom.BACKLIGHT_MIN = MIN(gSubMenuSelection - 1, gEeprom.BACKLIGHT_MIN);
			break;

		case MENU_TDR:
			gEeprom.DUAL_WATCH = (gEeprom.TX_VFO + 1) * (gSubMenuSelection & 1);
			gEeprom.CROSS_BAND_RX_TX = (gEeprom.TX_VFO + 1) * ((gSubMenuSelection & 2) > 0);

			gFlagReconfigureVfos = true;
			gUpdateStatus        = true;
			break;

		#ifdef ENABLE_VOICE
			case MENU_VOICE:
				gEeprom.VOICE_PROMPT = gSubMenuSelection;
				gUpdateStatus        = true;
				break;
		#endif

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
				uint8_t *const *fun=menu_side_fields;
				*fun[UI_MENU_GetCurrentMenuId()-MENU_F1SHRT] = gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].id;
			}
			break;

	}

	gRequestSaveSettings = true;
}

static void MENU_ClampSelection(int8_t Direction)
{
	int32_t Min;
	int32_t Max;

	if (!UnpackLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max))
	{
		int32_t Selection = gSubMenuSelection;
		if (Selection < Min) Selection = Min;
		else
		if (Selection > Max) Selection = Max;
		gSubMenuSelection = NUMBER_AddWithWraparound(Selection, Direction, Min, Max);
	}
}

void MENU_ShowCurrentSetting(void)
{
    const MenuField *field=FindMenuField(UI_MENU_GetCurrentMenuId());
    if(field) {gSubMenuSelection=*MenuFieldAddress(field);return;}
    switch (UI_MENU_GetCurrentMenuId())
	{

		case MENU_STEP:
			gSubMenuSelection = FREQUENCY_GetSortedIdxFromStepIdx(gTxVfo->STEP_SETTING);
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

		case MENU_OFFSET:
			gSubMenuSelection = gTxVfo->TX_OFFSET_FREQUENCY;
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

#ifdef ENABLE_VOX
		case MENU_VOX:
			gSubMenuSelection = gEeprom.VOX_SWITCH ? gEeprom.VOX_LEVEL + 1 : 0;
			break;
#endif

		case MENU_ABR_MIN:
			gSubMenuSelection = gEeprom.BACKLIGHT_MIN;
			break;

		case MENU_ABR_MAX:
			gSubMenuSelection = gEeprom.BACKLIGHT_MAX;
			break;

		case MENU_TDR:
			gSubMenuSelection = (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF) + (gEeprom.CROSS_BAND_RX_TX != CROSS_BAND_OFF) * 2;
			break;

#ifdef ENABLE_VOICE
		case MENU_VOICE:
			gSubMenuSelection = gEeprom.VOICE_PROMPT;
			break;
#endif

		case MENU_AUTOLK:
			gSubMenuSelection = gEeprom.AUTO_KEYPAD_LOCK;
			break;

		case MENU_S_ADD1:
			gSubMenuSelection = gTxVfo->SCANLIST1_PARTICIPATION;
			break;

		case MENU_S_ADD2:
			gSubMenuSelection = gTxVfo->SCANLIST2_PARTICIPATION;
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

		case MENU_F_LOCK:
			gSubMenuSelection = gSetting_F_LOCK;
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				gSubMenuSelection = gEeprom.BK4819_XTAL_FREQ_LOW;
				break;
		#endif

		case MENU_BATCAL:
			gSubMenuSelection = gBatteryCalibration[3];
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
			uint8_t *const *fun=menu_side_fields;
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

static void MENU_Key_0_to_9(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	uint8_t  Offset;
	int32_t  Min;
	int32_t  Max;
	uint16_t Value = 0;

	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && edit_index >= 0)
	{	// currently editing the channel name

		if (edit_index < 10)
		{
			if (Key <= KEY_9)
			{
				edit[edit_index] = '0' + Key - KEY_0;

				if (++edit_index >= 10)
				{	// exit edit
					gFlagAcceptSetting  = false;
					gAskForConfirmation = 1;
				}

				gRequestDisplayScreen = DISPLAY_MENU;
			}
		}

		return;
	}

	INPUTBOX_Append(Key);

	gRequestDisplayScreen = DISPLAY_MENU;

	if (!gIsInSubMenu) {
		// Number keys navigate within the current category
		const uint8_t maxItem = (gSelectedCategory >= 0 && gCategoryItemCount > 0)
			? gCategoryItemCount : gMenuListCount;

		switch (gInputBoxIndex) {
			case 2:
				gInputBoxIndex = 0;

				Value = (gInputBox[0] * 10) + gInputBox[1];

				if (Value > 0 && Value <= maxItem) {
					if (gSelectedCategory >= 0) {
						gCategoryItemIdx = Value - 1;
						VUURWERK_MENU_SyncCursor();
					} else {
						gMenuCursor = Value - 1;
					}
					gFlagRefreshSetting = true;
					return;
				}

				if (Value <= maxItem)
					break;

				gInputBox[0]   = gInputBox[1];
				gInputBoxIndex = 1;
				[[fallthrough]];
			case 1:
				Value = gInputBox[0];
				if (Value > 0 && Value <= maxItem) {
					if (gSelectedCategory >= 0) {
						gCategoryItemIdx = Value - 1;
						VUURWERK_MENU_SyncCursor();
					} else {
						gMenuCursor = Value - 1;
					}
					gFlagRefreshSetting = true;
					return;
				}
				break;
		}

		gInputBoxIndex = 0;

		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_OFFSET) {
		uint32_t Frequency;

		if (gInputBoxIndex < 6) { // invalid frequency
#ifdef ENABLE_VOICE
			gAnotherVoiceID = (VOICE_ID_t)Key;
#endif
			return;
		}

#ifdef ENABLE_VOICE
		gAnotherVoiceID = (VOICE_ID_t)Key;
#endif

		Frequency = StrToUL(INPUTBOX_GetAscii())*100;
		gSubMenuSelection = FREQUENCY_RoundToStep(Frequency, gTxVfo->StepFrequency);

		gInputBoxIndex = 0;
		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
		UI_MENU_GetCurrentMenuId() == MENU_DEL_CH ||
		UI_MENU_GetCurrentMenuId() == MENU_1_CALL ||
		UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
	{	// enter 3-digit channel number

		if (gInputBoxIndex < 3) {
#ifdef ENABLE_VOICE
			gAnotherVoiceID   = (VOICE_ID_t)Key;
#endif
			gRequestDisplayScreen = DISPLAY_MENU;
			return;
		}

		gInputBoxIndex = 0;

		Value = ((gInputBox[0] * 100) + (gInputBox[1] * 10) + gInputBox[2]) - 1;

		if (IS_MR_CHANNEL(Value)) {
#ifdef ENABLE_VOICE
			gAnotherVoiceID = (VOICE_ID_t)Key;
#endif
			gSubMenuSelection = Value;
			return;
		}

		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	if (UnpackLimits(UI_MENU_GetCurrentMenuId(), &Min, &Max)) {
		gInputBoxIndex = 0;
		gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
		return;
	}

	Offset = (Max >= 100) ? 3 : (Max >= 10) ? 2 : 1;

	switch (gInputBoxIndex) {
		case 1:
			Value = gInputBox[0];
			break;
		case 2:
			Value = (gInputBox[0] *  10) + gInputBox[1];
			break;
		case 3:
			Value = (gInputBox[0] * 100) + (gInputBox[1] * 10) + gInputBox[2];
			break;
	}

	if (Offset == gInputBoxIndex)
		gInputBoxIndex = 0;

	if (Value <= Max) {
		gSubMenuSelection = Value;
		return;
	}

	gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
}

static void MENU_Key_EXIT(bool bKeyPressed, bool bKeyHeld)
{
	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (!gCssBackgroundScan)
	{
		/* Backlight related menus set full brightness. Set it back to the configured value,
		   just in case we are exiting from one of them. */
		BACKLIGHT_TurnOn();

		if (gIsInSubMenu)
		{
			if (gInputBoxIndex == 0 || UI_MENU_GetCurrentMenuId() != MENU_OFFSET)
			{
				gAskForConfirmation = 0;
				gIsInSubMenu        = false;
				gInputBoxIndex      = 0;
				gFlagRefreshSetting = true;

				#ifdef ENABLE_VOICE
					gAnotherVoiceID = VOICE_ID_CANCEL;
				#endif
			}
			else
				gInputBox[--gInputBoxIndex] = 10;

			// ***********************

			gRequestDisplayScreen = DISPLAY_MENU;
			return;
		}

		#ifdef ENABLE_VOICE
			gAnotherVoiceID = VOICE_ID_CANCEL;
		#endif

		// go back to category picker instead of exiting menu
		VUURWERK_MENU_EnterCategoryPicker();
		gRequestDisplayScreen = DISPLAY_MENU;
	}
	else
	{
		MENU_StopCssScan();

		#ifdef ENABLE_VOICE
			gAnotherVoiceID   = VOICE_ID_SCANNING_STOP;
		#endif

		gRequestDisplayScreen = DISPLAY_MENU;
	}

	gPttWasReleased = true;
}

static void MENU_Key_MENU(const bool bKeyPressed, const bool bKeyHeld)
{
	if (bKeyHeld || !bKeyPressed)
		return;


    if (UI_MENU_GetCurrentMenuId() == MENU_ABOUT) {
		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
		VUURWERK_ABOUT_Show();
		return;
	}

	gBeepToPlay           = BEEP_1KHZ_60MS_OPTIONAL;
	gRequestDisplayScreen = DISPLAY_MENU;

	if (!gIsInSubMenu)
	{
		#ifdef ENABLE_VOICE
			if (UI_MENU_GetCurrentMenuId() != MENU_SCR)
				gAnotherVoiceID = MenuList[gMenuCursor].voice_id;
		#endif
        if (UI_MENU_GetCurrentMenuId() == MENU_UPCODE
			|| UI_MENU_GetCurrentMenuId() == MENU_DWCODE
#ifdef ENABLE_DTMF_CALLING
			|| UI_MENU_GetCurrentMenuId() == MENU_ANI_ID
#endif
			)
            return;
		#if 1
			if (UI_MENU_GetCurrentMenuId() == MENU_DEL_CH || UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
				if (!RADIO_CheckValidChannel(gSubMenuSelection, false, 0))
					return;  // invalid channel
		#endif

		gAskForConfirmation = 0;
		gIsInSubMenu        = true;

//		if (UI_MENU_GetCurrentMenuId() != MENU_D_LIST)
		{
			gInputBoxIndex      = 0;
			edit_index          = -1;
		}

		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
	{
		if (edit_index < 0)
		{	// enter channel name edit mode
			if (!RADIO_CheckValidChannel(gSubMenuSelection, false, 0))
				return;

			SETTINGS_FetchChannelName(edit, gSubMenuSelection);

			// pad the channel name out with '_'
			edit_index = strlen(edit);
			while (edit_index < 10)
				edit[edit_index++] = '_';
			edit[edit_index] = 0;
			edit_index = 0;  // 'edit_index' is going to be used as the cursor position

			// make a copy so we can test for change when exiting the menu item
			memcpy(edit_original, edit, sizeof(edit_original));

			return;
		}
		else
		if (edit_index >= 0 && edit_index < 10)
		{	// editing the channel name characters

			if (++edit_index < 10)
				return;	// next char

			// exit
			gFlagAcceptSetting  = false;
			gAskForConfirmation = 0;
			if (memcmp(edit_original, edit, sizeof(edit_original)) == 0) {
				// no change - drop it
				gIsInSubMenu = false;
			}
		}
	}

	// exiting the sub menu

	if (gIsInSubMenu)
	{
		if (UI_MENU_GetCurrentMenuId() == MENU_RESET  ||
			UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
			UI_MENU_GetCurrentMenuId() == MENU_DEL_CH ||
			UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME)
		{
			switch (gAskForConfirmation)
			{
				case 0:
					gAskForConfirmation = 1;
					break;

				case 1:
					gAskForConfirmation = 2;

					UI_DisplayMenu();

					if (UI_MENU_GetCurrentMenuId() == MENU_RESET)
					{
						#ifdef ENABLE_VOICE
							AUDIO_SetVoiceID(0, VOICE_ID_CONFIRM);
							AUDIO_PlaySingleVoice(true);
						#endif

						MENU_AcceptSetting();

						#if defined(ENABLE_OVERLAY)
							overlay_FLASH_RebootToBootloader();
						#else
							NVIC_SystemReset();
						#endif
					}

					gFlagAcceptSetting  = true;
					gIsInSubMenu        = false;
					gAskForConfirmation = 0;
			}
		}
		else
		{
			gFlagAcceptSetting = true;
			gIsInSubMenu       = false;
		}
	}

	#ifdef ENABLE_TONE_SEARCH
	SCANNER_Stop();
	#endif

	#ifdef ENABLE_VOICE
		if (UI_MENU_GetCurrentMenuId() == MENU_SCR)
			gAnotherVoiceID = (gSubMenuSelection == 0) ? VOICE_ID_SCRAMBLER_OFF : VOICE_ID_SCRAMBLER_ON;
		else
			gAnotherVoiceID = VOICE_ID_CONFIRM;
	#endif

	gInputBoxIndex = 0;
}

static void MENU_Key_STAR(const bool bKeyPressed, const bool bKeyHeld)
{
	if (bKeyHeld || !bKeyPressed)
		return;

	gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && edit_index >= 0)
	{	// currently editing the channel name

		if (edit_index < 10)
		{
			edit[edit_index] = '-';

			if (++edit_index >= 10)
			{	// exit edit
				gFlagAcceptSetting  = false;
				gAskForConfirmation = 1;
			}

			gRequestDisplayScreen = DISPLAY_MENU;
		}

		return;
	}

#ifdef ENABLE_TONE_SEARCH
	RADIO_SelectVfos();

	#ifdef ENABLE_NOAA
		if (!IS_NOAA_CHANNEL(gRxVfo->CHANNEL_SAVE) && gRxVfo->Modulation == MODULATION_FM)
	#else
		if (gRxVfo->Modulation ==  MODULATION_FM)
	#endif
	{
		if ((UI_MENU_GetCurrentMenuId() == MENU_R_CTCS || UI_MENU_GetCurrentMenuId() == MENU_R_DCS) && gIsInSubMenu)
		{	// scan CTCSS or DCS to find the tone/code of the incoming signal
			if (!SCANNER_IsScanning())
				MENU_StartCssScan();
			else
				MENU_StopCssScan();
		}

		gPttWasReleased = true;
		return;
	}

#endif

	gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
}

static void MENU_Key_UP_DOWN(bool bKeyPressed, bool bKeyHeld, int8_t Direction)
{
	uint8_t VFO;
	uint8_t Channel;
	bool    bCheckScanList;

	if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && gIsInSubMenu && edit_index >= 0)
	{	// change the character
		if (bKeyPressed && edit_index < 10 && Direction != 0)
		{
			const char   unwanted[] = "$%&!\"':;?^`|{}";
			char         c          = edit[edit_index] + Direction;
			unsigned int i          = 0;
			while (i < sizeof(unwanted) && c >= 32 && c <= 126)
			{
				if (c == unwanted[i++])
				{	// choose next character
					c += Direction;
					i = 0;
				}
			}
			edit[edit_index] = (c < 32) ? 126 : (c > 126) ? 32 : c;

			gRequestDisplayScreen = DISPLAY_MENU;
		}
		return;
	}

	if (!bKeyHeld)
	{
		if (!bKeyPressed)
			return;

		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

		gInputBoxIndex = 0;
	}
	else if (!bKeyPressed)
		return;

	if (SCANNER_IsScanning())
		return;

	if (!gIsInSubMenu) {
		if (gSelectedCategory >= 0 && gCategoryItemCount > 0) {
			// Navigate within category (filtered)
			gCategoryItemIdx = NUMBER_AddWithWraparound(
				gCategoryItemIdx, -Direction, 0, gCategoryItemCount - 1);
			VUURWERK_MENU_SyncCursor();
		} else {
			// Fallback: flat list
			gMenuCursor = NUMBER_AddWithWraparound(
				gMenuCursor, -Direction, 0, gMenuListCount - 1);
		}

		gFlagRefreshSetting = true;

		gRequestDisplayScreen = DISPLAY_MENU;

		if (UI_MENU_GetCurrentMenuId() != MENU_ABR
			&& UI_MENU_GetCurrentMenuId() != MENU_ABR_MIN
			&& UI_MENU_GetCurrentMenuId() != MENU_ABR_MAX
			&& gEeprom.BACKLIGHT_TIME == 0)
		{
			BACKLIGHT_TurnOff();
		}

		return;
	}

	if (UI_MENU_GetCurrentMenuId() == MENU_OFFSET) {
		int32_t Offset = (Direction * gTxVfo->StepFrequency) + gSubMenuSelection;
		if (Offset < 99999990) {
			if (Offset < 0)
				Offset = 99999990;
		}
		else
			Offset = 0;

		gSubMenuSelection     = FREQUENCY_RoundToStep(Offset, gTxVfo->StepFrequency);
		gRequestDisplayScreen = DISPLAY_MENU;
		return;
	}

	VFO = 0;

	switch (UI_MENU_GetCurrentMenuId())
	{
		case MENU_DEL_CH:
		case MENU_1_CALL:
		case MENU_MEM_NAME:
			bCheckScanList = false;
			break;

		case MENU_SLIST2:
			VFO = 1;
			[[fallthrough]];
		case MENU_SLIST1:
			bCheckScanList = true;
			break;

		default:
			MENU_ClampSelection(Direction);
			gRequestDisplayScreen = DISPLAY_MENU;
			return;
	}

	Channel = RADIO_FindNextChannel(gSubMenuSelection + Direction, Direction, bCheckScanList, VFO);
	if (Channel != 0xFF)
		gSubMenuSelection = Channel;

	gRequestDisplayScreen = DISPLAY_MENU;
}

void MENU_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld)
{
	// --- Category picker navigation ---
	if (gMenuShowCategoryPicker) {
		if (bKeyHeld || !bKeyPressed)
			goto menu_timeout;

		gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;

		const uint8_t visCount = VUURWERK_MENU_GetVisibleCategoryCount();

		switch (Key) {
			case KEY_UP:
				gCategoryPickerCursor = (gCategoryPickerCursor > 0)
					? gCategoryPickerCursor - 1 : visCount - 1;
				break;
			case KEY_DOWN:
				gCategoryPickerCursor = (gCategoryPickerCursor < visCount - 1)
					? gCategoryPickerCursor + 1 : 0;
				break;
			case KEY_MENU:
				gSelectedCategory = gCategoryPickerCursor;
				VUURWERK_MENU_SelectCategory(gSelectedCategory);
				gMenuShowCategoryPicker = false;
				gFlagRefreshSetting = true;
				break;
			case KEY_1: case KEY_2: case KEY_3: case KEY_4:
			case KEY_5: case KEY_6: case KEY_7: case KEY_8:
			case KEY_9:
			{
				uint8_t catIdx = Key - KEY_1;
				if (catIdx < visCount) {
					gSelectedCategory = catIdx;
					VUURWERK_MENU_SelectCategory(gSelectedCategory);
					gMenuShowCategoryPicker = false;
					gFlagRefreshSetting = true;
				}
				break;
			}
			case KEY_EXIT:
				gRequestDisplayScreen = DISPLAY_MAIN;
				if (gEeprom.BACKLIGHT_TIME == 0)
					BACKLIGHT_TurnOff();
				gPttWasReleased = true;
				goto menu_timeout;
			default:
				break;
		}
		gRequestDisplayScreen = DISPLAY_MENU;
		goto menu_timeout;
	}

	switch (Key) {
		case KEY_0...KEY_9:
			MENU_Key_0_to_9(Key, bKeyPressed, bKeyHeld);
			break;
		case KEY_MENU:
			MENU_Key_MENU(bKeyPressed, bKeyHeld);
			break;
		case KEY_UP:
			MENU_Key_UP_DOWN(bKeyPressed, bKeyHeld,  1);
			break;
		case KEY_DOWN:
			MENU_Key_UP_DOWN(bKeyPressed, bKeyHeld, -1);
			break;
		case KEY_EXIT:
			MENU_Key_EXIT(bKeyPressed, bKeyHeld);
			break;
		case KEY_STAR:
			MENU_Key_STAR(bKeyPressed, bKeyHeld);
			break;
		case KEY_F:
			if (UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && edit_index >= 0)
			{	// currently editing the channel name
				if (!bKeyHeld && bKeyPressed)
				{
					gBeepToPlay = BEEP_1KHZ_60MS_OPTIONAL;
					if (edit_index < 10)
					{
						edit[edit_index] = ' ';
						if (++edit_index >= 10)
						{	// exit edit
							gFlagAcceptSetting  = false;
							gAskForConfirmation = 1;
						}
						gRequestDisplayScreen = DISPLAY_MENU;
					}
				}
				break;
			}

			GENERIC_Key_F(bKeyPressed, bKeyHeld);
			break;
		case KEY_PTT:
			GENERIC_Key_PTT(bKeyPressed);
			break;
		default:
			if (!bKeyHeld && bKeyPressed)
				gBeepToPlay = BEEP_500HZ_60MS_DOUBLE_BEEP_OPTIONAL;
			break;
	}

menu_timeout:
	if (gScreenToDisplay == DISPLAY_MENU)
	{
		if (!gMenuShowCategoryPicker && (
			UI_MENU_GetCurrentMenuId() == MENU_VOL ||
			#ifdef ENABLE_F_CAL_MENU
				UI_MENU_GetCurrentMenuId() == MENU_F_CALI ||
		    #endif
			UI_MENU_GetCurrentMenuId() == MENU_BATCAL))
		{
			gMenuCountdown = menu_timeout_long_500ms;
		}
		else
		{
			gMenuCountdown = menu_timeout_500ms;
		}
	}
}
