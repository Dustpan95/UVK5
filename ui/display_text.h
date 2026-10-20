/* SPDX-License-Identifier: GPL-3.0-only
 * Generated losslessly from project UI text by tools/pack-display-text.py.
 * Original copyright/license notices remain in the source UI files and CREDITS.
 */
#ifndef UVK5_DISPLAY_TEXT_H
#define UVK5_DISPLAY_TEXT_H
#include <stdint.h>
enum { DISPLAY_TEXT_BYTES=2535, DISPLAY_HINT_BYTES=921, DISPLAY_RESOURCE_BYTES=3883 };
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
    __builtin_strcmp((s)," ABC")==0 ? 2434 : \
    __builtin_strcmp((s)," ABC_")==0 ? 2350 : \
    __builtin_strcmp((s),"%.*s.%s")==0 ? 2169 : \
    __builtin_strcmp((s),"%.3s.%.3s  ")==0 ? 1724 : \
    __builtin_strcmp((s),"%.8s")==0 ? 2032 : \
    __builtin_strcmp((s),"%.8s\n%.8s")==0 ? 2027 : \
    __builtin_strcmp((s),"%02u %02u.%02u%c")==0 ? 799 : \
    __builtin_strcmp((s),"%02u:%02u")==0 ? 2037 : \
    __builtin_strcmp((s),"%03u %02u.%02u%c")==0 ? 816 : \
    __builtin_strcmp((s),"%2d")==0 ? 2484 : \
    __builtin_strcmp((s),"%2u.%u")==0 ? 2273 : \
    __builtin_strcmp((s),"%3d.%05u")==0 ? 2097 : \
    __builtin_strcmp((s),"%3u.%05u")==0 ? 942 : \
    __builtin_strcmp((s),"%c  CH%03u")==0 ? 1928 : \
    __builtin_strcmp((s),"%c %3u.%05u")==0 ? 1736 : \
    __builtin_strcmp((s),"%c%.16s")==0 ? 2177 : \
    __builtin_strcmp((s),"%c%c%s")==0 ? 2280 : \
    __builtin_strcmp((s),"%c%s")==0 ? 2282 : \
    __builtin_strcmp((s),"%cACK WAIT %us")==0 ? 1143 : \
    __builtin_strcmp((s),"%cDATA TX %s")==0 ? 1620 : \
    __builtin_strcmp((s),"%cFREQ %3u.%05u")==0 ? 935 : \
    __builtin_strcmp((s),"%cFREQ NOT SET")==0 ? 1158 : \
    __builtin_strcmp((s),"%cTX AUDIO %u")==0 ? 1368 : \
    __builtin_strcmp((s),"%cTX AUDIO NOT SET")==0 ? 559 : \
    __builtin_strcmp((s),"%d")==0 ? 2516 : \
    __builtin_strcmp((s),"%d\n%u.%06u\nMHz")==0 ? 1173 : \
    __builtin_strcmp((s),"%d %s")==0 ? 2356 : \
    __builtin_strcmp((s),"%d%2d %2d %2d %3d")==0 ? 673 : \
    __builtin_strcmp((s),"%d*100ms")==0 ? 2106 : \
    __builtin_strcmp((s),"%d*10ms")==0 ? 2185 : \
    __builtin_strcmp((s),"%d.%02ukHz")==0 ? 1939 : \
    __builtin_strcmp((s),"%ddBm")==0 ? 2362 : \
    __builtin_strcmp((s),"%ds")==0 ? 2488 : \
    __builtin_strcmp((s),"%lu.%05lu%c")==0 ? 1748 : \
    __builtin_strcmp((s),"%s %c")==0 ? 2368 : \
    __builtin_strcmp((s),"%s %s")==0 ? 2374 : \
    __builtin_strcmp((s),"%s %u/%u")==0 ? 1429 : \
    __builtin_strcmp((s),"%s>%s")==0 ? 2380 : \
    __builtin_strcmp((s),"%u%%")==0 ? 1391 : \
    __builtin_strcmp((s),"%u.%02uV\n%u")==0 ? 1760 : \
    __builtin_strcmp((s),"%u.%02uV\n%u%%")==0 ? 1382 : \
    __builtin_strcmp((s),"%u.%02uV %u%%")==0 ? 1396 : \
    __builtin_strcmp((s),"%u.%05u")==0 ? 2193 : \
    __builtin_strcmp((s),"%u.%uHz")==0 ? 2201 : \
    __builtin_strcmp((s),"%u/67%s")==0 ? 2209 : \
    __builtin_strcmp((s),"%ud")==0 ? 2492 : \
    __builtin_strcmp((s),"%uh")==0 ? 2496 : \
    __builtin_strcmp((s),"*")==0 ? 2531 : \
    __builtin_strcmp((s),"*:DELETE")==0 ? 2115 : \
    __builtin_strcmp((s),"+%u.%01udB")==0 ? 1950 : \
    __builtin_strcmp((s),"--")==0 ? 2519 : \
    __builtin_strcmp((s),"1")==0 ? 2408 : \
    __builtin_strcmp((s),"1:.-/  #:NEXT")==0 ? 1410 : \
    __builtin_strcmp((s),"2")==0 ? 2414 : \
    __builtin_strcmp((s),"?")==0 ? 2420 : \
    __builtin_strcmp((s),"ACK RECEIVED")==0 ? 1633 : \
    __builtin_strcmp((s),"ACK WAIT")==0 ? 2124 : \
    __builtin_strcmp((s),"ACK WAIT SECONDS")==0 ? 833 : \
    __builtin_strcmp((s),"ALARM")==0 ? 2386 : \
    __builtin_strcmp((s),"ALL")==0 ? 2500 : \
    __builtin_strcmp((s),"ALL KEYS")==0 ? 2133 : \
    __builtin_strcmp((s),"ALL TRAFFIC")==0 ? 1772 : \
    __builtin_strcmp((s),"APRS")==0 ? 104 : \
    __builtin_strcmp((s),"APRS %s %u/%u")==0 ? 1424 : \
    __builtin_strcmp((s),"APRS ACTIVE")==0 ? 1784 : \
    __builtin_strcmp((s),"APRS DATA TX")==0 ? 1646 : \
    __builtin_strcmp((s),"APRS FREQ MHz")==0 ? 1438 : \
    __builtin_strcmp((s),"APRS FREQUENCY")==0 ? 1188 : \
    __builtin_strcmp((s),"APRS RX")==0 ? 2217 : \
    __builtin_strcmp((s),"APRS SETUP SESSION")==0 ? 578 : \
    __builtin_strcmp((s),"ATTEMPTS %u/%u")==0 ? 1203 : \
    __builtin_strcmp((s),"AUTO")==0 ? 2439 : \
    __builtin_strcmp((s),"Adaptive Squelch")==0 ? 850 : \
    __builtin_strcmp((s),"B SCAN CH%03u")==0 ? 1452 : \
    __builtin_strcmp((s),"BAT LOW")==0 ? 2225 : \
    __builtin_strcmp((s),"BEACON SENT")==0 ? 1796 : \
    __builtin_strcmp((s),"BK4819")==0 ? 2287 : \
    __builtin_strcmp((s),"BUSY")==0 ? 339 : \
    __builtin_strcmp((s),"CALLSIGN FULL")==0 ? 1466 : \
    __builtin_strcmp((s),"CANCELLED")==0 ? 2047 : \
    __builtin_strcmp((s),"CH-%.3s")==0 ? 2233 : \
    __builtin_strcmp((s),"CHANNEL ENTRY")==0 ? 1480 : \
    __builtin_strcmp((s),"CHECK CALL/SSID")==0 ? 951 : \
    __builtin_strcmp((s),"CHECK INPUT / TX SETUP")==0 ? 207 : \
    __builtin_strcmp((s),"CHECK RADIO")==0 ? 1808 : \
    __builtin_strcmp((s),"CHECK TX SETUP / BUSY")==0 ? 322 : \
    __builtin_strcmp((s),"Cs")==0 ? 2522 : \
    __builtin_strcmp((s),"D%03oI")==0 ? 2294 : \
    __builtin_strcmp((s),"D%03oN")==0 ? 2301 : \
    __builtin_strcmp((s),"DATA TX")==0 ? 1651 : \
    __builtin_strcmp((s),"DOWN")==0 ? 2444 : \
    __builtin_strcmp((s),"DTMF DECODE")==0 ? 1820 : \
    __builtin_strcmp((s),"DTMF ENTRY")==0 ? 1961 : \
    __builtin_strcmp((s),"EDIT QUICK MESSAGE")==0 ? 597 : \
    __builtin_strcmp((s),"EDIT QUICK MSGS")==0 ? 967 : \
    __builtin_strcmp((s),"EDIT RECIPIENT")==0 ? 1218 : \
    __builtin_strcmp((s),"EEPROM")==0 ? 2308 : \
    __builtin_strcmp((s),"EMPTY MESSAGE")==0 ? 1494 : \
    __builtin_strcmp((s),"EXIT")==0 ? 2449 : \
    __builtin_strcmp((s),"EXIT:DELETE  HOLD EXIT:CANCEL")==0 ? 0 : \
    __builtin_strcmp((s),"EXIT:INBOX")==0 ? 265 : \
    __builtin_strcmp((s),"EXIT:PACKETS")==0 ? 1659 : \
    __builtin_strcmp((s),"EXIT:STATIONS")==0 ? 1508 : \
    __builtin_strcmp((s),"EXPIRED")==0 ? 2241 : \
    __builtin_strcmp((s),"F")==0 ? 1709 : \
    __builtin_strcmp((s),"FAULT")==0 ? 2392 : \
    __builtin_strcmp((s),"FM, scan/DW off")==0 ? 983 : \
    __builtin_strcmp((s),"FREQUENCY ENTRY")==0 ? 999 : \
    __builtin_strcmp((s),"HEARD %us AGO")==0 ? 1522 : \
    __builtin_strcmp((s),"HOLD")==0 ? 2454 : \
    __builtin_strcmp((s),"ID:%4s")==0 ? 2315 : \
    __builtin_strcmp((s),"IDLE")==0 ? 2459 : \
    __builtin_strcmp((s),"LATITUDE DD MM.mm")==0 ? 691 : \
    __builtin_strcmp((s),"LIST%u")==0 ? 2322 : \
    __builtin_strcmp((s),"LK")==0 ? 2525 : \
    __builtin_strcmp((s),"LON DDD MM.mm")==0 ? 1536 : \
    __builtin_strcmp((s),"LOW BATTERY")==0 ? 1832 : \
    __builtin_strcmp((s),"Long press #")==0 ? 1672 : \
    __builtin_strcmp((s),"M")==0 ? 2313 : \
    __builtin_strcmp((s),"MENU")==0 ? 2464 : \
    __builtin_strcmp((s),"MENU   F+5:APRS   EXIT:A/B")==0 ? 30 : \
    __builtin_strcmp((s),"MENU/EXIT:STOP  UP/DN:DIR")==0 ? 57 : \
    __builtin_strcmp((s),"MENU:CANCEL EXIT:BACK")==0 ? 344 : \
    __builtin_strcmp((s),"MENU:DETAIL  EXIT:APRS")==0 ? 230 : \
    __builtin_strcmp((s),"MENU:NEXT  EXIT:BACK")==0 ? 476 : \
    __builtin_strcmp((s),"MENU:NEXT EXIT:CANCEL")==0 ? 366 : \
    __builtin_strcmp((s),"MENU:OPEN  EXIT:RADIO")==0 ? 388 : \
    __builtin_strcmp((s),"MENU:OPTIONS  EXIT:BACK")==0 ? 159 : \
    __builtin_strcmp((s),"MENU:READ  EXIT:APRS")==0 ? 497 : \
    __builtin_strcmp((s),"MENU:REPLY  EXIT:INBOX")==0 ? 253 : \
    __builtin_strcmp((s),"MENU:REPLY/NEW  EXIT:APRS")==0 ? 83 : \
    __builtin_strcmp((s),"MENU:SAVE  EXIT:CANCEL")==0 ? 276 : \
    __builtin_strcmp((s),"MENU:SAVE EXIT:CANCEL")==0 ? 410 : \
    __builtin_strcmp((s),"MENU:SELECT  EXIT:BODY")==0 ? 299 : \
    __builtin_strcmp((s),"MENU:SELECT EXIT:BACK")==0 ? 432 : \
    __builtin_strcmp((s),"MENU:SEND  EXIT:EDIT")==0 ? 518 : \
    __builtin_strcmp((s),"MENU:SEND EXIT:CANCEL")==0 ? 454 : \
    __builtin_strcmp((s),"MENU:SET CALL  EXIT:APRS")==0 ? 109 : \
    __builtin_strcmp((s),"MESSAGE FULL")==0 ? 1685 : \
    __builtin_strcmp((s),"MESSAGE OPTIONS")==0 ? 1015 : \
    __builtin_strcmp((s),"MESSAGE REJECTED")==0 ? 867 : \
    __builtin_strcmp((s),"MHz")==0 ? 1184 : \
    __builtin_strcmp((s),"MONITOR %u/%u")==0 ? 1550 : \
    __builtin_strcmp((s),"MSG %s")==0 ? 2329 : \
    __builtin_strcmp((s),"MULTI-TAP ABC/123")==0 ? 709 : \
    __builtin_strcmp((s),"MY CALL %s")==0 ? 1972 : \
    __builtin_strcmp((s),"MY CALLSIGN")==0 ? 1844 : \
    __builtin_strcmp((s),"N")==0 ? 595 : \
    __builtin_strcmp((s),"NEW MESSAGE")==0 ? 1856 : \
    __builtin_strcmp((s),"NOT SENT")==0 ? 2142 : \
    __builtin_strcmp((s),"NULL")==0 ? 2469 : \
    __builtin_strcmp((s),"No messages yet")==0 ? 1031 : \
    __builtin_strcmp((s),"No packets yet")==0 ? 1233 : \
    __builtin_strcmp((s),"No position yet")==0 ? 1047 : \
    __builtin_strcmp((s),"No stations yet")==0 ? 1063 : \
    __builtin_strcmp((s),"OFF")==0 ? 2504 : \
    __builtin_strcmp((s),"ON")==0 ? 594 : \
    __builtin_strcmp((s),"PACKETS")==0 ? 1664 : \
    __builtin_strcmp((s),"PAUSE")==0 ? 2398 : \
    __builtin_strcmp((s),"PERSONAL INBOX")==0 ? 1248 : \
    __builtin_strcmp((s),"PERSONAL INBOX %u")==0 ? 727 : \
    __builtin_strcmp((s),"PERSONAL MESSAGE")==0 ? 884 : \
    __builtin_strcmp((s),"POSITION NOT SET")==0 ? 901 : \
    __builtin_strcmp((s),"PRI%d:%u")==0 ? 2151 : \
    __builtin_strcmp((s),"PTT")==0 ? 2508 : \
    __builtin_strcmp((s),"PTT:RETURN TO VOICE")==0 ? 539 : \
    __builtin_strcmp((s),"Precision RF")==0 ? 1698 : \
    __builtin_strcmp((s),"QUEUED")==0 ? 2336 : \
    __builtin_strcmp((s),"QUICK MESSAGE")==0 ? 602 : \
    __builtin_strcmp((s),"QUICK MESSAGES")==0 ? 1263 : \
    __builtin_strcmp((s),"RADIO")==0 ? 153 : \
    __builtin_strcmp((s),"RADIO ALERT")==0 ? 1868 : \
    __builtin_strcmp((s),"READ\nMANUAL")==0 ? 1880 : \
    __builtin_strcmp((s),"RECIPIENT")==0 ? 1223 : \
    __builtin_strcmp((s),"RELEASE")==0 ? 2249 : \
    __builtin_strcmp((s),"REVIEW AND SEND")==0 ? 1079 : \
    __builtin_strcmp((s),"REVIEW BEACON")==0 ? 1564 : \
    __builtin_strcmp((s),"REVIEW POSITION")==0 ? 1095 : \
    __builtin_strcmp((s),"RX")==0 ? 2222 : \
    __builtin_strcmp((s),"RX/TX disabled")==0 ? 1278 : \
    __builtin_strcmp((s),"S")==0 ? 107 : \
    __builtin_strcmp((s),"S%u %ddBm Q%u %c")==0 ? 918 : \
    __builtin_strcmp((s),"S+W")==0 ? 2512 : \
    __builtin_strcmp((s),"SAVE DOES NOT SEND")==0 ? 616 : \
    __builtin_strcmp((s),"SCAN")==0 ? 2474 : \
    __builtin_strcmp((s),"SCAN %s %uc/s")==0 ? 1578 : \
    __builtin_strcmp((s),"SEND")==0 ? 630 : \
    __builtin_strcmp((s),"SEND BEACON")==0 ? 1892 : \
    __builtin_strcmp((s),"SESSION ONLY")==0 ? 1711 : \
    __builtin_strcmp((s),"SET YOUR CALLSIGN")==0 ? 745 : \
    __builtin_strcmp((s),"SETUP")==0 ? 224 : \
    __builtin_strcmp((s),"SHIFT %c%u.%03u %s")==0 ? 635 : \
    __builtin_strcmp((s),"SIDE1")==0 ? 2404 : \
    __builtin_strcmp((s),"SIDE2")==0 ? 2410 : \
    __builtin_strcmp((s),"SP")==0 ? 2528 : \
    __builtin_strcmp((s),"SQL%u %s %u.%ukHz")==0 ? 763 : \
    __builtin_strcmp((s),"STAR")==0 ? 2479 : \
    __builtin_strcmp((s),"STATIC BEACON")==0 ? 1592 : \
    __builtin_strcmp((s),"STATIC POSITION")==0 ? 1111 : \
    __builtin_strcmp((s),"STATIONS")==0 ? 1513 : \
    __builtin_strcmp((s),"STATIONS %u HEARD")==0 ? 781 : \
    __builtin_strcmp((s),"SURE?")==0 ? 2416 : \
    __builtin_strcmp((s),"Signal Quality")==0 ? 1293 : \
    __builtin_strcmp((s),"TIMEOUT")==0 ? 2257 : \
    __builtin_strcmp((s),"TO")==0 ? 2441 : \
    __builtin_strcmp((s),"TO %s")==0 ? 2422 : \
    __builtin_strcmp((s),"TX")==0 ? 1656 : \
    __builtin_strcmp((s),"TX %u:%02u")==0 ? 1983 : \
    __builtin_strcmp((s),"TX AUDIO 0-127")==0 ? 1308 : \
    __builtin_strcmp((s),"TX AUDIO LEVEL")==0 ? 1323 : \
    __builtin_strcmp((s),"TX BLOCKED")==0 ? 1994 : \
    __builtin_strcmp((s),"TX Compressor")==0 ? 1606 : \
    __builtin_strcmp((s),"TX DISABLE")==0 ? 2005 : \
    __builtin_strcmp((s),"TX FAILED")==0 ? 2057 : \
    __builtin_strcmp((s),"TX UNAVAILABLE")==0 ? 1338 : \
    __builtin_strcmp((s),"TX sag %umV")==0 ? 1904 : \
    __builtin_strcmp((s),"UNCONFIRMED")==0 ? 1916 : \
    __builtin_strcmp((s),"UP")==0 ? 227 : \
    __builtin_strcmp((s),"UP/DN:PACKET  EXIT:APRS")==0 ? 183 : \
    __builtin_strcmp((s),"UP/DN:PACKET  EXIT:RADIO")==0 ? 134 : \
    __builtin_strcmp((s),"UP/DOWN:HEMISPHERE")==0 ? 654 : \
    __builtin_strcmp((s),"VFO %c")==0 ? 2343 : \
    __builtin_strcmp((s),"VFO B ACTIVITY")==0 ? 1353 : \
    __builtin_strcmp((s),"VFO ERROR")==0 ? 2067 : \
    __builtin_strcmp((s),"VOLT HIGH")==0 ? 2077 : \
    __builtin_strcmp((s),"VOLTAGE")==0 ? 2265 : \
    __builtin_strcmp((s),"VUURWERK")==0 ? 2160 : \
    __builtin_strcmp((s),"W")==0 ? 2231 : \
    __builtin_strcmp((s),"WAIT!")==0 ? 2428 : \
    __builtin_strcmp((s),"WAITING FOR ACK")==0 ? 1127 : \
    __builtin_strcmp((s),"^")==0 ? 2533 : \
    __builtin_strcmp((s),"calib lost")==0 ? 2016 : \
    __builtin_strcmp((s),"to unlock")==0 ? 2087 : \
    -1)
#define DISPLAY_TEXT(s) (gDisplayText + DISPLAY_TEXT_OFFSET(s) + \
    0*sizeof(char[DISPLAY_TEXT_OFFSET(s)>=0 ? 1 : -1]))
#endif
