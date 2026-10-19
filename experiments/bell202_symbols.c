/* SPDX-License-Identifier: GPL-3.0-only
 * Protocol-based AX.25 FCS, HDLC stuffing and NRZI symbol experiment.
 * No RF driver, scheduler, operator interface or transmission authorization.
 */
#include "bell202_symbols.h"
typedef struct {
    void (*emit)(bool, void *);
    void *context;
    bool level;
    unsigned ones;
} Encoder;
static void Bit(Encoder *e, unsigned bit)
{
    if (!bit) e->level = !e->level;
    e->emit(e->level, e->context);
}
static void Byte(Encoder *e, uint8_t byte, bool stuff)
{
    for (unsigned i=0; i<8; ++i) {
        const unsigned bit=(byte>>i)&1;
        Bit(e,bit);
        if (stuff) {
            e->ones=bit ? e->ones+1 : 0;
            if (e->ones==5) { Bit(e,0); e->ones=0; }
        }
    }
}
bool BELL202_Encode(const uint8_t *frame, size_t length,
                    void (*symbol)(bool, void *), void *context)
{
    if (!frame || !symbol || length<16 || length>330) return false;
    Encoder e={.emit=symbol,.context=context};
    uint16_t crc=0xffff;
    /* 400 ms flag preamble; experiment value, not a selected beacon setting. */
    for (unsigned i=0;i<60;i++) Byte(&e,0x7e,false);
    for (size_t i=0;i<length;i++) {
        crc^=frame[i];
        for(unsigned j=0;j<8;j++) crc=(crc>>1)^((crc&1)?0x8408:0);
        Byte(&e,frame[i],true);
    }
    crc^=0xffff;
    Byte(&e,crc&255,true); Byte(&e,crc>>8,true);
    for(unsigned i=0;i<4;i++) Byte(&e,0x7e,false);
    return true;
}
