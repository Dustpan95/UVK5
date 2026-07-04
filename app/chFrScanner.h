#ifndef APP_CHFRSCANNER_H
#define APP_CHFRSCANNER_H

#include <stdbool.h>
#include <stdint.h>

// scan direction, if not equal SCAN_OFF indicates 
// that we are in a process of scanning channels/frequencies
extern int8_t            gScanStateDir;
extern bool              gScanKeepResult;
extern bool              gScanPauseMode;

#ifdef ENABLE_SCAN_RANGES
extern uint32_t          gScanRangeStart;
extern uint32_t          gScanRangeStop;
#endif

void CHFRSCANNER_Found(void);
void CHFRSCANNER_Stop(void);
void CHFRSCANNER_Start(const bool storeBackupSettings, const int8_t scan_direction);
// Returns true only when it advanced to a new channel/frequency (or started
// listening on a found signal) -- a real scan step. v1.3.3 adaptive dwell can
// choose to stay on the current channel for a full evaluation, which returns
// false so the scan-rate and Scan+Watch step counters are not fooled.
bool CHFRSCANNER_ContinueScanning(void);

#endif