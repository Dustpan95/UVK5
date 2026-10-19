/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "app/aprs_message.h"
#include "app/aprs_decode.h"
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
    assert(!MESSAGE_Acknowledge("N0CALL-7","MYCALL-1","01",false));
    MESSAGE_Key(KEY_EXIT);MESSAGE_Open(0);
    assert(!strcmp(gAPRS_Message.text,"BA ARRIVED SAFELY"));
    strcpy(gAPRS_Message.recipient,"N0CALL-7");
    assert(MESSAGE_MarkSent("MYCALL-1","01"));
    assert(!MESSAGE_Acknowledge("N0CALL-8","MYCALL-1","01",false));
    assert(!MESSAGE_Acknowledge("N0CALL-7","MYCALL-2","01",false));
    assert(!MESSAGE_Acknowledge("N0CALL-7","MYCALL-1","02",false));
    assert(MESSAGE_Acknowledge("N0CALL-7","MYCALL-1","01",false));
    assert(gAPRS_Message.delivery==MESSAGE_ACKED);
    assert(!MESSAGE_Acknowledge("N0CALL-7","MYCALL-1","01",true));
    assert(MESSAGE_MarkSent("MYCALL-1","01"));
    assert(MESSAGE_Acknowledge("N0CALL-7","MYCALL-1","01",true));
    assert(gAPRS_Message.delivery==MESSAGE_REJECTED);
    assert(!MESSAGE_MarkSent("bad","01"));assert(!MESSAGE_MarkSent("MYCALL","000001"));
    MESSAGE_Open("N0CALL-7");
    memset(gAPRS_Message.text,'A',67);gAPRS_Message.text[67]=0;
    commit();MESSAGE_Key(KEY_2);assert(strlen(gAPRS_Message.text)==67);
    MESSAGE_Key(KEY_MENU);MESSAGE_Key(KEY_MENU);MESSAGE_Key(KEY_MENU);
    assert(gAPRS_Message.page==MESSAGE_QUICK && strlen(gAPRS_Message.text)==67);
    MESSAGE_Key(KEY_EXIT);assert(gAPRS_Message.page==MESSAGE_COMPOSE);
    MESSAGE_Key(KEY_STAR);assert(strlen(gAPRS_Message.text)==66);
    strcpy(gAPRS_Message.recipient,"N0CALL-0");
    assert(MESSAGE_MarkSent("MYCALL-0","01"));
    assert(MESSAGE_Acknowledge("N0CALL","MYCALL","01",false));
    uint8_t frame[160];APRS_Record record;
    size_t n=MESSAGE_BuildFrame("MYCALL-15","N0CALL-7",gAPRS_Message.text,"Z1234",frame,sizeof(frame));
    assert(n==99);
    assert(frame[6]==0x60 && frame[13]==0x7f && frame[14]==3 && frame[15]==0xf0);
    assert(!memcmp(frame+16,":N0CALL-7 :",11));
    uint16_t crc=APRS_FCS(frame,n);frame[n]=crc;frame[n+1]=crc>>8;
    assert(APRS_ParseFrame(frame,n+2,&record));
    assert(record.kind==APRS_MESSAGE && !strcmp(record.source,"MYCALL-15"));
    assert(!strcmp(record.destination,"N0CALL-7") && strlen(record.text)==66);
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL",gAPRS_Message.text,"1",frame,20));
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL","bad{text","1",frame,sizeof(frame)));
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL","","1",frame,sizeof(frame)));
    assert(!MESSAGE_BuildFrame("MYCALL","N0CALL","hello","",frame,sizeof(frame)));
    puts("PASS multi-tap/timeout/delete, draft preservation, quick insert/bounds, review, blocked TX");
    puts("PASS call/SSID/ID validation, AX.25 message build/roundtrip, ACK source/recipient/ID matching");
}
