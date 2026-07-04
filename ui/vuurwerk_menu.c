/* Copyright (c) 2026 James Honiball (KC3TFZ)
 *
 * This file is part of VUURWERK and is dual-licensed:
 *   1. GPL v3 (when distributed as part of the VUURWERK firmware)
 *   2. Commercial license available from the author
 *
 * You may not extract, repackage, or redistribute this file
 * independently under any license other than GPL v3 as part
 * of the complete VUURWERK firmware, without written permission
 * from the author.
 *
 * Commercial licensing inquiries: jhoniball4@gmail.com
 */

#include <stddef.h>   // v1.3.1 (U3): NULL for the hint lookup
#include "vuurwerk_menu.h"
#include "menu.h"
#include "../misc.h"

// State variables
bool     gMenuShowCategoryPicker = true;
int8_t   gSelectedCategory       = -1;
uint8_t  gCategoryPickerCursor   = 0;

uint8_t  gCategoryItems[MAX_CATEGORY_ITEMS];
uint8_t  gCategoryItemCount = 0;
uint8_t  gCategoryItemIdx   = 0;

// Per-category last-visited item index. Survives picker round-trips
// within a power cycle so re-entering a category lands the cursor
// where the operator left it. Cleared on power-on (BSS zero-init).
static uint8_t sCategoryLastIdx[MCAT_COUNT];

// Category display names, packed into a single buffer with embedded
// null terminators. The 7-element offset table replaces a 28-byte
// pointer table; net data shrink is ~21 bytes plus a small function
// body delta. Order matches MenuCategory_t enum.
static const char sCategoryNames[] =
	"RECEIVE\0TONE\0TX\0SCAN\0CHANNEL\0CONFIG\0UNLOCK";
static const uint8_t sCategoryNameOffsets[MCAT_COUNT] = {
	0, 8, 13, 16, 21, 29, 36
};

const char* VUURWERK_MENU_GetCategoryName(uint8_t cat)
{
	if (cat < MCAT_COUNT)
		return sCategoryNames + sCategoryNameOffsets[cat];
	return "?";
}

// v1.3.1 (U3): plain-language hint line for cryptic menu items, rendered in
// the 3x5 mini font on menu row 6 while browsing (UI_DisplayMenu). Frozen
// menu names stay frozen; this is additive display only. Same packed-pool
// pattern as sCategoryNames above: one NUL-separated blob, walked in sHintIds
// order by VUURWERK_MENU_GetHint (no offsets -- edit texts freely, keep order).
// Width budget: 31 chars max at 4 px/char in gFont3x5 (128 px line).
// v1.4.4: longest hints tightened to fund the F+EXIT reset-to-simplex rescue.
static const char sHintPool[] =
	"REPEATER OFFSET DIR +/-\0"         // MENU_SFT_D   (v1.4.4 trims: NUL-walked,
	"REPEATER OFFSET FREQUENCY\0"       // MENU_OFFSET   order-matched to sHintIds --
	"FM BANDWIDTH WIDE/NARROW\0"        // MENU_W_N      offsets not used)
	"NO TX WHILE BUSY\0"                // MENU_BCL
	"AUDIO COMPANDER (NOISE CUT)\0"     // MENU_COMPAND
	"DEMODULATOR FM/AM/USB\0"           // MENU_AM
	"ADD CHANNEL TO SCAN LIST 1\0"      // MENU_S_ADD1
	"ADD CHANNEL TO SCAN LIST 2\0"      // MENU_S_ADD2
	"SCAN LIST TO SCAN: 1/2/ALL\0"      // MENU_S_LIST
	"VIEW SCAN LIST 1 CHANNELS\0"       // MENU_SLIST1
	"VIEW SCAN LIST 2 CHANNELS\0"       // MENU_SLIST2
	"SCAN RESUME MODE\0"                // MENU_SC_REV
	"SIDE1 SHORT PRESS\0"               // MENU_F1SHRT
	"SIDE1 LONG PRESS\0"                // MENU_F1LONG
	"SIDE2 SHORT PRESS\0"               // MENU_F2SHRT
	"SIDE2 LONG PRESS\0"                // MENU_F2LONG
	"MENU LONG PRESS\0"                 // MENU_MLONG
	"AUTO KEYPAD LOCK\0"                // 467  MENU_AUTOLK
	"TX TIMEOUT LIMIT\0"                // 484  MENU_TOT
	"BATTERY SAVE DUTY CYCLE\0"         // 501  MENU_SAVE
	"CH DISPLAY FREQ/NAME/NR\0"         // MENU_MDF
	"POWER-ON SCREEN MESSAGE\0"         // 555  MENU_PONMSG
	"BATTERY READOUT: VOLTS/PCT\0"      // 579  MENU_BAT_TXT
	"BACKLIGHT AUTO-OFF TIME\0"         // 606  MENU_ABR
	"BACKLIGHT MIN BRIGHTNESS\0"        // 630  MENU_ABR_MIN
	"BACKLIGHT MAX BRIGHTNESS\0"        // 655  MENU_ABR_MAX
	"BACKLIGHT ON DURING TX/RX\0"       // 680  MENU_ABR_ON_TX_RX
	"MUTE SQUELCH TAIL ON RX END\0"     // 706  MENU_STE
	"REPEATER SQUELCH TAIL DELAY\0"     // 734  MENU_RP_STE
	"ONE-KEY PRIORITY CHANNEL\0"        // 762  MENU_1_CALL
	"DTMF TX-START CODE\0"              // 787  MENU_UPCODE
	"DTMF TX-END CODE\0"                // 806  MENU_DWCODE
	"SEND DTMF ID ON PTT\0"             // 823  MENU_PTT_ID
	"DTMF SIDETONE IN SPEAKER\0"        // 843  MENU_D_ST
	"DTMF PRE-TONE DELAY\0"             // 868  MENU_D_PRE
	"LIVE DTMF DECODER ON SCREEN\0"     // 888  MENU_D_LIVE_DEC
	"AM OVERLOAD AUTO GAIN FIX\0"       // 916  MENU_AM_FIX
	"BATTERY VOLTAGE READOUT\0"         // 942  MENU_VOL
	"DUAL WATCH / CROSSBAND RX\0"       // 966  MENU_TDR
	"WATCH OTHER VFO IN SCAN";          // MENU_SCANWATCH

