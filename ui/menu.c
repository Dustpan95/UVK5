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
#include "ui/display_text.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#include "../app/menu.h"
#include "../app/scanner.h"
#include "dtmf.h"
#include "../bitmaps.h"
#include "../board.h"
#include "../dcs.h"
#include "../driver/backlight.h"
#include "../driver/bk4819.h"
#include "../driver/eeprom.h"
#include "../driver/st7565.h"
#include "../external/printf/printf.h"
#include "../frequencies.h"
#include "../helper/battery.h"
#include "../misc.h"
#include "../settings.h"
#include "helper.h"
#include "inputbox.h"
#include "menu.h"
#include "ui.h"
#include "vuurwerk_menu.h"

/* Voice IDs occupy storage only in builds that use voice prompts. */
#ifdef ENABLE_VOICE
#define MENU_ITEM(name, voice, id) {name, voice, id}
#else
#define MENU_ITEM(name, voice, id) {name, id}
#endif
const t_menu_item MenuList[] =
{
//   text,     voice ID,                               menu ID
	MENU_ITEM("Step", VOICE_ID_FREQUENCY_STEP, MENU_STEP),
	MENU_ITEM("TxPwr", VOICE_ID_POWER, MENU_TXP), // was "TXP"
	MENU_ITEM("RxDCS", VOICE_ID_DCS, MENU_R_DCS), // was "R_DCS"
	MENU_ITEM("RxCTCS", VOICE_ID_CTCSS, MENU_R_CTCS), // was "R_CTCS"
	MENU_ITEM("TxDCS", VOICE_ID_DCS, MENU_T_DCS), // was "T_DCS"
	MENU_ITEM("TxCTCS", VOICE_ID_CTCSS, MENU_T_CTCS), // was "T_CTCS"
	MENU_ITEM("TxODir", VOICE_ID_TX_OFFSET_FREQUENCY_DIRECTION, MENU_SFT_D), // was "SFT_D"
	MENU_ITEM("TxOffs", VOICE_ID_TX_OFFSET_FREQUENCY, MENU_OFFSET), // was "OFFSET"
	MENU_ITEM("W/N", VOICE_ID_CHANNEL_BANDWIDTH, MENU_W_N),
	MENU_ITEM("Scramb", VOICE_ID_SCRAMBLER_ON, MENU_SCR), // was "SCR"
	MENU_ITEM("BusyCL", VOICE_ID_BUSY_LOCKOUT, MENU_BCL), // was "BCL"
	MENU_ITEM("Compnd", VOICE_ID_INVALID, MENU_COMPAND),
	MENU_ITEM("Demodu", VOICE_ID_INVALID, MENU_AM), // was "AM"
	MENU_ITEM("ScAdd1", VOICE_ID_INVALID, MENU_S_ADD1),
	MENU_ITEM("ScAdd2", VOICE_ID_INVALID, MENU_S_ADD2),
	MENU_ITEM("ChSave", VOICE_ID_MEMORY_CHANNEL, MENU_MEM_CH), // was "MEM-CH"
	MENU_ITEM("ChDele", VOICE_ID_DELETE_CHANNEL, MENU_DEL_CH), // was "DEL-CH"
	MENU_ITEM("ChName", VOICE_ID_INVALID, MENU_MEM_NAME),

	MENU_ITEM("SList", VOICE_ID_INVALID, MENU_S_LIST),
	MENU_ITEM("SList1", VOICE_ID_INVALID, MENU_SLIST1),
	MENU_ITEM("SList2", VOICE_ID_INVALID, MENU_SLIST2),
	MENU_ITEM("ScnRev", VOICE_ID_INVALID, MENU_SC_REV),
#ifdef ENABLE_NOAA
	MENU_ITEM("NOAA-S", VOICE_ID_INVALID, MENU_NOAA_S),
#endif
	MENU_ITEM("F1Shrt", VOICE_ID_INVALID, MENU_F1SHRT),
	MENU_ITEM("F1Long", VOICE_ID_INVALID, MENU_F1LONG),
	MENU_ITEM("F2Shrt", VOICE_ID_INVALID, MENU_F2SHRT),
	MENU_ITEM("F2Long", VOICE_ID_INVALID, MENU_F2LONG),
	MENU_ITEM("M Long", VOICE_ID_INVALID, MENU_MLONG),

	MENU_ITEM("KeyLck", VOICE_ID_INVALID, MENU_AUTOLK), // was "AUTOLk"
	MENU_ITEM("TxTOut", VOICE_ID_TRANSMIT_OVER_TIME, MENU_TOT), // was "TOT"
	MENU_ITEM("BatSav", VOICE_ID_SAVE_MODE, MENU_SAVE), // was "SAVE"
	MENU_ITEM("Mic", VOICE_ID_INVALID, MENU_MIC),
#ifdef ENABLE_AUDIO_BAR
	MENU_ITEM("MicBar", VOICE_ID_INVALID, MENU_MIC_BAR),
#endif
	MENU_ITEM("ChDisp", VOICE_ID_INVALID, MENU_MDF), // was "MDF"
	MENU_ITEM("POnMsg", VOICE_ID_INVALID, MENU_PONMSG),
	MENU_ITEM("BatTxt", VOICE_ID_INVALID, MENU_BAT_TXT),
	MENU_ITEM("BackLt", VOICE_ID_INVALID, MENU_ABR), // was "ABR"
	MENU_ITEM("BLMin", VOICE_ID_INVALID, MENU_ABR_MIN),
	MENU_ITEM("BLMax", VOICE_ID_INVALID, MENU_ABR_MAX),
	MENU_ITEM("BltTRX", VOICE_ID_INVALID, MENU_ABR_ON_TX_RX),
	MENU_ITEM("Beep", VOICE_ID_BEEP_PROMPT, MENU_BEEP),
#ifdef ENABLE_VOICE
	MENU_ITEM("Voice", VOICE_ID_VOICE_PROMPT, MENU_VOICE),
#endif
	MENU_ITEM("Roger", VOICE_ID_INVALID, MENU_ROGER),
	MENU_ITEM("STE", VOICE_ID_INVALID, MENU_STE),
	MENU_ITEM("RP STE", VOICE_ID_INVALID, MENU_RP_STE),
	MENU_ITEM("1 Call", VOICE_ID_INVALID, MENU_1_CALL),
#ifdef ENABLE_ALARM
	MENU_ITEM("AlarmT", VOICE_ID_INVALID, MENU_AL_MOD),
#endif
#ifdef ENABLE_DTMF_CALLING
	MENU_ITEM("ANI ID", VOICE_ID_ANI_CODE, MENU_ANI_ID),
#endif
	MENU_ITEM("UPCode", VOICE_ID_INVALID, MENU_UPCODE),
	MENU_ITEM("DWCode", VOICE_ID_INVALID, MENU_DWCODE),
	MENU_ITEM("PTT ID", VOICE_ID_INVALID, MENU_PTT_ID),
	MENU_ITEM("D ST", VOICE_ID_INVALID, MENU_D_ST),
#ifdef ENABLE_DTMF_CALLING
    MENU_ITEM("D Resp", VOICE_ID_INVALID, MENU_D_RSP),
	MENU_ITEM("D Hold", VOICE_ID_INVALID, MENU_D_HOLD),
#endif
	MENU_ITEM("D Prel", VOICE_ID_INVALID, MENU_D_PRE),
#ifdef ENABLE_DTMF_CALLING
	MENU_ITEM("D Decd", VOICE_ID_INVALID, MENU_D_DCD),
	MENU_ITEM("D List", VOICE_ID_INVALID, MENU_D_LIST),
#endif
	MENU_ITEM("D Live", VOICE_ID_INVALID, MENU_D_LIVE_DEC), // live DTMF decoder
#ifdef ENABLE_AM_FIX
	MENU_ITEM("AM Fix", VOICE_ID_INVALID, MENU_AM_FIX),
#endif
#ifdef ENABLE_VOX
	MENU_ITEM("VOX", VOICE_ID_VOX, MENU_VOX),
#endif
	MENU_ITEM("BatVol", VOICE_ID_INVALID, MENU_VOL), // was "VOL"
	MENU_ITEM("RxMode", VOICE_ID_DUAL_STANDBY, MENU_TDR),
	MENU_ITEM("Sql", VOICE_ID_SQUELCH, MENU_SQL),
	MENU_ITEM("ScnWch", VOICE_ID_INVALID, MENU_SCANWATCH),
    MENU_ITEM("About", VOICE_ID_INVALID, MENU_ABOUT), // VUURWERK About screen (CONFIG category)

	// hidden menu items from here on
	// enabled if pressing both the PTT and upper side button at power-on
	MENU_ITEM("F Lock", VOICE_ID_INVALID, MENU_F_LOCK),
	MENU_ITEM("Tx 200", VOICE_ID_INVALID, MENU_200TX), // was "200TX"
	MENU_ITEM("Tx 350", VOICE_ID_INVALID, MENU_350TX), // was "350TX"
	MENU_ITEM("Tx 500", VOICE_ID_INVALID, MENU_500TX), // was "500TX"
	MENU_ITEM("350 En", VOICE_ID_INVALID, MENU_350EN), // was "350EN"
	MENU_ITEM("ScraEn", VOICE_ID_INVALID, MENU_SCREN), // was "SCREN"
#ifdef ENABLE_F_CAL_MENU
	MENU_ITEM("FrCali", VOICE_ID_INVALID, MENU_F_CALI), // reference xtal calibration
#endif
	MENU_ITEM("BatCal", VOICE_ID_INVALID, MENU_BATCAL), // battery voltage calibration
	MENU_ITEM("BatTyp", VOICE_ID_INVALID, MENU_BATTYP), // battery type 1600/2200mAh
	MENU_ITEM("Reset", VOICE_ID_INITIALISATION, MENU_RESET), // might be better to move this to the hidden menu items ?

	MENU_ITEM("", VOICE_ID_INVALID, 0xff)  // end of list - DO NOT delete or move this this
};

