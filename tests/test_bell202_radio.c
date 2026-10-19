/* SPDX-License-Identifier: GPL-3.0-only */
#include "experiments/bell202_radio.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "bsp/dp32g030/gpio.h"
#include "functions.h"
#include "radio.h"
#include "tx_soft_start.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
TestRadioGPIO testRadioGPIO;
FUNCTION_Type_t gCurrentFunction;
bool gSetting_ScrambleEnable;
volatile bool gTxTimeoutReached;
TxSoftStart_t gTxSoftStart;
static VFO_Info_t vfo;
static FREQ_Config_t frequency;
VFO_Info_t *gCurrentVfo=&vfo;
static uint16_t regs[128],original[128];
static uint32_t clock;
static unsigned writes,tones,abortAt,slowAt,compressorStops,leadStops;
static bool timeout,earlyAbort,clockReady;
uint32_t SYSTICK_Cycles(void) {clock+=100;return clock;}
bool SYSTICK_CanCountCycles(void) {return clockReady;}
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t reg) {return regs[reg];}
void BK4819_WriteRegister(BK4819_REGISTER_t reg,uint16_t value)
{
    regs[reg]=value;++writes;
    if(reg==BK4819_REG_71 && regs[0x50]==0x3b20) {
        assert(value==12389 || value==22714);++tones;
        assert(regs[0x70]==(0x8000|24<<8) && !(regs[0x31]&8));
        if(tones==abortAt) {
            if(timeout) gTxTimeoutReached=true;
            else testRadioGPIO.DATA=0;
        }
        if(tones==slowAt) clock+=9000;
    }
    clock+=3600;
}
void BK4819_EnterTxMute(void) {BK4819_WriteRegister(BK4819_REG_50,0xbb20);}
void BK4819_ExitTxMute(void) {BK4819_WriteRegister(BK4819_REG_50,0x3b20);}
void BK4819_EnableTXLink(void) {BK4819_WriteRegister(BK4819_REG_30,0xc3fa);}
void BK4819_SetAF(BK4819_AF_Type_t af) {assert(af==BK4819_AF_MUTE);BK4819_WriteRegister(BK4819_REG_47,0x6040);}
void BK4819_SetCompander(unsigned mode) {assert(!mode);BK4819_WriteRegister(BK4819_REG_31,regs[0x31]&~8u);}
void SYSTEM_DelayMs(uint32_t ms) {assert(ms==50);clock+=ms*48000u;if(earlyAbort) testRadioGPIO.DATA=0;}
void TX_COMPRESSOR_Stop(void) {++compressorStops;}
void CTCSS_LEAD_Stop(void) {++leadStops;}
static void Init(void)
{
    memset(&vfo,0,sizeof(vfo));memset(&frequency,0,sizeof(frequency));
    vfo.pTX=&frequency;vfo.Modulation=MODULATION_FM;gCurrentVfo=&vfo;
    gCurrentFunction=FUNCTION_TRANSMIT;testRadioGPIO.DATA=1u<<GPIOC_PIN_PTT;
    gTxTimeoutReached=gSetting_ScrambleEnable=gTxSoftStart.active=false;
    for(unsigned i=0;i<128;i++) regs[i]=original[i]=0x1234u+i;
    clock=0;writes=tones=abortAt=slowAt=compressorStops=leadStops=0;
    timeout=earlyAbort=false;clockReady=true;
}
static void Restored(void)
{
    const uint8_t saved[]={0x70,0x71,0x30,0x47,0x31};
    for(unsigned i=0;i<sizeof(saved);i++) assert(regs[saved[i]]==original[saved[i]]);
    assert(regs[0x50]==0xbb20 && compressorStops==1 && leadStops==1);
}
int main(void)
{
    uint8_t frame[16]={0};Init();
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_SENT);
    assert(tones>512);Restored();
    Init();abortAt=37;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_CANCELLED && tones==37);Restored();
    Init();abortAt=37;timeout=true;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_CANCELLED && tones==37);Restored();
    Init();slowAt=37;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_WRITE_SLOW && tones==37);Restored();
    Init();earlyAbort=true;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_CANCELLED && !tones);Restored();
    Init();gCurrentFunction=FUNCTION_FOREGROUND;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    Init();vfo.Modulation=MODULATION_AM;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    Init();gTxSoftStart.active=true;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    Init();frequency.CodeType=CODE_TYPE_CONTINUOUS_TONE;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    Init();vfo.SCRAMBLING_TYPE=1;gSetting_ScrambleEnable=true;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    Init();vfo.pTX=0;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    Init();testRadioGPIO.DATA=0;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_CANCELLED && !writes);
    Init();assert(BELL202_Radio(frame,sizeof(frame),128)==BELL202_INVALID && !writes);
    Init();clockReady=false;
    assert(BELL202_Radio(frame,sizeof(frame),24)==BELL202_NOT_READY && !writes);
    puts("PASS actual tone backend mark/space words, TX prerequisites, mute/register restoration on success/PTT/timeout/timing faults");
}
