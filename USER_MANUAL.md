# VUURWERK User Manual

**Firmware version:** v1.5.0
**Radio:** Quansheng UV-K5 V1 (DP32G030 hardware)

This is the operator manual. Plain-English walkthrough of every feature you can interact with, what to press, what you will see, and what to expect. Companion documents: FEATURES.md (technical reference with register maps and algorithm details), ACCESS_MAP.md (call-site cross-reference), WHAT_VUURWERK_DOES.md (high-level capability summary), CHANGELOG.md (release history).

If anything in this manual disagrees with the radio's actual behavior, the radio is right. Open an issue.

---

## Table of Contents

1. Getting Started
2. The Main Display
3. Frequency and Mode
4. Receiving
5. Transmitting
6. CTCSS and DCS
7. Spectrum Analyzer
8. Bandscope on the Main Screen
9. Special Functions
10. Channel Programming
11. About Screen (Diagnostics)
12. Customization
13. Boot-Time Diagnostics
14. Troubleshooting
15. Appendix: What Happens Automatically
16. Cheat Sheet

---

## 1. Getting Started

### Power-on sequence

Turn the radio on with the rotary switch. The boot sequence runs in this order:

1. The transceiver chip and EEPROM are checked for faults (Boot-Time Hardware Health Probe).
2. The welcome banner appears for about 2 seconds.
3. The main display takes over.

What you see in step 2 depends on (a) whether the radio is healthy, (b) which power-on display mode you have configured, and (c) which keys you were holding when you powered on.

### Normal boot (healthy radio)

By default you see the VUURWERK welcome banner. The large "VUURWERK" text occupies the top two lines of the display, the firmware version appears on line 3, and the tagline "Precision RF" appears on line 5.

```
+----------------------------+
|         VUURWERK           |
|                            |
|                            |
|          v1.5.0            |
|                            |
|        Precision RF        |
|                            |
|                            |
+----------------------------+
```

After about 2 seconds the banner is replaced by the main display, covered in Section 2.

The power-on display mode is configurable via MENU > CONFIG > POnMsg:

- `Full` (the VUURWERK banner above; this is the factory-default value, so fresh flashes land here)
- `Message` (also the VUURWERK banner; kept equivalent for radios configured before v1.3.2)
- `Voltage` (battery voltage and percent on the welcome screen)
- `None` (genuinely blank welcome screen)

v1.3.2 note: firmware before v1.3.2 booted `Full` and `None` to a solid black screen (an inherited stock "all pixels on" LCD-test behavior), so the banner only appeared if POnMsg happened to be `Message`. If your radio ever greeted you with a black flash at power-on, that was this, not a fault.

In `Voltage` mode the layout is:

```
+----------------------------+
|         VOLTAGE            |
|                            |
|       7.42V 78%            |
|                            |
|                            |
|                            |
|         v1.5.0             |
|                            |
+----------------------------+
```

### Hardware fault banners

If the boot health probe detects a problem with the transceiver chip or the calibration EEPROM, the welcome screen is replaced by a fault banner. These banners mean the radio cannot operate normally. They are diagnostic, not destructive.

#### BK4819 FAULT

```
+----------------------------+
|          BK4819            |
|                            |
|                            |
|           FAULT            |
|                            |
|                            |
|      RX/TX disabled        |
|                            |
+----------------------------+
```

**What it means:** the transceiver chip's first register read back `0xFFFF`, the signature of a wedged SPI bus or non-responsive chip.

**What to do:**

1. Power-cycle the radio (off, wait 5 seconds, on).
2. If the banner repeats, remove the battery for 30 seconds, then reinsert.
3. If the banner still repeats, the BK4819 chip or its SPI lines are damaged. The radio will not transmit or receive in this state. This usually indicates physical damage (drop, water, ESD).

#### EEPROM FAULT

```
+----------------------------+
|          EEPROM            |
|                            |
|                            |
|           FAULT            |
|                            |
|                            |
|        calib lost          |
|                            |
+----------------------------+
```

**What it means:** the factory-programmed battery calibration page in EEPROM (address `0x1F40`) read back as all `0xFF` bytes, the signature of a blanked or unreachable page. The battery indicator and S-meter will read wrong values because the radio is dividing by an absurd constant for voltage and RSSI conversion.

**What to do:**

1. Power-cycle the radio. If the fault is transient (corrupted flash from a recent power loss during write), it may clear.
2. If persistent, the EEPROM page needs to be restored via Chirp or a CPS programming tool with a valid `.dat` calibration file for your specific UV-K5. You will need a programming cable.
3. The radio will still partially operate (you may still be able to transmit and receive) but battery percent, voltage, and S-meter readings cannot be trusted until calibration is restored.

If both faults are present, the BK4819 banner takes priority because it is the more severe problem.

### "RELEASE / ALL KEYS"

```
+----------------------------+
|                            |
|          RELEASE           |
|                            |
|                            |
|         ALL KEYS           |
|                            |
|           SIDE1            |
|                            |
+----------------------------+
```

**What it means:** you powered on the radio while holding PTT plus another key. The radio detected this as a configuration gesture and is waiting for you to release the keys.

The bottom line names which key the radio sees you holding: `PTT`, `SIDE1`, `SIDE2`, `MENU`, `EXIT`, `UP`, `DOWN`, `STAR`, `F`, or a digit `0` through `9`. PTT is checked first and is the most common cause.

**What to do:**

1. Release every key. The banner will clear and the radio will continue to boot.
2. If you intended the gesture, finish reading the next subsection.
3. If you did not intend it, just power-cycle and it will not happen again.

### Power-on gestures (unlock category)

The UV-K5 has one boot gesture supported by VUURWERK:

**PTT + upper side button (SIDE1) at power-on**: unlocks the hidden `UNLOCK` menu category. This category contains the frequency-lock toggle, the 200/350/500 MHz transmit enables, the scrambler enable, the battery calibration, the battery type, and the factory reset. The `UNLOCK` category stays visible until you next power-cycle the radio.

Some of these items (200/350/500 MHz TX enables, scrambler) unlock hardware capabilities that may not be legal to transmit on in your jurisdiction. Know your local regulations before enabling.

To use the gesture:

1. Power the radio off.
2. Press and hold PTT + SIDE1.
3. Turn the rotary switch to power on while still holding both.
4. Release when the `RELEASE / ALL KEYS` screen appears.
5. Continue to the main display. The MENU key now reveals 7 categories instead of 6.

Power-cycling clears the unlock. This is intentional: it prevents the dangerous transmit-enable and factory-reset items from being one accidental press away during normal operation.

---

## 2. The Main Display

The main display is what you see after the boot sequence. It is divided into four horizontal zones.

```
+--------------------------------+
| S+W  [SP] [Q|||]  [F]  [BATT#] |  <- status bar: scan glyphs, split,
|                                |     quality, lock/F key, battery
| A  146.520.00       FM   H  N  |  <- VFO A: frequency (or channel name),
|                                |     modulation, TX power, narrow
| .:|.||:..|,.:|||,.:.|..:||:.,  |  <- center line: bandscope / toast /
|                                |     RSSI bar (priority in that order)
| B  446.000.00       FM   M     |  <- VFO B
|                                |
| 12.5kHz N FM                   |  <- context line: changes with what
+--------------------------------+     the radio is doing
```

The bottom context line has four personalities:

```
idle       | 12.5kHz N FM                   |  step, bandwidth, modulation
receiving  | RX S7 -65dBm Q:3 F             |  S-level, dBm, quality, class
transmit   | TX 0:12 PWR:H                  |  elapsed time, power level
scanning   | SCANNING 8c/s                  |  live channels-per-second
```

### Status bar (top)

The top row shows, left to right:

- **Scan indicator** (when scanning): `S+W` for Scan+Watch, `Cs` for CSS scan, `1` or `2` or `*` for memory-channel scan list, `S` for a general scan.
- **Dual-watch or cross-band indicator** (when one is enabled): rendered as a small TDR or XB bitmap.
- **`SP` (v1.3.1)**: VFO Split is armed. VFO B is walking the memory channel list by itself while you listen on VFO A; this glyph stays up the whole time split is armed, so an unexpectedly self-changing VFO B display is never a mystery again.
- **Key-lock or F-key icon**: a padlock bitmap when keys are locked, an `F` bitmap when the F key has just been pressed.
- **Signal quality** (during RX): the letter `Q` followed by 0 to 3 vertical bars indicating signal stability. More bars means less variance, that is, a steadier signal.
- **Battery icon** (rightmost): 0 to 4 fill bars.

