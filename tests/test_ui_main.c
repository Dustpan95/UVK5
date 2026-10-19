/* SPDX-License-Identifier: GPL-3.0-only
 * Exercise actual home dispatcher and calibrated signal conversion.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "misc.h"
#include "ui/ui.h"
#include "ui/main.h"
#include "ui/helper.h"
#include "helper/battery.h"
EEPROM_Config_t gEeprom;
VFO_Info_t *gRxVfo=&gEeprom.VfoInfo[0];
GUI_DisplayType_t gScreenToDisplay;
bool gLowBattery,gLowBatteryConfirmed,gAPRS_Browser,gUpdateDisplay;
uint8_t gKeypadLocked;
static unsigned home_calls,aprs_calls,lock_lines;
static bool home_ok=true;
static char popup[24];
int16_t BK4819_GetRSSI_dBm(void) {return -87;}
int8_t GAIN_STAGING_GetGainDiff(uint8_t vfo) {(void)vfo;return 0;}
bool UI_HomeDraw(void) {++home_calls;return home_ok;}
void APRS_Draw(void) {++aprs_calls;}
void UI_DisplayClear(void) {popup[0]=0;}
void ST7565_BlitFullScreen(void) {}
void UI_DisplayPopup(const char *s) {strcpy(popup,s);}
void UI_PrintString(const char *s,uint8_t a,uint8_t b,uint8_t c,uint8_t d) {
    (void)s;(void)a;(void)b;(void)c;(void)d;++lock_lines;
}
int main(void) {
    gAPRS_Browser=true;gLowBattery=true;UI_DisplayMain();
    assert(!strcmp(popup,"LOW BATTERY") && !aprs_calls && !home_calls);
    gLowBattery=false;gEeprom.KEY_LOCK=true;gKeypadLocked=1;UI_DisplayMain();
    assert(lock_lines==2 && !aprs_calls && !home_calls);
    gKeypadLocked=0;UI_DisplayMain();assert(aprs_calls==1 && !home_calls);
    gAPRS_Browser=false;UI_DisplayMain();assert(home_calls==1 && !popup[0]);
    home_ok=false;UI_DisplayMain();assert(!strcmp(popup,"VFO ERROR"));
    gScreenToDisplay=DISPLAY_MENU;UI_MAIN_TimeSlice500ms();assert(!gUpdateDisplay);
    gScreenToDisplay=DISPLAY_MAIN;UI_MAIN_TimeSlice500ms();assert(gUpdateDisplay);
    gRxVfo->Modulation=MODULATION_FM;gRxVfo->Band=0;
    gEeprom.S0_LEVEL=130;gEeprom.S9_LEVEL=76;uint8_t level=0;
    assert(UI_RX_Signal_dBm(&level)==-102 && level==4);
    gEeprom.S0_LEVEL=gEeprom.S9_LEVEL=100;
    UI_RX_Signal_dBm(&level);assert(!level);
    gEeprom.S0_LEVEL=50;UI_RX_Signal_dBm(&level);assert(!level);
    puts("PASS actual main-screen warning/lock/APRS priority, refresh and degenerate RSSI calibration");
}
