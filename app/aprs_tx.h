/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_TX_H
#define UVK5_APRS_TX_H
#include "aprs_message.h"
/* Transport model only; not connected to a normal firmware send action yet.
 * Policy is explicit: the caller chooses retries and bounded queue lifetime.
 * Time is wrapping 10 ms scheduler ticks, not coalescing foreground flags.
 */
typedef enum {
    APRS_TX_IDLE, APRS_TX_QUEUED, APRS_TX_WAIT_ACK, APRS_TX_SENT,
    APRS_TX_ACKED, APRS_TX_REJECTED, APRS_TX_UNCONFIRMED,
    APRS_TX_BLOCKED, APRS_TX_FAILED, APRS_TX_CANCELLED, APRS_TX_EXPIRED
} APRS_TX_State;
typedef enum { APRS_LINK_SENT, APRS_LINK_BUSY, APRS_LINK_DENIED, APRS_LINK_FAULT } APRS_LinkResult;
typedef struct {
    uint32_t acknowledgmentTicks, lifetimeTicks;
    uint8_t maxAttempts;
} APRS_TX_Policy;
typedef struct {
    uint32_t (*now)(void *);
    APRS_LinkResult (*send)(const uint8_t *,size_t,void *);
    void *context;
} APRS_TX_Link;
typedef struct {
    uint8_t frame[100];
    char sender[10],recipient[10],id[6];
    uint32_t nextTick,expiresTick;
    APRS_TX_Policy policy;
    APRS_TX_State state;
    uint8_t length,attempts;
    bool needsAcknowledgment;
} APRS_TX_Job;
extern APRS_TX_Job gAPRS_TX;
bool TX_QueueMessage(const char *sender,const char *recipient,const char *text,
                     const char *id,APRS_TX_Policy policy,uint32_t now);
bool TX_QueueBeacon(const char *sender,int32_t latitude,int32_t longitude,
                    uint32_t lifetimeTicks,uint32_t now);
bool TX_QueueControl(const char *sender,const char *recipient,const char *id,
                     bool rejected,uint32_t lifetimeTicks,uint32_t now);
void TX_Process(const APRS_TX_Link *link);
bool TX_Acknowledge(const char *source,const char *addressee,const char *id,bool rejected);
void TX_Cancel(void);
#endif
