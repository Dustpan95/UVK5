/* SPDX-License-Identifier: GPL-3.0-only
 * Experimental original-DP32G030/BK4819 Bell 202 tone backend.
 * Ordinary tone generator replaces the incompatible 1200/1800 FSK TX engine.
 * Register conventions follow the existing complete firmware BK4819 driver.
 * No normal build or user interface calls this function yet.
 */
#include "bell202_radio.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/systick.h"
#include "driver/system.h"
#include "bsp/dp32g030/gpio.h"
#include "functions.h"
#include "misc.h"
#include "radio.h"
#include "tx_compressor.h"
#include "tx_soft_start.h"
#include "ctcss_lead.h"
static bool Cancelled(void *context)
{
    (void)context;
    return gCurrentFunction!=FUNCTION_TRANSMIT || gTxTimeoutReached ||
        !GPIO_CheckBit(&GPIOC->DATA,GPIOC_PIN_PTT);
}
static uint32_t Now(void *context) {(void)context;return SYSTICK_Cycles();}
static void Tone(bool high,void *context)
{
    (void)context;
    /* Same 10.3244 register-units-per-Hz scaling convention as the existing driver. */
    BK4819_WriteRegister(BK4819_REG_71,high ? 22714 : 12389);
}
BELL202_Result BELL202_Radio(const uint8_t *frame,size_t length,uint8_t gain)
{
    if(!frame || length<16 || length>330 || gain>127) return BELL202_INVALID;
    if(!SYSTICK_CanCountCycles() || !gCurrentVfo || !gCurrentVfo->pTX || gCurrentFunction!=FUNCTION_TRANSMIT ||
       gCurrentVfo->Modulation!=MODULATION_FM || gTxSoftStart.active ||
       gCurrentVfo->pTX->CodeType!=CODE_TYPE_OFF ||
       (gCurrentVfo->SCRAMBLING_TYPE && gSetting_ScrambleEnable)) return BELL202_NOT_READY;
    if(Cancelled(0)) return BELL202_CANCELLED;
    TX_COMPRESSOR_Stop();CTCSS_LEAD_Stop();
    const uint8_t registers[]={0x70,0x71,0x30,0x47,0x31};
    uint16_t saved[sizeof(registers)];
    for(unsigned i=0;i<sizeof(registers);i++) saved[i]=BK4819_ReadRegister(registers[i]);
    BK4819_EnterTxMute();
    BK4819_SetCompander(0);
    BK4819_SetAF(BK4819_AF_MUTE);
    BK4819_WriteRegister(BK4819_REG_70,BK4819_REG_70_ENABLE_TONE1 | ((uint16_t)gain<<8));
    BK4819_EnableTXLink();
    Tone(false,0);
    SYSTEM_DelayMs(50);
    BELL202_Result result=BELL202_CANCELLED;
    if(!Cancelled(0)) {
        BK4819_ExitTxMute();
        static const BELL202_Output output={Now,Tone,Cancelled,0};
        result=BELL202_Transmit(frame,length,&output);
    }
    /* No roger, DTMF or subaudible tail. Remain muted until owner drops PA. */
    BK4819_EnterTxMute();
    for(unsigned i=0;i<sizeof(registers);i++) BK4819_WriteRegister(registers[i],saved[i]);
    return result;
}
