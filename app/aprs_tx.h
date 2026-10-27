/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_TX_H
#define UVK5_APRS_TX_H
#include "aprs_message.h"
typedef enum {
    APRS_TX_IDLE,APRS_TX_QUEUED,APRS_TX_SENT,APRS_TX_BLOCKED,APRS_TX_FAILED,
    APRS_TX_CANCELLED,APRS_TX_EXPIRED
} APRS_TX_State;
typedef enum { APRS_LINK_SENT,APRS_LINK_BUSY,APRS_LINK_DENIED,APRS_LINK_FAULT } APRS_LinkResult;
typedef struct {uint32_t retryTicks,lifetimeTicks;uint8_t maxAttempts;} APRS_TX_Policy;
typedef struct {
    uint32_t (*now)(void *);
    APRS_LinkResult (*send)(const uint8_t *,size_t,uint32_t frequency,void *);
    void *context;
} APRS_TX_Link;
typedef struct {
    uint8_t frame[100];
    char sender[10],recipient[10],id[6];
    uint32_t frequency,nextTick,expiresTick;
    APRS_TX_Policy policy;
    APRS_TX_State state;
    uint8_t length,attempts;
    bool message;
} APRS_TX_Job;
extern APRS_TX_Job gAPRS_TX;
bool TX_QueueMessage(uint32_t frequency,const char *sender,const char *recipient,const char *text,
                     const char *id,APRS_TX_Policy policy,uint32_t now);
bool TX_QueueBeacon(uint32_t frequency,const char *sender,int32_t latitude,int32_t longitude,
                    uint32_t lifetimeTicks,uint32_t now);
void TX_Tick(uint32_t now);
void TX_Process(const APRS_TX_Link *link);
void TX_Cancel(void);
void TX_CancelAll(void);
#endif
