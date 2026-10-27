/* SPDX-License-Identifier: GPL-3.0-only
 * Temporarily configure a RAM VFO for the selected APRS frequency, retaining
 * normal calibration and TX admission. Restore the complete saved VFO afterward.
 * Bound by the optional transport runtime. No hardware is accessed remotely.
 */
#include "aprs_hardware.h"
#include "app/app.h"
#include "app/chFrScanner.h"
#include "app/scanner.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "functions.h"
#include "misc.h"
#include "settings.h"
#include "vfo_split.h"
#include "tx_compressor.h"
#include "tx_soft_start.h"
#include "ctcss_lead.h"
extern uint32_t SCHEDULER_GetTicks10ms(void);
uint32_t HARDWARE_Now(void *context) {(void)context;return SCHEDULER_GetTicks10ms();}
bool HARDWARE_Interrupted(void *context)
{
    APRS_Hardware *h=context;
    return !h || !h->radio.enabled || (h->leased &&
        (gEeprom.TX_VFO!=h->index || gEeprom.RX_VFO!=h->index ||
         gRxVfo!=&gEeprom.VfoInfo[h->index] || gTxVfo!=gRxVfo || gCurrentVfo!=gRxVfo)) || SerialConfigInProgress() || APP_HasPendingWrites() ||
        !GPIO_CheckBit(&GPIOC->DATA,GPIOC_PIN_PTT) || gScanStateDir!=SCAN_OFF ||
        SCANNER_IsScanning() || gCssBackgroundScan || VFO_SPLIT_IsEnabled() ||
        gEeprom.DUAL_WATCH!=DUAL_WATCH_OFF || gEeprom.CROSS_BAND_RX_TX!=CROSS_BAND_OFF;
}
void HARDWARE_Restore(void *context)
{
    APRS_Hardware *h=context;
    if(!h || !h->leased) return;
    /* Drop RF before restoring any voice frequency or code. */
    gTxSoftStart.active=false;TX_COMPRESSOR_Stop();CTCSS_LEAD_Stop();BK4819_EnterTxMute();
    BK4819_SetupPowerAmplifier(0,gEeprom.VfoInfo[h->index].pTX->Frequency);
    BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE,false);
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED,false);
    VFO_Info_t *v=&gEeprom.VfoInfo[h->index];*v=h->voice;
    v->pRX=h->voice.pRX==&h->voice.freq_config_RX ? &v->freq_config_RX : &v->freq_config_TX;
    v->pTX=h->voice.pTX==&h->voice.freq_config_RX ? &v->freq_config_RX : &v->freq_config_TX;
    VfoState[h->index]=h->voiceState;
    gEeprom.TX_VFO=gEeprom.RX_VFO=h->index;
    gTxVfo=gRxVfo=gCurrentVfo=&gEeprom.VfoInfo[h->index];
    gRxVfoIsActive=h->rxActive;
    gTxTimerCountdown_500ms=0;gTxTimeoutReached=false;gFlagEndTransmission=false;
    FUNCTION_Select(FUNCTION_FOREGROUND);RADIO_SetupRegisters(false);
    h->leased=false;gUpdateDisplay=true;gUpdateStatus=1;
}
APRS_ChannelAdmission HARDWARE_Enter(uint32_t frequency,void *context)
{
    APRS_Hardware *h=context;
    if(!h || h->leased || !h->radio.enabled || h->radio.toneGain>127 ||
       h->power>OUTPUT_POWER_HIGH || gEeprom.TX_VFO>1 || gEeprom.RX_VFO!=gEeprom.TX_VFO ||
       !frequency || RX_freq_check(frequency) || TX_freq_check(frequency)) return APRS_CHANNEL_DENIED;
    VFO_Info_t *v=&gEeprom.VfoInfo[gEeprom.TX_VFO];
    if(gRxVfo!=v || gTxVfo!=v || gCurrentVfo!=v ||
       (v->pRX!=&v->freq_config_RX && v->pRX!=&v->freq_config_TX) ||
       (v->pTX!=&v->freq_config_RX && v->pTX!=&v->freq_config_TX)) return APRS_CHANNEL_DENIED;
    if(HARDWARE_Interrupted(h) || gCurrentFunction!=FUNCTION_FOREGROUND || g_SquelchLost ||
       (BK4819_ReadRegister(BK4819_REG_0C)&1u)) return APRS_CHANNEL_BUSY;
    h->index=gEeprom.TX_VFO;h->voice=*v;h->voiceState=VfoState[h->index];
    h->voice.pRX=v->pRX==&v->freq_config_RX ? &h->voice.freq_config_RX : &h->voice.freq_config_TX;
    h->voice.pTX=v->pTX==&v->freq_config_RX ? &h->voice.freq_config_RX : &h->voice.freq_config_TX;
    h->rxActive=gRxVfoIsActive;
    h->leased=true;
    RADIO_InitInfo(v,h->voice.CHANNEL_SAVE,frequency);
    v->Modulation=MODULATION_FM;v->OUTPUT_POWER=h->power;
    RADIO_ConfigureSquelchAndOutputPower(v);
    gRxVfoIsActive=true;
    RADIO_SetupRegisters(false);
    if(HARDWARE_Interrupted(h)) {HARDWARE_Restore(h);return APRS_CHANNEL_BUSY;}
    return APRS_CHANNEL_READY;
}
APRS_LinkResult HARDWARE_Send(const uint8_t *frame,size_t length,uint32_t frequency,void *context)
{
    APRS_Hardware *h=context;
    if(!h || !h->leased || h->index!=gEeprom.TX_VFO) return APRS_LINK_DENIED;
    if(HARDWARE_Interrupted(h)) return APRS_LINK_BUSY;
    return APRS_RadioSend(frame,length,frequency,&h->radio);
}