#undef MENU_ITEM

const uint8_t FIRST_HIDDEN_MENU_ITEM = MENU_F_LOCK;

const char* const gSubMenu_RXMode[] =
{
	DISPLAY_TEXT("MAIN\nONLY"), 		// TX and RX on main only
	DISPLAY_TEXT("DUAL RX\nRESPOND"), // Watch both and respond
	DISPLAY_TEXT("CROSS\nBAND"), 		// TX on main, RX on secondary
	DISPLAY_TEXT("MAIN TX\nDUAL RX") 	// always TX on main, but RX on both
};

#ifdef ENABLE_VOICE
	const char gSubMenu_VOICE[][4] =
	{
		"OFF",
		"CHI",
		"ENG"
	};
#endif

const char* const gSubMenu_MDF[] =
{
	DISPLAY_TEXT("FREQ"),
	DISPLAY_TEXT("CHANNEL\nNUMBER"),
	DISPLAY_TEXT("NAME"),
	DISPLAY_TEXT("NAME\n+\nFREQ")
};

#ifdef ENABLE_ALARM
	const char gSubMenu_AL_MOD[][5] =
	{
		"SITE",
		"TONE"
	};
#endif

#ifdef ENABLE_DTMF_CALLING
const char gSubMenu_D_RSP[][11] =
{
	"DO\nNOTHING",
	"RING",
	"REPLY",
	"BOTH"
};
#endif

