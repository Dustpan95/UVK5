/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_CHANNEL_H
#define UVK5_APRS_CHANNEL_H
#include "aprs_service.h"
typedef enum { APRS_CHANNEL_READY,APRS_CHANNEL_BUSY,APRS_CHANNEL_DENIED,APRS_CHANNEL_FAULT } APRS_ChannelAdmission;
typedef struct {
    uint32_t (*now)(void *);
    APRS_ChannelAdmission (*enter)(uint32_t frequency,void *);
    APRS_LinkResult (*send)(const uint8_t *,size_t,uint32_t frequency,void *);
    void (*restore)(void *);
    bool (*interrupted)(void *);
    void *context;
} APRS_ChannelOps;
typedef struct {
    APRS_ChannelOps ops;
    uint32_t frequency;
    bool configured,leased,fault;
} APRS_Channel;
/* Caller zero-initializes this foreground-only state. Hardware enter must save
 * voice context and tune the requested channel; a refused enter must leave voice
 * untouched. Restore must drop any data TX and restore saved voice context.
 * The runtime binds the actual hardware adapter.
 */
bool CHANNEL_Configure(APRS_Channel *channel,const APRS_ChannelOps *ops);
void CHANNEL_Process(APRS_Channel *channel);
/* PTT/key dispatch must restore voice before ordinary radio handling. Pending
 * No APRS receiver/ACK wait is started; this does not claim delivery. */
void CHANNEL_Interrupt(APRS_Channel *channel);
#endif
