/* SPDX-License-Identifier: GPL-3.0-only
 * Exercise actual gain staging concurrently with the diagnostic RX owner.
 * Simulated registers establish ownership/restoration, not RF performance. */
#include <assert.h>
#include <stdio.h>
#include "app/aprs_rx.h"
#include "audio.h"
#include "driver/bk4819.h"
#include "functions.h"
#include "gain_staging.h"
#include "settings.h"
#include "signal_classifier.h"

EEPROM_Config_t gEeprom;
VFO_Info_t *gRxVfo=&gEeprom.VfoInfo[0];
FUNCTION_Type_t gCurrentFunction=FUNCTION_FOREGROUND;
int8_t gScanStateDir;
bool gCssBackgroundScan,gUpdateDisplay;
volatile uint8_t gSerialConfigCountDown_500ms;
static uint16_t regs[128];
static unsigned gainWrites;
bool FUNCTION_IsRx(void) {return gCurrentFunction==FUNCTION_RECEIVE;}
void AUDIO_PlayBeep(BEEP_Type_t beep) {(void)beep;}
uint16_t BK4819_ReadRegister(BK4819_REGISTER_t r) {assert(r<128);return regs[r];}
void BK4819_WriteRegister(BK4819_REGISTER_t r,uint16_t v) {assert(r<128);regs[r]=v;if(r==0x13)gainWrites++;}
uint16_t BK4819_GetRSSI(void) {return 220;}
SignalClass_t SIGNAL_CLASSIFIER_GetClass(uint8_t vfo) {(void)vfo;return SIGNAL_CLASS_FAST;}

int main(void)
{
    gRxVfo->Modulation=MODULATION_FM;
    gRxVfo->pRX=&gRxVfo->freq_config_RX;gRxVfo->pRX->Frequency=14439000;
    regs[0x43]=0x49a8;regs[0x7e]=0x8357;regs[0x13]=0x03be;
    gAPRS_Enabled=gAPRS_Browser=true;APRS_Resume();
    for(unsigned profile=0;profile<4;profile++) {
        bool reference=profile==1 || profile==3;
        assert(APRS_UsesReferenceGain()==reference);
        unsigned start=gainWrites;
        for(unsigned tick=0;tick<200;tick++)GAIN_STAGING_10ms(0);
        assert(reference ? gainWrites==start : gainWrites>start);
        if(reference) {
            assert(regs[0x13]==0x03be && regs[0x7e]==0x3357);
            assert(GAIN_STAGING_GetGainDiff(0)==0);
        }
        const uint16_t expected[]={0x3628,0x3648,0x1348};
        for(unsigned bandwidth=0;bandwidth<3;bandwidth++) {
            APRS_Stop();gRxVfo->CHANNEL_BANDWIDTH=bandwidth;APRS_Resume();
            assert(regs[0x43]==((profile==1 || profile==2)?expected[bandwidth]:0x49a8));
        }
        APRS_Stop();
        assert(!APRS_UsesReferenceGain() && regs[0x43]==0x49a8 && regs[0x7e]==0x8357);
        /* Test an arbitrary voice state rather than only the stock defaults. */
        regs[0x13]=0x0123;APRS_Resume();APRS_Stop();assert(regs[0x13]==0x0123);
        gRxVfo->CHANNEL_BANDWIDTH=BK4819_FILTER_BW_WIDE;
        assert(APRS_Key(KEY_2,true,false));
    }
    /* REF must release gain immediately when the radio becomes ineligible. */
    assert(APRS_Key(KEY_2,true,false));assert(APRS_UsesReferenceGain());
    gCurrentFunction=FUNCTION_TRANSMIT;APRS_Tick500ms();assert(!APRS_UsesReferenceGain());
    unsigned stopped=gainWrites;GAIN_STAGING_10ms(0);assert(gainWrites==stopped);
    puts("PASS actual gain staging: baseline/filter activity, reference ownership, all bandwidths, reserved bits and voice/TX restoration");
}