const char* const gSubMenu_PTT_ID[] =
{
	DISPLAY_TEXT("OFF"),
	DISPLAY_TEXT("UP CODE"),
	DISPLAY_TEXT("DOWN CODE"),
	DISPLAY_TEXT("UP+DOWN\nCODE"),
	DISPLAY_TEXT("APOLLO\nQUINDAR")
};

const char * const gSubMenu_F_LOCK[] =
{
	DISPLAY_TEXT("DEFAULT+\n137-174\n400-470"),
	DISPLAY_TEXT("FCC HAM\n144-148\n420-450"),
	DISPLAY_TEXT("CE HAM\n144-146\n430-440"),
	DISPLAY_TEXT("GB HAM\n144-148\n430-440"),
	DISPLAY_TEXT("137-174\n400-430"),
	DISPLAY_TEXT("137-174\n400-438"),
	DISPLAY_TEXT("DISABLE\nALL"),
	DISPLAY_TEXT("UNLOCK\nALL"),
};

/* Shared with the input bounds so disabled features cannot remain selectable. */
const char gSubMenu_SCANWATCH[][15] = {
    "OFF", "SCAN A\nWATCH B", "SCAN B\nWATCH A",
#ifdef ENABLE_VFO_SPLIT
    "SPLIT",
#endif
};
const uint8_t gSubMenu_SCANWATCH_size = ARRAY_SIZE(gSubMenu_SCANWATCH);

