/* SPDX-License-Identifier: GPL-3.0-only
 * Experimental normal-radio TX acquisition, PA ramp and guaranteed RX restore.
 * No user interface or default task calls this code. No remote hardware access.
 */
#include "aprs_radio_owner.h"
#include "bell202_radio.h"
#include "app/aprs_rx.h"
#include "app/chFrScanner.h"
#include "app/scanner.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/systick.h"
#include "driver/system.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "vfo_split.h"
#include "tx_compressor.h"
#include "tx_soft_start.h"
#include "ctcss_lead.h"
static bool Cancelled(void)
{
    return gTxTimeoutReached || !GPIO_CheckBit(&GPIOC->DATA,GPIOC_PIN_PTT);
}
APRS_LinkResult APRS_RadioSend(const uint8_t *frame,size_t length,uint32_t frequency,void *context)
{
    const APRS_RadioConfig *config=context;
    if(!frame || length<16 || length>100 || !config || !config->enabled ||
       config->toneGain>127 || !SYSTICK_CanCountCycles() || SerialConfigInProgress() ||
       gEeprom.TX_VFO>1 || gEeprom.RX_VFO>1) return APRS_LINK_DENIED;
    VFO_Info_t *v=&gEeprom.VfoInfo[gEeprom.TX_VFO];
    if(!frequency || gRxVfo!=v || !v->pRX || !v->pTX ||
       v->pRX->Frequency!=v->pTX->Frequency || frequency!=v->pTX->Frequency ||
       v->Modulation!=MODULATION_FM || v->pTX->CodeType!=CODE_TYPE_OFF ||
       (v->SCRAMBLING_TYPE && gSetting_ScrambleEnable) || v->DTMF_PTT_ID_TX_MODE!=PTT_ID_OFF ||
       gEeprom.DUAL_WATCH!=DUAL_WATCH_OFF || gEeprom.CROSS_BAND_RX_TX!=CROSS_BAND_OFF ||
       gScanStateDir!=SCAN_OFF || SCANNER_IsScanning() || gCssBackgroundScan ||
       gVfoSplit.mode!=SPLIT_OFF) return APRS_LINK_DENIED;
    if(gCurrentFunction!=FUNCTION_FOREGROUND || g_SquelchLost || APRS_IsCapturing() ||
       !GPIO_CheckBit(&GPIOC->DATA,GPIOC_PIN_PTT) ||
       (BK4819_ReadRegister(BK4819_REG_0C)&1u)) return APRS_LINK_BUSY;
    APRS_Stop();
    /* Preserve normal band locks, battery/overvoltage and TX timeout checks.
     * PTT ID/code/scrambler prerequisites above keep startup free of ID tones.
     */
    RADIO_PrepareTX();
    if(gCurrentFunction!=FUNCTION_TRANSMIT) {APRS_Resume();return APRS_LINK_DENIED;}
    TX_COMPRESSOR_Stop();CTCSS_LEAD_Stop();BK4819_EnterTxMute();
    BELL202_Result result=BELL202_CANCELLED;
    for(unsigned i=0;i<TX_RAMP_STEPS && !Cancelled();i++) {
        SYSTEM_DelayMs(10);
        if(Cancelled()) break;
        TX_SOFT_START_Process();
    }
    if(!Cancelled() && !gTxSoftStart.active)
        result=BELL202_Radio(frame,length,config->toneGain);
    /* Every path after successful TX acquisition drops PA and restores RX.
     * Do not use the voice end-of-TX path: it can append roger/DTMF/tail tones.
     */
    gTxSoftStart.active=false;
    TX_COMPRESSOR_Stop();CTCSS_LEAD_Stop();BK4819_EnterTxMute();
    BK4819_SetupPowerAmplifier(0,v->pTX->Frequency);
    BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE,false);
    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED,false);
    gTxTimerCountdown_500ms=0;gTxTimeoutReached=false;gFlagEndTransmission=false;
    FUNCTION_Select(FUNCTION_FOREGROUND);
    RADIO_SetupRegisters(false);
    /* Listen for a prompt peer ACK before returning to the queued transport.
     * Do not wait for a coalesced foreground 500 ms tick after a blocking burst.
     */
    APRS_Resume();
    gUpdateDisplay=true;gUpdateStatus=1;
    return result==BELL202_SENT ? APRS_LINK_SENT : APRS_LINK_FAULT;
}
