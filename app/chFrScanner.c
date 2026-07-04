
#include "app/app.h"
#include "app/chFrScanner.h"
#include "driver/bk4819.h"   // VUURWERK v1.3.3: BK4819_GetRSSI (adaptive dwell)
#include "functions.h"
#include "misc.h"
#include "settings.h"
#include "scanwatch.h"
#include "toast.h"      // VUURWERK v1.3.0 (S4): EMPTY LIST refusal toast

int8_t            gScanStateDir;
bool              gScanKeepResult;
bool              gScanPauseMode;

#ifdef ENABLE_SCAN_RANGES
uint32_t          gScanRangeStart;
uint32_t          gScanRangeStop;
#endif

typedef enum {
	SCAN_NEXT_CHAN_SCANLIST1 = 0,
	SCAN_NEXT_CHAN_SCANLIST2,
	SCAN_NEXT_CHAN_DUAL_WATCH,
	SCAN_NEXT_CHAN_MR,
	SCAN_NEXT_NUM
} scan_next_chan_t;

scan_next_chan_t	currentScanList;
uint32_t            initialFrqOrChan;
uint8_t           	initialCROSS_BAND_RX_TX;
uint32_t            lastFoundFrqOrChan;

#ifdef ENABLE_ADAPTIVE_SCAN
// VUURWERK v1.3.3 adaptive scan dwell (bench-tunable). Each freshly-stepped
// channel first gets a short RSSI settle window (PRECHECK). If the squelch has
// not already opened (strong signal) and the RSSI is still at the noise floor,
// the channel is empty and we advance immediately -- skipping the ~90ms most of
// a scan spends on dead channels. Only a channel actually showing energy is
// held for the FULL dwell, so a weak signal still gets its complete squelch
// settle: sensitivity is unchanged by design, only empty channels get faster.
//
// The energy floor is the band's first-S-meter-bar RSSI calibration point
// (gEEPROM_RSSI_CALIB[band][0]) -- the same reference the on-screen meter uses,
// so "shows at least one bar" == "worth the full dwell". Conservative: below
// ~60ms total the PLL/squelch miss signals, so FULL stays at the stock 90ms.
// Tune on the bench with weak carriers: raise PRECHECK if any weak signal is
// skipped (RSSI not yet settled); it trades a little speed for margin.
#define SCAN_DWELL_PRECHECK_10ms   3   // 30ms quick RSSI settle before deciding
#define SCAN_DWELL_FULL_10ms       9   // 90ms full dwell for a channel with energy
static bool scan_precheck;
#endif

static void NextFreqChannel(void);
static void NextMemChannel(void);