const t_sidefunction gSubMenu_SIDEFUNCTIONS[] =
{
	{DISPLAY_TEXT_OFFSET("NONE"),			ACTION_OPT_NONE},
#ifdef ENABLE_FLASHLIGHT
	{DISPLAY_TEXT_OFFSET("FLASH\nLIGHT"),	ACTION_OPT_FLASHLIGHT},
#endif
	{DISPLAY_TEXT_OFFSET("POWER"),			ACTION_OPT_POWER},
	{DISPLAY_TEXT_OFFSET("MONITOR"),			ACTION_OPT_MONITOR},
	{DISPLAY_TEXT_OFFSET("SCAN"),			ACTION_OPT_SCAN},
#ifdef ENABLE_VOX
	{DISPLAY_TEXT_OFFSET("VOX"),				ACTION_OPT_VOX},
#endif
#ifdef ENABLE_ALARM
	{DISPLAY_TEXT_OFFSET("ALARM"),			ACTION_OPT_ALARM},
#endif
#ifdef ENABLE_FMRADIO
	{DISPLAY_TEXT_OFFSET("FM RADIO"),		ACTION_OPT_FM},
#endif
#ifdef ENABLE_TX1750
	{DISPLAY_TEXT_OFFSET("1750HZ"),			ACTION_OPT_1750},
#endif
	{DISPLAY_TEXT_OFFSET("LOCK\nKEYPAD"),	ACTION_OPT_KEYLOCK},
	{DISPLAY_TEXT_OFFSET("SWITCH\nVFO"),		ACTION_OPT_A_B},
	{DISPLAY_TEXT_OFFSET("VFO/MR"),			ACTION_OPT_VFO_MR},
	{DISPLAY_TEXT_OFFSET("SWITCH\nDEMODUL"),	ACTION_OPT_SWITCH_DEMODUL},
#ifdef ENABLE_BLMIN_TMP_OFF
	{DISPLAY_TEXT_OFFSET("BLMIN\nTMP OFF"),  ACTION_OPT_BLMIN_TMP_OFF}, 		//BackLight Minimum Temporay OFF
#endif
#ifdef ENABLE_SPECTRUM
	{DISPLAY_TEXT_OFFSET("SPECTRUM"),         ACTION_OPT_SPECTRUM}
#endif
};

const uint8_t gSubMenu_SIDEFUNCTIONS_size = ARRAY_SIZE(gSubMenu_SIDEFUNCTIONS);

bool    gIsInSubMenu;
uint8_t gMenuCursor;
int UI_MENU_GetCurrentMenuId() {
	if(gMenuCursor < ARRAY_SIZE(MenuList))
		return MenuList[gMenuCursor].menu_id;

	return MenuList[ARRAY_SIZE(MenuList)-1].menu_id;
}

uint8_t UI_MENU_GetMenuIdx(uint8_t id)
{
	for(uint8_t i = 0; i < ARRAY_SIZE(MenuList); i++)
		if(MenuList[i].menu_id == id)
			return i;
	return 0;
}

int32_t gSubMenuSelection;

// edit box
char    edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char    edit[17];
int     edit_index;


/* Packed resources keep their original byte rows and selection indices. */
static const struct { uint8_t id, width; uint16_t offset; } fixed_choices[] = {
    { MENU_TXP, sizeof(gSubMenu_TXP[0]), offsetof(DisplayResources, menu_TXP) },
    { MENU_SFT_D, sizeof(gSubMenu_SFT_D[0]), offsetof(DisplayResources, menu_SFT_D) },
    { MENU_W_N, sizeof(gSubMenu_W_N[0]), offsetof(DisplayResources, menu_W_N) },
    { MENU_COMPAND, sizeof(gSubMenu_RX_TX[0]), offsetof(DisplayResources, menu_RX_TX) },
    { MENU_ABR_ON_TX_RX, sizeof(gSubMenu_RX_TX[0]), offsetof(DisplayResources, menu_RX_TX) },
    { MENU_SAVE, sizeof(gSubMenu_SAVE[0]), offsetof(DisplayResources, menu_SAVE) },
    { MENU_TOT, sizeof(gSubMenu_TOT[0]), offsetof(DisplayResources, menu_TOT) },
    { MENU_SC_REV, sizeof(gSubMenu_SC_REV[0]), offsetof(DisplayResources, menu_SC_REV) },
    { MENU_BAT_TXT, sizeof(gSubMenu_BAT_TXT[0]), offsetof(DisplayResources, menu_BAT_TXT) },
    { MENU_PONMSG, sizeof(gSubMenu_PONMSG[0]), offsetof(DisplayResources, menu_PONMSG) },
    { MENU_ROGER, sizeof(gSubMenu_ROGER[0]), offsetof(DisplayResources, menu_ROGER) },
    { MENU_RESET, sizeof(gSubMenu_RESET[0]), offsetof(DisplayResources, menu_RESET) },
    { MENU_BATTYP, sizeof(gSubMenu_BATTYP[0]), offsetof(DisplayResources, menu_BATTYP) },
};

