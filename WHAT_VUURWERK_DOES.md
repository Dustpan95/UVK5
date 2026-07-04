# What VUURWERK Actually Does

**v1.5.0: Plain English Guide to Every Feature**

VUURWERK is custom firmware for the Quansheng UV-K5 radio. It keeps everything the stock radio does, then adds more than forty features on top, all fitting in the same 60KB of flash memory. Here's what each one does and why it matters.

## The headline arc: v1.3.3 through v1.5.0

The recent releases turned the Voice-Seeking Spectrum into a complete
find-a-conversation instrument (directional seek on UP/DOWN, PTT voice lock,
frequency carry-over on exit), added the F+EXIT reset-to-simplex rescue key,
made the spectrum exit fully deterministic (no VFO flip, no deaf radio), made
scanning adaptive (2-3x faster on quiet bands), and retired the little-used
MTI display mode. See the versioned "Changes" subsections at the bottom of
this document and Sections 3, 6, and 7 of USER_MANUAL.md for the operator
details.

## Numbering notes

Activity Log retired in v1.2.5 (Feature #27 slot intentionally empty) -- the 128x64 LCD couldn't show enough log context to be useful in HT form factor. MTI spectrum mode (former Feature #20) retired in v1.4.0. Numbering keeps the original slots so long-time readers stay oriented.

---

## Transmit (TX) Improvements

### 1. TX Soft Start
**What it does:** When you press PTT, the radio's power amplifier used to slam on instantly. That creates a "click" or "pop" that other operators hear. VUURWERK ramps the power up smoothly over 60 milliseconds using an S-shaped curve, the same shape used in professional broadcast equipment. The result is a clean, click-free key-up.

### 2. CTCSS Lead-In
**What it does:** Many repeaters require a sub-audible tone (CTCSS) to open. The problem is that stock radios start sending your voice at the same instant as the tone. The repeater hasn't decoded the tone yet, so it clips your first word. VUURWERK mutes your microphone for 150ms after PTT, letting the tone reach the repeater first. Then your voice comes through clean.

### 3. TX Audio Compressor
**What it does:** Makes your transmitted audio louder and more consistent. If you speak quietly, it boosts you. If you shout, it holds you back. It measures your mic level 100 times per second, computes the average loudness, and adjusts the microphone gain in real time. The attack is fast (5ms, catches sudden loud sounds) and the release is slow (300ms, smooth fade back up). The result: you sound punchy and clear, like a commercial two-way radio.

---

## Receive (RX) Signal Processing

### 4. FM Gain Staging
**What it does:** The stock radio has fixed receive gain: it doesn't adapt to signal strength. Strong signals overdrive the front end and sound distorted. Weak signals get buried. VUURWERK watches the signal strength and automatically adjusts the radio's internal amplifier chain (LNA, mixer, PGA) to keep the audio clean at any signal level. It reacts fast when a strong signal appears (to prevent overload) and releases slowly (to avoid pumping). When you change frequency, it resets to be ready for whatever's there.

### 5. RSSI Smoothing Filter
**What it does:** The raw signal strength reading from the radio chip bounces around rapidly, making the S-meter jittery and hard to read. VUURWERK applies an exponential moving average: each new reading is blended with the previous ones, producing a smooth, stable S-meter that still responds quickly to real signal changes. Think of it like shock absorbers for your signal meter.

### 6. RSSI Histogram
**What it does:** Quietly builds a statistical picture of the signal environment on your frequency. It sorts hundreds of signal strength readings into bins and finds the "noise floor," the level where most readings cluster. This tells the smart squelch system exactly where the noise ends and real signals begin, without you having to set anything.

### 7. Signal Classifier
**What it does:** Identifies what kind of signal you're receiving by measuring how fast it appears. FM voice signals pop up almost instantly (under 50ms). SSB and AM voice ramp up over 50-200ms. Carriers and CW are slow risers (over 200ms). Random noise never stabilizes. The classifier labels each signal as F (fast/FM), N (normal/SSB), S (slow/carrier), or ~ (noise) and displays the symbol on the status line during RX. This classification also feeds into the gain staging system so it can adjust hold times appropriately for each signal type.

### 8. Signal Quality Meter
**What it does:** Measures how stable a signal is over time. A solid, nearby signal has very little variation in strength. A signal bouncing off buildings or mountains fluctuates constantly. VUURWERK tracks this variance and rates the signal as Excellent, Good, Fair, or Poor, giving you a quick read on propagation conditions without needing test equipment.

### 9. S-Meter Compensation
**What it does:** When the gain staging system adjusts the radio's internal amplifiers, the raw signal strength reading shifts. Without correction, your S-meter would show the wrong value after a gain change. VUURWERK knows exactly how much gain it added or removed and compensates the S-meter reading so it always reflects the true signal strength.

### 10. AM Fix
**What it does:** Inherited from the OneOfEleven project. Prevents the BK4819 chip's AM demodulator from saturating on strong signals, which would cause severe distortion. VUURWERK shares the gain lookup table between this module and FM gain staging to save memory.

---

## Squelch

### 11. Squelch Tail Elimination
**What it does:** When someone stops transmitting on a repeater, there's usually a brief burst of noise ("KSSHHH") before the squelch closes. This is the "squelch tail" and it's annoying. VUURWERK watches the radio chip's CTCSS tone detector. The instant the tone disappears, it mutes the audio, before the noise burst reaches the speaker. After 150ms of silence, it unmutes. This works with all three industry methods (Motorola, Kenwood, and Chinese radios) because they all cause the tone to disappear. You can turn this on or off in the STE menu.

### 12. Intelligence-Based Squelch
**What it does:** The stock squelch uses a single fixed threshold: if the signal is above it, the squelch opens. VUURWERK reads three different measurements from the radio chip simultaneously: signal strength, audio noise level, and a glitch/transient counter. It blends these together to compute a "voice probability" score from 0 to 100. When it's confident there's a real voice signal, it loosens the squelch to let it through. When it detects noise or interference, it tightens the squelch. The result: fewer false opens on noisy bands, and real signals get through more easily. This runs all the time in the background.

### 13. Adaptive Squelch State
**What it does:** A supporting data module that tracks the noise floor and squelch open count. This information is consumed by the intelligence-based squelch to make better decisions about what's noise and what's signal.

---

## Scanning and Monitoring

### 14. Scan+Watch
**What it does:** Scans through frequencies on one VFO while periodically checking the other VFO for activity, like having two radios in one. Every four scan steps, it briefly hops to the watch VFO, listens for 100ms, and if it hears something, it stays there for 2 seconds. If the signal persists, it keeps listening. If it drops off, scanning resumes. This is a feature normally found only on radios costing $300+.

### 15. Intelligent Dual-Watch
**What it does:** Stock dual-watch alternates between two VFOs at a fixed rate. VUURWERK makes it smarter: it tracks activity on each VFO and adjusts the dwell times proportionally. A busier VFO gets shorter dwell time (you'll catch it anyway), while the quieter one gets more listening time (so you don't miss brief transmissions). The base dwell is a stock-class 150ms (restored in v1.3.1) with an activity bias on top, so a call starting on the other VFO is caught within a couple hundred milliseconds. Activity counters decay over time so the system adapts to changing conditions. Since v1.3.3 the alternation also pauses while you are holding a key, so manual tuning is never interrupted. This runs automatically whenever dual-watch is enabled.

### 16. VFO Split
**What it does:** Allows independent transmit and receive frequencies on separate VFOs. The radio periodically hops to VFO B to check for activity, then returns to VFO A. If it detects a signal on B above -100 dBm, it alerts you. Supports scanning memory channels, frequency ranges, or scan lists on the B side. Enable it from the menu: go to the Scan category, select ScnWch (Scan+Watch), and choose SPLIT. The 5-state hop machine (IDLE, HOP_TO_B, SETTLE, READ_RSSI, RETURN_TO_A) runs every 300ms by default without interrupting your VFO A operation.

### 17. Bandscope
**What it does:** Shows a miniature spectrum display on the main screen: a scrolling timeline of signal strength on your current frequency. New readings scroll in from the right. Peak dots show the strongest signal seen at each position, decaying over time. A dotted line marks the noise floor. Toggle with F+7. It's like a heartbeat monitor for your frequency.

---

## Spectrum Analyzer (3 Modes)

The spectrum analyzer (F+5) scans a range of frequencies and shows what's active. Press the Star key to cycle through three display modes: NORM, PEAK, VOX. (The former fourth mode, MTI, was retired in v1.4.0 -- see the numbering notes.)

**Keymap (v1.5.0).** Inside the spectrum view, SIDE1 / SIDE2 step the span up / down in every mode (auto-repeat while held). UP / DOWN are mode-aware: span step in NORM / PEAK, directional voice seek in VOX. MENU short = blacklist the current peak bin. PTT parks on the peak (NORM / PEAK) or locks the voice you are hearing (VOX). EXIT leaves -- always back on your own VFO, on your displayed frequency.

### 18. NORM: Normal Spectrum
**What it does:** Standard spectrum display. Each frequency bin shows a bar proportional to signal strength. Simple, clean, effective.

### 19. PEAK: Peak Hold
**What it does:** Same as NORM, but adds dots above the bars showing the strongest signal seen at each frequency. The peaks slowly decay over time. Useful for catching brief transmissions you might otherwise miss.

### 20. (retired) MTI: Moving Target Indicator
Removed in v1.4.0. A sweep-to-sweep change display borrowed from radar; research found essentially nobody used it, and its flash bought back the About screen's feature list plus RAM. PEAK -- which people genuinely rely on -- stays.

### 21. VOX: Voice-Seeking Spectrum
**What it does:** The headline feature. Instead of showing all signals equally, it identifies which ones are likely carrying voice. For each frequency bin, it reads the radio chip's noise indicator alongside the signal strength. Clean audio with a strong signal scores high. Noisy, weak, or interference-heavy bins score low -- so carriers and birdies stay flat while conversations get bars.

The display shows confidence: bins below the hop threshold of 50 show just a floor dot, 50-74 draw half-height bars, 75+ draw full-height bars, and every eligible bin gets a marker tick at the very top of the plot so the hop targets form a clear row of dots.

**Seeking (v1.3.5):** UP walks to the next voice bin above the cursor, DOWN below -- no wrap-around, so UP always raises frequency. When there is no voice that way, the key falls through to plain band navigation, so UP/DOWN always moves you; hold a key and the radio patrols the band, auto-stopping on voice with an ascending two-tone chord. The sweep also auto-listens on its own: when the strongest bin isn't voice, it seeks the strongest actual voice bin in the span, and it holds through natural speech pauses (the noise gate is the hold authority, with 2-strike hysteresis so a single flutter can't chop a QSO).