void CHFRSCANNER_Start(const bool storeBackupSettings, const int8_t scan_direction)
{
	if (storeBackupSettings) {
		initialCROSS_BAND_RX_TX = gEeprom.CROSS_BAND_RX_TX;
		gEeprom.CROSS_BAND_RX_TX = CROSS_BAND_OFF;
		gScanKeepResult = false;
	}
	
	RADIO_SelectVfos();

	// VUURWERK v1.3.0 (F10): arm the Scan+Watch state machine from the
	// persistent menu setting on every scan launch. CHFRSCANNER_Stop
	// still disables the live machine; the armed setting survives, so
	// the operator no longer has to re-set ScnWch before every scan.
	if (gScanWatchArmed != 0 && gScanWatchArmed <= 2)
		SCANWATCH_Enable(gScanWatchArmed - 1);

	gNextMrChannel   = gRxVfo->CHANNEL_SAVE;
	currentScanList = SCAN_NEXT_CHAN_SCANLIST1;
	gScanStateDir    = scan_direction;

	if (IS_MR_CHANNEL(gNextMrChannel))
	{	// channel mode
		if (storeBackupSettings) {
			initialFrqOrChan = gRxVfo->CHANNEL_SAVE;
			lastFoundFrqOrChan = initialFrqOrChan;
		}
		// VUURWERK v1.3.0 (S4): refuse to start when the selected scan
		// list has no valid channels. Stock fell into a quasi-no-op loop
		// (FindNextChannel 0xFF -> MR_CHANNEL_FIRST -> invalid-channel
		// fallback to the freq channel) that showed SCANNING while
		// walking nothing. Probing here, after the backup block, lets
		// CHFRSCANNER_Stop restore cross-band state and park cleanly;
		// this also covers the mid-scan list-toggle restart path.
		if (RADIO_FindNextChannel(gNextMrChannel, scan_direction,
		                          (gEeprom.SCAN_LIST_DEFAULT < 2),
		                          gEeprom.SCAN_LIST_DEFAULT) == 0xFF) {
			CHFRSCANNER_Stop();
			TOAST_Show("EMPTY LIST");
			return;
		}
		NextMemChannel();
	}
	else
	{	// frequency mode
		if (storeBackupSettings) {
			initialFrqOrChan = gRxVfo->freq_config_RX.Frequency;
			lastFoundFrqOrChan = initialFrqOrChan;
		}
		NextFreqChannel();
	}

	gScanPauseDelayIn_10ms = scan_pause_delay_in_2_10ms;
	gScheduleScanListen    = false;
	gRxReceptionMode       = RX_MODE_NONE;
	gScanPauseMode         = false;
}

bool CHFRSCANNER_ContinueScanning(void)
{
#ifdef ENABLE_ADAPTIVE_SCAN
	// v1.3.3: this fired at the end of the short pre-check window. If a strong
	// signal already opened the squelch (FUNCTION_INCOMING) fall straight
	// through to the listen path. Otherwise decide on RSSI: energy above the
	// band's first-bar floor -> extend to the full dwell and stay put (a weak
	// signal's squelch gets its complete settle); only noise -> advance now.
	if (scan_precheck)
	{
		scan_precheck = false;
		if (gCurrentFunction != FUNCTION_INCOMING &&
		    BK4819_GetRSSI() >= gEEPROM_RSSI_CALIB[gRxVfo->Band][0])
		{
			gScanPauseDelayIn_10ms = SCAN_DWELL_FULL_10ms - SCAN_DWELL_PRECHECK_10ms;
			gScheduleScanListen    = false;
			return false;   // stayed on channel -- not a scan step
		}
	}
#endif

	if (IS_FREQ_CHANNEL(gNextMrChannel))
	{
		if (gCurrentFunction == FUNCTION_INCOMING)
			APP_StartListening(gMonitor ? FUNCTION_MONITOR : FUNCTION_RECEIVE);
		else
			NextFreqChannel();  // switch to next frequency
	}
	else
	{
		if (gCurrentCodeType == CODE_TYPE_OFF && gCurrentFunction == FUNCTION_INCOMING)
			APP_StartListening(gMonitor ? FUNCTION_MONITOR : FUNCTION_RECEIVE);
		else
			NextMemChannel();    // switch to next channel
	}

	gScanPauseMode      = false;
	gRxReceptionMode    = RX_MODE_NONE;
	gScheduleScanListen = false;
	return true;   // advanced to a new channel (or started listening)
}

void CHFRSCANNER_Found(void)
{
	switch (gEeprom.SCAN_RESUME_MODE)
	{
		case SCAN_RESUME_TO:
			if (!gScanPauseMode)
			{
				gScanPauseDelayIn_10ms = scan_pause_delay_in_1_10ms;
				gScheduleScanListen    = false;
				gScanPauseMode         = true;
			}
			break;

		case SCAN_RESUME_CO:
		case SCAN_RESUME_SE:
			gScanPauseDelayIn_10ms = 0;
			gScheduleScanListen    = false;
			break;
	}

	if (IS_MR_CHANNEL(gRxVfo->CHANNEL_SAVE)) { //memory scan
		lastFoundFrqOrChan = gRxVfo->CHANNEL_SAVE;
	}
	else { // frequency scan
		lastFoundFrqOrChan = gRxVfo->freq_config_RX.Frequency;
	}


	gScanKeepResult = true;
}