static void __attribute__((noinline)) MenuValue(const char *text,uint8_t row)
{
    UI_PrintString(text,50,LCD_WIDTH-1,row,8);
}
void UI_DisplayMenu(void)
{
	const unsigned int menu_list_width = 6; // max no. of characters on the menu list (left side)
	const unsigned int menu_item_x1    = (8 * menu_list_width) + 2;
	const unsigned int menu_item_x2    = LCD_WIDTH - 1;
	unsigned int       i;
	char               String[64];  // bigger cuz we can now do multi-line in one string (use '\n' char)

#ifdef ENABLE_DTMF_CALLING
	char               Contact[16];
#endif

	// --- Category picker screen ---
	if (gMenuShowCategoryPicker) {
		UI_DisplayClear();

		const uint8_t visCount = VUURWERK_MENU_GetVisibleCategoryCount();

		// scroll window
		uint8_t top = 0;
		if (visCount > FRAME_LINES) {
			if (gCategoryPickerCursor >= FRAME_LINES - 1)
				top = gCategoryPickerCursor - (FRAME_LINES - 2);
			if (top > visCount - FRAME_LINES)
				top = visCount - FRAME_LINES;
		}

		for (uint8_t row = 0; row < FRAME_LINES; row++) {
			const uint8_t c = top + row;
			if (c >= visCount)
				break;

			sprintf(String, DISPLAY_TEXT("%d %s"), c + 1, VUURWERK_MENU_GetCategoryName(c));
			UI_PrintStringSmallNormal(String, 2, 0, row);

			if (c == gCategoryPickerCursor) {
				for (uint8_t x = 0; x < LCD_WIDTH; x++)
					gFrameBuffer[row][x] ^= 0xFF;
			}
		}

		ST7565_BlitFullScreen();
		return;
	}

	/* Foreground-owned inputs are stable throughout one rendered frame. */
	const uint8_t menu_id = UI_MENU_GetCurrentMenuId();
	const int32_t selection = gSubMenuSelection;

	// --- Category items mode ---
	UI_DisplayClear();

	// Draw three filtered items (prev, current, next)
	for (i = 0; i < 3; i++)
	{
		int8_t  offset = (int8_t)i - 1;  // -1, 0, +1
		int16_t idx    = (int16_t)gCategoryItemIdx + offset;

		// Wrap within category
		if (idx < 0)                          idx += gCategoryItemCount;
		if (idx >= (int16_t)gCategoryItemCount) idx -= gCategoryItemCount;

		// Skip duplicates for small categories
		if (gCategoryItemCount <= 1 && i != 1)
			continue;
		if (gCategoryItemCount == 2 && i == 2)
			continue;

		const uint8_t menuIdx = gCategoryItems[idx];
		UI_PrintString(MenuList[menuIdx].name, 0, 0, i * 2, 8);
	}

	// invert the current menu list item pixels
	for (i = 0; i < (8 * menu_list_width); i++)
	{
		gFrameBuffer[2][i] ^= 0xFF;
		gFrameBuffer[3][i] ^= 0xFF;
	}

	// draw vertical separating dotted line
	for (i = 0; i < 7; i++)
		gFrameBuffer[i][(8 * menu_list_width) + 1] = 0xAA;

	// draw the little sub-menu triangle marker
	if (gIsInSubMenu)
		memcpy(gFrameBuffer[0] + (8 * menu_list_width) + 1, BITMAP_CurrentIndicator, sizeof(BITMAP_CurrentIndicator));

	// draw category name and position
	// VUURWERK v1.3.1 (U3): while browsing (not in a submenu), items with a
	// plain-language hint show it here in the 3x5 mini font instead of the
	// category/position chrome ("RxMode" -> "DUAL WATCH / CROSSBAND RX").
	// Items without a hint keep the original line. Display-only; names,
	// ordering and values are untouched.
	{
		const char *hint = !gIsInSubMenu
			? VUURWERK_MENU_GetHint(menu_id) : NULL;
		if (hint) {
			UI_DisplaySmallest(hint, 0, 50, false, true);
		} else {
			if (gSelectedCategory >= 0)
				sprintf(String, DISPLAY_TEXT("%s %u/%u"), VUURWERK_MENU_GetCategoryName(gSelectedCategory), gCategoryItemIdx + 1, gCategoryItemCount);
			else
				sprintf(String, DISPLAY_TEXT("%2u.%u"), 1 + gMenuCursor, gMenuListCount);
			UI_PrintStringSmallNormal(String, 2, 0, 6);
		}
	}

	// **************

	memset(String, 0, sizeof(String));

	bool already_printed = false;

	/* Brightness is set to max in some entries of this menu. Return it to the configured brightness
	   level the "next" time we enter here.I.e., when we move from one menu to another.
	   It also has to be set back to max when pressing the Exit key. */

	BACKLIGHT_TurnOn();

    bool fixed_choice = false;
    for (unsigned row=0; row<ARRAY_SIZE(fixed_choices); ++row) {
        if (fixed_choices[row].id == menu_id) {
            const char *choices = (const char *)&gDisplayResources + fixed_choices[row].offset;
            strcpy(String, choices + selection * fixed_choices[row].width);
            fixed_choice = true;
            break;
        }
    }
    if (!fixed_choice) switch (menu_id)
	{
		case MENU_SQL:
			sprintf(String, DISPLAY_TEXT("%d"), selection);
			break;

		case MENU_MIC:
			{	// display the mic gain in actual dB rather than just an index number
				const uint8_t mic = gMicGain_dB2[selection];
				sprintf(String, DISPLAY_TEXT("+%u.%01udB"), mic / 2, mic % 2);
			}
			break;

		#ifdef ENABLE_AUDIO_BAR
			case MENU_MIC_BAR:
				strcpy(String, gSubMenu_OFF_ON[selection]);
				break;
		#endif

		case MENU_STEP: {
			uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(selection)];
			sprintf(String, DISPLAY_TEXT("%d.%02ukHz"), step / 100, step % 100);
			break;
		}

		case MENU_R_DCS:
		case MENU_T_DCS:
			if (selection == 0)
				strcpy(String, DISPLAY_TEXT("OFF"));
			else if (selection < 105)
				sprintf(String, DISPLAY_TEXT("D%03oN"), DCS_Options[selection -   1]);
			else
				sprintf(String, DISPLAY_TEXT("D%03oI"), DCS_Options[selection - 105]);
			break;

		case MENU_R_CTCS:
		case MENU_T_CTCS:
		{
			if (selection == 0)
				strcpy(String, DISPLAY_TEXT("OFF"));
			else
				sprintf(String, DISPLAY_TEXT("%u.%uHz"), CTCSS_Options[selection - 1] / 10, CTCSS_Options[selection - 1] % 10);
			break;
		}

		case MENU_OFFSET:
			if (!gIsInSubMenu || gInputBoxIndex == 0)
			{
				sprintf(String, DISPLAY_TEXT("%3d.%05u"), selection / 100000, abs(selection) % 100000);
				MenuValue(String,1);
			}
			else
			{
				const char * ascii = INPUTBOX_GetAscii();
				sprintf(String, DISPLAY_TEXT("%.3s.%.3s  "),ascii, ascii + 3);
				MenuValue(String,1);
			}

			MenuValue(DISPLAY_TEXT("MHz"),3);

			already_printed = true;
			break;

		case MENU_SCR:
			strcpy(String, gSubMenu_SCRAMBLER[selection]);
			#if 1
				if (selection > 0 && gSetting_ScrambleEnable)
					BK4819_EnableScramble(selection - 1);
				else
					BK4819_DisableScramble();
			#endif
			break;

		#ifdef ENABLE_VOX
			case MENU_VOX:
				if (selection == 0)
					strcpy(String, DISPLAY_TEXT("OFF"));
				else
					sprintf(String, DISPLAY_TEXT("%d"), selection);
				break;
		#endif

		case MENU_ABR:
			strcpy(String, gSubMenu_BACKLIGHT[selection]);
			if(BACKLIGHT_GetBrightness() < 4)
				BACKLIGHT_SetBrightness(4);
			break;

		case MENU_ABR_MIN:
		case MENU_ABR_MAX:
			sprintf(String, DISPLAY_TEXT("%d"), selection);
			if(gIsInSubMenu)
				BACKLIGHT_SetBrightness(selection);
			else if(BACKLIGHT_GetBrightness() < 4)
				BACKLIGHT_SetBrightness(4);
			break;

		case MENU_AM:
			strcpy(String, gModulationStr[selection]);
			break;

		case MENU_AUTOLK:
			strcpy(String, (selection == 0) ? DISPLAY_TEXT("OFF") : DISPLAY_TEXT("AUTO"));
			break;

		#ifdef ENABLE_AM_FIX
			case MENU_AM_FIX:
		#endif
		case MENU_BCL:
		case MENU_BEEP:
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
			strcpy(String, gSubMenu_OFF_ON[selection]);
			break;

        case MENU_MEM_CH:
        case MENU_1_CALL:
        case MENU_DEL_CH:
        case MENU_MEM_NAME:
        {
            const bool naming=menu_id==MENU_MEM_NAME;
            const bool valid=RADIO_CheckValidChannel(selection,false,0);
            const bool editing=naming && gIsInSubMenu && edit_index>=0;
            UI_GenerateChannelStringEx(String,valid,selection);
            MenuValue(String,0);
            uint32_t frequency=0;
            if(valid && (naming || !gAskForConfirmation))
                frequency=SETTINGS_FetchChannelFrequency(selection);
            if(!naming || valid) {
                if(editing) {
                    UI_PrintString(edit,menu_item_x1,0,2,8);
                    if(edit_index<10) UI_PrintString(DISPLAY_TEXT("^"),menu_item_x1+8*edit_index,0,4,8);
                } else {
                    SETTINGS_FetchChannelName(String,selection);
                    MenuValue(String[0] ? String:DISPLAY_TEXT("--"),2);
                }
            }
            if(valid && !gAskForConfirmation) {
                UI_FormatFrequency(String,frequency,false);
                MenuValue(String,4+editing);
            }
            already_printed=true;
            break;
        }

		case MENU_TDR:
			strcpy(String, gSubMenu_RXMode[selection]);
			break;

		#ifdef ENABLE_VOICE
			case MENU_VOICE:
				strcpy(String, gSubMenu_VOICE[selection]);
				break;
		#endif

		case MENU_MDF:
			strcpy(String, gSubMenu_MDF[selection]);
			break;

		case MENU_RP_STE:
			if (selection == 0)
				strcpy(String, DISPLAY_TEXT("OFF"));
			else
				sprintf(String, DISPLAY_TEXT("%d*100ms"), selection);
			break;

		case MENU_S_LIST:
			if (selection < 2)
				sprintf(String, DISPLAY_TEXT("LIST%u"), 1 + selection);
			else
				strcpy(String, DISPLAY_TEXT("ALL"));
			break;

		#ifdef ENABLE_ALARM
			case MENU_AL_MOD:
				sprintf(String, gSubMenu_AL_MOD[selection]);
				break;
		#endif

#ifdef ENABLE_DTMF_CALLING
		case MENU_ANI_ID:
			strcpy(String, gEeprom.ANI_DTMF_ID);
			break;
#endif
		case MENU_UPCODE:
			sprintf(String, DISPLAY_TEXT("%.8s\n%.8s"), gEeprom.DTMF_UP_CODE, gEeprom.DTMF_UP_CODE + 8);
			break;

		case MENU_DWCODE:
			sprintf(String, DISPLAY_TEXT("%.8s\n%.8s"), gEeprom.DTMF_DOWN_CODE, gEeprom.DTMF_DOWN_CODE + 8);
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_RSP:
			strcpy(String, gSubMenu_D_RSP[selection]);
			break;

		case MENU_D_HOLD:
			sprintf(String, DISPLAY_TEXT("%ds"), selection);
			break;
#endif
		case MENU_D_PRE:
			sprintf(String, DISPLAY_TEXT("%d*10ms"), selection);
			break;

		case MENU_PTT_ID:
			strcpy(String, gSubMenu_PTT_ID[selection]);
			break;

#ifdef ENABLE_DTMF_CALLING
		case MENU_D_LIST:
			gIsDtmfContactValid = DTMF_GetContact((int)selection - 1, Contact);
			if (!gIsDtmfContactValid)
				strcpy(String, DISPLAY_TEXT("NULL"));
			else
				memcpy(String, Contact, 8);
			break;
#endif

		case MENU_VOL:
			sprintf(String, DISPLAY_TEXT("%u.%02uV\n%u%%"),
				gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100,
				BATTERY_VoltsToPercent(gBatteryVoltageAverage));
			break;

		case MENU_F_LOCK:
			if(!gIsInSubMenu && gUnlockAllTxConfCnt>0 && gUnlockAllTxConfCnt<10)
				strcpy(String, DISPLAY_TEXT("READ\nMANUAL"));
			else
				strcpy(String, gSubMenu_F_LOCK[selection]);
			break;

		#ifdef ENABLE_F_CAL_MENU
			case MENU_F_CALI:
				{
					const uint32_t value   = 22656 + selection;
					const uint32_t xtal_Hz = (0x4f0000u + value) * 5;

					writeXtalFreqCal(selection, false);

					sprintf(String, DISPLAY_TEXT("%d\n%u.%06u\nMHz"),
						selection,
						xtal_Hz / 1000000, xtal_Hz % 1000000);
				}
				break;
		#endif

		case MENU_BATCAL:
		{
			const uint16_t vol = (uint32_t)gBatteryVoltageAverage * gBatteryCalibration[3] / selection;
			sprintf(String, DISPLAY_TEXT("%u.%02uV\n%u"), vol / 100, vol % 100, selection);
			break;
		}

		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
			strcpy(String, gDisplayText+gSubMenu_SIDEFUNCTIONS[selection].name_offset);
			break;


        case MENU_SCANWATCH:
		{
			strcpy(String, gSubMenu_SCANWATCH[(unsigned)selection < gSubMenu_SCANWATCH_size ? selection : 0]);
			break;
		}

	}

	if (!already_printed)
	{	// we now do multi-line text in a single string

		unsigned int y;
		unsigned int lines = 1;
		unsigned int len   = strlen(String);
		bool         small = false;

		if (len > 0)
		{
			// count number of lines
			for (i = 0; i < len; i++)
			{
				if (String[i] == '\n' && i < (len - 1))
				{	// found new line char
					lines++;
					String[i] = 0;  // null terminate the line
				}
			}

			if (lines > 3)
			{	// use small text
				small = true;
				if (lines > 7)
					lines = 7;
			}

			// center vertically'ish
			if (small)
				y = 3 - ((lines + 0) / 2);  // untested
			else
				y = 2 - ((lines + 0) / 2);

			// draw the text lines
			for (i = 0; i < len && lines > 0; lines--)
			{
				if (small)
					UI_PrintStringSmallNormal(String + i, menu_item_x1, menu_item_x2, y);
				else
					MenuValue(String + i,y);

				// look for start of next line
				while (i < len && String[i] >= 32)
					i++;

				// hop over the null term char(s)
				while (i < len && String[i] < 32)
					i++;

				y += small ? 1 : 2;
			}
		}
	}

	if (menu_id == MENU_SLIST1 || menu_id == MENU_SLIST2)
	{
		i = (menu_id == MENU_SLIST1) ? 0 : 1;
		char *pPrintStr = String;

		if (selection < 0) {
			pPrintStr = DISPLAY_TEXT("NULL");
		} else {
			UI_GenerateChannelStringEx(String, true, selection);
			pPrintStr = String;
		}

		// channel number
		MenuValue(pPrintStr,0);

		SETTINGS_FetchChannelName(String, selection);
		pPrintStr = String[0] ? String : DISPLAY_TEXT("--");

		// channel name and scan-list
		if (selection < 0 || !gEeprom.SCAN_LIST_ENABLED[i]) {
			MenuValue(pPrintStr,2);
		} else {
			UI_PrintStringSmallNormal(pPrintStr, menu_item_x1, menu_item_x2, 2);

			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH1[i])) {
				sprintf(String, DISPLAY_TEXT("PRI%d:%u"), 1, gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
				MenuValue(String,3);
			}

			if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH2[i])) {
				sprintf(String, DISPLAY_TEXT("PRI%d:%u"), 2, gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
				MenuValue(String,5);
			}
		}
	}

