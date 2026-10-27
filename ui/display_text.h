/* SPDX-License-Identifier: GPL-3.0-only
 * Generated losslessly from project UI text by tools/pack-display-text.py.
 * Original copyright/license notices remain in the source UI files and CREDITS.
 */
#ifndef UVK5_DISPLAY_TEXT_H
#define UVK5_DISPLAY_TEXT_H
#include <stdint.h>
enum { DISPLAY_TEXT_BYTES=2330, DISPLAY_HINT_BYTES=921, DISPLAY_RESOURCE_BYTES=3678 };
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
    __builtin_strcmp((s)," ABC")==0 ? 2226 : \
    __builtin_strcmp((s)," ABC_")==0 ? 1836 : \
    __builtin_strcmp((s),"%.3s.%.3s  ")==0 ? 1352 : \
    __builtin_strcmp((s),"%.8s")==0 ? 1369 : \
    __builtin_strcmp((s),"%.8s\n%.8s")==0 ? 1364 : \
    __builtin_strcmp((s),"%2d")==0 ? 2231 : \
    __builtin_strcmp((s),"%2u.%u")==0 ? 1842 : \
    __builtin_strcmp((s),"%3d.%05u")==0 ? 1849 : \
    __builtin_strcmp((s),"%3u.%05u")==0 ? 1858 : \
    __builtin_strcmp((s),"%c  CH%03u")==0 ? 1374 : \
    __builtin_strcmp((s),"%c%.16s")==0 ? 1867 : \
    __builtin_strcmp((s),"%c%s")==0 ? 2235 : \
    __builtin_strcmp((s),"%cDATA TX %s")==0 ? 1385 : \
    __builtin_strcmp((s),"%cFREQ ")==0 ? 1875 : \
    __builtin_strcmp((s),"%cFREQ NOT SET")==0 ? 673 : \
    __builtin_strcmp((s),"%cTX AUDIO %u")==0 ? 688 : \
    __builtin_strcmp((s),"%cTX AUDIO NOT SET")==0 ? 425 : \
    __builtin_strcmp((s),"%d")==0 ? 2240 : \
    __builtin_strcmp((s),"%d\n%u.%06u\nMHz")==0 ? 702 : \
    __builtin_strcmp((s),"%d %s")==0 ? 1883 : \
    __builtin_strcmp((s),"%d%2d %2d %2d %3d")==0 ? 444 : \
    __builtin_strcmp((s),"%d*100ms")==0 ? 1889 : \
    __builtin_strcmp((s),"%d*10ms")==0 ? 1898 : \
    __builtin_strcmp((s),"%d.%02ukHz")==0 ? 1398 : \
    __builtin_strcmp((s),"%ddBm")==0 ? 1906 : \
    __builtin_strcmp((s),"%ds")==0 ? 2243 : \
    __builtin_strcmp((s),"%s %c")==0 ? 1912 : \
    __builtin_strcmp((s),"%s %s")==0 ? 1918 : \
    __builtin_strcmp((s),"%s %u/%u")==0 ? 1924 : \
    __builtin_strcmp((s),"%u%%")==0 ? 726 : \
    __builtin_strcmp((s),"%u.%02uV\n%u")==0 ? 1409 : \
    __builtin_strcmp((s),"%u.%02uV\n%u%%")==0 ? 717 : \
    __builtin_strcmp((s),"%u.%02uV %u%%")==0 ? 731 : \
    __builtin_strcmp((s),"%u.%05u")==0 ? 1933 : \
    __builtin_strcmp((s),"%u.%uHz")==0 ? 1941 : \
    __builtin_strcmp((s),"%u/67%s")==0 ? 1949 : \
    __builtin_strcmp((s),"*")==0 ? 2247 : \
    __builtin_strcmp((s),"*:DELETE")==0 ? 1957 : \
    __builtin_strcmp((s),"+%u.%01udB")==0 ? 1421 : \
    __builtin_strcmp((s),"--")==0 ? 2249 : \
    __builtin_strcmp((s),"1")==0 ? 2136 : \
    __builtin_strcmp((s),"137-174\n400-430")==0 ? 745 : \
    __builtin_strcmp((s),"137-174\n400-438")==0 ? 761 : \
    __builtin_strcmp((s),"1750HZ")==0 ? 1966 : \
    __builtin_strcmp((s),"1:.-/  #:NEXT")==0 ? 777 : \
    __builtin_strcmp((s),"2")==0 ? 2142 : \
    __builtin_strcmp((s),"?")==0 ? 2157 : \
    __builtin_strcmp((s),"ALARM")==0 ? 1973 : \
    __builtin_strcmp((s),"ALL")==0 ? 1498 : \
    __builtin_strcmp((s),"ALL KEYS")==0 ? 1979 : \
    __builtin_strcmp((s),"APOLLO\nQUINDAR")==0 ? 791 : \
    __builtin_strcmp((s),"APRS ACTIVE")==0 ? 1432 : \
    __builtin_strcmp((s),"APRS DATA TX")==0 ? 1444 : \
    __builtin_strcmp((s),"APRS FREQ MHz")==0 ? 806 : \
    __builtin_strcmp((s),"APRS FREQUENCY")==0 ? 820 : \
    __builtin_strcmp((s),"APRS RX")==0 ? 1988 : \
    __builtin_strcmp((s),"APRS SETUP SESSION")==0 ? 462 : \
    __builtin_strcmp((s),"APRS TX")==0 ? 1996 : \
    __builtin_strcmp((s),"ATTEMPTS %u/%u")==0 ? 835 : \
    __builtin_strcmp((s),"AUTO")==0 ? 2252 : \
    __builtin_strcmp((s),"Adaptive Squelch")==0 ? 850 : \
    __builtin_strcmp((s),"B SCAN CH%03u")==0 ? 867 : \
    __builtin_strcmp((s),"BAT LOW")==0 ? 2004 : \
    __builtin_strcmp((s),"BK4819")==0 ? 2012 : \
    __builtin_strcmp((s),"BLMIN\nTMP OFF")==0 ? 881 : \
    __builtin_strcmp((s),"BUSY")==0 ? 146 : \
    __builtin_strcmp((s),"CALLSIGN FULL")==0 ? 895 : \
    __builtin_strcmp((s),"CANCELLED")==0 ? 1457 : \
    __builtin_strcmp((s),"CE HAM\n144-146\n430-440")==0 ? 83 : \
    __builtin_strcmp((s),"CH-%.3s")==0 ? 2019 : \
    __builtin_strcmp((s),"CHANNEL\nNUMBER")==0 ? 909 : \
    __builtin_strcmp((s),"CHANNEL ENTRY")==0 ? 924 : \
    __builtin_strcmp((s),"CHECK CALL/SSID")==0 ? 938 : \
    __builtin_strcmp((s),"CHECK INPUT / TX SETUP")==0 ? 106 : \
    __builtin_strcmp((s),"CHECK RADIO")==0 ? 1467 : \
    __builtin_strcmp((s),"CHECK TX SETUP / BUSY")==0 ? 129 : \
    __builtin_strcmp((s),"CROSS\nBAND")==0 ? 1479 : \
    __builtin_strcmp((s),"Cs")==0 ? 2257 : \
    __builtin_strcmp((s),"D%03oI")==0 ? 2027 : \
    __builtin_strcmp((s),"D%03oN")==0 ? 2034 : \
    __builtin_strcmp((s),"DATA TX")==0 ? 1449 : \
    __builtin_strcmp((s),"DEFAULT+\n137-174\n400-470")==0 ? 151 : \
    __builtin_strcmp((s),"DELIVERY UNKNOWN")==0 ? 954 : \
    __builtin_strcmp((s),"DISABLE\nALL")==0 ? 1490 : \
    __builtin_strcmp((s),"DOWN")==0 ? 2260 : \
    __builtin_strcmp((s),"DOWN CODE")==0 ? 1502 : \
    __builtin_strcmp((s),"DTMF DECODE")==0 ? 1512 : \
    __builtin_strcmp((s),"DTMF ENTRY")==0 ? 1524 : \
    __builtin_strcmp((s),"DUAL RX\nRESPOND")==0 ? 971 : \
    __builtin_strcmp((s),"EDIT QUICK MESSAGE")==0 ? 481 : \
    __builtin_strcmp((s),"EDIT QUICK MSGS")==0 ? 987 : \
    __builtin_strcmp((s),"EDIT RECIPIENT")==0 ? 1003 : \
    __builtin_strcmp((s),"EEPROM")==0 ? 2041 : \
    __builtin_strcmp((s),"EMPTY MESSAGE")==0 ? 1018 : \
    __builtin_strcmp((s),"EXIT")==0 ? 2265 : \
    __builtin_strcmp((s),"EXIT:BACK")==0 ? 235 : \
    __builtin_strcmp((s),"EXIT:DELETE  HOLD EXIT:CANCEL")==0 ? 0 : \
    __builtin_strcmp((s),"EXPIRED")==0 ? 2048 : \
    __builtin_strcmp((s),"F")==0 ? 893 : \
    __builtin_strcmp((s),"FAULT")==0 ? 2056 : \
    __builtin_strcmp((s),"FCC HAM\n144-148\n420-450")==0 ? 176 : \
    __builtin_strcmp((s),"FLASH\nLIGHT")==0 ? 1535 : \
    __builtin_strcmp((s),"FM RADIO")==0 ? 2062 : \
    __builtin_strcmp((s),"FREQ")==0 ? 1626 : \
    __builtin_strcmp((s),"FREQUENCY ENTRY")==0 ? 1032 : \
    __builtin_strcmp((s),"GB HAM\n144-148\n430-440")==0 ? 200 : \
    __builtin_strcmp((s),"HOLD")==0 ? 2270 : \
    __builtin_strcmp((s),"ID:%4s")==0 ? 2071 : \
    __builtin_strcmp((s),"IDLE")==0 ? 2275 : \
    __builtin_strcmp((s),"LATITUDE DD MM.mm")==0 ? 500 : \
    __builtin_strcmp((s),"LIST%u")==0 ? 2078 : \
    __builtin_strcmp((s),"LK")==0 ? 2280 : \
    __builtin_strcmp((s),"LOCK\nKEYPAD")==0 ? 1547 : \
    __builtin_strcmp((s),"LON DDD MM.mm")==0 ? 1048 : \
    __builtin_strcmp((s),"LOW BATTERY")==0 ? 1559 : \
    __builtin_strcmp((s),"Long press #")==0 ? 1571 : \
    __builtin_strcmp((s),"MAIN\nONLY")==0 ? 1584 : \
    __builtin_strcmp((s),"MAIN TX\nDUAL RX")==0 ? 1062 : \
    __builtin_strcmp((s),"MENU")==0 ? 2283 : \
    __builtin_strcmp((s),"MENU   F+5:APRS   EXIT:A/B")==0 ? 30 : \
    __builtin_strcmp((s),"MENU/EXIT:STOP  UP/DN:DIR")==0 ? 57 : \
    __builtin_strcmp((s),"MENU:CANCEL EXIT:BACK")==0 ? 223 : \
    __builtin_strcmp((s),"MENU:NEXT  EXIT:BACK")==0 ? 518 : \
    __builtin_strcmp((s),"MENU:NEXT EXIT:CANCEL")==0 ? 245 : \
    __builtin_strcmp((s),"MENU:OPEN  EXIT:RADIO")==0 ? 267 : \
    __builtin_strcmp((s),"MENU:OPTIONS  EXIT:BACK")==0 ? 289 : \
    __builtin_strcmp((s),"MENU:SAVE  EXIT:CANCEL")==0 ? 313 : \
    __builtin_strcmp((s),"MENU:SAVE EXIT:CANCEL")==0 ? 336 : \
    __builtin_strcmp((s),"MENU:SELECT  EXIT:BODY")==0 ? 358 : \
    __builtin_strcmp((s),"MENU:SELECT EXIT:BACK")==0 ? 381 : \
    __builtin_strcmp((s),"MENU:SEND  EXIT:EDIT")==0 ? 539 : \
    __builtin_strcmp((s),"MENU:SEND EXIT:CANCEL")==0 ? 403 : \
    __builtin_strcmp((s),"MESSAGE FULL")==0 ? 1594 : \
    __builtin_strcmp((s),"MESSAGE OPTIONS")==0 ? 1078 : \
    __builtin_strcmp((s),"MHz")==0 ? 713 : \
    __builtin_strcmp((s),"MONITOR")==0 ? 2085 : \
    __builtin_strcmp((s),"MULTI-TAP ABC/123")==0 ? 560 : \
    __builtin_strcmp((s),"MY CALLSIGN")==0 ? 1607 : \
    __builtin_strcmp((s),"N")==0 ? 479 : \
    __builtin_strcmp((s),"NAME")==0 ? 2288 : \
    __builtin_strcmp((s),"NAME\n+\nFREQ")==0 ? 1619 : \
    __builtin_strcmp((s),"NEW MESSAGE")==0 ? 1631 : \
    __builtin_strcmp((s),"NONE")==0 ? 2293 : \
    __builtin_strcmp((s),"NOT SENT")==0 ? 2093 : \
    __builtin_strcmp((s),"NULL")==0 ? 2298 : \
    __builtin_strcmp((s),"OFF")==0 ? 891 : \
    __builtin_strcmp((s),"ON")==0 ? 478 : \
    __builtin_strcmp((s),"POSITION NOT SET")==0 ? 1094 : \
    __builtin_strcmp((s),"POWER")==0 ? 2102 : \
    __builtin_strcmp((s),"PRI%d:%u")==0 ? 2108 : \
    __builtin_strcmp((s),"PTT")==0 ? 2303 : \
    __builtin_strcmp((s),"PTT:RETURN TO VOICE")==0 ? 578 : \
    __builtin_strcmp((s),"Precision RF")==0 ? 1643 : \
    __builtin_strcmp((s),"QUEUED")==0 ? 2117 : \
    __builtin_strcmp((s),"QUICK MESSAGE")==0 ? 486 : \
    __builtin_strcmp((s),"QUICK MESSAGES")==0 ? 1111 : \
    __builtin_strcmp((s),"RADIO")==0 ? 283 : \
    __builtin_strcmp((s),"RADIO ALERT")==0 ? 1656 : \
    __builtin_strcmp((s),"READ\nMANUAL")==0 ? 1668 : \
    __builtin_strcmp((s),"RECIPIENT")==0 ? 1008 : \
    __builtin_strcmp((s),"RELEASE")==0 ? 2124 : \
    __builtin_strcmp((s),"REVIEW AND SEND")==0 ? 1126 : \
    __builtin_strcmp((s),"REVIEW BEACON")==0 ? 1142 : \
    __builtin_strcmp((s),"REVIEW POSITION")==0 ? 1156 : \
    __builtin_strcmp((s),"RX")==0 ? 1075 : \
    __builtin_strcmp((s),"RX/TX disabled")==0 ? 1172 : \
    __builtin_strcmp((s),"S")==0 ? 1001 : \
    __builtin_strcmp((s),"S%u %ddBm Q%u %c")==0 ? 1187 : \
    __builtin_strcmp((s),"S+W")==0 ? 2307 : \
    __builtin_strcmp((s),"SAVE DOES NOT SEND")==0 ? 598 : \
    __builtin_strcmp((s),"SCAN")==0 ? 2311 : \
    __builtin_strcmp((s),"SCAN %s %uc/s")==0 ? 1204 : \
    __builtin_strcmp((s),"SEND")==0 ? 612 : \
    __builtin_strcmp((s),"SEND BEACON")==0 ? 1680 : \
    __builtin_strcmp((s),"SESSION ONLY")==0 ? 1692 : \
    __builtin_strcmp((s),"SETUP")==0 ? 123 : \
    __builtin_strcmp((s),"SHIFT %c%u.%03u %s")==0 ? 617 : \
    __builtin_strcmp((s),"SIDE1")==0 ? 2132 : \
    __builtin_strcmp((s),"SIDE2")==0 ? 2138 : \
    __builtin_strcmp((s),"SP")==0 ? 2316 : \
    __builtin_strcmp((s),"SPECTRUM")==0 ? 2144 : \
    __builtin_strcmp((s),"SQL%u %s %u.%ukHz")==0 ? 636 : \
    __builtin_strcmp((s),"STAR")==0 ? 2319 : \
    __builtin_strcmp((s),"STATIC BEACON")==0 ? 1218 : \
    __builtin_strcmp((s),"STATIC POSITION")==0 ? 1232 : \
    __builtin_strcmp((s),"SURE?")==0 ? 2153 : \
    __builtin_strcmp((s),"SWITCH\nDEMODUL")==0 ? 1248 : \
    __builtin_strcmp((s),"SWITCH\nVFO")==0 ? 1705 : \
    __builtin_strcmp((s),"Signal Quality")==0 ? 1263 : \
    __builtin_strcmp((s),"TIMEOUT")==0 ? 2159 : \
    __builtin_strcmp((s),"TO")==0 ? 2254 : \
    __builtin_strcmp((s),"TO %s")==0 ? 2167 : \
    __builtin_strcmp((s),"TX")==0 ? 1454 : \
    __builtin_strcmp((s),"TX %u:%02u")==0 ? 1716 : \
    __builtin_strcmp((s),"TX AUDIO 0-127")==0 ? 1278 : \
    __builtin_strcmp((s),"TX AUDIO LEVEL")==0 ? 1293 : \
    __builtin_strcmp((s),"TX BLOCKED")==0 ? 1727 : \
    __builtin_strcmp((s),"TX Compressor")==0 ? 1308 : \
    __builtin_strcmp((s),"TX DISABLE")==0 ? 1738 : \
    __builtin_strcmp((s),"TX FAILED")==0 ? 1749 : \
    __builtin_strcmp((s),"TX SENT")==0 ? 2173 : \
    __builtin_strcmp((s),"TX UNAVAILABLE")==0 ? 1322 : \
    __builtin_strcmp((s),"TX sag %umV")==0 ? 1759 : \
    __builtin_strcmp((s),"UNLOCK\nALL")==0 ? 1771 : \
    __builtin_strcmp((s),"UP")==0 ? 126 : \
    __builtin_strcmp((s),"UP CODE")==0 ? 2181 : \
    __builtin_strcmp((s),"UP+DOWN\nCODE")==0 ? 1782 : \
    __builtin_strcmp((s),"UP/DOWN:HEMISPHERE")==0 ? 654 : \
    __builtin_strcmp((s),"VFO %c")==0 ? 2189 : \
    __builtin_strcmp((s),"VFO B ACTIVITY")==0 ? 1337 : \
    __builtin_strcmp((s),"VFO ERROR")==0 ? 1795 : \
    __builtin_strcmp((s),"VFO/MR")==0 ? 2196 : \
    __builtin_strcmp((s),"VOLT HIGH")==0 ? 1805 : \
    __builtin_strcmp((s),"VOLTAGE")==0 ? 2203 : \
    __builtin_strcmp((s),"VOX")==0 ? 2324 : \
    __builtin_strcmp((s),"VUURWERK")==0 ? 2211 : \
    __builtin_strcmp((s),"W")==0 ? 2010 : \
    __builtin_strcmp((s),"WAIT!")==0 ? 2220 : \
    __builtin_strcmp((s),"^")==0 ? 2328 : \
    __builtin_strcmp((s),"calib lost")==0 ? 1815 : \
    __builtin_strcmp((s),"to unlock")==0 ? 1826 : \
    -1)
#define DISPLAY_TEXT(s) (gDisplayText + DISPLAY_TEXT_OFFSET(s) + \
    0*sizeof(char[DISPLAY_TEXT_OFFSET(s)>=0 ? 1 : -1]))
#endif
