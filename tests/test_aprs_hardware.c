/* SPDX-License-Identifier: GPL-3.0-only */
#include "experiments/aprs_hardware.h"
#include "app/app.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "bsp/dp32g030/gpio.h"
#include "settings.h"
#include "misc.h"
#include "tx_soft_start.h"
#include "vfo_split.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
EEPROM_Config_t gEeprom;
VFO_Info_t *gTxVfo,*gRxVfo,*gCurrentVfo;
VfoState_t VfoState[2];
FUNCTION_Type_t gCurrentFunction;
TxSoftStart_t gTxSoftStart;
VfoSplit_t gVfoSplit;
TestRadioGPIO testRadioGPIO;
bool gAPRS_Enabled,gRxVfoIsActive,g_SquelchLost,gCssBackgroundScan,gFlagEndTransmission,gUpdateDisplay;
volatile uint8_t gSerialConfigCountDown_500ms;
volatile uint16_t gTxTimerCountdown_500ms;
volatile bool gTxTimeoutReached;
uint8_t gUpdateStatus;
int8_t gScanStateDir;
static bool pendingWrites,scan,capture,irq,denied,interruptOnTune,muted;
static unsigned tunes,stops,resumes,sends,paOff,redOff;
static APRS_Hardware hardware;
static EEPROM_Config_t original;
static VfoState_t originalState;
bool APP_HasPendingWrites(void) {return pendingWrites;}
bool SCANNER_IsScanning(void) {return scan;}
bool APRS_IsCapturing(void) {return capture;}
uint32_t SCHEDULER_GetTicks10ms(void) {return 12345;}
int32_t RX_freq_check(uint32_t f) {return f==14439000 ? 0 : -1;}
int32_t TX_freq_check(uint32_t f) {return denied || f!=14439000 ? -1 : 0;}
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t r) {assert(r==BK4819_REG_0C);return irq;}
void APRS_Stop(void) {++stops;}
void APRS_Resume(void) {++resumes;}
void TX_COMPRESSOR_Stop(void) {}
void CTCSS_LEAD_Stop(void) {}
void BK4819_EnterTxMute(void) {muted=true;}
void BK4819_SetupPowerAmplifier(uint8_t bias,uint32_t f) {assert(!bias && f==14439000 && muted);++paOff;}
void BK4819_ToggleGpioOut(BK4819_GPIO_PIN_t pin,bool on)
{
    assert(!on && muted);
    if(pin==BK4819_GPIO5_PIN1_RED) ++redOff;
    else assert(pin==BK4819_GPIO1_PIN29_PA_ENABLE);
}
void FUNCTION_Select(FUNCTION_Type_t f) {assert(f==FUNCTION_FOREGROUND && muted && paOff);gCurrentFunction=f;}
void RADIO_InitInfo(VFO_Info_t *v,uint8_t channel,uint32_t frequency)
{
    assert(v==gTxVfo && stops && frequency==14439000);
    memset(v,0,sizeof(*v));v->CHANNEL_SAVE=channel;
    v->pRX=&v->freq_config_RX;v->pTX=&v->freq_config_TX;
    v->pRX->Frequency=v->pTX->Frequency=frequency;v->Modulation=MODULATION_AM;
}
void RADIO_ConfigureSquelchAndOutputPower(VFO_Info_t *v)
{
    assert(v==gTxVfo && v->Modulation==MODULATION_FM && v->OUTPUT_POWER==OUTPUT_POWER_MID);
    v->TXP_CalculatedSetting=77;
}
void RADIO_SetupRegisters(bool foreground)
{
    assert(!foreground);++tunes;
    if(tunes==1) {
        assert(gRxVfo->pRX->Frequency==14439000 && gAPRS_Enabled && gRxVfoIsActive);
        if(interruptOnTune) testRadioGPIO.DATA=0;
    } else {
        assert(!memcmp(&gEeprom,&original,sizeof(original)) && muted && paOff);
        assert(VfoState[gEeprom.TX_VFO]==originalState && !gAPRS_Enabled && !gRxVfoIsActive);
    }
}
APRS_LinkResult APRS_RadioSend(const uint8_t *frame,size_t length,uint32_t frequency,void *config)
{
    assert(frame && length==16 && frequency==14439000 && config==&hardware.radio);
    assert(gRxVfo->pRX->Frequency==frequency && gRxVfo->pTX->Frequency==frequency);
    ++sends;return APRS_LINK_SENT;
}
static void Init(unsigned slot)
{
    memset(&hardware,0,sizeof(hardware));memset(&gEeprom,0,sizeof(gEeprom));memset(&gVfoSplit,0,sizeof(gVfoSplit));
    hardware.radio=(APRS_RadioConfig){true,24};hardware.power=OUTPUT_POWER_MID;
    gEeprom.TX_VFO=gEeprom.RX_VFO=slot;gTxVfo=gRxVfo=gCurrentVfo=&gEeprom.VfoInfo[slot];
    for(unsigned i=0;i<2;++i) {
        VFO_Info_t *v=&gEeprom.VfoInfo[i];memset(v,0x2a,sizeof(*v));
        v->pRX=&v->freq_config_TX;v->pTX=&v->freq_config_RX; /* reversed voice VFO */
        v->FrequencyReverse=true;v->pRX->Frequency=14652000+i;v->pTX->Frequency=14592000+i;
        v->Modulation=MODULATION_AM;v->pRX->CodeType=CODE_TYPE_CONTINUOUS_TONE;v->pTX->CodeType=CODE_TYPE_DIGITAL;
    }
    gCurrentFunction=FUNCTION_FOREGROUND;gAPRS_Enabled=gRxVfoIsActive=false;
    gSerialConfigCountDown_500ms=0;gScanStateDir=0;g_SquelchLost=gCssBackgroundScan=false;
    pendingWrites=scan=capture=irq=denied=interruptOnTune=muted=false;
    testRadioGPIO.DATA=1u<<GPIOC_PIN_PTT;tunes=stops=resumes=sends=paOff=redOff=0;
    original=gEeprom;originalState=VfoState[slot]=VFO_STATE_BUSY;
}
int main(void)
{
    uint8_t frame[16]={0};
    for(unsigned slot=0;slot<2;++slot) {
        Init(slot);assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_READY);
        assert(hardware.leased && tunes==1 && resumes==1);
        assert(hardware.voice.pRX==&hardware.voice.freq_config_TX &&
            hardware.voice.pRX->Frequency==14652000+slot && hardware.voice.pTX->Frequency==14592000+slot);
        assert(HARDWARE_Send(frame,sizeof(frame),14439000,&hardware)==APRS_LINK_SENT && sends==1);
        HARDWARE_Restore(&hardware);
        assert(!hardware.leased && tunes==2 && stops==2 && resumes==2 && paOff==1 && redOff==1);
        assert(!memcmp(&gEeprom,&original,sizeof(original)));
        HARDWARE_Restore(&hardware);assert(tunes==2);
    }
    Init(0);denied=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_DENIED && !tunes);
    Init(0);pendingWrites=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_BUSY && !tunes);
    Init(0);irq=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_BUSY && !tunes);
    Init(0);capture=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_BUSY && !tunes);
    Init(0);g_SquelchLost=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_BUSY && !tunes);
    Init(0);scan=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_BUSY && !tunes);
    Init(0);hardware.radio.enabled=false;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_DENIED && !tunes);
    Init(0);hardware.radio.toneGain=128;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_DENIED && !tunes);
    Init(0);interruptOnTune=true;assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_BUSY);
    assert(!hardware.leased && tunes==2 && !memcmp(&gEeprom,&original,sizeof(original)));
    Init(0);assert(HARDWARE_Enter(14439000,&hardware)==APRS_CHANNEL_READY);
    pendingWrites=true;assert(HARDWARE_Send(frame,16,14439000,&hardware)==APRS_LINK_BUSY && !sends);
    HARDWARE_Restore(&hardware);assert(!memcmp(&gEeprom,&original,sizeof(original)));
    Init(0);assert(HARDWARE_Now(&hardware)==12345);
    assert(HARDWARE_Send(frame,16,14439000,&hardware)==APRS_LINK_DENIED);
    puts("PASS actual hardware adapter: RAM retuning, normal TX delegation, band/save/RX admission, interrupt rollback, both VFOs and exact reversed/code/offset/calibration restoration; no EEPROM writes");
}