void CHFRSCANNER_Stop(void)
{
	if (SCANWATCH_IsActive())
	{
		// If we were on the watch VFO, switch back to scan VFO first
		if (SCANWATCH_IsOnWatchVFO())
		{
			gEeprom.RX_VFO = gScanWatch.scan_vfo;
			gRxVfo         = &gEeprom.VfoInfo[gEeprom.RX_VFO];
		}
		SCANWATCH_Disable();
	}

	if(initialCROSS_BAND_RX_TX != CROSS_BAND_OFF) {
		gEeprom.CROSS_BAND_RX_TX = initialCROSS_BAND_RX_TX;
		initialCROSS_BAND_RX_TX = CROSS_BAND_OFF;
	}
	
	gScanStateDir = SCAN_OFF;

	const uint32_t chFr = gScanKeepResult ? lastFoundFrqOrChan : initialFrqOrChan;
	const bool channelChanged = chFr != initialFrqOrChan;
	if (IS_MR_CHANNEL(gNextMrChannel)) {
		gEeprom.MrChannel[gEeprom.RX_VFO]     = chFr;
		gEeprom.ScreenChannel[gEeprom.RX_VFO] = chFr;
		RADIO_ConfigureChannel(gEeprom.RX_VFO, VFO_CONFIGURE_RELOAD);

		if(channelChanged) {
			SETTINGS_SaveVfoIndices();
			gUpdateStatus = true;
		}
	}
	else {
		gRxVfo->freq_config_RX.Frequency = chFr;
		RADIO_ApplyOffset(gRxVfo);
		RADIO_ConfigureSquelchAndOutputPower(gRxVfo);
		if(channelChanged) {
			SETTINGS_SaveChannel(gRxVfo->CHANNEL_SAVE, gEeprom.RX_VFO, gRxVfo, 1);
		}
	}

	RADIO_SetupRegisters(true);
	gUpdateDisplay = true;
}

static void NextFreqChannel(void)
{
#ifdef ENABLE_SCAN_RANGES
	if(gScanRangeStart) {
		gRxVfo->freq_config_RX.Frequency = APP_SetFreqByStepAndLimits(gRxVfo, gScanStateDir, gScanRangeStart, gScanRangeStop);
	}
	else
#endif
		gRxVfo->freq_config_RX.Frequency = APP_SetFrequencyByStep(gRxVfo, gScanStateDir);

	RADIO_ApplyOffset(gRxVfo);
	RADIO_ConfigureSquelchAndOutputPower(gRxVfo);
	RADIO_SetupRegisters(true);

#ifdef ENABLE_ADAPTIVE_SCAN
	// VUURWERK v1.3.3: arm the short pre-check window; the full dwell is
	// restored in CHFRSCANNER_ContinueScanning only if this channel shows
	// energy (empty channels advance after just the pre-check).
	gScanPauseDelayIn_10ms = SCAN_DWELL_PRECHECK_10ms;
	scan_precheck          = true;
#elif defined(ENABLE_FASTER_CHANNEL_SCAN)
	gScanPauseDelayIn_10ms = 9;   // 90ms
#else
	gScanPauseDelayIn_10ms = scan_pause_delay_in_6_10ms;
#endif

	gUpdateDisplay     = true;
}