### VFO lines

The radio has two virtual VFOs (A and B). Each VFO line shows the frequency (or memory channel name, depending on display mode), the modulation (`FM`, `AM`, or `USB`), the TX power level (`L`, `M`, or `H`), and a bandwidth indicator (`N` for narrow, blank for wide).

The active VFO (the one currently selected for transmit) is rendered with a marker. Toggle between A and B with F+2 (Section 3).

### Center line

The center line shows whichever of these is active right now, in this priority order:

1. **Toast notification** (1 second): a bordered overlay confirming a shortcut action, for example `PWR: HIGH` after F+6. The toast wins over everything else for its 1-second lifetime.
2. **Bandscope**: a 128-pixel scrolling timeline of RF activity on the current frequency, when bandscope is enabled (F+7).
3. **RSSI bar**: a horizontal signal-strength bar (compile-time option).

If none of these is active the center line is blank.

### Context status line (bottom)

The bottom line changes its content based on what the radio is doing right now. The four states and their exact formats:

- **Idle**: `12.5kHz N FM` (step, bandwidth, modulation).
- **Receive**: `RX S7 -65dBm Q:3 F` (S-level 0 to 9, raw RSSI in dBm, signal quality 0 to 3, signal classifier symbol). The classifier symbol is `F` (fast/FM), `N` (normal/SSB), `S` (slow/carrier), or `~` (noise).
- **Transmit**: `TX 0:12 PWR:H` (elapsed time mm:ss since PTT press, current power level).
- **Scanning**: `SCANNING 8c/s` (the suffix shows current scan rate in channels per second). If scan rate has not stabilized yet, the suffix is omitted and you see just `SCANNING`.

The classifier symbol on the RX line is computed from the rise time of the signal envelope, so its value is meaningful from the first few samples after the squelch opens.

---

## 3. Frequency and Mode

These are the basic operating controls: which VFO is active, what mode it is in, what modulation it uses, what band it covers.

### Switching between VFO A and VFO B (F+2)

Press F, then 2. The active VFO toggles. Toast: `VFO: A` or `VFO: B`.

The TX VFO is the one you will transmit on if you press PTT. The RX VFO can be the same or different depending on whether you have dual-watch or cross-band enabled (Section 4).

### Toggling VFO mode and Memory mode (F+3)

Press F, then 3. The active VFO toggles between VFO mode (direct frequency entry) and MR mode (memory channel recall). Toast:

- `VFO MODE`: direct frequency entry. UP and DOWN tune by the configured step size.
- `MR MODE`: memory channel recall. UP and DOWN walk through saved channels.
- `NO CHANNELS`: you attempted to switch to MR mode but there are no saved channels yet. Save at least one channel first (Section 10).

### Cycling modulation (F+0)

Press F, then 0. The active VFO modulation cycles FM, AM, USB, FM. Toast:

- `MOD: FM`
- `MOD: AM`
- `MOD: USB`

FM is the default for most amateur and commercial communications. AM is useful for airband (118 to 137 MHz). USB is single-sideband for HF-like operation on the supported bands.

### Cycling bands (F+1)

Press F, then 1. If you are in VFO mode the frequency jumps to the start of the next band in the supported list (50 MHz, 144 MHz, 350 MHz if enabled, 430 MHz, 470 MHz, and so on). Toast `BAND` confirms.

If you are in MR mode this gesture has no effect and you see the toast `FREQ ONLY`. Switch to VFO mode first (F+3).

The 350 MHz band is skipped unless the hidden `350 En` option is set to ON in the UNLOCK category.

### Resetting a VFO to simplex (F+EXIT) -- the rescue key

Press F, then EXIT. The active VFO is wiped back to a clean simplex state and the change is saved. Toast: `SIMPLEX`.

```
   before F+EXIT                       after F+EXIT
   +--------------------------+       +--------------------------+
   | A 146.520.00   +offset   |       | A 146.520.00   FM        |
   |   RX tone 123.0, reverse |  -->  |   RX = TX (true simplex) |
   |   scrambler, AM          |       |   no tones, no scrambler |
   +--------------------------+       +--------------------------+
     green RX light, no audio           hears everything again
```

What it clears, all in one press:

- RX and TX CTCSS tones and DCS codes (set to OFF)
- Repeater offset and offset direction (TX frequency = RX frequency)
- Frequency reverse (off)
- Scrambler (off)
- Modulation (back to FM)

Use it whenever a VFO behaves deaf or strange and you do not want to hunt
through menus for the leftover setting. This is the one-gesture cure for the
classic "green light but silent" condition (Section 6 explains why that
happens).

F+EXIT works in frequency (VFO) mode only. On a memory channel you get the
`FREQ ONLY` toast and the channel is not touched -- memory channels are never
modified by this gesture.

### Setting the frequency step

The step controls how much UP and DOWN move the VFO each press. Configure via MENU > RECEIVE > Step. Available values: 2.5, 5, 6.25, 10, 12.5, 25 kHz, and the 8.33 kHz airband step.

### Setting the channel bandwidth

Bandwidth is configured per channel via MENU > RECEIVE > W/N. Wide is 25 kHz (most VHF/UHF FM uses this). Narrow is 12.5 kHz (commercial NFM, airband AM, and most digital uses this). The `N` indicator appears on the VFO line when narrow is selected; wide leaves it blank.

---

## 4. Receiving

### Cycling squelch level (F+4)

Press F, then 4. The squelch level cycles 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0. Toast: `SQL: 0` through `SQL: 9`.

Squelch determines how strong an incoming signal must be before the audio opens. Level 0 is open (all noise lets through); level 9 requires a very strong signal. A typical setting is 1 to 3 for casual monitoring and 5 to 7 for noisy environments.

Note: this is the *carrier* squelch. If an RX CTCSS/DCS tone is configured on the channel, level 0 does not bypass the tone gate -- see "Green light but no sound?" in Section 6.

VUURWERK runs Intelligence-Based Squelch on top of the user-set level. Even at low levels you will hear less noise breakthrough than stock firmware because the voice-probability scorer closes the squelch on noise-like signals automatically (Appendix).

### Reading the RX status line

When the squelch opens and the radio enters receive, the bottom status line switches to:

`RX S7 -65dBm Q:3 F`

The fields:

- `S7`: S-level 0 to 9, derived from the dBm value.
- `-65dBm`: raw RSSI in dBm, compensated for any FM gain-staging adjustment in effect.
- `Q:3`: signal quality. 0 = Poor, 1 = Fair, 2 = Good, 3 = Excellent. Quality is computed from the variance of the last 8 RSSI samples on the same frequency. Quality resets when you change channels.
- `F`: signal rise-time classifier. `F` is fast-rise (FM voice), `N` is normal (SSB voice), `S` is slow (carrier or CW), `~` is noise.

The Q value and the classifier symbol let you tell apart different kinds of signals at a glance: a clean QSO on a strong repeater reads Q:3 F; a fading mobile station Q:1 or Q:2 F; a carrier or birdie Q:3 S; an unstable noisy signal Q:0 with the `~` classifier.

### Dual-watch and cross-band

Dual-watch lets the radio monitor both VFOs while you are operating on one. Configure via MENU > RECEIVE > RxMode:

