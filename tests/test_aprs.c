/* SPDX-License-Identifier: Apache-2.0 */
#include "app/aprs_decode.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t frame[APRS_FRAME_MAX], bits[240];
static size_t bitpos;
static unsigned level, ones;
static void address(uint8_t *p, const char *s, unsigned ssid, bool last) {
    size_t n = strlen(s);
    for (unsigned i=0; i<6; i++) p[i] = (i<n ? s[i] : ' ') << 1;
    p[6] = 0x60 | ssid << 1 | last;
}
static uint16_t fcs(uint16_t n) {
    uint16_t crc = APRS_FCS(frame, n);
    frame[n] = crc; frame[n+1] = crc >> 8;
    return n+2;
}
static uint16_t packet(const char *dest, const uint8_t *info, size_t n) {
    assert(n+18 <= sizeof(frame));
    address(frame,dest,0,false); address(frame+7,"N0CALL",15,true);
    frame[14]=3; frame[15]=0xf0;
    memcpy(frame+16,info,n);
    return fcs(16+n);
}
static void bit(unsigned b) {
    if (!b) level ^= 1;
    assert(bitpos < sizeof(bits)*8);
    bits[bitpos/8] |= level << (7-bitpos%8); ++bitpos;
}
static void byte(uint8_t b, bool stuff) {
    for(unsigned i=0;i<8;i++) {
        unsigned v=(b>>i)&1; bit(v);
        if(v) ++ones; else ones=0;
        if(stuff && ones==5) { bit(0); ones=0; }
    }
}
static uint16_t encode(uint16_t n, unsigned polarity) {
    memset(bits,0,sizeof(bits)); bitpos=ones=0; level=polarity;
    for(unsigned i=0;i<4;i++) byte(0x7e,false);
    for(unsigned i=0;i<n;i++) byte(frame[i],true);
    for(unsigned i=0;i<4;i++) byte(0x7e,false);
    return (bitpos+7)/8;
}
static void base91(char *out, uint32_t value) {
    for(int i=3;i>=0;--i) { out[i]=33+value%91; value/=91; }
}
int main(void) {
    APRS_Record r;
    assert(APRS_FCS((const uint8_t *)"123456789",9)==0x906e);
    const char *pos="!4903.50N/07201.75W-Test";
    uint16_t n=packet("APRS",(const uint8_t *)pos,strlen(pos));
    assert(APRS_ParseFrame(frame,n,&r));
    assert(r.kind==APRS_POSITION && strcmp(r.source,"N0CALL-15")==0);
    assert(r.latitude==49058333 && r.longitude==-72029166);
    for(unsigned polarity=0;polarity<2;polarity++) {
        uint16_t count=encode(n,polarity);
        assert(APRS_DecodeBits(bits,count,&r));
        assert(r.latitude==49058333);
    }
    frame[20]^=1;
    assert(!APRS_ParseFrame(frame,n,&r));
    assert(!APRS_DecodeBits(bits,1,&r));
    n=packet("APRS",(const uint8_t*)":MYCALL   :HELLO{12345",22);
    assert(APRS_ParseFrame(frame,n,&r) && r.kind==APRS_MESSAGE);
    assert(!strcmp(r.text,"HELLO") && !strcmp(r.messageId,"12345"));
    n=packet("APRS",(const uint8_t*)":MYCALL   :ack12345",19);
    assert(APRS_ParseFrame(frame,n,&r) && r.kind==APRS_ACK && !strcmp(r.messageId,"12345"));
    n=packet("APRS",(const uint8_t*)":MYCALL   :rejab1",17);
    assert(APRS_ParseFrame(frame,n,&r) && r.kind==APRS_REJECT && !strcmp(r.messageId,"ab1"));
    n=packet("APRS",(const uint8_t*)":MYCALL   :ackBAD!",18);
    assert(APRS_ParseFrame(frame,n,&r) && r.kind==APRS_MESSAGE && !r.messageId[0]);
    puts("PASS message ID/ACK/rejection parsing and malformed-ID handling");
    puts("PASS CRC, AX.25, HDLC/NRZI both polarities, corruption/truncation");

    const char *timestamp="/092345z4903.50S/07201.75E>";
    n=packet("APRS",(const uint8_t *)timestamp,strlen(timestamp));
    assert(APRS_ParseFrame(frame,n,&r) && r.latitude==-49058333 && r.longitude==72029166);
    const char *badpos="!9903.50N/07201.75W>";
    n=packet("APRS",(const uint8_t *)badpos,strlen(badpos));
    assert(!APRS_ParseFrame(frame,n,&r));
    badpos="!9000.01N/18000.00E>";
    n=packet("APRS",(const uint8_t *)badpos,strlen(badpos));
    assert(!APRS_ParseFrame(frame,n,&r));
    char compressed[]="!/!!!!!!!!>!!!";
    base91(compressed+2, (90-40)*380926u);
    base91(compressed+6, (180-75)*190463u);
    n=packet("APRS",(uint8_t *)compressed,strlen(compressed));
    assert(APRS_ParseFrame(frame,n,&r) && r.latitude==40000000 && r.longitude==-75000000);
    const uint8_t mice[]={0x60,100,29,103,28,28,28,'>','/'};
    n=packet("490S5P",mice,sizeof(mice));
    assert(APRS_ParseFrame(frame,n,&r) && r.latitude==49058333 && r.longitude==-72029166);
    puts("PASS uncompressed, timestamped, compressed and Mic-E positions; coordinate bounds");

    char message[90]=":DEST-12  :";
    memset(message+11,'A',67); memcpy(message+78,"{123",5);
    n=packet("APRS",(uint8_t *)message,82);
    assert(APRS_ParseFrame(frame,n,&r) && r.kind==APRS_MESSAGE);
    assert(strcmp(r.destination,"DEST-12")==0 && strlen(r.text)==67);
    assert(APRS_DecodeBits(bits,encode(n,0),&r) && strlen(r.text)==67);
    const char *msg=":DEST     :Hello\x01world{99";
    n=packet("APRS",(const uint8_t *)msg,strlen(msg));
    assert(APRS_ParseFrame(frame,n,&r) && strcmp(r.text,"Hello.world")==0);
    frame[14]=0x13; n=fcs(n-2); assert(!APRS_ParseFrame(frame,n,&r));
    frame[14]=3; frame[15]=0xcf; n=fcs(n-2); assert(!APRS_ParseFrame(frame,n,&r));
    frame[15]=0xf0; frame[6]|=1; n=fcs(n-2); assert(!APRS_ParseFrame(frame,n,&r));
    puts("PASS full-length messages, addressee, sanitization, non-APRS rejection");

    /* Deterministic malformed-input sweep against the actual firmware parser. */
    unsigned seed=1;
    for(unsigned iteration=0;iteration<20000;iteration++) {
        uint16_t size=iteration%sizeof(frame);
        for(unsigned i=0;i<sizeof(frame);i++) { seed=seed*1664525u+1013904223u;frame[i]=seed>>24; }
        APRS_ParseFrame(frame,size,&r);
        if(size>=2) { fcs(size-2); APRS_ParseFrame(frame,size,&r); }
        for(unsigned i=0;i<sizeof(bits);i++) {seed=seed*1664525u+1013904223u;bits[i]=seed>>24;}
        APRS_DecodeBits(bits,iteration%sizeof(bits),&r);
    }
    assert(!APRS_ParseFrame(NULL,0,&r));
    assert(!APRS_DecodeBits(bits,241,&r));
    puts("PASS 20,000 malformed frame/capture cases");
}
