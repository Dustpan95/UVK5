/* SPDX-License-Identifier: GPL-3.0-only
 * Generated losslessly from project UI text by tools/pack-display-text.py.
 * Original copyright/license notices remain in the source UI files and CREDITS.
 */
#ifndef UVK5_DISPLAY_TEXT_H
#define UVK5_DISPLAY_TEXT_H
#include <stdint.h>
enum { DISPLAY_TEXT_BYTES=2893, DISPLAY_HINT_BYTES=921, DISPLAY_RESOURCE_BYTES=4241 };
typedef struct {
    char hints[DISPLAY_HINT_BYTES], text[DISPLAY_TEXT_BYTES];
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
    __builtin_strcmp((s)," ABC")==0 ? 2786 : \
    __builtin_strcmp((s)," ABC_")==0 ? 2696 : \
    __builtin_strcmp((s),"%.*s.%s")==0 ? 2485 : \
    __builtin_strcmp((s),"%.3s.%.3s  ")==0 ? 1921 : \
    __builtin_strcmp((s),"%.8s")==0 ? 2310 : \
    __builtin_strcmp((s),"%.8s\n%.8s")==0 ? 2305 : \
    __builtin_strcmp((s),"%02u:%02u")==0 ? 2315 : \
    __builtin_strcmp((s),"%2d")==0 ? 2846 : \
    __builtin_strcmp((s),"%2u.%u")==0 ? 2605 : \
    __builtin_strcmp((s),"%3d.%05u")==0 ? 2395 : \
    __builtin_strcmp((s),"%3u.%05u")==0 ? 1003 : \
    __builtin_strcmp((s),"%c  CH%03u")==0 ? 2173 : \
    __builtin_strcmp((s),"%c %3u.%05u")==0 ? 1933 : \
    __builtin_strcmp((s),"%c%.16s")==0 ? 2493 : \
    __builtin_strcmp((s),"%c%c%s")==0 ? 2612 : \
    __builtin_strcmp((s),"%c%s")==0 ? 2614 : \
    __builtin_strcmp((s),"%cACK WAIT %us")==0 ? 1268 : \
    __builtin_strcmp((s),"%cDATA TX %s")==0 ? 1804 : \
    __builtin_strcmp((s),"%cFREQ %3u.%05u")==0 ? 996 : \
    __builtin_strcmp((s),"%cFREQ NOT SET")==0 ? 1283 : \
    __builtin_strcmp((s),"%cTX AUDIO %u")==0 ? 1538 : \
    __builtin_strcmp((s),"%cTX AUDIO NOT SET")==0 ? 654 : \
    __builtin_strcmp((s),"%d")==0 ? 2874 : \
    __builtin_strcmp((s),"%d\n%u.%06u\nMHz")==0 ? 1298 : \
    __builtin_strcmp((s),"%d %s")==0 ? 2702 : \
    __builtin_strcmp((s),"%d%2d %2d %2d %3d")==0 ? 768 : \
    __builtin_strcmp((s),"%d*100ms")==0 ? 2404 : \
    __builtin_strcmp((s),"%d*10ms")==0 ? 2501 : \
    __builtin_strcmp((s),"%d.%02ukHz")==0 ? 2184 : \
    __builtin_strcmp((s),"%ddBm")==0 ? 2708 : \
    __builtin_strcmp((s),"%ds")==0 ? 2850 : \
    __builtin_strcmp((s),"%lu.%05lu%c")==0 ? 1945 : \
    __builtin_strcmp((s),"%s %c")==0 ? 2714 : \
    __builtin_strcmp((s),"%s %s")==0 ? 2720 : \
    __builtin_strcmp((s),"%s %u/%u")==0 ? 1599 : \
    __builtin_strcmp((s),"%s>%s")==0 ? 2726 : \
    __builtin_strcmp((s),"%u%%")==0 ? 1561 : \
    __builtin_strcmp((s),"%u.%02uV\n%u")==0 ? 1957 : \
    __builtin_strcmp((s),"%u.%02uV\n%u%%")==0 ? 1552 : \
    __builtin_strcmp((s),"%u.%02uV %u%%")==0 ? 1566 : \
    __builtin_strcmp((s),"%u.%05u")==0 ? 2509 : \
    __builtin_strcmp((s),"%u.%uHz")==0 ? 2517 : \
    __builtin_strcmp((s),"%u/67%s")==0 ? 2525 : \
    __builtin_strcmp((s),"%ud")==0 ? 2854 : \
    __builtin_strcmp((s),"%uh")==0 ? 2858 : \
    __builtin_strcmp((s),"*")==0 ? 2889 : \
    __builtin_strcmp((s),"*:DELETE")==0 ? 2413 : \
    __builtin_strcmp((s),"+%u.%01udB")==0 ? 2195 : \
    __builtin_strcmp((s),"--")==0 ? 2877 : \
    __builtin_strcmp((s),"1")==0 ? 2760 : \
    __builtin_strcmp((s),"137-174\n400-430")==0 ? 1012 : \
    __builtin_strcmp((s),"137-174\n400-438")==0 ? 1028 : \
    __builtin_strcmp((s),"1750HZ")==0 ? 2619 : \
    __builtin_strcmp((s),"1:.-/  #:NEXT")==0 ? 1580 : \
    __builtin_strcmp((s),"2")==0 ? 2766 : \
    __builtin_strcmp((s),"?")==0 ? 2772 : \
    __builtin_strcmp((s),"ACK RECEIVED")==0 ? 1817 : \
    __builtin_strcmp((s),"ACK WAIT")==0 ? 2422 : \
    __builtin_strcmp((s),"ACK WAIT SECONDS")==0 ? 894 : \
    __builtin_strcmp((s),"ALARM")==0 ? 2732 : \
    __builtin_strcmp((s),"ALL")==0 ? 2025 : \
    __builtin_strcmp((s),"ALL KEYS")==0 ? 2431 : \
    __builtin_strcmp((s),"ALL TRAFFIC")==0 ? 1969 : \
    __builtin_strcmp((s),"APOLLO\nQUINDAR")==0 ? 1313 : \
    __builtin_strcmp((s),"APRS")==0 ? 104 : \
    __builtin_strcmp((s),"APRS %s %u/%u")==0 ? 1594 : \
    __builtin_strcmp((s),"APRS ACTIVE")==0 ? 1981 : \
    __builtin_strcmp((s),"APRS DATA TX")==0 ? 1830 : \
    __builtin_strcmp((s),"APRS FREQ MHz")==0 ? 1608 : \
    __builtin_strcmp((s),"APRS FREQUENCY")==0 ? 1328 : \
    __builtin_strcmp((s),"APRS RX")==0 ? 2533 : \
    __builtin_strcmp((s),"APRS SETUP SESSION")==0 ? 673 : \
    __builtin_strcmp((s),"ATTEMPTS %u/%u")==0 ? 1343 : \
    __builtin_strcmp((s),"AUTO")==0 ? 2791 : \
    __builtin_strcmp((s),"Adaptive Squelch")==0 ? 911 : \
    __builtin_strcmp((s),"B SCAN CH%03u")==0 ? 1622 : \
    __builtin_strcmp((s),"BAT LOW")==0 ? 2541 : \
    __builtin_strcmp((s),"BEACON SENT")==0 ? 1993 : \
    __builtin_strcmp((s),"BK4819")==0 ? 2626 : \
    __builtin_strcmp((s),"BLMIN\nTMP OFF")==0 ? 1636 : \
    __builtin_strcmp((s),"BUSY")==0 ? 434 : \
    __builtin_strcmp((s),"CALLSIGN FULL")==0 ? 1650 : \
    __builtin_strcmp((s),"CANCELLED")==0 ? 2325 : \
    __builtin_strcmp((s),"CE HAM\n144-146\n430-440")==0 ? 256 : \
    __builtin_strcmp((s),"CH-%.3s")==0 ? 2549 : \
    __builtin_strcmp((s),"CHANNEL\nNUMBER")==0 ? 1358 : \
    __builtin_strcmp((s),"CHANNEL ENTRY")==0 ? 1664 : \
    __builtin_strcmp((s),"CHECK CALL/SSID")==0 ? 1044 : \
    __builtin_strcmp((s),"CHECK INPUT / TX SETUP")==0 ? 279 : \
    __builtin_strcmp((s),"CHECK RADIO")==0 ? 2005 : \
    __builtin_strcmp((s),"CHECK TX SETUP / BUSY")==0 ? 417 : \
    __builtin_strcmp((s),"CROSS\nBAND")==0 ? 2206 : \
    __builtin_strcmp((s),"Cs")==0 ? 2880 : \
    __builtin_strcmp((s),"D%03oI")==0 ? 2633 : \
    __builtin_strcmp((s),"D%03oN")==0 ? 2640 : \
    __builtin_strcmp((s),"DATA TX")==0 ? 1835 : \
    __builtin_strcmp((s),"DEFAULT+\n137-174\n400-470")==0 ? 109 : \
    __builtin_strcmp((s),"DISABLE\nALL")==0 ? 2017 : \
    __builtin_strcmp((s),"DOWN")==0 ? 2796 : \
    __builtin_strcmp((s),"DOWN CODE")==0 ? 2335 : \
    __builtin_strcmp((s),"DTMF DECODE")==0 ? 2029 : \
    __builtin_strcmp((s),"DTMF ENTRY")==0 ? 2217 : \
    __builtin_strcmp((s),"DUAL RX\nRESPOND")==0 ? 1060 : \
    __builtin_strcmp((s),"EDIT QUICK MESSAGE")==0 ? 692 : \
    __builtin_strcmp((s),"EDIT QUICK MSGS")==0 ? 1076 : \
    __builtin_strcmp((s),"EDIT RECIPIENT")==0 ? 1373 : \
    __builtin_strcmp((s),"EEPROM")==0 ? 2647 : \
    __builtin_strcmp((s),"EMPTY MESSAGE")==0 ? 1678 : \
    __builtin_strcmp((s),"EXIT")==0 ? 2801 : \
    __builtin_strcmp((s),"EXIT:DELETE  HOLD EXIT:CANCEL")==0 ? 0 : \
    __builtin_strcmp((s),"EXIT:INBOX")==0 ? 360 : \
    __builtin_strcmp((s),"EXIT:PACKETS")==0 ? 1843 : \
    __builtin_strcmp((s),"EXIT:STATIONS")==0 ? 1692 : \
    __builtin_strcmp((s),"EXPIRED")==0 ? 2557 : \
    __builtin_strcmp((s),"F")==0 ? 1648 : \
    __builtin_strcmp((s),"FAULT")==0 ? 2738 : \
    __builtin_strcmp((s),"FCC HAM\n144-148\n420-450")==0 ? 184 : \
    __builtin_strcmp((s),"FLASH\nLIGHT")==0 ? 2041 : \
    __builtin_strcmp((s),"FM RADIO")==0 ? 2440 : \
    __builtin_strcmp((s),"FM, scan/DW off")==0 ? 1092 : \
    __builtin_strcmp((s),"FREQ")==0 ? 2096 : \
    __builtin_strcmp((s),"FREQUENCY ENTRY")==0 ? 1108 : \
    __builtin_strcmp((s),"GB HAM\n144-148\n430-440")==0 ? 302 : \
    __builtin_strcmp((s),"HEARD %us AGO")==0 ? 1706 : \
    __builtin_strcmp((s),"HOLD")==0 ? 2806 : \
    __builtin_strcmp((s),"ID:%4s")==0 ? 2654 : \
    __builtin_strcmp((s),"IDLE")==0 ? 2811 : \
    __builtin_strcmp((s),"LATITUDE DD MM.mm")==0 ? 786 : \
    __builtin_strcmp((s),"LIST%u")==0 ? 2661 : \
    __builtin_strcmp((s),"LK")==0 ? 2883 : \
    __builtin_strcmp((s),"LOCK\nKEYPAD")==0 ? 2053 : \
    __builtin_strcmp((s),"LON DDD MM.mm")==0 ? 1720 : \
    __builtin_strcmp((s),"LOW BATTERY")==0 ? 2065 : \
    __builtin_strcmp((s),"Long press #")==0 ? 1856 : \
    __builtin_strcmp((s),"M")==0 ? 2474 : \
    __builtin_strcmp((s),"MAIN\nONLY")==0 ? 2345 : \
    __builtin_strcmp((s),"MAIN TX\nDUAL RX")==0 ? 1124 : \
    __builtin_strcmp((s),"MENU")==0 ? 2816 : \
    __builtin_strcmp((s),"MENU   F+5:APRS   EXIT:A/B")==0 ? 30 : \
    __builtin_strcmp((s),"MENU/EXIT:STOP  UP/DN:DIR")==0 ? 57 : \
    __builtin_strcmp((s),"MENU:CANCEL EXIT:BACK")==0 ? 439 : \
    __builtin_strcmp((s),"MENU:DETAIL  EXIT:APRS")==0 ? 325 : \
    __builtin_strcmp((s),"MENU:NEXT  EXIT:BACK")==0 ? 571 : \
    __builtin_strcmp((s),"MENU:NEXT EXIT:CANCEL")==0 ? 461 : \
    __builtin_strcmp((s),"MENU:OPEN  EXIT:RADIO")==0 ? 483 : \
    __builtin_strcmp((s),"MENU:OPTIONS  EXIT:BACK")==0 ? 208 : \
    __builtin_strcmp((s),"MENU:READ  EXIT:APRS")==0 ? 592 : \
    __builtin_strcmp((s),"MENU:REPLY  EXIT:INBOX")==0 ? 348 : \
    __builtin_strcmp((s),"MENU:REPLY/NEW  EXIT:APRS")==0 ? 83 : \
    __builtin_strcmp((s),"MENU:SAVE  EXIT:CANCEL")==0 ? 371 : \
    __builtin_strcmp((s),"MENU:SAVE EXIT:CANCEL")==0 ? 505 : \
    __builtin_strcmp((s),"MENU:SELECT  EXIT:BODY")==0 ? 394 : \
    __builtin_strcmp((s),"MENU:SELECT EXIT:BACK")==0 ? 527 : \
    __builtin_strcmp((s),"MENU:SEND  EXIT:EDIT")==0 ? 613 : \
    __builtin_strcmp((s),"MENU:SEND EXIT:CANCEL")==0 ? 549 : \
    __builtin_strcmp((s),"MENU:SET CALL  EXIT:APRS")==0 ? 134 : \
    __builtin_strcmp((s),"MESSAGE FULL")==0 ? 1869 : \
    __builtin_strcmp((s),"MESSAGE OPTIONS")==0 ? 1140 : \
    __builtin_strcmp((s),"MESSAGE REJECTED")==0 ? 928 : \
    __builtin_strcmp((s),"MHz")==0 ? 1309 : \
    __builtin_strcmp((s),"MONITOR")==0 ? 2565 : \
    __builtin_strcmp((s),"MONITOR %u/%u")==0 ? 1734 : \
    __builtin_strcmp((s),"MSG %s")==0 ? 2668 : \
    __builtin_strcmp((s),"MULTI-TAP ABC/123")==0 ? 804 : \
    __builtin_strcmp((s),"MY CALL %s")==0 ? 2228 : \
    __builtin_strcmp((s),"MY CALLSIGN")==0 ? 2077 : \
    __builtin_strcmp((s),"N")==0 ? 690 : \
    __builtin_strcmp((s),"NAME")==0 ? 2821 : \
    __builtin_strcmp((s),"NAME\n+\nFREQ")==0 ? 2089 : \
    __builtin_strcmp((s),"NEW MESSAGE")==0 ? 2101 : \
    __builtin_strcmp((s),"NONE")==0 ? 2826 : \
    __builtin_strcmp((s),"NOT SENT")==0 ? 2449 : \
    __builtin_strcmp((s),"NULL")==0 ? 2831 : \
    __builtin_strcmp((s),"No messages yet")==0 ? 1156 : \
    __builtin_strcmp((s),"No packets yet")==0 ? 1388 : \
    __builtin_strcmp((s),"No position yet")==0 ? 1172 : \
    __builtin_strcmp((s),"No stations yet")==0 ? 1188 : \
    __builtin_strcmp((s),"OFF")==0 ? 1646 : \
    __builtin_strcmp((s),"ON")==0 ? 689 : \
    __builtin_strcmp((s),"PACKETS")==0 ? 1848 : \
    __builtin_strcmp((s),"PAUSE")==0 ? 2744 : \
    __builtin_strcmp((s),"PERSONAL INBOX")==0 ? 1403 : \
    __builtin_strcmp((s),"PERSONAL INBOX %u")==0 ? 822 : \
    __builtin_strcmp((s),"PERSONAL MESSAGE")==0 ? 945 : \
    __builtin_strcmp((s),"POSITION NOT SET")==0 ? 962 : \
    __builtin_strcmp((s),"POWER")==0 ? 2750 : \
    __builtin_strcmp((s),"PRI%d:%u")==0 ? 2458 : \
    __builtin_strcmp((s),"PTT")==0 ? 2862 : \
    __builtin_strcmp((s),"PTT:RETURN TO VOICE")==0 ? 634 : \
    __builtin_strcmp((s),"Precision RF")==0 ? 1882 : \
    __builtin_strcmp((s),"QUEUED")==0 ? 2675 : \
    __builtin_strcmp((s),"QUICK MESSAGE")==0 ? 697 : \
    __builtin_strcmp((s),"QUICK MESSAGES")==0 ? 1418 : \
    __builtin_strcmp((s),"RADIO")==0 ? 178 : \
    __builtin_strcmp((s),"RADIO ALERT")==0 ? 2113 : \
    __builtin_strcmp((s),"READ\nMANUAL")==0 ? 2125 : \
    __builtin_strcmp((s),"RECIPIENT")==0 ? 1378 : \
    __builtin_strcmp((s),"RELEASE")==0 ? 2573 : \
    __builtin_strcmp((s),"REVIEW AND SEND")==0 ? 1204 : \
    __builtin_strcmp((s),"REVIEW BEACON")==0 ? 1748 : \
    __builtin_strcmp((s),"REVIEW POSITION")==0 ? 1220 : \
    __builtin_strcmp((s),"RX")==0 ? 1137 : \
    __builtin_strcmp((s),"RX/TX disabled")==0 ? 1433 : \
    __builtin_strcmp((s),"S")==0 ? 107 : \
    __builtin_strcmp((s),"S%u %ddBm Q%u %c")==0 ? 979 : \
    __builtin_strcmp((s),"S+W")==0 ? 2866 : \
    __builtin_strcmp((s),"SAVE DOES NOT SEND")==0 ? 711 : \
    __builtin_strcmp((s),"SCAN")==0 ? 2836 : \
    __builtin_strcmp((s),"SCAN %s %uc/s")==0 ? 1762 : \
    __builtin_strcmp((s),"SEND")==0 ? 725 : \
    __builtin_strcmp((s),"SEND BEACON")==0 ? 2137 : \
    __builtin_strcmp((s),"SESSION ONLY")==0 ? 1895 : \
    __builtin_strcmp((s),"SET YOUR CALLSIGN")==0 ? 840 : \
    __builtin_strcmp((s),"SETUP")==0 ? 296 : \
    __builtin_strcmp((s),"SHIFT %c%u.%03u %s")==0 ? 730 : \
    __builtin_strcmp((s),"SIDE1")==0 ? 2756 : \
    __builtin_strcmp((s),"SIDE2")==0 ? 2762 : \
    __builtin_strcmp((s),"SP")==0 ? 2886 : \
    __builtin_strcmp((s),"SPECTRUM")==0 ? 2467 : \
    __builtin_strcmp((s),"SQL%u %s %u.%ukHz")==0 ? 858 : \
    __builtin_strcmp((s),"STAR")==0 ? 2841 : \
    __builtin_strcmp((s),"STATIC BEACON")==0 ? 1776 : \
    __builtin_strcmp((s),"STATIC POSITION")==0 ? 1236 : \
    __builtin_strcmp((s),"STATIONS")==0 ? 1697 : \
    __builtin_strcmp((s),"STATIONS %u HEARD")==0 ? 876 : \
    __builtin_strcmp((s),"SURE?")==0 ? 2768 : \
    __builtin_strcmp((s),"SWITCH\nDEMODUL")==0 ? 1448 : \
    __builtin_strcmp((s),"SWITCH\nVFO")==0 ? 2239 : \
    __builtin_strcmp((s),"Signal Quality")==0 ? 1463 : \
    __builtin_strcmp((s),"TIMEOUT")==0 ? 2581 : \
    __builtin_strcmp((s),"TO")==0 ? 2793 : \
    __builtin_strcmp((s),"TO %s")==0 ? 2774 : \
    __builtin_strcmp((s),"TX")==0 ? 1840 : \
    __builtin_strcmp((s),"TX %u:%02u")==0 ? 2250 : \
    __builtin_strcmp((s),"TX AUDIO 0-127")==0 ? 1478 : \
    __builtin_strcmp((s),"TX AUDIO LEVEL")==0 ? 1493 : \
    __builtin_strcmp((s),"TX BLOCKED")==0 ? 2261 : \
    __builtin_strcmp((s),"TX Compressor")==0 ? 1790 : \
    __builtin_strcmp((s),"TX DISABLE")==0 ? 2272 : \
    __builtin_strcmp((s),"TX FAILED")==0 ? 2355 : \
    __builtin_strcmp((s),"TX UNAVAILABLE")==0 ? 1508 : \
    __builtin_strcmp((s),"TX sag %umV")==0 ? 2149 : \
    __builtin_strcmp((s),"UNCONFIRMED")==0 ? 2161 : \
    __builtin_strcmp((s),"UNLOCK\nALL")==0 ? 2283 : \
    __builtin_strcmp((s),"UP")==0 ? 299 : \
    __builtin_strcmp((s),"UP CODE")==0 ? 2589 : \
    __builtin_strcmp((s),"UP+DOWN\nCODE")==0 ? 1908 : \
    __builtin_strcmp((s),"UP/DN:PACKET  EXIT:APRS")==0 ? 232 : \
    __builtin_strcmp((s),"UP/DN:PACKET  EXIT:RADIO")==0 ? 159 : \
    __builtin_strcmp((s),"UP/DOWN:HEMISPHERE")==0 ? 749 : \
    __builtin_strcmp((s),"VFO %c")==0 ? 2682 : \
    __builtin_strcmp((s),"VFO B ACTIVITY")==0 ? 1523 : \
    __builtin_strcmp((s),"VFO ERROR")==0 ? 2365 : \
    __builtin_strcmp((s),"VFO/MR")==0 ? 2689 : \
    __builtin_strcmp((s),"VOLT HIGH")==0 ? 2375 : \
    __builtin_strcmp((s),"VOLTAGE")==0 ? 2597 : \
    __builtin_strcmp((s),"VOX")==0 ? 2870 : \
    __builtin_strcmp((s),"VUURWERK")==0 ? 2476 : \
    __builtin_strcmp((s),"W")==0 ? 2547 : \
    __builtin_strcmp((s),"WAIT!")==0 ? 2780 : \
    __builtin_strcmp((s),"WAITING FOR ACK")==0 ? 1252 : \
    __builtin_strcmp((s),"^")==0 ? 2891 : \
    __builtin_strcmp((s),"calib lost")==0 ? 2294 : \
    __builtin_strcmp((s),"to unlock")==0 ? 2385 : \
    -1)
#define DISPLAY_TEXT(s) (gDisplayText + DISPLAY_TEXT_OFFSET(s) + \
    0*sizeof(char[DISPLAY_TEXT_OFFSET(s)>=0 ? 1 : -1]))
#endif
