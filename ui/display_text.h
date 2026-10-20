/* SPDX-License-Identifier: GPL-3.0-only
 * Generated losslessly from project UI text by tools/pack-display-text.py.
 * Original copyright/license notices remain in the source UI files and CREDITS.
 */
#ifndef UVK5_DISPLAY_TEXT_H
#define UVK5_DISPLAY_TEXT_H
#include <stdint.h>
enum { DISPLAY_TEXT_BYTES=2862, DISPLAY_HINT_BYTES=921, DISPLAY_RESOURCE_BYTES=4210 };
typedef struct {
    char text[DISPLAY_TEXT_BYTES];
    char menu_TXP[3][5];
    char menu_SFT_D[3][4];
    char menu_W_N[2][7];
    char menu_OFF_ON[2][4];
    char menu_SAVE[5][4];
    char menu_TOT[11][7];
    char menu_SC_REV[3][8];
    char menu_PONMSG[4][8];
    char menu_ROGER[3][6];
    char menu_RESET[2][4];
    char menu_BACKLIGHT[8][7];
    char menu_RX_TX[4][6];
    char menu_BAT_TXT[3][8];
    char menu_BATTYP[2][9];
    char menu_SCRAMBLER[11][7];
    char hints[DISPLAY_HINT_BYTES];
} DisplayResources;
extern DisplayResources gDisplayResources;
#define gDisplayText (gDisplayResources.text)
void DISPLAY_TEXT_Init(void);
#define gSubMenu_TXP (*(const char (*)[3][5])gDisplayResources.menu_TXP)
#define gSubMenu_SFT_D (*(const char (*)[3][4])gDisplayResources.menu_SFT_D)
#define gSubMenu_W_N (*(const char (*)[2][7])gDisplayResources.menu_W_N)
#define gSubMenu_OFF_ON (*(const char (*)[2][4])gDisplayResources.menu_OFF_ON)
#define gSubMenu_SAVE (*(const char (*)[5][4])gDisplayResources.menu_SAVE)
#define gSubMenu_TOT (*(const char (*)[11][7])gDisplayResources.menu_TOT)
#define gSubMenu_SC_REV (*(const char (*)[3][8])gDisplayResources.menu_SC_REV)
#define gSubMenu_PONMSG (*(const char (*)[4][8])gDisplayResources.menu_PONMSG)
#define gSubMenu_ROGER (*(const char (*)[3][6])gDisplayResources.menu_ROGER)
#define gSubMenu_RESET (*(const char (*)[2][4])gDisplayResources.menu_RESET)
#define gSubMenu_BACKLIGHT (*(const char (*)[8][7])gDisplayResources.menu_BACKLIGHT)
#define gSubMenu_RX_TX (*(const char (*)[4][6])gDisplayResources.menu_RX_TX)
#define gSubMenu_BAT_TXT (*(const char (*)[3][8])gDisplayResources.menu_BAT_TXT)
#define gSubMenu_BATTYP (*(const char (*)[2][9])gDisplayResources.menu_BATTYP)
#define gSubMenu_SCRAMBLER (*(const char (*)[11][7])gDisplayResources.menu_SCRAMBLER)
#define DISPLAY_TEXT_OFFSET(s) ( \
    __builtin_strcmp((s),"")==0 ? 29 : \
    __builtin_strcmp((s)," ABC")==0 ? 2755 : \
    __builtin_strcmp((s)," ABC_")==0 ? 2355 : \
    __builtin_strcmp((s),"%.3s.%.3s  ")==0 ? 1788 : \
    __builtin_strcmp((s),"%.8s")==0 ? 1805 : \
    __builtin_strcmp((s),"%.8s\n%.8s")==0 ? 1800 : \
    __builtin_strcmp((s),"%02u:%02u")==0 ? 1810 : \
    __builtin_strcmp((s),"%2d")==0 ? 2760 : \
    __builtin_strcmp((s),"%2u.%u")==0 ? 2361 : \
    __builtin_strcmp((s),"%3d.%05u")==0 ? 2368 : \
    __builtin_strcmp((s),"%3u.%05u")==0 ? 2377 : \
    __builtin_strcmp((s),"%c  CH%03u")==0 ? 1820 : \
    __builtin_strcmp((s),"%c%.16s")==0 ? 2386 : \
    __builtin_strcmp((s),"%c%c%s")==0 ? 2394 : \
    __builtin_strcmp((s),"%c%s")==0 ? 2396 : \
    __builtin_strcmp((s),"%cACK WAIT %us")==0 ? 894 : \
    __builtin_strcmp((s),"%cDATA TX %s")==0 ? 1831 : \
    __builtin_strcmp((s),"%cFREQ ")==0 ? 2401 : \
    __builtin_strcmp((s),"%cFREQ NOT SET")==0 ? 909 : \
    __builtin_strcmp((s),"%cTX AUDIO %u")==0 ? 924 : \
    __builtin_strcmp((s),"%cTX AUDIO NOT SET")==0 ? 571 : \
    __builtin_strcmp((s),"%d")==0 ? 2764 : \
    __builtin_strcmp((s),"%d\n%u.%06u\nMHz")==0 ? 938 : \
    __builtin_strcmp((s),"%d %s")==0 ? 2409 : \
    __builtin_strcmp((s),"%d%2d %2d %2d %3d")==0 ? 590 : \
    __builtin_strcmp((s),"%d*100ms")==0 ? 2415 : \
    __builtin_strcmp((s),"%d*10ms")==0 ? 2424 : \
    __builtin_strcmp((s),"%d.%02ukHz")==0 ? 1844 : \
    __builtin_strcmp((s),"%ddBm")==0 ? 2432 : \
    __builtin_strcmp((s),"%ds")==0 ? 2767 : \
    __builtin_strcmp((s),"%s %c")==0 ? 2438 : \
    __builtin_strcmp((s),"%s %s")==0 ? 2444 : \
    __builtin_strcmp((s),"%s %u/%u")==0 ? 1064 : \
    __builtin_strcmp((s),"%s>%s")==0 ? 2450 : \
    __builtin_strcmp((s),"%u%%")==0 ? 962 : \
    __builtin_strcmp((s),"%u.%02uV\n%u")==0 ? 1855 : \
    __builtin_strcmp((s),"%u.%02uV\n%u%%")==0 ? 953 : \
    __builtin_strcmp((s),"%u.%02uV %u%%")==0 ? 967 : \
    __builtin_strcmp((s),"%u.%05u")==0 ? 2456 : \
    __builtin_strcmp((s),"%u.%uHz")==0 ? 2464 : \
    __builtin_strcmp((s),"%u/67%s")==0 ? 2472 : \
    __builtin_strcmp((s),"%ud")==0 ? 2771 : \
    __builtin_strcmp((s),"%uh")==0 ? 2775 : \
    __builtin_strcmp((s),"*")==0 ? 2779 : \
    __builtin_strcmp((s),"*:DELETE")==0 ? 2480 : \
    __builtin_strcmp((s),"+%u.%01udB")==0 ? 1867 : \
    __builtin_strcmp((s),"--")==0 ? 2781 : \
    __builtin_strcmp((s),"1")==0 ? 2673 : \
    __builtin_strcmp((s),"137-174\n400-430")==0 ? 981 : \
    __builtin_strcmp((s),"137-174\n400-438")==0 ? 997 : \
    __builtin_strcmp((s),"1750HZ")==0 ? 2489 : \
    __builtin_strcmp((s),"1:.-/  #:NEXT")==0 ? 1013 : \
    __builtin_strcmp((s),"2")==0 ? 2679 : \
    __builtin_strcmp((s),"?")==0 ? 2694 : \
    __builtin_strcmp((s),"ACK RECEIVED")==0 ? 1878 : \
    __builtin_strcmp((s),"ACK WAIT")==0 ? 2496 : \
    __builtin_strcmp((s),"ACK WAIT SECONDS")==0 ? 1027 : \
    __builtin_strcmp((s),"ALARM")==0 ? 2505 : \
    __builtin_strcmp((s),"ALL")==0 ? 1981 : \
    __builtin_strcmp((s),"ALL KEYS")==0 ? 2511 : \
    __builtin_strcmp((s),"ALL TRAFFIC")==0 ? 1891 : \
    __builtin_strcmp((s),"APOLLO\nQUINDAR")==0 ? 1044 : \
    __builtin_strcmp((s),"APRS")==0 ? 104 : \
    __builtin_strcmp((s),"APRS %s %u/%u")==0 ? 1059 : \
    __builtin_strcmp((s),"APRS ACTIVE")==0 ? 1903 : \
    __builtin_strcmp((s),"APRS DATA TX")==0 ? 1915 : \
    __builtin_strcmp((s),"APRS FREQ MHz")==0 ? 1073 : \
    __builtin_strcmp((s),"APRS FREQUENCY")==0 ? 1087 : \
    __builtin_strcmp((s),"APRS RX")==0 ? 2520 : \
    __builtin_strcmp((s),"APRS SETUP SESSION")==0 ? 608 : \
    __builtin_strcmp((s),"ATTEMPTS %u/%u")==0 ? 1102 : \
    __builtin_strcmp((s),"AUTO")==0 ? 2784 : \
    __builtin_strcmp((s),"Adaptive Squelch")==0 ? 1117 : \
    __builtin_strcmp((s),"B SCAN CH%03u")==0 ? 1134 : \
    __builtin_strcmp((s),"BAT LOW")==0 ? 2528 : \
    __builtin_strcmp((s),"BEACON SENT")==0 ? 1928 : \
    __builtin_strcmp((s),"BK4819")==0 ? 2536 : \
    __builtin_strcmp((s),"BLMIN\nTMP OFF")==0 ? 1148 : \
    __builtin_strcmp((s),"BUSY")==0 ? 172 : \
    __builtin_strcmp((s),"CALLSIGN FULL")==0 ? 1162 : \
    __builtin_strcmp((s),"CANCELLED")==0 ? 1940 : \
    __builtin_strcmp((s),"CE HAM\n144-146\n430-440")==0 ? 109 : \
    __builtin_strcmp((s),"CH-%.3s")==0 ? 2543 : \
    __builtin_strcmp((s),"CHANNEL\nNUMBER")==0 ? 1176 : \
    __builtin_strcmp((s),"CHANNEL ENTRY")==0 ? 1191 : \
    __builtin_strcmp((s),"CHECK CALL/SSID")==0 ? 1205 : \
    __builtin_strcmp((s),"CHECK INPUT / TX SETUP")==0 ? 132 : \
    __builtin_strcmp((s),"CHECK RADIO")==0 ? 1950 : \
    __builtin_strcmp((s),"CHECK TX SETUP / BUSY")==0 ? 155 : \
    __builtin_strcmp((s),"CROSS\nBAND")==0 ? 1962 : \
    __builtin_strcmp((s),"Cs")==0 ? 2789 : \
    __builtin_strcmp((s),"D%03oI")==0 ? 2551 : \
    __builtin_strcmp((s),"D%03oN")==0 ? 2558 : \
    __builtin_strcmp((s),"DATA TX")==0 ? 1920 : \
    __builtin_strcmp((s),"DEFAULT+\n137-174\n400-470")==0 ? 177 : \
    __builtin_strcmp((s),"DISABLE\nALL")==0 ? 1973 : \
    __builtin_strcmp((s),"DOWN")==0 ? 2792 : \
    __builtin_strcmp((s),"DOWN CODE")==0 ? 1985 : \
    __builtin_strcmp((s),"DTMF DECODE")==0 ? 1995 : \
    __builtin_strcmp((s),"DTMF ENTRY")==0 ? 2007 : \
    __builtin_strcmp((s),"DUAL RX\nRESPOND")==0 ? 1221 : \
    __builtin_strcmp((s),"EDIT QUICK MESSAGE")==0 ? 627 : \
    __builtin_strcmp((s),"EDIT QUICK MSGS")==0 ? 1237 : \
    __builtin_strcmp((s),"EDIT RECIPIENT")==0 ? 1253 : \
    __builtin_strcmp((s),"EEPROM")==0 ? 2565 : \
    __builtin_strcmp((s),"EMPTY MESSAGE")==0 ? 1268 : \
    __builtin_strcmp((s),"EXIT")==0 ? 2797 : \
    __builtin_strcmp((s),"EXIT:DELETE  HOLD EXIT:CANCEL")==0 ? 0 : \
    __builtin_strcmp((s),"EXIT:INBOX")==0 ? 374 : \
    __builtin_strcmp((s),"EXIT:PACKETS")==0 ? 2018 : \
    __builtin_strcmp((s),"EXIT:STATIONS")==0 ? 1282 : \
    __builtin_strcmp((s),"EXPIRED")==0 ? 2572 : \
    __builtin_strcmp((s),"F")==0 ? 1160 : \
    __builtin_strcmp((s),"FAULT")==0 ? 2580 : \
    __builtin_strcmp((s),"FCC HAM\n144-148\n420-450")==0 ? 202 : \
    __builtin_strcmp((s),"FLASH\nLIGHT")==0 ? 2031 : \
    __builtin_strcmp((s),"FM RADIO")==0 ? 2586 : \
    __builtin_strcmp((s),"FM, scan/DW off")==0 ? 1296 : \
    __builtin_strcmp((s),"FREQ")==0 ? 2133 : \
    __builtin_strcmp((s),"FREQUENCY ENTRY")==0 ? 1312 : \
    __builtin_strcmp((s),"GB HAM\n144-148\n430-440")==0 ? 226 : \
    __builtin_strcmp((s),"HEARD %us AGO")==0 ? 1328 : \
    __builtin_strcmp((s),"HOLD")==0 ? 2802 : \
    __builtin_strcmp((s),"ID:%4s")==0 ? 2595 : \
    __builtin_strcmp((s),"IDLE")==0 ? 2807 : \
    __builtin_strcmp((s),"LATITUDE DD MM.mm")==0 ? 646 : \
    __builtin_strcmp((s),"LIST%u")==0 ? 2602 : \
    __builtin_strcmp((s),"LK")==0 ? 2812 : \
    __builtin_strcmp((s),"LOCK\nKEYPAD")==0 ? 2043 : \
    __builtin_strcmp((s),"LON DDD MM.mm")==0 ? 1342 : \
    __builtin_strcmp((s),"LOW BATTERY")==0 ? 2055 : \
    __builtin_strcmp((s),"Long press #")==0 ? 2067 : \
    __builtin_strcmp((s),"M")==0 ? 2509 : \
    __builtin_strcmp((s),"MAIN\nONLY")==0 ? 2080 : \
    __builtin_strcmp((s),"MAIN TX\nDUAL RX")==0 ? 1356 : \
    __builtin_strcmp((s),"MENU")==0 ? 2815 : \
    __builtin_strcmp((s),"MENU   F+5:APRS   EXIT:A/B")==0 ? 30 : \
    __builtin_strcmp((s),"MENU/EXIT:STOP  UP/DN:DIR")==0 ? 57 : \
    __builtin_strcmp((s),"MENU:CANCEL EXIT:BACK")==0 ? 249 : \
    __builtin_strcmp((s),"MENU:DETAIL  EXIT:APRS")==0 ? 271 : \
    __builtin_strcmp((s),"MENU:NEXT  EXIT:BACK")==0 ? 664 : \
    __builtin_strcmp((s),"MENU:NEXT EXIT:CANCEL")==0 ? 294 : \
    __builtin_strcmp((s),"MENU:OPEN  EXIT:RADIO")==0 ? 316 : \
    __builtin_strcmp((s),"MENU:OPTIONS  EXIT:BACK")==0 ? 338 : \
    __builtin_strcmp((s),"MENU:READ  EXIT:APRS")==0 ? 685 : \
    __builtin_strcmp((s),"MENU:REPLY  EXIT:INBOX")==0 ? 362 : \
    __builtin_strcmp((s),"MENU:REPLY/NEW  EXIT:APRS")==0 ? 83 : \
    __builtin_strcmp((s),"MENU:SAVE  EXIT:CANCEL")==0 ? 385 : \
    __builtin_strcmp((s),"MENU:SAVE EXIT:CANCEL")==0 ? 408 : \
    __builtin_strcmp((s),"MENU:SELECT  EXIT:BODY")==0 ? 430 : \
    __builtin_strcmp((s),"MENU:SELECT EXIT:BACK")==0 ? 453 : \
    __builtin_strcmp((s),"MENU:SEND  EXIT:EDIT")==0 ? 706 : \
    __builtin_strcmp((s),"MENU:SEND EXIT:CANCEL")==0 ? 475 : \
    __builtin_strcmp((s),"MENU:SET CALL  EXIT:APRS")==0 ? 497 : \
    __builtin_strcmp((s),"MESSAGE FULL")==0 ? 2090 : \
    __builtin_strcmp((s),"MESSAGE OPTIONS")==0 ? 1372 : \
    __builtin_strcmp((s),"MESSAGE REJECTED")==0 ? 1388 : \
    __builtin_strcmp((s),"MHz")==0 ? 949 : \
    __builtin_strcmp((s),"MONITOR")==0 ? 2609 : \
    __builtin_strcmp((s),"MONITOR %u/%u")==0 ? 1405 : \
    __builtin_strcmp((s),"MSG %s")==0 ? 2617 : \
    __builtin_strcmp((s),"MULTI-TAP ABC/123")==0 ? 727 : \
    __builtin_strcmp((s),"MY CALL %s")==0 ? 2103 : \
    __builtin_strcmp((s),"MY CALLSIGN")==0 ? 2114 : \
    __builtin_strcmp((s),"N")==0 ? 625 : \
    __builtin_strcmp((s),"NAME")==0 ? 2820 : \
    __builtin_strcmp((s),"NAME\n+\nFREQ")==0 ? 2126 : \
    __builtin_strcmp((s),"NEW MESSAGE")==0 ? 2138 : \
    __builtin_strcmp((s),"NONE")==0 ? 2825 : \
    __builtin_strcmp((s),"NOT SENT")==0 ? 2624 : \
    __builtin_strcmp((s),"NULL")==0 ? 2830 : \
    __builtin_strcmp((s),"No messages yet")==0 ? 1419 : \
    __builtin_strcmp((s),"No packets yet")==0 ? 1435 : \
    __builtin_strcmp((s),"No position yet")==0 ? 1450 : \
    __builtin_strcmp((s),"No stations yet")==0 ? 1466 : \
    __builtin_strcmp((s),"OFF")==0 ? 1158 : \
    __builtin_strcmp((s),"ON")==0 ? 624 : \
    __builtin_strcmp((s),"PACKETS")==0 ? 2023 : \
    __builtin_strcmp((s),"PAUSE")==0 ? 2633 : \
    __builtin_strcmp((s),"PERSONAL INBOX")==0 ? 1482 : \
    __builtin_strcmp((s),"PERSONAL INBOX %u")==0 ? 745 : \
    __builtin_strcmp((s),"PERSONAL MESSAGE")==0 ? 1497 : \
    __builtin_strcmp((s),"POSITION NOT SET")==0 ? 1514 : \
    __builtin_strcmp((s),"POWER")==0 ? 2639 : \
    __builtin_strcmp((s),"PRI%d:%u")==0 ? 2645 : \
    __builtin_strcmp((s),"PTT")==0 ? 2835 : \
    __builtin_strcmp((s),"PTT:RETURN TO VOICE")==0 ? 763 : \
    __builtin_strcmp((s),"Precision RF")==0 ? 2150 : \
    __builtin_strcmp((s),"QUEUED")==0 ? 2654 : \
    __builtin_strcmp((s),"QUICK MESSAGE")==0 ? 632 : \
    __builtin_strcmp((s),"QUICK MESSAGES")==0 ? 1531 : \
    __builtin_strcmp((s),"RADIO")==0 ? 332 : \
    __builtin_strcmp((s),"RADIO ALERT")==0 ? 2163 : \
    __builtin_strcmp((s),"READ\nMANUAL")==0 ? 2175 : \
    __builtin_strcmp((s),"RECIPIENT")==0 ? 1258 : \
    __builtin_strcmp((s),"RELEASE")==0 ? 2661 : \
    __builtin_strcmp((s),"REVIEW AND SEND")==0 ? 1546 : \
    __builtin_strcmp((s),"REVIEW BEACON")==0 ? 1562 : \
    __builtin_strcmp((s),"REVIEW POSITION")==0 ? 1576 : \
    __builtin_strcmp((s),"RX")==0 ? 1369 : \
    __builtin_strcmp((s),"RX/TX disabled")==0 ? 1592 : \
    __builtin_strcmp((s),"S")==0 ? 107 : \
    __builtin_strcmp((s),"S%u %ddBm Q%u %c")==0 ? 1607 : \
    __builtin_strcmp((s),"S+W")==0 ? 2839 : \
    __builtin_strcmp((s),"SAVE DOES NOT SEND")==0 ? 783 : \
    __builtin_strcmp((s),"SCAN")==0 ? 2843 : \
    __builtin_strcmp((s),"SCAN %s %uc/s")==0 ? 1624 : \
    __builtin_strcmp((s),"SEND")==0 ? 797 : \
    __builtin_strcmp((s),"SEND BEACON")==0 ? 2187 : \
    __builtin_strcmp((s),"SESSION ONLY")==0 ? 2199 : \
    __builtin_strcmp((s),"SET YOUR CALLSIGN")==0 ? 802 : \
    __builtin_strcmp((s),"SETUP")==0 ? 149 : \
    __builtin_strcmp((s),"SHIFT %c%u.%03u %s")==0 ? 820 : \
    __builtin_strcmp((s),"SIDE1")==0 ? 2669 : \
    __builtin_strcmp((s),"SIDE2")==0 ? 2675 : \
    __builtin_strcmp((s),"SP")==0 ? 2848 : \
    __builtin_strcmp((s),"SPECTRUM")==0 ? 2681 : \
    __builtin_strcmp((s),"SQL%u %s %u.%ukHz")==0 ? 839 : \
    __builtin_strcmp((s),"STAR")==0 ? 2851 : \
    __builtin_strcmp((s),"STATIC BEACON")==0 ? 1638 : \
    __builtin_strcmp((s),"STATIC POSITION")==0 ? 1652 : \
    __builtin_strcmp((s),"STATIONS")==0 ? 1287 : \
    __builtin_strcmp((s),"STATIONS %u HEARD")==0 ? 857 : \
    __builtin_strcmp((s),"SURE?")==0 ? 2690 : \
    __builtin_strcmp((s),"SWITCH\nDEMODUL")==0 ? 1668 : \
    __builtin_strcmp((s),"SWITCH\nVFO")==0 ? 2212 : \
    __builtin_strcmp((s),"Signal Quality")==0 ? 1683 : \
    __builtin_strcmp((s),"TIMEOUT")==0 ? 2696 : \
    __builtin_strcmp((s),"TO")==0 ? 2786 : \
    __builtin_strcmp((s),"TO %s")==0 ? 2704 : \
    __builtin_strcmp((s),"TX")==0 ? 1925 : \
    __builtin_strcmp((s),"TX %u:%02u")==0 ? 2223 : \
    __builtin_strcmp((s),"TX AUDIO 0-127")==0 ? 1698 : \
    __builtin_strcmp((s),"TX AUDIO LEVEL")==0 ? 1713 : \
    __builtin_strcmp((s),"TX BLOCKED")==0 ? 2234 : \
    __builtin_strcmp((s),"TX Compressor")==0 ? 1728 : \
    __builtin_strcmp((s),"TX DISABLE")==0 ? 2245 : \
    __builtin_strcmp((s),"TX FAILED")==0 ? 2256 : \
    __builtin_strcmp((s),"TX UNAVAILABLE")==0 ? 1742 : \
    __builtin_strcmp((s),"TX sag %umV")==0 ? 2266 : \
    __builtin_strcmp((s),"UNCONFIRMED")==0 ? 2278 : \
    __builtin_strcmp((s),"UNLOCK\nALL")==0 ? 2290 : \
    __builtin_strcmp((s),"UP")==0 ? 152 : \
    __builtin_strcmp((s),"UP CODE")==0 ? 2710 : \
    __builtin_strcmp((s),"UP+DOWN\nCODE")==0 ? 2301 : \
    __builtin_strcmp((s),"UP/DN:PACKET  EXIT:APRS")==0 ? 522 : \
    __builtin_strcmp((s),"UP/DN:PACKET  EXIT:RADIO")==0 ? 546 : \
    __builtin_strcmp((s),"UP/DOWN:HEMISPHERE")==0 ? 875 : \
    __builtin_strcmp((s),"VFO %c")==0 ? 2718 : \
    __builtin_strcmp((s),"VFO B ACTIVITY")==0 ? 1757 : \
    __builtin_strcmp((s),"VFO ERROR")==0 ? 2314 : \
    __builtin_strcmp((s),"VFO/MR")==0 ? 2725 : \
    __builtin_strcmp((s),"VOLT HIGH")==0 ? 2324 : \
    __builtin_strcmp((s),"VOLTAGE")==0 ? 2732 : \
    __builtin_strcmp((s),"VOX")==0 ? 2856 : \
    __builtin_strcmp((s),"VUURWERK")==0 ? 2740 : \
    __builtin_strcmp((s),"W")==0 ? 2534 : \
    __builtin_strcmp((s),"WAIT!")==0 ? 2749 : \
    __builtin_strcmp((s),"WAITING FOR ACK")==0 ? 1772 : \
    __builtin_strcmp((s),"^")==0 ? 2860 : \
    __builtin_strcmp((s),"calib lost")==0 ? 2334 : \
    __builtin_strcmp((s),"to unlock")==0 ? 2345 : \
    -1)
#define DISPLAY_TEXT(s) (gDisplayText + DISPLAY_TEXT_OFFSET(s) + \
    0*sizeof(char[DISPLAY_TEXT_OFFSET(s)>=0 ? 1 : -1]))
#endif
