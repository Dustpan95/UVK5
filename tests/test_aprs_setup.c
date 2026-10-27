/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_setup.h"
#include "app/aprs_runtime.h"
#include "app/aprs_session.h"
#include "app/aprs_message.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
APRS_RuntimeSettings gAPRS_Runtime;
APRS_Profile gAPRS_Profile;
APRS_Message gAPRS_Message;
static bool enabled;
static unsigned beacons;
int RX_freq_check(uint32_t f) {return f<13600000 || f>17400000;}
int TX_freq_check(uint32_t f) {return RX_freq_check(f);}
bool RUNTIME_Enable(bool on) {enabled=on;return true;}
bool RUNTIME_Enabled(void) {return enabled;}
bool RUNTIME_Beacon(void) {++beacons;return true;}
void MESSAGE_EditCall(void) {}
void MESSAGE_EditQuick(unsigned slot) {(void)slot;}
bool SESSION_SetPosition(int32_t a,int32_t b) {gAPRS_Profile.latitude=a;gAPRS_Profile.longitude=b;gAPRS_Profile.positionSet=true;return true;}
static void Digits(const char *s) {while(*s) SETUP_Key(*s++-'0');}
int main(void)
{
    int32_t value=123;
    assert(SETUP_ParseDM("900000",false,false,&value) && value==90000000);
    assert(SETUP_ParseDM("1800000",true,true,&value) && value==-180000000);
    assert(SETUP_ParseDM("412345",false,true,&value) && value==-41390833);
    assert(SETUP_ParseDM("000001",false,false,&value) && value==167);
    const char *bad[]={"900001","916000","126000","-12345","12345","1234567","12x456"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++) assert(!SETUP_ParseDM(bad[i],false,false,&value));
    assert(!SETUP_ParseDM("1800001",true,false,&value));
    assert(!SETUP_ParseDM("1810000",true,false,&value));
    /* Exhaustive minute round trip: every accepted hundredth survives the
     * microdegree conversion and the real beacon encoder's rounding rule. */
    for(unsigned m=0;m<6000;m++) {
        char input[8];snprintf(input,sizeof(input),"00%04u",m);
        assert(SETUP_ParseDM(input,false,false,&value));
        assert(((uint32_t)value*3+250)/500==m);
    }
    SETUP_Open();SETUP_Key(KEY_DOWN);SETUP_Key(KEY_MENU);
    Digits("412345");SETUP_Key(KEY_MENU);assert(gAPRS_Setup.page==SETUP_LONGITUDE);
    Digits("0873930");SETUP_Key(KEY_UP);SETUP_Key(KEY_MENU);
    assert(gAPRS_Setup.page==SETUP_POSITION_REVIEW && !gAPRS_Profile.positionSet && !beacons);
    SETUP_Key(KEY_MENU);assert(gAPRS_Profile.positionSet && gAPRS_Profile.longitude==-87655000 && !beacons);
    /* A cancelled replacement leaves the saved position unchanged. */
    SETUP_Open();SETUP_Key(KEY_DOWN);SETUP_Key(KEY_MENU);Digits("000000");SETUP_Key(KEY_EXIT);
    assert(gAPRS_Profile.latitude==41390833 && !beacons);
    /* Invalid input remains editable, and deletion is bounded. */
    SETUP_Open();SETUP_Key(KEY_DOWN);SETUP_Key(KEY_MENU);Digits("906000");SETUP_Key(KEY_MENU);
    assert(gAPRS_Setup.error && gAPRS_Setup.page==SETUP_LATITUDE);
    for(unsigned i=0;i<10;i++) SETUP_Key(KEY_STAR);
    assert(!gAPRS_Setup.length && !gAPRS_Setup.digits[0]);
    puts("PASS degrees/minutes bounds, exhaustive hundredth-minute round trip, review/save/cancel and no implicit beacon");
}