**Locking (v1.3.6):** while listening to a station, tap PTT. A `LOCKED` banner flashes and the radio parks on that frequency as an open-squelch monitor -- you hear both sides of the QSO and anyone else who keys up, through any pause, however long. UP or DOWN releases the lock and resumes seeking.

**Carry-over (v1.4.2, hardened through v1.5.0):** exit while locked and -- if the radio is in frequency mode and the lock is inside your VFO's band -- the locked frequency is written into your VFO and saved. You leave the spectrum already tuned, ready to answer in simplex. In memory-channel mode the exit is a provable no-op: a memory channel can never be modified from the spectrum.

---

## User Interface

### 22. RSSI Bar
**What it does:** A visual signal strength meter on the main display, compensated for gain staging adjustments so it always reads accurately in FM mode.

### 23. Context-Aware Status Line
**What it does:** The bottom line of the display changes based on what the radio is doing. Idle shows the firmware name. RX shows signal info. TX shows power. Scan shows progress. It's always relevant information without cluttering the screen.

### 24. Boot Screen
**What it does:** Shows the VUURWERK name and version when the radio powers on.

### 25. About Screen
**What it does:** Navigate to Menu > CONFIG > About to see firmware info and feature highlights.

### 26. Categorized Menu
**What it does:** The stock radio has 60+ menu items in one flat list. VUURWERK organizes them into 7 categories: Receive, Tone, Transmit, Scan, Channel, Config, and a hidden Unlock category. You scroll through categories first, then items within. Finding settings takes seconds instead of minutes.