- `Main only`: single VFO.
- `Dual watch`: alternate between A and B. VUURWERK chooses dwell times based on which VFO has more activity (Intelligent Dual-Watch, runs automatically). As of v1.3.1 the base dwell is 150 ms (was 500 ms), so a call starting on the other VFO is caught about four times sooner -- you stop losing the first word. The cost is slightly higher idle drain while dual watch is armed. As of v1.3.3, dual watch yields while you are holding a key: stepping through frequencies with UP/DOWN held no longer fights the alternation engine (which could retune the radio under your thumb mid-hold, even yanking you onto the other VFO's memory channel). Alternation resumes the moment you release.
- `Cross-band`: receive on one VFO and transmit on the other. Useful for repeater operation where input and output are on different bands.

While browsing the menu, the RxMode item shows the hint `DUAL WATCH / CROSSBAND RX` on the bottom line (v1.3.1) -- if you were looking for "dual watch" and could not find it, this is it.

### Channel scanning (long-press STAR)

Hold the STAR key for about 1 second. The radio starts scanning channels (in MR mode) or stepping through frequencies (in VFO mode). Toast:

- `SCAN ON`
- `SCAN OFF`

While scanning, the status bar shows the scan glyph (`S`, `1`, `2`, `*` depending on scan list, or `S+W` if Scan+Watch is active). The bottom status line shows `SCANNING 8c/s` (8 channels per second in this example). The rate comes from a rolling 100-tick counter that observes scan events.

A short press of STAR from the main screen does not start scanning. To start scanning, long-press STAR or use a bound side button.

As of v1.2.8, scanning yields the front-end gain to the radio's hardware AGC for the duration of the scan, so weak signals that earlier firmware could walk past while scanning are now heard reliably. Sitting on a single frequency is unaffected.

As of v1.3.3 the scan is adaptive: each channel gets a ~30 ms energy pre-check, and only channels showing signal are held for the full evaluation dwell -- empty channels advance immediately. Scanning a quiet band runs roughly 2-3x faster, and any channel with actual energy still gets the identical full check, so weak-signal sensitivity is unchanged. Memory channels that carry a CTCSS/DCS tone get a longer dwell (500 ms) so the tone decoder has time to confirm before the scanner steps on -- before v1.3.3, coded channels were effectively skipped.

### Reverse repeater offset (F+8)

Press F, then 8. The offset direction reverses for the current VFO. Toast: `REVERSE` or `NORMAL`. Use this to listen on a repeater input frequency (to hear who is keying the input directly, without going through the repeater).

---

## 5. Transmitting

### Pressing PTT

When you press PTT, several VUURWERK features fire automatically. They are described in the Appendix because you do not interact with them directly: TX Soft Start ramps power up smoothly to avoid the click; CTCSS Lead-In mutes your microphone for 150 ms so any configured tone reaches full amplitude before your voice; TX Audio Compressor levels your audio in real time.

The display switches the bottom status line to:

`TX 0:12 PWR:H`

`0:12` is the elapsed time in mm:ss since you pressed PTT. `PWR:H` is the current power level (`L`, `M`, or `H`).

If you have configured a TX timeout (MENU > TX > TxTOut), the radio will automatically un-key at the timeout to prevent stuck-PTT lockout.

### Cycling TX power (F+6)

Press F, then 6. The TX power cycles LOW, MID, HIGH, LOW. Toast:

- `PWR: LOW`
- `PWR: MID`
- `PWR: HIGH`

The change is saved to the current channel.

### Live battery indicator during TX

Unlike stock firmware, the battery indicator on the status bar continues to update while you are transmitting. The icon, voltage, and percent are all recomputed every 500 ms. This lets you watch voltage sag in real time as you key up.

The battery sag delta (the difference between voltage before PTT and the minimum voltage seen during TX) is recorded and shown on the About screen (Section 11).

---

## 6. CTCSS and DCS

CTCSS (sub-audible tones) and DCS (digital codes) are used to share a frequency with other users without hearing them. Your radio decodes the tone or code from an incoming signal and only opens the squelch if it matches what you have configured.

### Configuring receive and transmit tones

The TONE category in the menu has these items:

- `RxCTCS`: receive CTCSS tone. Set to a Hz value (67.0 to 254.1 Hz) to require that tone for squelch to open, or OFF to disable.
- `TxCTCS`: transmit CTCSS tone. Set to the Hz value the repeater requires, or OFF.
- `RxDCS`: receive DCS code. Set to a code (D023N through D754I) or OFF.
- `TxDCS`: transmit DCS code. Set or OFF.
- `STE`: Squelch Tail Elimination on or off (Appendix).
- `RP STE`: Repeater STE, an alternate tail-elimination mode for repeater operation, 0 (off) to 10.
- `Scramb`: Scrambler, OFF or 1 to 10. Most uses are illegal in most jurisdictions; the scrambler is hidden behind the UNLOCK category for that reason.

### Green light but no sound? That is tone squelch working

This is the most misunderstood behavior on the radio, so here is exactly what
happens when an RX tone or code is configured:

```
   signal arrives on frequency
             |
             v
   carrier squelch opens  ------------>  GREEN LED turns ON
             |
             v
   RX tone configured?  -- no -->  AUDIO plays immediately
             |
            yes
             |
             v
   does the signal carry            -- yes -->  AUDIO plays
   YOUR tone (or code)?
             |
             no
             |
             v
   GREEN LIGHT STAYS ON, AUDIO STAYS MUTED
   (the radio is politely waiting for a tone that never comes)
```

The green LED means "there is a carrier here." Audio means "the carrier also
passed the tone gate." A VFO with a leftover `RxCTCS` or `RxDCS` setting will
light green on every transmission and play none of them -- it looks exactly
like a broken radio and is exactly not one.

Three things every operator should know about this gate:

1. **Squelch level 0 does NOT bypass tone squelch.** F+4 to SQL 0 opens the
   *carrier* squelch fully, but the tone gate is a separate, independent
   check. If an RX tone is set, SQL 0 still gives you green-light-and-silence
   on non-matching signals.
2. **Monitor DOES bypass it.** A side button bound to `Monitor` (Section 12)
   forces the audio path open, tones and all. That is the correct way to
   listen "underneath" the tone gate.
3. **F+EXIT clears it.** The reset-to-simplex gesture (Section 3) wipes the
   RX/TX tones and codes off the active VFO in one press. If you are not sure
   why a VFO is silent, this is the fastest fix.

### Doing a CSS scan from the main screen

If you do not know what tone a repeater uses, the CSS scanner can find it for you. From the main screen, with the radio in FM mode and tuned to a frequency you can hear an active signal on:

1. Press F.
2. Press the STAR key.

The radio launches the CSS scanner. Behavior depends on the modulation:

- **FM**: the scanner runs the pre-flight RSSI gate first (v1.2.7). If the current frequency reads below carrier-present threshold (about -110 dBm), the scanner aborts immediately with the `NO SIGNAL` toast and a 500 Hz double-beep. Park on a frequency with an active carrier (a repeater you can hear breaking squelch, or a known active simplex channel) before launching the scan. If RSSI is above threshold, the scanner runs and the status bar shows `Cs`.
- **AM or USB**: the scanner cannot run because the BK4819 chip only populates the CTCSS detection registers in FM mode. You see the toast `FM ONLY` and a rejection beep.

While the CSS scanner is running, three outcomes are possible:

1. **Tone found**: the scanner locks onto a tone or code. You hear a single 1 kHz beep (the FOUND beep). The matched tone or code is shown on screen. Press EXIT to apply it to your RX configuration, or walk away. v1.2.7 made CTCSS detection 1-confirmation (previously 2-confirmation), so CTCSS scans now lock as fast as DCS scans.
2. **Timeout (30 seconds, v1.2.7)**: the scanner has run for 30 seconds without finding anything. The Soft-Timeout Watchdog flips it to FAILED state and you hear a 500 Hz double-beep. The previous timeout was 60 seconds; with the v1.2.7 dwell-time reduction (210 ms to 120 ms), CTCSS single-match, and pre-flight RSSI gate, typical successful scans complete in 3-10 seconds, so 30 seconds is the new "this is fruitless" cap.
3. **You cancel**: press EXIT at any time.

### Doing a CSS scan from the menu

The menu path is MENU > TONE > RxCTCS or RxDCS. Long-press MENU on the item to launch a background CSS scan. The status bar shows `Cs`, a dot animation overlay in the menu indicates progress, and the same three outcomes apply (FOUND beep, timeout, or cancel).

The menu-launched scan is useful when you want to scan without leaving the menu (for example to update an existing channel's RX tone configuration).

### Squelch Tail Elimination

Set MENU > TONE > STE to ON. With STE on, when the other party stops transmitting, the radio detects the loss of CTCSS tone and mutes the audio about 20 to 30 ms before the noise burst arrives. With STE off, you hear the brief "KSSHHH" tail. STE works with all major reverse-burst methods (Motorola 120-degree, Kenwood 180-degree, Chinese) because it is based on tone presence rather than reverse-burst detection.

VUURWERK's STE is confidence-weighted (v1.2.5): fluttery signals get a slightly longer confirmation window (30 ms) before the mute fires, so brief 20 to 25 ms detector dropouts during mobile fade no longer pre-mute live audio.

As of v1.3.1, STE also covers DCS channels. The radio watches the DCS code detector the same way it watches the CTCSS tone detector, and mutes when the other station's turn-off code begins. DCS uses a slightly longer internal mute window (250 ms instead of 150 ms) because the DCS turn-off code itself runs about 180 ms before the carrier drops. The same TONE > STE switch controls both; OFF bypasses both completely.

---

## 7. Spectrum Analyzer

The spectrum analyzer is the radio's full-screen band instrument: it sweeps a
span of frequencies, draws what it hears, auto-listens to anything over the
trigger level, and -- in VOX mode -- hunts specifically for human voices and
lets you lock onto them.

### Launching and leaving

From the main screen, press F, then 5. The analyzer takes over the whole
display (no toast -- the entire screen changing is the confirmation).

Press EXIT to leave. The exit is deterministic: you always come back on the
VFO you left from, tuned to your displayed frequency, hearing normally --
whether or not you were locked, and whether or not dual watch is on. (Earlier
firmware could flip you to the other VFO or leave the receiver mistuned on
exit; both failure modes are fixed as of v1.5.0.)

### Reading the screen

```
+--------------------------------+
| -130/-50              [BATT##] |  <- status bar: dB floor/ceiling, battery
| 146.552.50        FM     VOX   |  <- peak/listen frequency, modulation,
| 64x                       25k  |     MODE LABEL (top-right corner);
| 25.00k    #          |         |     left edge: bins + scan step,
|         | #    ,     |         |     right: listening bandwidth
| ......_.|.#..:,:,....|._...... |  <- bars: one per frequency bin
| ...:....|....:...^....|....:.  |  <- tick rail + PEAK ARROW (^)
| 145.760       >80.0k   148.960 |  <- span start, move step, span end
+--------------------------------+
       dotted horizontal line across the plot = TRIGGER LEVEL
```

- **Mode label** (`NORM`, `PEAK`, or `VOX`) lives in the top-right corner of
  the plot. Pressing STAR also flashes the new mode name in a brief center
  overlay so you never have to squint.
- **Peak arrow**: the small arrow on the tick rail points at the strongest
  bin (in VOX, the bin being listened to). This is where PTT will act.
- **Trigger level**: the dotted horizontal line. When a bin's bar crosses it,
  the analyzer stops and listens there. Press F to nudge the trigger down
  (more sensitive) while sweeping.
- **Span numbers**: bottom line shows the sweep's start and end frequencies
  and the move step (how far UP/DOWN/SIDE keys shift the span).

### Cycling display modes (STAR)

Press STAR inside the analyzer to cycle the three display modes:

```
      STAR              STAR              STAR
 NORM ----->  PEAK  ----->  VOX  -----> (back to NORM)
```

- `NORM`: live spectrum. Each bin's bar is proportional to signal strength.
- `PEAK`: live bars plus peak-hold dots that decay slowly -- catches brief
  transmissions you would otherwise miss.
- `VOX`: voice-seeking spectrum. Bars are drawn only for bins that score as
  probable voice (details below).

A brief center overlay (`NORM` / `PEAK` / `VOX`) confirms each press.
Cycling the mode also releases a VOX lock if one was held.

### Key map -- sweep view (all modes)

| Key | Action |
|---|---|
| UP / DOWN | NORM/PEAK: move the span up/down. VOX: seek the next voice (see below). Auto-repeats while held. |
| SIDE1 / SIDE2 | Move the span up / down in every mode (pure navigation, never seeks). Auto-repeats while held. |
| STAR | Cycle display mode NORM / PEAK / VOX |
| PTT | VOX: lock the voice you are hearing. NORM/PEAK: park on the peak (STILL view, below). |
| MENU | Blacklist the current peak bin (skipped on later sweeps and hops) |
| EXIT | Leave the spectrum analyzer |
| F | Nudge the trigger level down |
| 1 / 7 | Scan step wider / narrower (how much frequency each bin covers) |
| 2 / 8 | Move step bigger / smaller (how far UP/DOWN/SIDE shift the span) |
| 3 / 9 | dB ceiling up / down (vertical scale) |
| 4 | Cycle bin count: 128 / 64 / 32 / 16 |
| 5 | Direct frequency entry (digits, STAR = decimal point, MENU = accept, EXIT = backspace/cancel) |
| 6 | Cycle listening bandwidth: 25k / 12.5k / 6.25k |
| 0 | Cycle modulation: FM / AM / USB |

### VOX mode -- the voice hunt

In VOX mode every bin is scored 0-95 for voice probability from its signal
strength and audio-noise reading. The display shows confidence, not just
power:

```
   .                        .        <- top tick = hop-eligible bin
                  #
        ,         #                     # full bar   voice score 75+
   _____,_________#_____._______        , half bar   voice score 50-74
                                        _ floor dot  below 50 (not a target)
```

Every bin with a bar also gets a tick at the very top of the plot, so the
row of top dots is your map of exactly where UP and DOWN can land.

**The workflow, key by key:**

```
 [F+5] -> [STAR to VOX] -> sweep hunts -> voice found: auto-listen
                                               |
              +--------------------------------+
              v
   UP / DOWN   seek the next voice above / below the cursor.
               Ascending chord = landed on voice.
               No voice that way? The key steps the span instead,
               so UP/DOWN always moves you. Hold to patrol the band;
               it auto-stops when a voice appears.
              |
             PTT
              v
   +--------------------+
   |       LOCKED       |   flashes over the center of the plot
   +--------------------+
   The radio parks on this frequency with the squelch open. You hear
   the whole channel -- both sides of the QSO, anyone who keys up --
   and the lock holds through any pause, however long.
              |
       +------+------+
       |             |
   UP / DOWN       EXIT
   releases the    leaves the spectrum. In frequency mode, the locked
   lock, resumes   frequency CARRIES OVER: it becomes your VFO
   seeking         frequency, saved, ready for PTT-to-answer.
```

**A worked example.** You are on 146.520 wondering where the activity is:

```
 1. F+5            spectrum opens around your frequency
 2. STAR, STAR     corner reads VOX; carriers vanish, voices get bars
 3. (sweep stops)  auto-listen on the strongest voice: a QSO on 146.550
 4. UP             chord -- hopped to the next voice up, 146.580
 5. DOWN           chord -- back on 146.550; this one is interesting
 6. PTT            LOCKED flashes; you monitor the whole exchange
 7. EXIT           main screen: VFO A now reads 146.550.00, saved.
                   Press PTT and join the conversation.
```

Notes on the lock and carry-over:

- The lock is a monitor. PTT never transmits inside the spectrum.
- Carry-over happens only in frequency (VFO) mode, only when the locked
  frequency is inside your VFO's current band, and never when frequency
  reverse is active. In memory-channel mode a locked exit returns you to
  your channel unchanged -- a memory channel cannot be modified from the
  spectrum, by construction.
- The carried frequency is rounded to your VFO's step and saved, so it
  survives a power cycle.
- Without a lock, EXIT returns you to exactly the frequency you entered
  from, as always.

**The sound you will hear in VOX mode:** a short ascending two-tone chord
(800 then 1200 Hz) every time a hop lands on voice. A seek that finds no
voice makes no sound -- the span visibly moving *is* the feedback.

### STILL view (park and measure) -- NORM/PEAK only

In NORM or PEAK, PTT parks the radio on the current peak frequency and opens
the STILL view: a large S-meter with exact dBm readout, plus a register-level
gain editor for advanced use.

```
+--------------------------------+
| -130/-50              [BATT##] |
| 146.552.50        FM      25k  |  <- parked frequency
| |''''|''''|''''|''''|''''|'''  |  <- signal meter with scale
| S: 9   -62 dBm                 |  <- S-level and dBm
|                                |
|  LNAs   LNA    PGA    IF       |  <- register editor (MENU to cycle,
|  3      2      6      10922    |     UP/DOWN to adjust selection)
+--------------------------------+
```

| Key (STILL) | Action |
|---|---|
| UP / DOWN | Fine-tune frequency (or adjust the selected register when the editor is active) |
| MENU | Cycle register editor selection (LNAs / LNA / PGA / IF) |
| STAR / F | Trigger level up / down |
| SIDE1 | Toggle monitor mode (hold squelch open) |
| SIDE2 | Toggle backlight |
| 5 | Direct frequency entry |
| 0 | Cycle modulation |
| 6 | Cycle listening bandwidth |
| 3 / 9 | dB ceiling up / down |
| EXIT | Back to the sweep (or close the register editor first) |

In VOX mode PTT never opens this view -- it locks the voice instead. (In a
scan-range span wider than 128 steps, where the voice arrays cannot map to
bins, PTT simply does nothing rather than surprising you with the editor.)

---

## 8. Bandscope on the Main Screen

### Toggling the bandscope (F+7)

Press F, then 7. The center line of the main display switches between the bandscope and whatever was there before (blank or RSSI bar). Toast: `SCOPE ON` or `SCOPE OFF`.

### What you see

The bandscope is a small horizontal timeline showing RF activity on your current frequency. It is 128 pixels wide (the full LCD width), 8 pixels tall, and lives on the center line of the main display.

```
+----------------------------+
| status bar                 |
|                            |
| A 146.520.00      FM  H  N |
|                            |
| ........|||..|...|.||||... |  bandscope
|                            |
| B 446.000.00      FM  M  W |
|                            |
| 12.5kHz N FM               |
+----------------------------+
```

New samples scroll in from the right. Peak dots above the bars show the strongest signal seen at each horizontal position over time; they decay slowly. A dotted line marks the noise floor as estimated by the RSSI Histogram (Appendix). The dotted line lets you see at a glance which signals are above background noise and which are riding the floor.

The bandscope shares the center line with toast notifications. When you fire a shortcut that emits a toast, the toast appears over the bandscope for 1 second, then the bandscope returns.

The bandscope does not interrupt audio. It samples the BK4819 RSSI register at idle priority.

---

## 9. Special Functions

### Calling-frequency jump (F+9)

Press F, then 9. The radio jumps to the standard simplex calling frequency for the band you are currently on, sets the correct step size, and sets the appropriate modulation. Toast:

- `CALL FREQ`: jumped to a calling frequency.
- `RESTORED`: jumped back (second press of F+9 restores your previous frequency, step, and modulation).
- `NO BAND`: your current frequency does not match any known band, so there is no calling frequency to jump to.

Supported bands and their calling frequencies:

| Band | Range | Calling | Step | Mode |
|---|---|---|---|---|
| 2 m SSB | 144.000 to 144.300 MHz | 144.200 MHz | 1 kHz | USB |
| 2 m FM | 144.000 to 148.000 MHz | 146.520 MHz | 5 kHz | FM |
| 70 cm SSB | 432.000 to 433.000 MHz | 432.100 MHz | 5 kHz | USB |
| 70 cm FM | 420.000 to 450.000 MHz | 446.000 MHz | 25 kHz | FM |
| MURS | 151.000 to 154.000 MHz | 151.940 MHz | 11.25 kHz | FM |
| FRS/GMRS | 462.000 to 467.000 MHz | 462.5625 MHz | 12.5 kHz | FM |
| CB | 26.900 to 27.400 MHz | 27.185 MHz | 10 kHz | FM |
| Marine | 156.000 to 162.000 MHz | 156.800 MHz | 25 kHz | FM |
| Airband | 118.000 to 136.000 MHz | 121.500 MHz | 25 kHz | AM |

The radio checks the narrower SSB segments before the wider FM segments for the 2 m and 70 cm bands, so a frequency that falls in both will route to the SSB calling frequency.

### Keypad lock (long-press F)

Hold the F key for about 1 second. The keypad locks, and you see a padlock icon in the status bar. Toast: `LOCKED` or `UNLOCKED`.

While the keypad is locked, only PTT works. Hold F again to unlock. (Auto-keylock can also be configured to fire automatically after a timeout: MENU > CONFIG > KeyLck.)

### Scan toggle (long-press STAR)

Hold the STAR key for about 1 second. Scanning toggles, with a 1 kHz confirmation beep. Toast: `SCAN ON` or `SCAN OFF`.

This is the easiest way to start or stop scanning from the main screen. See Section 4 for what scanning does.

### Menu navigation

Press MENU. The radio shows the category picker:

```
+----------------------------+
|                            |
|        RECEIVE             |  (cursor on selected)
|        TONE                |
|        TX                  |
|        SCAN                |
|        CHANNEL             |
|        CONFIG              |
|                            |
+----------------------------+
```

If the UNLOCK category is enabled via the boot gesture (Section 1), a seventh entry named `UNLOCK` is also shown.

Within the category picker:

- **UP / DOWN** scroll through categories.
- **MENU** enters the selected category.
- **Number keys 1 to 7** jump directly to a category (1 = RECEIVE, 2 = TONE, 3 = TX, 4 = SCAN, 5 = CHANNEL, 6 = CONFIG, 7 = UNLOCK if visible).
- **EXIT** leaves the menu and returns to the main screen.

Inside a category, use UP / DOWN to scroll the item list, MENU to select, EXIT to back out.

When you re-enter a category, the cursor returns to the item you last viewed within that category (until you power-cycle).

A long press of MENU on the main screen triggers the configurable `M Long` action (Section 12).

---

## 10. Channel Programming

Memory channels store a frequency along with everything about how to receive and transmit on it: modulation, bandwidth, CTCSS/DCS tones, power, offset, scrambler, and the channel name.

### Saving a channel

1. Configure the active VFO with the frequency, modulation, power, and tone settings you want.
2. MENU > CHANNEL > ChSave.
3. Select a channel number (1 to 200, or NULL to pick the next free).
4. Press MENU to save. The radio confirms with an audible cue.

### Deleting a channel

MENU > CHANNEL > ChDele. Select the channel number to delete and press MENU.

### Naming a channel

MENU > CHANNEL > ChName. The radio enters name-edit mode. Use UP / DOWN to change a character, EXIT or MENU to move to the next position. The name can be up to 10 characters (uppercase letters, digits, a few symbols).

### Channel display format

MENU > CONFIG > ChDisp. Three options:

- `Freq`: show the frequency.
- `Channel`: show the channel number.
- `Name`: show the channel name (if configured) or fall back to the number.

### One-call channel (1 Call)

MENU > CHANNEL > 1 Call. Sets which channel is recalled by the radio's "1-Call" gesture (an inherited egzumer feature that tunes the radio to a designated memory channel quickly).

### Using CHIRP with VUURWERK

You can program channels, side-keys, and settings from a computer with [CHIRP](https://chirpmyradio.com) using a community-contributed driver (`chirp/uvk5_vuurwerk.py` in the firmware repo). You do not need to flash back to stock egzumer first -- the driver recognises VUURWERK's firmware identification string directly.

To load it: open CHIRP, enable Developer mode in CHIRP's settings, restart CHIRP, then File > Load Module and select `uvk5_vuurwerk.py`. Connect the radio with a Kenwood 2-pin cable, then Radio > Download from Radio and pick Quansheng, then UV-K5. The driver reads and writes channels, the programmable side-keys, and the settings block; it does not handle the spectrum analyzer settings. See `chirp/README.md` for the full how-to and credits. The driver is contributed by mjwgeek and is GPL.

---

## 11. About Screen (Diagnostics)

### Access

MENU > CONFIG > About. The About screen takes over the full display.

### What you see

The About screen is a single-page diagnostic readout:

```
+----------------------------+
|    VUURWERK v1.5.0         |  row 0: brand and version
|    Precision RF            |  row 1: tagline
|  Adaptive Squelch          |  rows 2-5: four feature
|  Signal Quality            |    highlights (static text,
|  Spectrum+                 |    not runtime state)
|  TX Compressor             |
|  TX sag 280mV              |  row 6: sag from your last TX
+----------------------------+
```

- **Row 0**: `VUURWERK v1.5.0`. Firmware brand and version -- your quickest way to confirm which build is on the radio.
- **Row 1**: `Precision RF`. Tagline.
- **Rows 2 through 5**: `Adaptive Squelch`, `Signal Quality`, `Spectrum+`, `TX Compressor`. Four active-feature highlights. These are static text; they do not reflect runtime state.
- **Row 6**: `TX sag NNNNmV`. The voltage sag from your most recent transmission, in millivolts.

### Reading the TX sag value

The TX sag value is the difference between the battery voltage immediately before you pressed PTT and the minimum voltage observed during the subsequent transmission. It is updated at the end of every transmission. A higher number means your battery sagged more under load.

There is no fixed interpretation scale. Build your own baseline by reading the sag value on several known-good transmissions and tracking it over time. Increasing sag for the same power level and duration over weeks or months indicates your battery is aging. If the field reads `0mV` immediately after a transmission, the ADC briefly produced `min > pre` and the unsigned-subtract guard caught it; retry.

### Exit

Press EXIT or MENU to leave the About screen. The screen also times out automatically after about 10 seconds and returns you to the menu.

---

## 12. Customization

This section covers the menu items used to configure the radio. Categories below correspond to the categorized menu's top-level groups (Section 9).

### The menu hint line (v1.3.1)

While browsing any category, the bottom line of the menu screen helps you decode cryptic item names: 40 items show a plain-language description in a small font while highlighted. Examples:

```
+--------------------------------+
| TxODir |                       |
|========|                       |
|#RxMode#|   DUAL RX RESPOND     |
|========|                       |
| BltTRX |                       |
|        |                       |
| DUAL WATCH / CROSSBAND RX      |
+--------------------------------+
```

`RxMode` reads `DUAL WATCH / CROSSBAND RX`, `TxODir` reads `REPEATER OFFSET DIRECTION +/-`, `ScnRev` reads `SCAN RESUME MODE AFTER SIGNAL`, `D Prel` reads `DTMF PRE-TONE DELAY`, and so on. Items whose names already explain themselves (Step, TxPwr, ChName, Beep...) keep the normal category/position line (`CONFIG 4/15`). The hint appears only while browsing; once you enter an item to edit it, the line returns to the category indicator. Nothing about the menu has moved or been renamed -- the hints are pure additive help.

### Backlight (CONFIG category)

- `BackLt`: backlight auto-off timer. Values: `OFF`, `5s`, `10s`, `20s`, `1min`, `2min`, `4min`, `ON`. After the timer expires the backlight starts a 2-second fade to off (Appendix).
- `BLMin`: minimum brightness (0 to 10), the level the backlight dims to during the fade.
- `BLMax`: maximum brightness (1 to 10), the level when the backlight is on.
- `BltTRX`: backlight on TX/RX. Values: `OFF`, `TX`, `RX`, `TX+RX`. **This setting controls the Backlight TX/RX Activity Refresh feature.** When set to one of TX, RX, or TX+RX, VUURWERK re-arms the backlight countdown every 500 ms during transmit or receive (whichever matches), so the screen never goes dark mid-conversation. With `OFF`, the stock one-shot behavior applies (backlight comes on briefly at TX/RX entry then times out normally even if you are still keyed up). If you find the screen going dark during conversations, set this to `TX+RX`.

### Power-on display (CONFIG > POnMsg)

Selects what the welcome screen shows at boot. Options: `Full` (VUURWERK banner), `Message` (blank), `Voltage` (battery readout), `None` (blank). See Section 1 for the visuals.

### Battery display (CONFIG > BatTxt)

Selects how the battery icon's text overlay reads: `None`, `Voltage`, or `Percent`.

### Beep and voice prompts

- `Beep`: ON or OFF. Key-click beep on every button press.
- `Voice` (if compiled in): voice prompts read out menu navigation and key actions.

### Auto key-lock (CONFIG > KeyLck)

Configurable auto-lock timer. Values: `OFF`, `15s`, `30s`, `60s`, `2min`, `5min`. When set, the keypad auto-locks after the timer expires without input.

To stop the keypad from re-locking on its own, set this to `OFF`. Unlocking the keypad with a long-press of F clears the current lock but does NOT disable the timer -- if `KeyLck` is still set, the keypad will auto-lock again after the timeout. A keypad that keeps re-locking (and, as a side effect, blocks shortcuts such as F+3 VFO/MR while locked) is almost always this timer doing its job; switch `KeyLck` to `OFF` to turn it off entirely.

### Side-button assignments (CONFIG)

The four side-button gestures and the long-press MENU gesture each have a configurable action:

- `F1Shrt`: short press of upper side button (SIDE1).
- `F1Long`: long press of SIDE1.
- `F2Shrt`: short press of lower side button (SIDE2).
- `F2Long`: long press of SIDE2.
- `M Long`: long press of MENU.

For each gesture you pick from this list of available actions:

- `Flashlight`: toggle flashlight.
- `Power`: cycle TX power. Toast: `PWR LOW`, `PWR MID`, `PWR HIGH`.
- `Monitor`: open squelch while held. Toast: `MON ON` or `MON OFF`.
- `Scan`: toggle channel scanning. Toast: `SCAN ON` or `SCAN OFF`.
- `VOX`: toggle VOX TX (if compiled in).
- `Alarm`: emergency alarm tone (if compiled in).
- `FM`: launch FM broadcast radio (if compiled in).
- `1750`: transmit 1750 Hz tone (used by some repeater systems).
- `Keylock`: toggle keypad lock. Toast: `LOCKED` or `UNLOCKED`.
- `A/B`: toggle VFO A/B. Toast: `VFO: A` or `VFO: B`.
- `VFO/MR`: toggle VFO/Memory mode. Toast: `VFO MODE` or `MR MODE`.
- `Switch Demodul`: cycle modulation. Toast: `MOD: FM`, `MOD: AM`, `MOD: USB`.
- `BLMin Tmp Off`: temporarily turn the backlight all the way off.
- `Spectrum`: launch the spectrum analyzer.

Each gesture you assign emits the corresponding toast when its bound action fires. This gives you the same 1-second on-screen confirmation that F-key shortcuts produce, regardless of which button you pressed.

### Battery (CONFIG)

- `BatSav`: battery saver mode. Off or one of four sleep-duty ratios (1:1, 1:2, 1:3, 1:4). Higher ratios mean more sleep time and longer battery life at the cost of some responsiveness.
- `BatVol`: shows the live battery voltage (read-only display).

### Hidden UNLOCK category

Visible only when unlocked via the PTT + SIDE1 boot gesture (Section 1).

- `F Lock`: lockout for protected frequency ranges.
- `Tx 200`: enable transmit in the 200 MHz range.
- `Tx 350`: enable transmit in the 350 MHz range.
- `Tx 500`: enable transmit in the 500 MHz range.
- `350 En`: include the 350 MHz band in band cycling.
- `ScraEn`: enable the scrambler menu.
- `BatCal`: battery calibration.
- `BatTyp`: battery type (1600 mAh or 2200 mAh).
- `Reset`: factory reset.

Some of these items (Tx 200/350/500, ScraEn) unlock hardware capabilities that may not be legal to transmit on in your jurisdiction. Know your local regulations before enabling.

---

## 13. Boot-Time Diagnostics

This section is the deep dive for the boot health probe and the fault banners introduced in Section 1.

### What the probe checks

Two checks run once, immediately after the transceiver chip is initialized but before the welcome banner renders:

1. **BK4819 liveness**: read register 0x00. The expected value is whatever the chip initialization wrote (typically not 0xFFFF). If the read returns 0xFFFF, the SPI bus is wedged or the chip is not responding. Bit 0 of the fault byte is set.
2. **EEPROM calibration page**: read 8 bytes from address 0x1F40 (the factory battery calibration page). If all 8 bytes are 0xFF, the page is blanked or unreachable. Bit 1 of the fault byte is set.

If any fault bit is set, the welcome banner is replaced by the fault banner (Section 1).

### Why this matters

Without the probe, a wedged SPI bus or blanked EEPROM would still let the radio boot, but RX and TX would be silently broken (BK4819 fault) or the battery indicator and S-meter would silently report garbage (EEPROM fault). You would have no way to distinguish a faulty radio from a working radio in a poor RF environment.

The probe surfaces the fault with a clear banner so you know to investigate the hardware, not the operating conditions.

### Recovery paths

For BK4819 FAULT:

- Power-cycle the radio.
- Remove the battery for 30 seconds, then reinsert.
- If persistent, the chip or its SPI lines are damaged. The radio will not transmit or receive. Repair or replacement.

For EEPROM FAULT:

- Power-cycle. Transient faults from a corrupted flash write may clear.
- If persistent, reflash the calibration page via Chirp or a CPS programming tool with a valid `.dat` file for your specific UV-K5.
- The radio may still partially operate; battery and S-meter readings cannot be trusted until calibration is restored.

### Power-on key trapping

If the radio detects that PTT plus another key was held during power-on, it shows the `RELEASE / ALL KEYS` screen (Section 1). The bottom line names the held key. Only the PTT + SIDE1 combination is wired to do something (unlock the UNLOCK category); other combinations are detected but ignored, and just wait for you to release.

---

## 14. Troubleshooting

### Green RX light but no audio on one VFO

The classic. Almost always a per-VFO setting, not a fault:

```
   green LED = carrier present         silence = a gate is closed
   +------------------------------------------------------------+
   |  RX CTCSS/DCS tone set?   -> audio waits for a tone that   |
   |                              never comes (Section 6)       |
   |  Repeater offset+reverse? -> radio listens off the         |
   |                              displayed frequency           |
   |  Scrambler on?            -> audio present but garbled     |
   |  Modulation AM/USB?       -> FM voice unintelligible       |
   +------------------------------------------------------------+
```

The one-gesture fix: **F+EXIT** (frequency mode). It clears all of the above
on the active VFO and toasts `SIMPLEX` (Section 3). Note that squelch level 0
does not bypass a configured RX tone -- use a Monitor side button or F+EXIT.

### Toasts are not appearing

Toast notifications only render on the main screen. If you are inside the menu, the spectrum analyzer, or the About screen, F-key shortcuts may fire but the toast will not be visible. Return to the main screen if you want to see the toast.

### The scan will not stop

Long-press STAR to toggle scanning off. You should see `SCAN OFF` toast and hear a confirmation beep. If a side-button is bound to `Scan`, pressing that button also toggles scanning.

If scanning still does not stop, EXIT may clear it depending on which scan type is active. As a last resort, power-cycle.

### The flashlight will not turn off

The flashlight has a 30-minute auto-off watchdog (Appendix). On a low-battery pack (battery indicator at level 2 or below), the watchdog shortens to 10 minutes. SOS mode is preserved indefinitely (no watchdog) because that is emergency signaling.

To turn the flashlight off manually, press the FLASHLIGHT button (or the bound side button) until the LED mode cycles to OFF.

### CSS scan never finds anything

The CSS scanner reads BK4819 registers that only populate in FM mode. If you launch F+* while in AM or USB you see `FM ONLY` and a rejection beep. Switch to FM first (F+0) and retry.

If you are in FM and the scanner still finds nothing within 30 seconds (v1.2.7 timeout, was 60 seconds), the Soft-Timeout Watchdog will flip the scanner to FAILED state and emit a 500 Hz double-beep. This is normal behavior on a frequency with no active CTCSS or DCS signaling.

### CSS scanner says NO SIGNAL

v1.2.7 added a pre-flight RSSI gate. Before launching the BK4819 scan engine, the scanner samples RSSI on the current frequency. If it reads below carrier-present threshold (about -110 dBm), the scan aborts immediately with the `NO SIGNAL` toast and a 500 Hz double-beep. This is intentional: CTCSS scans need an actual carrier to work against, and previously the scanner would run for 30-60 seconds against dead air before giving up.

The fix is to park on a frequency where you can hear a real signal (a repeater you can hear breaking squelch, an active simplex channel, etc.) before launching the scan. If you want to scan an open frequency that legitimately has weak signals, you can also try opening squelch manually (the MONITOR side-button binding) to feed RSSI above the threshold; the scanner will then proceed.

### Backlight goes dark mid-conversation

Check MENU > CONFIG > BltTRX. If it is set to `OFF`, the backlight follows the stock timeout even during TX or RX. Set it to `TX`, `RX`, or `TX+RX` (whichever matches when you want the screen to stay lit), and VUURWERK will re-arm the backlight timer every 500 ms during the matching state.

### Battery indicator frozen on PTT

Not a bug. VUURWERK keeps the battery icon and voltage live during TX, updating every 500 ms. If you are seeing the icon update during TX, that is correct. Stock firmware (and every other UV-K5 fork) freezes the icon on PTT; VUURWERK fixed this.

### The About screen looks different than what an older manual described

The About screen contents have changed across versions. The current (v1.5.0) About screen has exactly the fields documented in Section 11. Older versions had a dual-watch trip counter, an uptime counter, and a `Features:` header row; all have been removed. If you see different fields than Section 11 describes, you may be on a different firmware version. Check the About screen's top row for the version.

### Exiting the spectrum used to flip my VFO or leave the radio deaf

Both were real and both are fixed in v1.5.0. Every spectrum exit now returns you to the VFO you entered from, retuned to your displayed frequency, with audio alive -- including the case where you held the EXIT button a little long. If you see either symptom, check the About screen: you are running an older build; upgrade.

### Welcome banner shows BK4819 FAULT or EEPROM FAULT

See Section 13 for the diagnostic procedure.

---

## 15. Appendix: What Happens Automatically

These features run in the background. You do not interact with them directly, but you benefit from them every time you use the radio. Each is one line.

### TX chain (on every PTT)

- **TX Soft Start** (#1): ramps PA power up on a 60 ms S-curve to avoid the click that other operators hear at key-up.
- **CTCSS Lead-In** (#2): mutes your microphone for the first 150 ms of every PTT so any configured CTCSS tone reaches full amplitude before your voice does.
- **TX Audio Compressor** (#3): monitors your mic level 100 times per second and adjusts mic gain so loud and quiet audio sound consistent on the other end. (v1.3.1 note: bench-measured this release. At normal speaking distance the compressor is essentially transparent -- the mic envelope sits below its threshold except syllable peaks -- and on loud or close speech it compresses progressively, about 2:1 to 3:1, up to a designed ceiling of roughly 7.5 dB. An earlier audit claim that it over-compressed all speech was disproven by these measurements; constants are unchanged.)

### RX intelligence (continuously during RX)

- **FM Gain Staging** (#4): adaptive front-end gain control for FM mode. Resets on channel change.
- **RSSI EWMA Filter** (#5): smooths the raw signal-strength readings so the S-meter is stable.
- **RSSI Histogram** (#6): statistical noise-floor estimator that feeds the smart squelch and the bandscope.
- **Signal Rise-Time Classifier** (#7): labels every signal as `F` (fast/FM), `N` (normal/SSB), `S` (slow/carrier), or `~` (noise). Symbol shown on the RX status line.
- **Signal Quality Metrics** (#8): variance-based stability rating (0 to 3 bars) shown as Q on the status line.
- **S-Meter Compensation** (#9): corrects the RSSI for any gain-staging adjustment so the S-meter always reports the true signal strength.
- **AM Fix** (#10): prevents the BK4819 demodulator from saturating on strong AM signals.
- **Intelligence-Based Squelch** (#12): voice-probability scoring across three hardware measurements. Loosens the squelch on real voice and holds it through speech pauses; it never tightens beyond your configured squelch level. As of v1.3.1 it also evaluates while the radio is parked with the squelch closed (every 50 ms): when the parked signal pattern scores strongly voice-like and the channel's noise floor has been learned, the open threshold is eased by up to 2 steps (~1 dB) so weak voice opens the squelch sooner. The bias is removed the moment confidence drops, and the parked evaluator never runs during any scan, while VFO Split is armed, or in power save.
- **Adaptive Squelch State** (#13): tracks the noise floor and squelch open count to inform Intelligence-Based Squelch.

### Scan and watch background

- **Intelligent Dual-Watch** (#15): when dual-watch is enabled, the radio tracks activity on each VFO and adjusts dwell times so a busier VFO gets shorter dwell (you will catch it anyway) and a quieter one gets longer (so you do not miss brief activity). v1.3.1: the base dwell dropped from 500 ms to 150 ms (+/-50 activity bias), restoring stock-class detection latency -- a call starting on the other VFO is now caught about four times sooner. v1.3.3: alternation pauses while any key is held and resumes on release, so manual tuning is never interrupted; a corrupted dual-watch setting byte now reads back as OFF instead of silently enabling the mode.
- **Adaptive Scan** (v1.3.3): a ~30 ms energy pre-check on every stepped channel lets empty channels advance immediately while any channel showing energy gets the identical full dwell -- 2-3x faster on quiet bands, weak-signal sensitivity unchanged. Tone-coded memory channels get a 500 ms dwell so the decoder can confirm the tone.
- **Scan Rate Telemetry** (#30): live channels-per-second readout appended to the SCANNING status line.
- **Scan+Watch persistence** (v1.3.0): the ScnWch setting (SCAN A WATCH B / SCAN B WATCH A) is now persistent for the power cycle. It arms automatically every time you start a scan and survives the scan stopping; before v1.3.0 it was one-shot and silently reset to OFF after every scan.
- **VFO Split walks the channel list** (v1.3.0): SPLIT mode now genuinely steps VFO B through every valid memory channel, one hop at a time, and VFO B's display shows the channel currently being checked. Before v1.3.0 it re-checked VFO B's parked channel forever. Note: SPLIT checks channels while you listen on your active VFO; power save is disabled while SPLIT is armed (a 300 ms hop watch cannot sleep), so expect higher battery drain, same as during a scan.
- **Empty scan list refusal** (v1.3.0): starting a memory scan with scan list 1 or 2 selected and no channels in that list now stops immediately with an `EMPTY LIST` toast instead of displaying SCANNING while walking nothing.
- **VFO Split announces itself** (v1.3.1): while split is armed the status bar shows `SP`, and when VFO B finds activity the radio shows an `ACT 446.00625` toast (the frequency where activity was heard) and sounds a single 880 Hz cue. The cue plays even if key beeps are turned off -- arming split is opting into an audible alert; that is the feature's purpose. Before v1.3.1 the alert was computed but never displayed anywhere. v1.3.3: split stops launching new hops while you are holding a key (an in-flight hop still finishes and restores VFO A cleanly); hops resume on release.
- **Battery sag warning** (v1.3.1): after a transmission, if the battery voltage sagged 0.20 V or more under TX load, a `SAG 0.25V`-style toast reports it. Healthy packs sag 0.05 to 0.10 V; repeated 0.20 V+ sag means the pack is tiring (see the About screen's TX sag line for the exact last value).

### Squelch behavior

- **Squelch Tail Elimination** (#11): monitors the CTCSS tone detector (and, since v1.3.1, the DCS code detector) and mutes the audio before the noise burst at end of transmission. Confidence-weighted: fluttery signals demand a slightly longer confirmation window so brief tone dropouts do not pre-mute. Menu-toggleable via TONE > STE.

### UI passives

- **RSSI Bar** (#22): optional signal-strength bar on the center line (compile-time option).
- **Context-Aware Status Line** (#23): the bottom line of the display automatically switches its content based on what the radio is doing (idle, RX, TX, scanning).
- **Boot Screen** (#24): the welcome banner at power-on (Section 1).
- **Toast Notification Subsystem** (#29): the engine that powers the 1-second overlay you see after every F-key shortcut and every bound side-button action.
- **Side-Button Toast Feedback** (#28): when a side-button binding fires, the toast subsystem emits a confirmation overlay matching the bound action.

### Reliability and diagnostics

- **Flashlight Auto-Off Watchdog** (#32): extinguishes the flashlight after 30 minutes of unattended ON or BLINK runtime (10 minutes on a low-battery pack). SOS preserved indefinitely. Mode change resets the timer.
- **CSS Scan Soft-Timeout Watchdog** (#33): 30-second cap on CSS scan (tightened in v1.2.7), emits a double-beep at timeout.
- **Audio Palette VOX-Hop Cue** (#34): ascending two-tone confirmation chord (800 then 1200 Hz) whenever a voice-hop lands on a voice bin. A seek that finds nothing simply navigates -- motion is the feedback.
- **Boot-Time Hardware Health Probe** (#35): BK4819 and EEPROM self-test at boot (Section 13).
- **Live Battery Voltage During TX** (#36): 500 ms update tick during transmit so the battery indicator is current.
- **Backlight Fade-Out Tail** (#37): 2-second taper to off instead of an instant cut.
- **Backlight TX/RX Activity Refresh** (#38): re-arms the backlight during transmit or receive if `BltTRX` is set to TX, RX, or TX+RX (Section 12).
- **TX Battery Sag Delta Tracker** (#39): per-transmission voltage sag captured and shown on the About screen (Section 11).
- **CSS Scan Status-Bar Glyph** (#40): `Cs` glyph in the status bar during CSS scan, distinct from channel scan.
- **CSS Scan FOUND Beep** (#41): 1 kHz beep when the CSS scanner locks onto a tone.
- **Quiet Backlight PWM** (#42): reprograms the PWM carrier to about 6.7 kHz at boot, above the audio passband, to eliminate the 1 kHz whine that bleeds into TX audio when the screen is dim.

---

## 16. Cheat Sheet

This page is built from the operating sections above. If a row here disagrees with an operating section, the operating section wins.

### F+key shortcuts (from the main screen)

| Press | Action | Toast |
|---|---|---|
| F then 0 | Cycle modulation | `MOD: FM` / `MOD: AM` / `MOD: USB` |
| F then 1 | Cycle band (VFO mode only) | `BAND` or `FREQ ONLY` |
| F then 2 | Toggle VFO A/B | `VFO: A` / `VFO: B` |
| F then 3 | Toggle VFO/MR mode | `VFO MODE` / `MR MODE` / `NO CHANNELS` |
| F then 4 | Cycle squelch level | `SQL: 0` through `SQL: 9` |
| F then 5 | Launch spectrum analyzer | (no toast) |
| F then 6 | Cycle TX power | `PWR: LOW` / `PWR: MID` / `PWR: HIGH` |
| F then 7 | Toggle bandscope | `SCOPE ON` / `SCOPE OFF` |
| F then 8 | Reverse offset | `REVERSE` / `NORMAL` |
| F then 9 | Calling-frequency jump | `CALL FREQ` / `RESTORED` / `NO BAND` |
| F then EXIT | Reset VFO to simplex (freq mode only) | `SIMPLEX` or `FREQ ONLY` |
| F then STAR | CSS scanner (FM only) | `FM ONLY` if not in FM |

Bonus: EXIT alone (no F, nothing to cancel) toggles VFO A/B, same as F+2.

### Long-press gestures

| Gesture | Action | Toast |
|---|---|---|
| Long F | Toggle keypad lock | `LOCKED` / `UNLOCKED` |
| Long STAR | Toggle scanning | `SCAN ON` / `SCAN OFF` |
| Long MENU | Configured `M Long` action | depends on binding |
| PTT + SIDE1 at power-on | Unlock UNLOCK menu category | (no toast; UNLOCK appears in menu) |

### Inside the spectrum analyzer (sweep view)

| Press | Action |
|---|---|
| STAR | Cycle mode NORM / PEAK / VOX (brief mode-flash overlay; releases a lock) |
| UP / DOWN | Directional voice seek (VOX) or span step (NORM / PEAK); auto-repeats |
| SIDE1 / SIDE2 | Span UP / DOWN in every mode (auto-repeats while held) |
| PTT | VOX: lock the voice (`LOCKED` banner). NORM / PEAK: park on peak (STILL) |
| MENU short | Blacklist current peak bin |
| F | Trigger level down |
| 1 / 7 | Scan step wider / narrower |
| 2 / 8 | Move step bigger / smaller |
| 3 / 9 | dB ceiling up / down |
| 4 | Cycle bin count 128 / 64 / 32 / 16 |
| 5 | Direct frequency entry |
| 6 | Cycle listening bandwidth 25k / 12.5k / 6.25k |
| 0 | Cycle modulation FM / AM / USB |
| EXIT | Return to main screen (locked frequency carries over in freq mode) |

### Status bar glyphs

| Glyph | Meaning |
|---|---|
| `S` | General scan running |
| `1`, `2`, `*` | Memory-channel scan, list 1 / list 2 / all |
| `S+W` | Scan+Watch active |
| `Cs` | CSS scan running |
| TDR icon | Dual-watch active |
| XB icon | Cross-band active |
| `SP` | VFO Split armed (v1.3.1) |
| padlock icon | Keypad locked |
| `F` icon | F key was just pressed |
| `Q` plus 0 to 3 bars | Signal quality during RX |
| battery icon | Battery level 0 to 4 bars |

### Bottom status line formats

| State | Format |
|---|---|
| Idle | `12.5kHz N FM` (step, bandwidth, modulation) |
| RX | `RX S7 -65dBm Q:3 F` |
| TX | `TX 0:12 PWR:H` |
| Scanning | `SCANNING 8c/s` (or `SCANNING` before rate stabilizes) |

### Audible cues

| Sound | Meaning |
|---|---|
| Short ascending two-tone (800 Hz then 1200 Hz) | VOX-hop landed on a voice bin |
| Soft 500 Hz double-beep | CSS scan timeout / NO SIGNAL abort, or rejected key press |
| Single 1 kHz beep | CSS scan found a tone, or long-STAR scan toggle confirmation |
| Single 880 Hz tone (200 ms) | VFO Split heard activity on a memory channel (v1.3.1; plays even with Beep off, paired with the `ACT` frequency toast) |
| Click beep | Stock key-press feedback (toggle via CONFIG > Beep) |

### Welcome banner identification

| Banner | Meaning |
|---|---|
| `VUURWERK` / `v1.5.0` / `Precision RF` | Normal boot |
| `VOLTAGE` / `N.NNV NN%` | POnMsg set to Voltage |
| `BK4819` / `FAULT` / `RX/TX disabled` | Transceiver fault (Section 13) |
| `EEPROM` / `FAULT` / `calib lost` | Calibration page fault (Section 13) |
| `RELEASE` / `ALL KEYS` / [key name] | Powered on with PTT plus another key held |

### Menu category navigation

| Press | Action |
|---|---|
| MENU (on main screen) | Open category picker |
| 1 to 7 (in picker) | Jump to category 1 to 7 (UNLOCK only if unlocked) |
| MENU (in picker) | Enter selected category |
| EXIT (anywhere) | Back out one level, or leave menu |

---

*VUURWERK v1.5.0. Last verified against source 2026-07-04.*

*Standard: KC3TFZ.*