static void NextMemChannel(void)
{
	static unsigned int prev_mr_chan = 0;
	const bool          enabled      = (gEeprom.SCAN_LIST_DEFAULT < 2) ? gEeprom.SCAN_LIST_ENABLED[gEeprom.SCAN_LIST_DEFAULT] : true;
	const int           chan1        = (gEeprom.SCAN_LIST_DEFAULT < 2) ? gEeprom.SCANLIST_PRIORITY_CH1[gEeprom.SCAN_LIST_DEFAULT] : -1;
	const int           chan2        = (gEeprom.SCAN_LIST_DEFAULT < 2) ? gEeprom.SCANLIST_PRIORITY_CH2[gEeprom.SCAN_LIST_DEFAULT] : -1;
	const unsigned int  prev_chan    = gNextMrChannel;
	unsigned int        chan         = 0;

	if (enabled)
	{
		switch (currentScanList)
		{
			case SCAN_NEXT_CHAN_SCANLIST1:
				prev_mr_chan = gNextMrChannel;
	
				if (chan1 >= 0)
				{
					if (RADIO_CheckValidChannel(chan1, false, 0))
					{
						currentScanList = SCAN_NEXT_CHAN_SCANLIST1;
						gNextMrChannel   = chan1;
						break;
					}
				}
				[[fallthrough]];
			case SCAN_NEXT_CHAN_SCANLIST2:
				if (chan2 >= 0)
				{
					if (RADIO_CheckValidChannel(chan2, false, 0))
					{
						currentScanList = SCAN_NEXT_CHAN_SCANLIST2;
						gNextMrChannel   = chan2;
						break;
					}
				}
				[[fallthrough]];
				
			// this bit doesn't yet work if the other VFO is a frequency
			case SCAN_NEXT_CHAN_DUAL_WATCH:
				// dual watch is enabled - include the other VFO in the scan
//				if (gEeprom.DUAL_WATCH != DUAL_WATCH_OFF)
//				{
//					chan = (gEeprom.RX_VFO + 1) & 1u;
//					chan = gEeprom.ScreenChannel[chan];
//					if (IS_MR_CHANNEL(chan))
//					{
//						currentScanList = SCAN_NEXT_CHAN_DUAL_WATCH;
//						gNextMrChannel   = chan;
//						break;
//					}
//				}

			default:
			case SCAN_NEXT_CHAN_MR:
				currentScanList = SCAN_NEXT_CHAN_MR;
				gNextMrChannel   = prev_mr_chan;
				chan             = 0xff;
				break;
		}
	}

	if (!enabled || chan == 0xff)
	{
		chan = RADIO_FindNextChannel(gNextMrChannel + gScanStateDir, gScanStateDir, (gEeprom.SCAN_LIST_DEFAULT < 2) ? true : false, gEeprom.SCAN_LIST_DEFAULT);
		if (chan == 0xFF)
		{	// no valid channel found
			chan = MR_CHANNEL_FIRST;
		}
		
		gNextMrChannel = chan;
	}

	if (gNextMrChannel != prev_chan)
	{
		gEeprom.MrChannel[    gEeprom.RX_VFO] = gNextMrChannel;
		gEeprom.ScreenChannel[gEeprom.RX_VFO] = gNextMrChannel;

		RADIO_ConfigureChannel(gEeprom.RX_VFO, VFO_CONFIGURE_RELOAD);
		RADIO_SetupRegisters(true);

		gUpdateDisplay = true;
	}

#ifdef ENABLE_ADAPTIVE_SCAN
	// VUURWERK v1.3.3: short pre-check window (see NextFreqChannel).
	gScanPauseDelayIn_10ms = SCAN_DWELL_PRECHECK_10ms;
	scan_precheck          = true;
#elif defined(ENABLE_FASTER_CHANNEL_SCAN)
	gScanPauseDelayIn_10ms = 9;  // 90ms .. <= ~60ms it misses signals (squelch response and/or PLL lock time) ?
#else
	gScanPauseDelayIn_10ms = scan_pause_delay_in_3_10ms;
#endif

	if (enabled)
		if (++currentScanList >= SCAN_NEXT_NUM)
			currentScanList = SCAN_NEXT_CHAN_SCANLIST1;  // back round we go
}
