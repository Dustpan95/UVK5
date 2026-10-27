/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/aprs_message.h"
#include "app/aprs_frame.h"
static void commit(void) { MESSAGE_Key(KEY_F); }
static void to_review(void) {
    MESSAGE_Key(KEY_MENU);assert(gAPRS_Message.page==MESSAGE_OPTIONS);
    MESSAGE_Key(KEY_DOWN);MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.page==MESSAGE_REVIEW);
}
int main(void) {
    const char *valid[]={"A","N0CALL","N0CALL-0","N0CALL-15","123456-9"};
    const char *invalid[]={"","ABCDEFG","N0CALL-16","N0CALL-01","N0CALL-","N0CALL-111","n0call","ABC DEF","N0CALL/1"};
    for(unsigned i=0;i<sizeof(valid)/sizeof(*valid);i++) assert(MESSAGE_ValidCall(valid[i]));
    for(unsigned i=0;i<sizeof(invalid)/sizeof(*invalid);i++) assert(!MESSAGE_ValidCall(invalid[i]));
    assert(!MESSAGE_ValidCall(0));
    MESSAGE_Open(0);assert(gAPRS_Message.page==MESSAGE_RECIPIENT);
    MESSAGE_Key(KEY_MENU);assert(gAPRS_Message.page==MESSAGE_RECIPIENT && gAPRS_Message.error==MESSAGE_BAD_CALL);
    MESSAGE_Open("N0CALL-7");assert(gAPRS_Message.page==MESSAGE_COMPOSE);
    MESSAGE_Key(KEY_2);MESSAGE_Key(KEY_2);assert(!strcmp(gAPRS_Message.text,"B"));
    commit();MESSAGE_Key(KEY_2);assert(!strcmp(gAPRS_Message.text,"BA"));
    MESSAGE_Tick500ms();MESSAGE_Tick500ms();MESSAGE_Key(KEY_2);
    assert(!strcmp(gAPRS_Message.text,"BAA"));
    MESSAGE_Key(KEY_STAR);assert(!strcmp(gAPRS_Message.text,"BA"));
    MESSAGE_Key(KEY_MENU);MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.page==MESSAGE_QUICK);
    MESSAGE_Key(KEY_MENU);assert(!strcmp(gAPRS_Message.text,"BA ARRIVED SAFELY"));
    assert(gAPRS_Message.page==MESSAGE_COMPOSE);
    to_review();MESSAGE_Key(KEY_EXIT);assert(gAPRS_Message.page==MESSAGE_COMPOSE);
    to_review();MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.page==MESSAGE_STATUS && gAPRS_Message.delivery==MESSAGE_TX_UNAVAILABLE);
    MESSAGE_Key(KEY_EXIT);MESSAGE_Open(0);
    assert(!strcmp(gAPRS_Message.text,"BA ARRIVED SAFELY"));
    strcpy(gAPRS_Message.recipient,"N0CALL-7");
    MESSAGE_Open("N0CALL-7");
    memset(gAPRS_Message.text,'A',67);gAPRS_Message.text[67]=0;
    commit();MESSAGE_Key(KEY_2);assert(strlen(gAPRS_Message.text)==67);
    MESSAGE_Key(KEY_MENU);MESSAGE_Key(KEY_MENU);MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.page==MESSAGE_QUICK && strlen(gAPRS_Message.text)==67);
    MESSAGE_Key(KEY_EXIT);assert(gAPRS_Message.page==MESSAGE_COMPOSE);
    MESSAGE_Key(KEY_STAR);assert(strlen(gAPRS_Message.text)==66);
    strcpy(gAPRS_Message.recipient,"N0CALL-0");
    uint8_t frame[160];
    size_t n=MESSAGE_BuildFrame("MYCALL-15","N0CALL-7",gAPRS_Message.text,"Z1234",frame,sizeof(frame));
    assert(n==99);
    assert(frame[6]==0x60 && frame[13]==0x7f && frame[14]==3 && frame[15]==0xf0);
    assert(!memcmp(frame+16,":N0CALL-7 :",11));
    uint16_t crc=APRS_FCS(frame,n);frame[n]=crc;frame[n+1]=crc>>8;
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL",gAPRS_Message.text,"1",frame,20));
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL","bad{text","1",frame,sizeof(frame)));
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL","","1",frame,sizeof(frame)));
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL","hello","",frame,sizeof(frame)));
    n=MESSAGE_BuildBeacon("MYCALL-15",49058333,-72029166,'/','-',frame,sizeof(frame));
    assert(n==36 && !memcmp(frame+16,"!4903.50N/07201.75W-",20));
    crc=APRS_FCS(frame,n);frame[n]=crc;frame[n+1]=crc>>8;
    n=MESSAGE_BuildBeacon("MYCALL",89999999,179999999,'/','-',frame,sizeof(frame));
    assert(n==36 && !memcmp(frame+16,"!9000.00N/18000.00E-",20));
    assert(!MESSAGE_BuildBeacon("MYCALL",INT32_MIN,0,'/','-',frame,sizeof(frame)));
    assert(!MESSAGE_BuildBeacon("MYCALL",0,180000001,'/','-',frame,sizeof(frame)));
    assert(!MESSAGE_BuildBeacon("MYCALL",0,0,'?','-',frame,sizeof(frame)));
    assert(!MESSAGE_BuildBeacon("MYCALL",0,0,'/','-',frame,35));
    assert(APRS_FCS((const uint8_t *)"123456789",9)==0x906e);
    puts("PASS outgoing callsign/multi-tap/draft/review limits, exact AX.25 message/beacon bytes, coordinate bounds and independent FCS vector");
}