---

## v1.2.5+ Additions

### 28. Side-Button Toast Feedback
**What it does:** The 1-second toast overlay that confirms F+key shortcuts now also fires for SIDE1, SIDE2, and long-press MENU presses. Whichever shortcut path you used, the radio answers the same way. Closes the UX-parity gap where F+key actions had visible confirmation but side-button actions executed silently.

### 29. Toast Notification Subsystem
**What it does:** The toast popup engine extracted into its own module. Same 1-second centered overlay, same 10ms tick decrement, now reusable from any caller. Powers the F+key shortcut feedback, the side-button feedback (Feature #28), the F-hold lock toast, and the scan-state toasts.

### 30. Scan Rate Telemetry
**What it does:** While scanning, the status line now shows the live channels-per-second sweep rate. A rolling 100-tick counter measures how fast the scanner is actually advancing through channels and the renderer appends "NNc/s" to the SCANNING text. Lets you see at a glance whether scanning is healthy or whether something has slowed it down.

### 31. F-Key UX Hardening
**What it does:** Three small reliability fixes wrapped around the F-key dispatch path. Long-pressing F (the keypad lock gesture) now toasts "LOCKED" or "UNLOCKED" so you can tell whether the lock actually toggled. F+3 (VFO/MR toggle) toasts "NO CHANNELS" when memory mode is empty instead of the misleading "VFO MODE" stock behavior. F+STAR (CSS scanner) is now gated to FM mode only and toasts "FM ONLY" if you try it in AM or USB, because the BK4819 CTCSS detector only populates in FM and the scan would otherwise loop forever. STAR long-hold (scan toggle) gets its own "SCAN ON" / "SCAN OFF" toast.

### 32. Flashlight Auto-Off Watchdog
**What it does:** A forgotten flashlight in a pocket will drain a UV-K5 pack overnight on every stock fork. VUURWERK ticks a watchdog every 10ms in software (no hardware timer added) and extinguishes the flashlight after 30 minutes of unattended ON or BLINK. SOS mode is preserved indefinitely because that's emergency signaling. On a low-battery pack (battery indicator level 2 or lower) the timeout shortens to 10 minutes so a half-drained pack doesn't get fully drained by a forgotten LED. Mode changes reset the timer so tapping the flashlight key extends the session. Standard watchdog technique, original implementation for the UV-K5 platform.

### 33. CSS Scan Soft-Timeout Watchdog
**What it does:** Companion to Feature #31's F+* gate. If a CSS scan is launched and runs for 60 seconds without finding a tone (the FM-with-no-tones-present trap that even the gate cannot fully prevent), this watchdog flips the scanner state to FAILED and queues an audible double-beep. The stock scanner state machine handles BK4819 teardown via its existing default branch. Closes the unbounded-scan trap that egzumer's binary `ENABLE_NO_CODE_SCAN_TIMEOUT` flag turns into either 16 seconds or forever.

### 34. Audio Palette: VOX-Hop "Found Voice" Two-Tone Chord
**What it does:** In VOX spectrum mode (Feature #21), when the voice-hop search finds a bin with high voice probability, the radio plays a short ascending two-tone chord (800 Hz then 1200 Hz) so you hear that a hop happened before the audio from the new bin even starts. Companion to the existing rejection idiom: on a sweep that finishes without finding voice, the radio plays a soft 500 Hz double-beep instead, so silence never leaves you wondering whether the keypress registered.

### 35. Boot-Time Hardware Health Probe
**What it does:** When the radio powers on, before the welcome screen renders, it runs a two-part health check. First it reads BK4819 register 0x00 after init and looks for the wedged-SPI 0xFFFF signature, which means the transceiver chip is not responding. Then it reads the factory battery-calibration page in EEPROM and looks for an all-0xFF byte pattern, which means the calibration is blank or the I2C bus is wedged. Either fault surfaces as a specific welcome banner: "BK4819 FAULT / RX/TX disabled" or "EEPROM FAULT / calib lost".

### 36. Live Battery Voltage During TX
**What it does:** Stock firmware freezes the battery indicator the instant you press PTT and only updates it when you release. VUURWERK adds a 500ms tick during transmit that advances the existing battery-voltage ring buffer and recomputes the on-screen battery percent and icon. You see voltage sag in real time as you transmit. Closes a long-standing gap in the stock firmware.

### 37. Backlight Fade-Out Tail
**What it does:** When the backlight idle timer expires, stock firmware cuts the backlight to black instantly. VUURWERK ramps it down over 2 seconds via the existing PWM brightness API so the screen tapers off smoothly. Original implementation of a standard backlight-fade UX pattern, adapted to the BK4819 platform's PWM constraints.

### 38. Backlight TX/RX Activity Refresh
**What it does:** Companion to Feature #37. If you have the per-mode "backlight on TX" or "backlight on RX" setting enabled, VUURWERK re-arms the backlight countdown every 500ms while you are actually transmitting or receiving, so the screen never goes dark mid-conversation. The stock firmware's one-shot backlight-on at TX/RX entry would still time out mid-call; this extends that to a hold-for-duration. Operators historically worked around this by setting backlight-always-on at the cost of battery life. VUURWERK now keeps it lit only when needed.

### 39. TX Battery Sag Delta Tracker
**What it does:** Each time you transmit, the radio records the battery voltage at PTT press, watches for the minimum during transmit, and at PTT release computes the sag delta in 10mV units. The About screen shows the most recent delta as "TX sag NNNNmV". Lets you see whether your pack is healthy (small sag) or worn out (large sag) without test equipment. Per-TX battery accounting, original to VUURWERK.

### 40. CSS Scan Status-Bar Glyph
**What it does:** A small "Cs" glyph in the status bar tells you a CSS scan is running, distinct from the existing channel-scan and SCAN+WATCH glyphs. Three concurrent scan types are now visually distinguishable at a glance.

### 41. CSS Scan FOUND Beep
**What it does:** When the CSS scanner locks onto a tone, the radio emits a single 1 kHz beep on the FOUND state edge. Symmetric with Feature #33's failure cue. Both the F+* path (from the main screen) and the menu path (R_CTCS / R_DCS submenus) trigger the same beep. A one-byte BSS latch prevents repeated beeping while the FOUND state persists.

### 42. Quiet Backlight PWM
**What it does:** Stock backlight PWM runs at about 1 kHz, which sits right in the middle of the audio passband and bleeds an audible whine into outbound TX audio whenever the screen is dimmed during transmit. VUURWERK reprograms the PWM prescaler at boot to push the carrier up to about 6.7 kHz, well above the 3 kHz audio passband. The whine drops below noise floor. kamilsss655's NUNU fork demonstrated this technique in 2024 via stock-driver source modification; VUURWERK ports the same fix as a LAW-1-safe one-line hook in main.c so the driver stays byte-identical with the egzumer parent.

---

## v1.3.3+ Additions

### 43. Adaptive Scan (v1.3.3)
**What it does:** Every stepped channel gets a ~30 ms energy pre-check; only channels reading above the band's first-bar level are held for the full evaluation dwell, so empty channels advance immediately. Scanning a quiet band runs about 2-3x faster, and any channel showing energy still gets the identical full check -- weak-signal sensitivity is unchanged by design. Tone-coded memory channels get a 500 ms dwell so the CTCSS/DCS decoder actually has time to confirm (they used to be walked past on the flat 200 ms carrier dwell).

### 44. F+EXIT: Reset to Simplex (v1.4.4)
**What it does:** The rescue key. One gesture wipes the active VFO back to a clean simplex state: RX and TX CTCSS/DCS cleared, repeater offset and direction cleared (TX = RX), frequency reverse off, scrambler off, modulation back to FM -- saved, so it sticks. Toast: `SIMPLEX`. This is the cure for the classic "green light but silent" VFO, where a leftover RX tone holds the audio shut for a tone that never comes. Frequency mode only; memory channels are never touched (`FREQ ONLY` toast).

### 45. Deterministic Spectrum Exit (v1.4.3-v1.5.0)
**What it does:** Leaving the spectrum analyzer is now boring, which is the point. Every exit -- locked or not, dual watch on or off, even if you hold the EXIT button a beat too long -- returns you to the VFO you entered from, retunes the chip to your displayed frequency, and leaves the audio path alive. This closed out the two longest-running field reports in the project: "exiting the spectrum flips me to the other VFO" and "after the spectrum the radio looks fine but hears nothing."

---

## v1.5.0 Changes

**The stabilization release.** Every spectrum entry/exit path, the VOX lock lifecycle, the carry-over fences, tone squelch gating, and the F+EXIT reset were exercised end-to-end and re-verified before shipping. Two subtle exit bugs found and fixed in the process: holding EXIT slightly long could re-trigger a VFO swap on the main screen after leaving, and one exit path could leave the radio mistuned and mute until something else retuned it. Both gone.

## v1.4.x Changes

**F+EXIT reset-to-simplex added** (v1.4.4, fixed to actually fire in v1.4.5 -- the chord initially collided with a stock key-cancel path and performed a VFO flip instead; v1.4.5 also made it reset modulation to FM).

**VOX lock carry-over re-engineered** (v1.4.1-v1.4.2): the original carry-over could flip the active VFO and mute the receiver on exit. It was removed, then rebuilt as a direct VFO write that mirrors the known-good normal exit path exactly. Crossband and reverse edge cases hardened in v1.4.5.

**Deterministic exit** (v1.4.3): every spectrum exit re-syncs to the operator's own VFO and grants dual watch a fresh dwell, so the "flips me to the B side" pattern -- dual watch parking you on whichever side had signal -- can't fire at the moment of exit.

**MTI spectrum mode removed** (v1.4.0): the spectrum now cycles NORM / PEAK / VOX. The freed flash restored the About screen's feature list plus 128 bytes of RAM.

**PTT in VOX only ever locks** (v1.4.0/v1.4.5): it no longer drops you into the register-editor screen, in any span width.

## v1.3.4 / v1.3.5 Changes

**The signature bottom-line RX readout came back** (v1.3.4): the "RX S7 -65dBm Q:3 F" line returned to its established bottom-row placement, now sharing one calibrated dBm computation with the center RSSI bar so both always agree.

**UP/DOWN in VOX became a directional voice seek** (v1.3.5): UP always raises frequency toward the next voice, DOWN lowers, no wrap-around orbiting, and a quiet band simply navigates instead of dead-ending on a beep. Two latent bugs fixed with it: a fresh VOX entry could show one sweep of phantom voice bins, and a hop could land on (and silently un-blacklist) a blacklisted bin.

## v1.3.3 Changes

**The radio keeps its hands off the dial while yours are on it.** A field report: holding UP to step through the 2 m band, the radio suddenly showed a memory channel instead. Source analysis found the autonomous engines did not respect operator input: dual watch kept flipping VFOs and retuning every 150 ms while a key was held (fighting every step), and VFO Split kept launching hops that rewrite VFO B's display mid-keyhold. Both now yield while any key is held and resume the moment you release -- the same courtesy the power-save engine has always shown. Dual watch still catches calls exactly as before when your hands are off the keys.

**A scrambled setting can no longer switch dual watch on by itself.** If the dual-watch configuration byte in EEPROM reads back corrupt, stock firmware "defaulted" it to ON -- a damaged radio would quietly start retuning itself. It now defaults to OFF, matching how the crossband setting has always been validated. Valid settings are untouched.

## v1.3.2 Changes

**The boot banner now actually shows up.** Since the project's v1.2.4 baseline, the "Full" power-on display mode -- the factory-default setting on every radio -- booted to a solid black screen instead of the VUURWERK banner, because stock firmware treats "full" as an all-pixels-on LCD test. The banner only appeared if your POnMsg setting happened to be "Message". A May cleanup commit made "None" black too (stock "None" is blank). As of v1.3.2: Full and Message both show the VUURWERK banner with the version number, Voltage shows the voltage screen, None is genuinely blank. Fresh flashes and factory-default radios now greet you properly. Found because an operator's recovery from a botched flash reset POnMsg to the factory default and the "new" black splash got reported within the hour.

## v1.3.1 Changes

**The menu now explains itself.** Forty cryptic menu items show a plain-language hint on the bottom line while you browse them: highlight `RxMode` and the screen says `DUAL WATCH / CROSSBAND RX`; highlight `TxODir` and it says `REPEATER OFFSET DIRECTION +/-`. Nothing was renamed and nothing moved -- if you knew the menu before, you still know it; if you did not, the radio now tells you. This came directly from a field report: an operator who wanted dual watch could not find it because nothing on the radio said "dual watch."

**VFO Split announces itself.** Also from a field report: split was armed, VFO B started walking channels on its own, and it looked exactly like a scanning bug. Now an `SP` glyph sits in the status bar the whole time split is armed, and when split actually finds activity it shows an `ACT` toast with the frequency and sounds a single 880 Hz cue (the alert plays even with key beeps off -- arming split is opting into being alerted). Before this release the activity alert was computed internally and displayed nowhere.

**Dual watch reacts about four times faster.** The alternating dwell dropped from 500 ms back to a stock-class 150 ms base. You stop losing the first word of a call that starts on the other VFO.

**Weak voice opens the squelch sooner.** The intelligence-based squelch used to engage only after the squelch had already opened. It now also evaluates while the radio is parked, and when the signal pattern scores strongly voice-like it eases the open threshold by up to 2 steps (~1 dB). It never runs during scans, while split is armed, or in power save, and it backs off the moment confidence drops.

**DCS channels get tail elimination too.** STE previously covered CTCSS only; DCS transmissions ended with the full noise burst. The same state machine now watches the DCS code detector, with a slightly longer mute window matched to the DCS turn-off code's ~180 ms duration.

**A tired battery tells you.** If the pack sags 0.20 V or more under TX load, a `SAG 0.25V`-style toast appears after you unkey. Healthy packs sag 0.05 to 0.10 V.

**The spectrum analyzer sweeps a little faster.** One redundant register read per sweep step was eliminated (it re-read a value that cannot change mid-sweep).

**Honesty corner:** the TX audio compressor was bench-measured this release after an audit claimed it over-compressed all speech. The measurements disproved the claim: at normal speaking distance it is essentially transparent, and on loud or close speech it compresses progressively (about 2:1 to 3:1) up to a designed ~7.5 dB ceiling -- which is correct behavior for voice, so nothing was changed. The diagnostic build that settled the question stays available for future tuning. Also: one supposed v1.3.1 fix (silent keypad-lock digit presses) was dropped during implementation because code review showed the firmware already beeps there -- the audit claim was wrong, and we do not ship fixes for bugs that do not exist.

## v1.3.0 Changes

**The 0.4-second deaf dip after changing channels is gone.** Every FM channel change or VFO flip used to slam the receiver front end to minimum gain (-93 dB) for almost half a second while an internal gain ladder climbed back up, and tuning onto a moderately strong signal could drop it entirely for half a second. The gain logic treated the stock-gain entry as the bottom of the ladder; it now enters the ladder where stock gain actually sits. The same fix is applied to the AM gain manager (one-shot dive after boot).

**Dual watch no longer misses calls right after traffic.** The receiver gain register is shared between both VFOs, but each VFO kept its own private notion of what was applied. After a transmission ended on one VFO, the other VFO could spend a second or two listening through stale attenuation. A once-per-second refresh now re-asserts the active VFO's gain.

**Exiting the spectrum analyzer no longer leaves the radio deaf.** Stock firmware retuned to the left edge of the sweep span on exit, hundreds of kHz below the displayed frequency, until some unrelated event retuned the radio. Exit now returns to your VFO frequency.

**AM channels with a stray CTCSS code no longer chop.** An AM (e.g. airband) channel that carried a leftover CTCSS code in its channel record, with STE on, muted and unmuted about five times a second. STE now arms only when CTCSS is genuinely in play (FM).

**F+9 calling-frequency jump works correctly under dual watch.** It used to act on whichever VFO the dual-watch background flip happened to hold, and the restore could land on the other VFO. It now always operates on your active VFO and restores to the VFO it saved from.

**Squelch returns to its set level after a conversation.** The adaptive squelch loosens while you are receiving voice; that loosened threshold used to persist after the signal ended, letting noise pop the squelch more easily until you changed channels. It is restored the moment reception ends.

**VFO Split now genuinely patrols your memories.** SPLIT mode used to re-check VFO B's parked channel forever while claiming to scan; it now steps through every valid memory channel and VFO B's display shows the channel being checked. Power save is disabled while SPLIT is armed (a 300 ms patrol cannot sleep) -- expect scan-like battery drain.

**Scan+Watch is a setting now, not a one-shot.** ScnWch survives scan stop and re-arms on every scan start. Armed-but-idle radios also no longer flip VFOs during power-save wakeups with dual watch off.

**Starting a memory scan on an empty scan list says EMPTY LIST** and stays parked, instead of showing SCANNING while walking nothing.

**Smaller details:** the CSS tone scanner's NO SIGNAL abort no longer leaks a previous session's save prompt; the AGC lock in the spectrum analyzer's gain editor now actually engages in AM mode (a stock encoding bug ate the request); the TX status line shows the transmitting VFO's power letter under crossband; the signal classifier letter (F/N/S/~) no longer carries the previous channel's verdict across a hop.

**~900 bytes smaller than v1.2.8, with all of the above fixed.** Four reclamations paid for everything: an unreachable scanner mode and dead VFO Split code were removed, a long-dead data structure was deleted, and the bold small font was dropped (bold UI text now renders in the regular small font -- the only visible cost in this release).

**LAW 1B.** VUURWERK is now a true fork: stock egzumer files may be edited to fix bugs or measurably improve the firmware. Every such edit is marker-tagged in the source.

## v1.2.8 Changes

### Scanner deafness fix (front-end gain staging)
**What it does:** the radio received fine when parked on a frequency but went deaf once you started scanning. VUURWERK's FM gain-staging (the software front-end AGC that adapts receiver gain to signal strength) was running during scans with no scan guard, and a per-hop off-by-one slammed the BK4819 front end to minimum gain (-93 dB) for most of each channel's ~90 ms dwell. The starved gain pulled the measured RSSI below the squelch threshold, so weak signals were walked past. v1.2.8 makes gain-staging stand aside during scans and lets the radio's hardware AGC own gain, the same as stock egzumer, so scanning hears signals reliably again. Parked reception is unchanged. Reported by Scott, KC7WDG.

### Scan+Watch audio restore
**What it does:** when Scan+Watch finished listening on the watch VFO and returned to the scan VFO, audio could stay muted because the radio was left in its receiving state with no audio re-enable. v1.2.8 returns it to a clean scanning state, so audio comes back on the next signal found.

### F+3 "NO CHANNELS" toast fix
**What it does:** pressing F+3 to toggle between Frequency and Memory mode always flashed a misleading "NO CHANNELS" message, even when the toggle succeeded, because the check read a channel field that updates a moment later. v1.2.8 reads the field that updates immediately, so you now see "MR MODE" / "VFO MODE" on success and "NO CHANNELS" only when memory really is empty.

### Community CHIRP driver
**What it does:** a community-contributed CHIRP driver (`uvk5_vuurwerk.py`, by mjwgeek) lets you read and write channels, side-keys, and settings from a computer without flashing back to stock egzumer first. It ships in the `chirp/` folder with a how-to; it does not touch the spectrum analyzer settings. GPL.

---

## v1.2.7 Changes

### SIDE2 keymap symmetry fix
**What it does:** v1.2.6 bound SIDE2 short-press to freq-down with a 640 ms long-press for backlight toggle, on the theory of "clean short/long separation." Operators found this wrong: SIDE1 hold gives continuous freq-up auto-repeat, but holding SIDE2 toggled the backlight instead. v1.2.7 makes SIDE2 symmetric with SIDE1: short press is one freq-down step, hold is continuous freq-down auto-repeat. Backlight toggle is no longer reachable from inside spectrum view; use EXIT, main-screen toggle, F+5 to re-enter. The keymap state-machine code (release-dispatch + hold counter + dual-binding latch) is gone, simplifying the input loop.

### CSS scanner robustness
**What it does:** the CSS scanner was unreliable in v1.2.5 / v1.2.6 -- operators reported it rarely locked onto tones. v1.2.7 rebuilt it with four changes:
- Pre-flight RSSI gate: before launching the BK4819 scan engine, the scanner samples RSSI on the current frequency. Below carrier-present threshold (~-110 dBm) the scan aborts with a `NO SIGNAL` toast and 500 Hz double-beep. Park on an active carrier before scanning.
- CTCSS now locks on first confirmation (was 2). The asymmetry with DCS (which already locked on first confirmation) made CTCSS feel broken; with the pre-flight gate plus the BK4819 detector's internal voting, single-match is reliable.
- Dwell time reduced from 210 ms to 120 ms. The BK4819 detector converges in ~80-120 ms for the worst-case low CTCSS tones; 210 ms was over-conservative.
- Soft-timeout tightened from 60 s to 30 s. Typical successful scans now complete in 3-10 s; 30 s is the new "fruitless" cap.

### Economic re-engineering
**What it does:** to offset the SIDE2 state-machine removal and CSS work cost, v1.2.7 reclaimed bytes from VUURWERK feature modules. Five dead-code / redundant-init / API-tightening passes: TX_COMPRESSOR_GetGainReduction removed (zero callers, dead code), RSSI_HISTOGRAM_Init removed (BSS zero-init covers it), GAIN_STAGING_Reset made static (zero external callers, LTO inlines), bandscope.c parallel memmoves consolidated, SIGNAL_CLASSIFIER_Init removed (BSS zero-init covers it). Plus one CSS-scanner-scoped consolidation: the CDCSS and CTCSS arms in SCANNER_TimeSlice10ms became structurally identical after the 1-confirmation change and were merged. Net flash freed ~200 bytes vs v1.2.6.

### License header enforcement sweep
**What it does:** every VUURWERK-original module now carries the canonical dual-license header with the commercial-licensing contact line. The LICENSE Dual-Licensing Notice section now explicitly enumerates what "outside the complete VUURWERK firmware" means (extraction, porting, repackaging, commercial use). README gained a License section near the top with operator-facing summary. The contributions registry in LICENSE is unchanged (LAW 7). Zero flash impact -- comments only.

---

## v1.2.6 Changes

### Spectrum view keymap reorganization
**What it does:** Inside the spectrum analyzer, SIDE1 now steps frequency UP (with auto-repeat when held), SIDE2 now steps frequency DOWN (release-dispatched, no auto-repeat), long-press SIDE2 (about 640 ms) toggles the backlight, and short-press MENU blacklists the current peak bin. UP / DOWN still voice-hop in VOX and frequency-step in NORM / PEAK / MTI, exactly as before. Operators upgrading from v1.2.5 will need to relearn that SIDE1 and SIDE2 are now the universal frequency-step controls (was: SIDE1=blacklist, SIDE2=backlight). Short-press and long-press are unambiguously separated on SIDE2: the freq-down on release is suppressed when the long-press fires, so you never get a stray step when you wanted to toggle the backlight.

### Voice-hop blacklist respect
**What it does:** With Blacklist now moving to MENU (more accessible than the old SIDE1 binding), the VOX UP / DOWN voice-hop loop also now skips bins you have blacklisted. Previously a blacklisted bin with strong voice probability would still be tuned by the hop.

### Spectrum mode-change flash
**What it does:** When you press STAR to cycle modes (NORM / PEAK / MTI / VOX), a brief `MODE <name>` overlay appears in the middle of the spectrum display for about half a second, in addition to the permanent small label in the top-right corner. Operators who report only ever seeing "NORM" get a clear confirmation that the mode actually changed.

### VOX bar threshold matches hop threshold
**What it does:** In VOX mode, bins with voice probability below 50 now drop to a floor dot (previously 35) and only bins at or above 50 draw any bar. Every visible bar is now a valid hop target. A 1-pixel marker tick at the very top of the spectrum display flags every hop-eligible bin, so the row of dots across the top tells you exactly where UP / DOWN can land.

---

## The Architecture

VUURWERK features live in their own modules as thin hooks that run after the stock code does its thing, each touching only its assigned hardware registers. Since v1.3.0 the project is a true fork: a stock file may be edited when that fixes a bug or measurably improves the firmware, but every such edit is marker-tagged in the source, so the boundary between inherited and original code stays auditable. The radio never loses any stock functionality; VUURWERK adds to it.

---

*VUURWERK (Afrikaans: fireworks). Light in the dark. Signal through the noise.*

*Firmware design and architecture: James Honiball (KC3TFZ)*
*Copyright (c) 2026 James Honiball*