static const uint8_t sHintIds[] = {
	MENU_SFT_D,   MENU_OFFSET,  MENU_W_N,     MENU_BCL,     MENU_COMPAND,
	MENU_AM,      MENU_S_ADD1,  MENU_S_ADD2,  MENU_S_LIST,  MENU_SLIST1,
	MENU_SLIST2,  MENU_SC_REV,  MENU_F1SHRT,  MENU_F1LONG,  MENU_F2SHRT,
	MENU_F2LONG,  MENU_MLONG,   MENU_AUTOLK,  MENU_TOT,     MENU_SAVE,
	MENU_MDF,     MENU_PONMSG,  MENU_BAT_TXT, MENU_ABR,     MENU_ABR_MIN,
	MENU_ABR_MAX, MENU_ABR_ON_TX_RX, MENU_STE, MENU_RP_STE, MENU_1_CALL,
	MENU_UPCODE,  MENU_DWCODE,  MENU_PTT_ID,  MENU_D_ST,    MENU_D_PRE,
	MENU_D_LIVE_DEC, MENU_AM_FIX, MENU_VOL,   MENU_TDR,     MENU_SCANWATCH,
};

// VUURWERK v1.3.3 reclamation: the 80-byte sHintOffsets table was removable
// -- the hints are NUL-separated in sHintPool in the same order as sHintIds,
// so walk the pool as we scan the ids (one hop per string). Also kills the
// "regenerate every offset by hand when editing a hint" maintenance hazard.
const char* VUURWERK_MENU_GetHint(uint8_t menu_id)
{
	const char *p = sHintPool;
	for (uint8_t i = 0; i < ARRAY_SIZE(sHintIds); i++) {
		if (sHintIds[i] == menu_id)
			return p;
		while (*p++) ;   // skip to and past this hint's NUL
	}
	return NULL;
}

uint8_t VUURWERK_MENU_GetVisibleCategoryCount(void)
{
	if (gF_LOCK)
		return MCAT_COUNT;     // 7 categories including UNLOCK
	return MCAT_COUNT - 1;     // 6 categories excluding UNLOCK
}

