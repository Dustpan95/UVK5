/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/menu.h"
#include "ui/menu.h"
#include "misc.h"
#include "settings.h"
#include "radio.h"
#include "scanwatch.h"
#include "driver/bk4819.h"
#include "ui/ui.h"
#include "app/aprs_rx.h"
#include "app/chFrScanner.h"
#include "dtmf.h"
#include "helper/battery.h"
EEPROM_Config_t gEeprom;
VFO_Info_t *gTxVfo,*gRxVfo,*gCurrentVfo;
int REFERENCE_MENU_GetLimits(uint8_t id,int32_t *minimum,int32_t *maximum);
void REFERENCE_MENU_AcceptSetting(void);
void REFERENCE_MENU_ShowCurrentSetting(void);
typedef struct {
    uint8_t tx350,tx200,tx500,backlight,battery,enable350,scramble,am;
    bool reset,reconfigure;
} Globals;
static void SetupGlobals(unsigned seed) {
    gSetting_350TX=gSetting_200TX=gSetting_500TX=seed;
    gSetting_backlight_on_tx_rx=seed ? 3:1;gSetting_battery_text=seed ? 2:0;
    gSetting_350EN=gSetting_ScrambleEnable=seed;
#ifdef ENABLE_AM_FIX
    gSetting_AM_fix=seed;
#endif
    gFlagResetVfos=gFlagReconfigureVfos=seed;
}
static Globals SnapshotGlobals(void) {
    Globals g={gSetting_350TX,gSetting_200TX,gSetting_500TX,gSetting_backlight_on_tx_rx,gSetting_battery_text,gSetting_350EN,gSetting_ScrambleEnable,
#ifdef ENABLE_AM_FIX
    gSetting_AM_fix,
#else
    0,
#endif
    gFlagResetVfos,gFlagReconfigureVfos};
    return g;
}
static unsigned tested;
int main(void) {
    for(unsigned id=0;id<256;id++) {
        int32_t min_a=-1234,max_a=4321,min_b=min_a,max_b=max_a;
        int a=REFERENCE_MENU_GetLimits(id,&min_a,&max_a),b=MENU_GetLimits(id,&min_b,&max_b);
        assert(a==b && min_a==min_b && max_a==max_b);
    }
    const uint8_t fields[]={MENU_SQL,MENU_SAVE,MENU_ABR,MENU_BEEP,MENU_TOT,MENU_SC_REV,MENU_MDF,MENU_STE,MENU_RP_STE,MENU_1_CALL,MENU_S_LIST,MENU_D_ST,MENU_PONMSG,MENU_ROGER,MENU_BATTYP,MENU_TXP,MENU_SFT_D,MENU_W_N,MENU_SCR,MENU_BCL,MENU_PTT_ID,MENU_AM,MENU_F1SHRT,MENU_F1LONG,MENU_F2SHRT,MENU_F2LONG,MENU_MLONG,MENU_350TX,MENU_200TX,MENU_500TX,MENU_ABR_ON_TX_RX,MENU_BAT_TXT,MENU_350EN,MENU_SCREN,MENU_AM_FIX,MENU_R_DCS,MENU_T_DCS,MENU_R_CTCS,MENU_T_CTCS};
    for(unsigned which=0;which<2;which++) {
        gTxVfo=&gEeprom.VfoInfo[which];
        for(unsigned id=0;id<sizeof fields;id++) {
            gMenuCursor=UI_MENU_GetMenuIdx(fields[id]);
            const bool tone=fields[id]==MENU_R_DCS || fields[id]==MENU_T_DCS || fields[id]==MENU_R_CTCS || fields[id]==MENU_T_CTCS;
            for(unsigned type=0;type<(tone ? 5u:1u);type++)
            for(unsigned seed=0;seed<2;seed++) for(int value=-2;value<256;value++) {
                memset(&gEeprom,0x55,sizeof gEeprom);SetupGlobals(seed);
                gTxVfo->freq_config_RX.CodeType=gTxVfo->freq_config_TX.CodeType=type==4 ? 255:type;
                gTxVfo->freq_config_RX.Code=gTxVfo->freq_config_TX.Code=(uint8_t)(value+73+seed);
                gSubMenuSelection=value;gRequestSaveSettings=seed;gRequestSaveChannel=seed ? 2:0;gVfoConfigureMode=seed ? VFO_CONFIGURE:VFO_CONFIGURE_RELOAD;
                REFERENCE_MENU_AcceptSetting();
                EEPROM_Config_t expected=gEeprom;Globals expected_globals=SnapshotGlobals();
                int32_t selection=gSubMenuSelection;
                bool save=gRequestSaveSettings;unsigned channel=gRequestSaveChannel,mode=gVfoConfigureMode;
                memset(&gEeprom,0x55,sizeof gEeprom);SetupGlobals(seed);
                gTxVfo->freq_config_RX.CodeType=gTxVfo->freq_config_TX.CodeType=type==4 ? 255:type;
                gTxVfo->freq_config_RX.Code=gTxVfo->freq_config_TX.Code=(uint8_t)(value+73+seed);
                gSubMenuSelection=value;gRequestSaveSettings=seed;gRequestSaveChannel=seed ? 2:0;gVfoConfigureMode=seed ? VFO_CONFIGURE:VFO_CONFIGURE_RELOAD;
                MENU_AcceptSetting();
                assert(!memcmp(&gEeprom,&expected,sizeof gEeprom));
                Globals actual_globals=SnapshotGlobals();assert(!memcmp(&actual_globals,&expected_globals,sizeof actual_globals));
                assert(selection==gSubMenuSelection && save==gRequestSaveSettings && channel==gRequestSaveChannel && mode==gVfoConfigureMode);
                REFERENCE_MENU_ShowCurrentSetting(); selection=gSubMenuSelection;
                gSubMenuSelection=-123;MENU_ShowCurrentSetting();assert(selection==gSubMenuSelection);tested++;
            }
        }
    }
    printf("PASS %u menu field edits/getters match original settings and save/configuration flags\n",tested);
}
/* No selected field operation may reach hardware or persistent storage. */
char gDTMF_RX_live[20];uint8_t gDTMF_RX_live_timeout,gScanWatchArmed;int8_t gScanStateDir;
uint16_t gBatteryCalibration[6];bool gAPRS_AlertSound,gAPRS_Enabled;
void SETTINGS_SaveChannelName(uint8_t channel,const char *name) {(void)channel;(void)name;assert(0);}
void SETTINGS_UpdateChannel(uint8_t channel,const VFO_Info_t *v,bool keep) {(void)channel;(void)v;(void)keep;assert(0);}
void SETTINGS_LoadCalibration(void) {assert(0);}
void SETTINGS_FactoryReset(bool all) {(void)all;assert(0);}
void SETTINGS_SaveBatteryCalibration(const uint16_t *c) {(void)c;assert(0);}
void BK4819_DisableDTMF(void) {assert(0);}
void APRS_Stop(void) {assert(0);}
void SCANWATCH_Disable(void) {assert(0);}
void SCANWATCH_Enable(uint8_t vfo) {(void)vfo;assert(0);}
uint8_t RADIO_FindNextChannel(uint8_t channel,int8_t direction,bool scan,uint8_t radio) {(void)channel;(void)direction;(void)scan;(void)radio;assert(0);return 0;}
