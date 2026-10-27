/* SPDX-License-Identifier: GPL-3.0-only */
#include "experiments/aprs_radio_owner.h"
#include "experiments/bell202_radio.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "bsp/dp32g030/gpio.h"
#include "functions.h"
#include "settings.h"
#include "tx_soft_start.h"
#include "vfo_split.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
EEPROM_Config_t gEeprom;
VFO_Info_t *gRxVfo,*gCurrentVfo;
FUNCTION_Type_t gCurrentFunction;
TestRadioGPIO testRadioGPIO;
TxSoftStart_t gTxSoftStart;
VfoSplit_t gVfoSplit;
bool gSetting_ScrambleEnable,gCssBackgroundScan,g_SquelchLost,gFlagEndTransmission,gUpdateDisplay;
volatile bool gTxTimeoutReached;
volatile uint16_t gTxTimerCountdown_500ms;
volatile uint8_t gSerialConfigCountDown_500ms;
uint8_t gUpdateStatus;
int8_t gScanStateDir;
static bool clockReady,scanning,allowed,pending,pa,red,muted;
static unsigned prepare,ramp,delays,abortAt,restore,modem,compressorStops;
static BELL202_Result modemResult;
bool SYSTICK_CanCountCycles(void) {return clockReady;}
bool SCANNER_IsScanning(void) {return scanning;}
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t reg) {assert(reg==BK4819_REG_0C);return pending;}
void RADIO_PrepareTX(void)
{
    ++prepare;
    if(!allowed) return;
    gCurrentFunction=FUNCTION_TRANSMIT;gCurrentVfo=gRxVfo;
    gTxSoftStart.active=true;pa=red=true;gTxTimerCountdown_500ms=60;
}
void TX_COMPRESSOR_Stop(void) {++compressorStops;}
void CTCSS_LEAD_Stop(void) {}
void BK4819_EnterTxMute(void) {muted=true;}
void SYSTEM_DelayMs(uint32_t ms)
{
    assert(ms==10 && muted && pa);++delays;
    if(delays==abortAt) testRadioGPIO.DATA=0;
}
void TX_SOFT_START_Process(void)
{
    assert(pa && muted);++ramp;
    if(ramp==TX_RAMP_STEPS) gTxSoftStart.active=false;
}
BELL202_Result BELL202_Radio(const uint8_t *frame,size_t length,uint8_t gain)
{
    assert(frame && length==16 && gain==24 && muted && pa);
    assert(!gTxSoftStart.active && ramp==TX_RAMP_STEPS);
    ++modem;return modemResult;
}
void BK4819_SetupPowerAmplifier(const uint8_t bias,const uint32_t frequency)
{
    assert(!bias && frequency==14439000 && muted);
}
void BK4819_ToggleGpioOut(BK4819_GPIO_PIN_t pin,bool on)
{
    assert(!on && muted);
    if(pin==BK4819_GPIO1_PIN29_PA_ENABLE) pa=on;
    else {assert(pin==BK4819_GPIO5_PIN1_RED);red=on;}
}
void FUNCTION_Select(FUNCTION_Type_t function)
{
    assert(function==FUNCTION_FOREGROUND && !pa && !red && muted);
    gCurrentFunction=function;
}
void RADIO_SetupRegisters(bool foreground)
{
    assert(!foreground && !pa && !red && !gTxSoftStart.active && muted);
    assert(gCurrentFunction==FUNCTION_FOREGROUND);++restore;
}
static void Init(void)
{
    memset(&gEeprom,0,sizeof(gEeprom));memset(&gVfoSplit,0,sizeof(gVfoSplit));
    gRxVfo=gCurrentVfo=&gEeprom.VfoInfo[0];
    gRxVfo->pRX=&gRxVfo->freq_config_RX;gRxVfo->pTX=&gRxVfo->freq_config_TX;
    gRxVfo->pRX->Frequency=gRxVfo->pTX->Frequency=14439000;
    gRxVfo->Modulation=MODULATION_FM;
    gCurrentFunction=FUNCTION_FOREGROUND;testRadioGPIO.DATA=1u<<GPIOC_PIN_PTT;
    clockReady=allowed=true;scanning=pending=pa=red=muted=false;
    prepare=ramp=delays=abortAt=restore=modem=compressorStops=0;
    gSetting_ScrambleEnable=gCssBackgroundScan=g_SquelchLost=gFlagEndTransmission=gUpdateDisplay=false;
    gTxTimeoutReached=gTxSoftStart.active=false;
    gTxTimerCountdown_500ms=gSerialConfigCountDown_500ms=gUpdateStatus=0;gScanStateDir=0;
    modemResult=BELL202_SENT;
}
static void Clean(void)
{
    assert(!pa && !red && !gTxSoftStart.active && restore==1 && prepare==1);
    assert(!gTxTimerCountdown_500ms && !gTxTimeoutReached && !gFlagEndTransmission);
    assert(gUpdateDisplay && gUpdateStatus && compressorStops==2);
}
int main(void)
{
    uint8_t frame[16]={0};APRS_RadioConfig config={true,24};
    Init();assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_SENT);
    assert(modem==1 && ramp==6);Clean();
    Init();abortAt=3;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_FAULT);
    assert(!modem && ramp==2);Clean();
    Init();modemResult=BELL202_LATE;
    assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_FAULT);Clean();
    Init();allowed=false;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED);
    assert(prepare==1 && !restore && !pa && !modem);
    Init();config.enabled=false;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    config.enabled=true;
    Init();g_SquelchLost=true;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_BUSY && !prepare);
    Init();pending=true;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_BUSY && !prepare);
    Init();testRadioGPIO.DATA=0;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_BUSY && !prepare);
    Init();gCurrentFunction=FUNCTION_RECEIVE;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_BUSY && !prepare);
    Init();gScanStateDir=1;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();scanning=true;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();gSerialConfigCountDown_500ms=1;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();gEeprom.TX_VFO=2;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();gRxVfo->pTX->Frequency++;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();gRxVfo->Modulation=MODULATION_AM;assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();gRxVfo->DTMF_PTT_ID_TX_MODE=PTT_ID_APOLLO;
    assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    Init();assert(APRS_RadioSend(frame,16,14582500,&config)==APRS_LINK_DENIED && !prepare);
    Init();gRxVfo->pRX->Frequency=gRxVfo->pTX->Frequency=14582500;
    assert(APRS_RadioSend(frame,16,14439000,&config)==APRS_LINK_DENIED && !prepare);
    puts("PASS actual radio owner explicit enable/admission, normal-TX checks, muted PA ramp and RX restoration on completion/PTT/timing faults");
}