uint8_t VUURWERK_MENU_GetCategoryForMenuId(uint8_t menu_id)
{
	switch (menu_id)
	{
		// RECEIVE
		case MENU_SQL:
		case MENU_STEP:
		case MENU_W_N:
		case MENU_AM:
		case MENU_COMPAND:
#ifdef ENABLE_AM_FIX
		case MENU_AM_FIX:
#endif
		case MENU_TDR:
			return MCAT_RECEIVE;

		// TONE
		case MENU_R_CTCS:
		case MENU_T_CTCS:
		case MENU_R_DCS:
		case MENU_T_DCS:
		case MENU_STE:
		case MENU_RP_STE:
		case MENU_SCR:
			return MCAT_TONE;

		// TRANSMIT (includes former DTMF items)
		case MENU_TXP:
		case MENU_SFT_D:
		case MENU_OFFSET:
		case MENU_TOT:
		case MENU_MIC:
		case MENU_BCL:
		case MENU_ROGER:
		case MENU_UPCODE:       // was DTMF category
		case MENU_DWCODE:       // was DTMF category
		case MENU_PTT_ID:       // was DTMF category
		case MENU_D_ST:         // was DTMF category
		case MENU_D_PRE:        // was DTMF category
		case MENU_D_LIVE_DEC:   // was DTMF category
			return MCAT_TRANSMIT;

		// SCAN
		case MENU_S_LIST:
		case MENU_SLIST1:
		case MENU_SLIST2:
		case MENU_S_ADD1:
		case MENU_S_ADD2:
		case MENU_SC_REV:
		case MENU_SCANWATCH:    // Scan+Watch + VFO Split
			return MCAT_SCAN;

		// CHANNEL
		case MENU_MEM_CH:
		case MENU_DEL_CH:
		case MENU_MEM_NAME:
		case MENU_MDF:
		case MENU_1_CALL:
			return MCAT_CHANNEL;

		// CONFIG (merged DISPLAY + SYSTEM)
		case MENU_ABR:
		case MENU_ABR_MIN:
		case MENU_ABR_MAX:
		case MENU_ABR_ON_TX_RX:
		case MENU_BAT_TXT:
		case MENU_PONMSG:
		case MENU_BEEP:
		case MENU_AUTOLK:
		case MENU_F1SHRT:
		case MENU_F1LONG:
		case MENU_F2SHRT:
		case MENU_F2LONG:
		case MENU_MLONG:
		case MENU_VOL:          // was SYSTEM
		case MENU_SAVE:         // was SYSTEM
		case MENU_ABOUT:        // VUURWERK About screen
			return MCAT_CONFIG;

		// UNLOCK (hidden items)
		case MENU_F_LOCK:
		case MENU_200TX:
		case MENU_350TX:
		case MENU_500TX:
		case MENU_350EN:
		case MENU_SCREN:
		case MENU_BATCAL:
		case MENU_BATTYP:
		case MENU_RESET:
			return MCAT_UNLOCK;

		default:
			return MCAT_CONFIG;  // safe fallback
	}
}

void VUURWERK_MENU_SelectCategory(uint8_t cat)
{
	gCategoryItemCount = 0;
	gCategoryItemIdx   = 0;

	// Walk all visible MenuList items and pick those matching the category
	for (uint8_t i = 0; i < gMenuListCount; i++)
	{
		if (VUURWERK_MENU_GetCategoryForMenuId(MenuList[i].menu_id) == cat)
		{
			if (gCategoryItemCount < MAX_CATEGORY_ITEMS)
				gCategoryItems[gCategoryItemCount++] = i;
		}
	}

	// Restore the cursor to where the operator left this category.
	// CSS-scan re-entry (app/menu.c:107) overrides this afterwards by
	// scanning gCategoryItems[] for the matched code, which is correct.
	if (cat < MCAT_COUNT)
	{
		uint8_t saved = sCategoryLastIdx[cat];
		if (saved < gCategoryItemCount)
			gCategoryItemIdx = saved;
	}

	VUURWERK_MENU_SyncCursor();
}

void VUURWERK_MENU_SyncCursor(void)
{
	if (gCategoryItemCount > 0)
		gMenuCursor = gCategoryItems[gCategoryItemIdx];
}

void VUURWERK_MENU_EnterCategoryPicker(void)
{
	// Remember the cursor position inside the category we're leaving
	// so a later re-entry of the same category restores it.
	if (gSelectedCategory >= 0 && gSelectedCategory < (int8_t)MCAT_COUNT
		&& gCategoryItemCount > 0)
	{
		sCategoryLastIdx[gSelectedCategory] = gCategoryItemIdx;
	}

	gMenuShowCategoryPicker = true;

	// Park the picker cursor on the just-left category. UNLOCK is
	// hidden when gF_LOCK is false, so clamp against visible count.
	if (gSelectedCategory >= 0
		&& gSelectedCategory < (int8_t)VUURWERK_MENU_GetVisibleCategoryCount())
	{
		gCategoryPickerCursor = (uint8_t)gSelectedCategory;
	}
	else
	{
		gCategoryPickerCursor = 0;
	}

	gSelectedCategory = -1;
}

void VUURWERK_MENU_Init(void)
{
	gMenuShowCategoryPicker = true;
	gSelectedCategory       = -1;
	gCategoryPickerCursor   = 0;
	gCategoryItemCount      = 0;
	gCategoryItemIdx        = 0;
}
