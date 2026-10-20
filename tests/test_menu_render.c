/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/menu.h"
#include "ui/menu.h"
#include "ui/vuurwerk_menu.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"
#include "settings.h"
#include "misc.h"
#include "helper/battery.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
void REFERENCE_UI_DisplayMenu(void);
EEPROM_Config_t gEeprom;
uint8_t gAskForConfirmation;
uint8_t gFrameBuffer[FRAME_LINES][LCD_WIDTH], gStatusLine[LCD_WIDTH];
const char gModulationStr[MODULATION_UKNOWN][4] = {"FM", "AM", "USB"};
static unsigned trace[32], ntrace;
static void Record(unsigned v) { assert(ntrace<32); trace[ntrace++]=v; }
void ST7565_BlitFullScreen(void) { Record(1); }
void BACKLIGHT_TurnOn(void) { Record(2); }
void BACKLIGHT_SetBrightness(uint8_t b) { Record(100+b); }
uint8_t BACKLIGHT_GetBrightness(void) { return 8; }
void BK4819_EnableScramble(uint8_t b) { Record(200+b); }
void BK4819_DisableScramble(void) { Record(300); }
bool RADIO_CheckValidChannel(uint16_t ch, bool scan, uint8_t list) { (void)scan;(void)list;return ch<200 && ch%3; }
uint32_t SETTINGS_FetchChannelFrequency(int ch) { return 14400000u+ch*2500u; }
void SETTINGS_FetchChannelName(char *s,int ch) { strcpy(s,ch>=0 && ch%2?"TEST CH":""); }
static unsigned cases;
static bool visited[256];
static void Compare(void) {
    uint8_t pixels[sizeof(gFrameBuffer)]; unsigned expected[32];
    ntrace=0; REFERENCE_UI_DisplayMenu();
    memcpy(pixels,gFrameBuffer,sizeof pixels); memcpy(expected,trace,sizeof trace); unsigned count=ntrace;
    ntrace=0; UI_DisplayMenu();
    if(memcmp(pixels,gFrameBuffer,sizeof pixels)||count!=ntrace||memcmp(expected,trace,count*sizeof *trace)) {
        fprintf(stderr,"menu=%d value=%d edit=%d confirm=%d differs\n",UI_MENU_GetCurrentMenuId(),gSubMenuSelection,gIsInSubMenu,gAskForConfirmation); assert(0);
    }
    if (!gMenuShowCategoryPicker) visited[UI_MENU_GetCurrentMenuId()]=true;
    ++cases;
}
int main(void) {
    gBatteryVoltageAverage=750; gBatteryCalibration[3]=1900;
    gEeprom.SCAN_LIST_ENABLED[0]=gEeprom.SCAN_LIST_ENABLED[1]=true;
    gEeprom.SCANLIST_PRIORITY_CH1[0]=2; gEeprom.SCANLIST_PRIORITY_CH2[0]=4;
    strcpy(edit,"TEST EDIT"); memset(gInputBox,10,sizeof gInputBox);
    while(MenuList[gMenuListCount].menu_id!=0xff) ++gMenuListCount;
    VUURWERK_MENU_Init();
    for(unsigned cat=0;cat<MCAT_COUNT;cat++) {
        gSelectedCategory=cat; VUURWERK_MENU_SelectCategory(cat); gMenuShowCategoryPicker=false;
        for(unsigned idx=0;idx<gCategoryItemCount;idx++) {
            gCategoryItemIdx=idx; VUURWERK_MENU_SyncCursor();
            int32_t lo=0,hi=0; if(MENU_GetLimits(UI_MENU_GetCurrentMenuId(),&lo,&hi)) lo=hi=0;
            for(int32_t v=lo;v<=hi;v+=(hi-lo>1000? (hi-lo)/4:1)) {
                gSubMenuSelection=v;
                for(unsigned mode=0;mode<6;mode++) {
                    gIsInSubMenu=mode&1;gAskForConfirmation=mode/2; edit_index=mode;gInputBoxIndex=mode;
                    Compare();
                }
            }
        }
    }
    /* Cover entries outside the category window too, including hidden settings. */
    for(unsigned idx=0;idx<gMenuListCount;idx++) {
        if(visited[MenuList[idx].menu_id]) continue;
        gSelectedCategory=-1;gCategoryItemCount=1;gCategoryItemIdx=0;gCategoryItems[0]=idx;gMenuCursor=idx;
        int32_t lo=0,hi=0; if(MENU_GetLimits(UI_MENU_GetCurrentMenuId(),&lo,&hi)) lo=hi=0;
        for(int32_t v=lo;v<=hi;v+=(hi-lo>1000?(hi-lo)/4:1)) {
            gSubMenuSelection=v;
            for(unsigned mode=0;mode<6;mode++) {gIsInSubMenu=mode&1;gAskForConfirmation=mode/2;edit_index=(int)mode-1;gInputBoxIndex=mode;Compare();}
        }
    }
    for(unsigned idx=0;idx<gMenuListCount;idx++) assert(visited[MenuList[idx].menu_id]);
    gMenuShowCategoryPicker=true;
    for(gCategoryPickerCursor=0;gCategoryPickerCursor<VUURWERK_MENU_GetVisibleCategoryCount();gCategoryPickerCursor++) Compare();
    assert(cases>10000);
    printf("PASS %u real menu frames and hardware preview traces match original renderer\n",cases);
}
