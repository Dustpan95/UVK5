/* SPDX-License-Identifier: GPL-3.0-only
 * Actual UART framing, command bounds and reply serialization; mocked DMA/IO. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bsp/dp32g030/dma.h"
static __typeof__(*DMA_CH0) testDma;
#undef DMA_CH0
#define DMA_CH0 (&testDma)
#define ENABLE_OVERLAY
#include "app/uart.c"
uint8_t UART_DMA_Buffer[256];
static uint8_t wire[256];
static unsigned wireSize,reads;
uint8_t APRS_ReadDiagnosticCapture(uint16_t offset,uint8_t size,uint8_t *out)
{
    ++reads;if(offset>=1008) return 0;
    if(size>64) size=64;
    if(size>1008-offset) size=1008-offset;
    for(unsigned i=0;i<size;i++) out[i]=(uint8_t)(offset+i);
    return size;
}
void UART_Send(const void *p,uint32_t n) {assert(wireSize+n<=sizeof(wire));memcpy(wire+wireSize,p,n);wireSize+=n;}
uint16_t CRC_Calculate(const void *p,uint16_t n)
{
    const uint8_t *s=p;uint16_t crc=0;
    for(unsigned i=0;i<n;i++) {crc^=(uint16_t)s[i]<<8;for(unsigned bit=0;bit<8;bit++) crc=(crc&0x8000) ? (crc<<1)^0x1021:crc<<1;}
    return crc;
}
static unsigned cases;
static void Check(unsigned offset,unsigned wanted,bool encrypted,unsigned start,unsigned bodySize,unsigned declared,bool corrupt)
{
    uint8_t body[10]={0x40,5,declared,0,offset,offset>>8,wanted,0};
    uint16_t crc=CRC_Calculate(body,bodySize);body[bodySize]=crc;body[bodySize+1]=crc>>8;
    if(corrupt) body[bodySize]^=1;
    if(encrypted) for(unsigned i=0;i<bodySize+2;i++) body[i]^=Obfuscation[i%16];
    uint8_t frame[18]={0xab,0xcd,bodySize,0};memcpy(frame+4,body,bodySize+2);frame[bodySize+6]=0xdc;frame[bodySize+7]=0xba;
    unsigned length=bodySize+8;memset(UART_DMA_Buffer,0,sizeof(UART_DMA_Buffer));gUART_WriteIndex=start;bIsEncrypted=encrypted;
    for(unsigned i=0;i<length;i++) UART_DMA_Buffer[(start+i)%256]=frame[i];
    testDma.ST=(start+length-1)%256;assert(!UART_IsCommandAvailable());
    testDma.ST=(start+length)%256;bool available=UART_IsCommandAvailable();assert(available==!corrupt);
    wireSize=reads=0;
    if(available) {assert(UART_Command.Header.ID==0x0540);CMD_0540();}
    if(corrupt || bodySize!=8 || declared!=4) {assert(!wireSize && !reads);++cases;return;}
    assert(reads==1);unsigned n=offset>=1008 ? 0:wanted;
    if(n>64)n=64;
    if(n>1008-offset && offset<1008)n=1008-offset;
    assert(wireSize==n+16 && wire[0]==0xab && wire[1]==0xcd && wire[2]==n+8 && !wire[3]);
    assert(wire[wireSize-2]==0xdc && wire[wireSize-1]==0xba);
    if(encrypted) for(unsigned i=0;i<n+10;i++) wire[4+i]^=Obfuscation[i%16];
    assert(wire[4]==0x41 && wire[5]==5 && wire[6]==n+4 && !wire[7]);
    assert(wire[8]==(uint8_t)offset && wire[9]==offset>>8 && wire[10]==n && wire[11]==1);
    for(unsigned i=0;i<n;i++) assert(wire[12+i]==(uint8_t)(offset+i));
    assert(wire[12+n]==0xff && wire[13+n]==0xff);++cases;
}
int main(void)
{
    const unsigned offsets[]={0,1,63,64,960,1007,1008,65535},sizes[]={0,1,63,64,65,255};
    for(unsigned enc=0;enc<2;enc++) for(unsigned start=0;start<256;start+=17)
      for(unsigned o=0;o<8;o++) for(unsigned n=0;n<6;n++) Check(offsets[o],sizes[n],enc,start,8,4,false);
    for(unsigned n=4;n<=8;n++) Check(0,64,true,250,n,4,false);
    Check(0,64,true,250,8,3,false);Check(0,64,true,250,8,4,true);
    printf("PASS %u actual UART diagnostic requests/replies, XOR/CRC, partial frames, DMA wrap, truncated commands and bounded reads\n",cases);
}
