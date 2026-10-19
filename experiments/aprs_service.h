/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UVK5_APRS_SERVICE_H
#define UVK5_APRS_SERVICE_H
#include "app/aprs_tx.h"
#include "app/aprs_decode.h"
/* Session-only service prototype, absent from the normal firmware build.
 * The caller supplies explicit timing while ACK listening/routing is designed.
 * Three attempts means the initial message plus at most two retries.
 */
#define APRS_ACK_DEFAULT_TICKS 1000u /* Approved 10 seconds at 10 ms/tick. */
typedef struct {
    uint32_t frequency,acknowledgmentTicks,lifetimeTicks,controlLifetimeTicks;
    bool enabled;
} APRS_ServiceSettings;
typedef enum { APRS_SERVICE_IGNORED,APRS_SERVICE_ACK_QUEUED,
               APRS_SERVICE_ACK_REFUSED,APRS_SERVICE_DELIVERY_UPDATED } APRS_ServiceEvent;
bool SERVICE_Configure(const APRS_ServiceSettings *settings);
bool SERVICE_Enabled(void);
bool SERVICE_Message(const char *recipient,const char *text,const char *id,uint32_t now);
bool SERVICE_Beacon(uint32_t now);
/* Invoke before mailbox duplicate suppression: retransmissions also need ACKs.
 * A queued ACK is not a transmitted ACK. No ACKs of ACK/reject frames occur.
 */
APRS_ServiceEvent SERVICE_Receive(const APRS_Record *record,uint32_t now);
void SERVICE_Process(const APRS_TX_Link *link);
#endif