#ifdef ENABLE_TONE_SEARCH
	if ((menu_id == MENU_R_CTCS || menu_id == MENU_R_DCS) && gCssBackgroundScan) {
		// === VUURWERK v1.2.5 background CSS scan dot animation ===
		// Mirrors ui/scanner.c:69-72 idiom; gScanProgressIndicator is
		// ticked every 500 ms by SCANNER_TimeSlice500ms whether or not
		// ENABLE_NO_CODE_SCAN_TIMEOUT is set, so the cadence is free.
		const uint8_t n = (gScanProgressIndicator & 7) + 1;
		strcpy(String, DISPLAY_TEXT("SCAN"));
		memset(String + 4, '.', n);
		String[4 + n] = '\0';
		MenuValue(String,4);
		// === END VUURWERK ===
	}

#endif

#ifdef ENABLE_DTMF_CALLING
	if (menu_id == MENU_D_LIST && gIsDtmfContactValid) {
		Contact[11] = 0;
		memcpy(&gDTMF_ID, Contact + 8, 4);
		sprintf(String, DISPLAY_TEXT("ID:%4s"), gDTMF_ID);
		MenuValue(String,4);
	}
#endif

	if (menu_id == MENU_R_CTCS ||
	    menu_id == MENU_T_CTCS ||
	    menu_id == MENU_R_DCS  ||
	    menu_id == MENU_T_DCS
#ifdef ENABLE_DTMF_CALLING
	    || menu_id == MENU_D_LIST
#endif
	) {
		sprintf(String, DISPLAY_TEXT("%2d"), selection);
		UI_PrintStringSmallNormal(String, 105, 0, 0);
	}

	if ((menu_id == MENU_RESET    ||
	     menu_id == MENU_MEM_CH   ||
	     menu_id == MENU_MEM_NAME ||
	     menu_id == MENU_DEL_CH) && gAskForConfirmation)
	{	// display confirmation
		char *pPrintStr = (gAskForConfirmation == 1) ? DISPLAY_TEXT("SURE?") : DISPLAY_TEXT("WAIT!");
		MenuValue(pPrintStr,5);
	}

	ST7565_BlitFullScreen();
}
