/* Modified for UVK5: APRS receive / display integration; see docs/INTEGRATION.md. */
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


/* Original hint text: input to tools/pack-menu-hints.py. */
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
