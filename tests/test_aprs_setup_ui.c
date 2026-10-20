/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/aprs_setup.h"
#include "app/aprs_runtime.h"
#include "app/aprs_session.h"
#include "app/aprs_message.h"
#include "app/aprs_tx.h"
#include "driver/st7565.h"
#include "font.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
APRS_Setup gAPRS_Setup;
APRS_RuntimeSettings gAPRS_Runtime={.acknowledgmentSeconds=10};
APRS_Profile gAPRS_Profile={.callsign="N0CALL",.latitude=41390833,.longitude=-87655000};
APRS_Message gAPRS_Message;
APRS_TX_Job gAPRS_TX;
char gAPRS_Quick[APRS_QUICK_COUNT][APRS_MESSAGE_MAX+1]={"ARRIVED SAFELY","NEED ASSISTANCE","CAN YOU COPY?"};
uint8_t gFrameBuffer[FRAME_LINES][LCD_WIDTH],gStatusLine[LCD_WIDTH];
char gInputBox[8];
uint8_t gInputBoxIndex;
static unsigned blits;
bool RUNTIME_Enabled(void) {return false;}
const char *MESSAGE_QuickEditText(void) {return "THIS QUICK MESSAGE IS STILL BEING EDITED";}
void _putchar(char c) {(void)c;}
void ST7565_BlitFullScreen(void) {++blits;}
void APRS_FormatCoordinate(char *s,int32_t v,char positive,char negative)
{
    unsigned abs=v<0 ? -v : v;sprintf(s,"%u.%05u%c",abs/1000000,abs%1000000/10,v<0 ? negative : positive);
}
static void Image(const char *directory,unsigned page)
{
    char path[256];snprintf(path,sizeof(path),"%s/setup-%u.pbm",directory,page);
    FILE *f=fopen(path,"w");assert(f);fprintf(f,"P1\n128 64\n");
    for(unsigned y=0;y<64;y++) {
        for(unsigned x=0;x<128;x++) fprintf(f,"%u ",((y<8 ? gStatusLine[x] : gFrameBuffer[(y-8)/8][x])>>(y%8))&1);
        fputc('\n',f);
    }
    fclose(f);
}
int main(int argc,char **argv)
{
    assert(argc==2);FONT_Init();gAPRS_Setup.latitude=gAPRS_Profile.latitude;gAPRS_Setup.longitude=gAPRS_Profile.longitude;
    for(unsigned p=SETUP_HOME;p<=SETUP_BEACON;p++) {
        gAPRS_Setup.page=p;strcpy(gAPRS_Setup.digits,p==SETUP_LONGITUDE ? "0873930" : "412345");
        gAPRS_Setup.length=strlen(gAPRS_Setup.digits);gAPRS_Setup.negative=p==SETUP_LONGITUDE;
        if(p==SETUP_AUDIO || p==SETUP_WAIT) {strcpy(gAPRS_Setup.digits,p==SETUP_AUDIO ? "25" : "10");gAPRS_Setup.length=2;}
        if(p==SETUP_FREQUENCY) {strcpy(gAPRS_Setup.digits,"14439000");gAPRS_Setup.length=8;}
        SETUP_Draw();Image(argv[1],p);gAPRS_Setup.error=true;SETUP_Draw();gAPRS_Setup.error=false;
    }
    for(unsigned page=MESSAGE_RECIPIENT;page<=MESSAGE_QUICK_EDIT;page++) {
        gAPRS_Message.page=page;strcpy(gAPRS_Message.recipient,"N0CALL-15");
        memset(gAPRS_Message.text,'W',APRS_MESSAGE_MAX);gAPRS_Message.text[APRS_MESSAGE_MAX]=0;
        MESSAGE_Draw();
    }
    gAPRS_Message.page=MESSAGE_STATUS;strcpy(gAPRS_TX.recipient,"N0CALL-15");
    for(unsigned state=APRS_TX_IDLE;state<=APRS_TX_EXPIRED;state++) {gAPRS_TX.state=state;MESSAGE_Draw();}
    assert(blits==2*SETUP_BEACON+MESSAGE_QUICK_EDIT+APRS_TX_EXPIRED+1);
    puts("PASS actual setup/editor/status framebuffer rendering for all pages and TX states under ASan/UBSan");
}
