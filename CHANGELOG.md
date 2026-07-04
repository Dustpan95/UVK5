# VUURWERK Changelog

## v1.5.0 (July 2026) -- THE STABILIZATION RELEASE

Two live bugs that match the field reports exactly, both now fixed:

- **The residual VFO flip:** holding EXIT a beat too long when leaving the
  spectrum re-latched the still-pressed key as a FRESH main-screen press ~100ms later --
  and EXIT on the main screen is the VFO A/B toggle. Operator-timing dependent,
  bidirectional, immune to the v1.4.3 exit determinism (it is a real post-exit
  keypress). Fixed: APP_RunSpectrum drains the keyboard until release before returning.
- **A still-live deaf-on-exit path:** Tick()'s body continued after
  DeInitSpectrum returned, so one more Scan() step re-tuned the chip to the sweep
  position and re-wrote REG_30 from the stale scan-time cache (AF-DAC bit off) --
  radio mistuned + audio-DAC-dead until the next reconfigure (masked when dual watch
  retunes within seconds; naked with DW off). Fixed: Tick returns immediately after
  HandleUserInput when the spectrum has deinitialized, and the exit retune marks the
  REG_30 cache dirty so it pulses the freshly restored register value.

**Binary:** 61,396 / 61,440 bytes (.bin, 44 free). SHA-256 packed
14b54b2e84623d4009f619d42e6a3f82230b66d77d7e513b0636f3dfb1ddee15. Clean canonical
Docker build; version verified in-binary; zero warnings.

## v1.4.5 (July 2026)

Fixes all six findings from the full VOX-era code review (v1.3.5..v1.4.4, every change
re-verified line-by-line against source).

**CRITICAL: the v1.4.4 F+EXIT rescue never fired.** Stock app.c:1898 cancels the F-key on
EXIT before MAIN_Key_EXIT runs, so F+EXIT was dead code -- the chord fell through to
COMMON_SwitchVFOs and performed a VFO A/B FLIP instead of the simplex reset. (Field impact:
the operator's "F+EXIT then still deaf" tests never actually cleared the tones.) Fixed by
removing KEY_EXIT from the F-cancel list (MAIN_Key_EXIT consumes the flag itself;
PTT/SIDE1/SIDE2 keep the stock cancel).

Also fixed from the review:
- F+EXIT now resets Modulation to FM too (an AM/USB slot renders FM voice unintelligible).
- Spectrum exit: retune moved AFTER RADIO_SelectVfos and now tunes gRxVfo -- with crossband
  ON the old order left the crossband watch frequency deaf until the next reconfigure.
- Spectrum exit: dual-watch countdown is reloaded BEFORE the flag is cleared (the 10ms ISR
  could fire between the stores and re-latch an instant flip -- rare but real).
- PTT in a >128-step scan-range span that displays "VOX" no longer drops into the STILL
  register editor (does nothing; VOX lock unavailable there by design).
- VOX-lock carry-over: skipped when FrequencyReverse is set (would mistune by the offset),
  and the frequency is step-rounded before persisting (reload rounds anyway; unrounded saves
  could shift by half a step after reboot).

Review also verified CLEAN: the VOX lock lifecycle, the v1.4.2 carry-over memory-channel
fences, the MTI removal (no stale references), the hint-pool trim (40/40 entries matched),
and the v1.4.3 exit for all dual-watch/crossband-OFF cases.

**Binary:** 61,376 / 61,440 bytes (.bin, 64 free). SHA-256 packed
1a95658a40e3090db42690c9a61774eeb3d5fe9de3944ca8297ef0815dd937b7. Clean canonical Docker
build; version string verified in-binary; zero warnings.

## v1.4.4 (July 2026)

F+EXIT = RESET VFO TO TRUE SIMPLEX (operator-requested rescue key).

v1.4.3 field results: the exit VFO-flip is CONFIRMED FIXED (exit lands on the side you
entered from). The remaining symptom -- one VFO deaf from power-on while the other hears,
green RX light, squelch irrelevant -- is per-VFO configuration, not a code path: an RX
CTCSS/DCS holds audio for a tone that never comes (app.c:224), a repeater offset with
FrequencyReverse listens off the displayed frequency, or a scrambler garbles RX. All are
per-VFO fields; B lacking them is exactly why B hears everywhere.

New: press F then EXIT on a VFO (frequency mode) to wipe it back to a clean simplex state:
RX/TX CTCSS-DCS cleared, repeater offset + direction cleared (TX = RX, true simplex),
frequency reverse off, scrambler off. Persists via the blessed keypad-commit path
(SaveChannel + ConfigureChannel + SetupRegisters). Memory channels are never touched
("FREQ ONLY" toast). Toast: "SIMPLEX". This is the requested "reset to simplex" feature and
the one-gesture cure for a polluted VFO, whatever the polluted field.

Flash funded by tightening the longest menu hint texts (display text only; the hint pool is
NUL-walked in order, so texts are freely editable -- no code change).

**Binary:** 61,372 / 61,440 bytes (.bin, 68 free). SHA-256 packed
1c137921e333bad4e7e93e1c18c2ef97b24b80c282e6e4bf75f1b866432f1d56. Clean canonical Docker
build; version string verified in-binary; zero warnings.

## v1.4.3 (July 2026)

Deterministic spectrum exit -- the definitive fix for "exiting the spectrum flips me to the
other VFO."

Field forensics (operator report): the radio was deaf on VFO A from POWER-ON (before any
spectrum use), B worked everywhere, and the "flip" happened on a plain no-lock exit -- so
neither symptom was the carry-over. Source trace: with dual watch and crossband both off,
NO code path can change the active VFO (RADIO_SelectVfos pins RX=TX radio.c:519; the
alternator is gated at app.c:922). The flip pattern (radio parks on whichever side has
signal) is dual watch operating by design, amplified by the modal spectrum session: the
stale gRxVfoIsActive/RX_VFO from a pre-entry park rides through, and the DW countdown
drains-and-latches during the sweep, so the flip fires the instant the spectrum exits.
The per-VFO "green light but no audio" is RX tone squelch (app.c:224): an RX CTCSS/DCS
code configured on that VFO opens carrier squelch but holds audio for the tone -- a radio
SETTING, not firmware (menu R-CTCS / R-DCS -> OFF to clear).

Fix: every spectrum exit (lock or not) now re-syncs RX to the operator's TX VFO
(RADIO_SelectVfos), clears the stale receive flag, and grants dual watch a fresh full
dwell so it cannot alternate within 10 ms of the exit. You always come back on the side
you left from; dual watch resumes its normal duty cycle afterward. The v1.4.2 direct-write
carry-over is retained unchanged.

**Binary:** 61,388 / 61,440 bytes (.bin, 52 free); packed 61,406. SHA-256 packed
4771eec942d04384c4086be78890fc6d2a5eeac51e35cade931393044743bd2a. Clean canonical Docker
build; version string verified in-binary; zero warnings.

## v1.4.2 (July 2026)

Carry-over re-done correctly: DIRECT VFO write, no reconfigure. The exit VFO-flip + deaf
radio in v1.3.6-v1.4.0 came from the carry-over persisting via gRequestSaveChannel, which
made ProcessKey's exit tail run RADIO_ConfigureChannel + RADIO_SelectVfos +
RADIO_SetupRegisters. With crossband AND dual-watch both confirmed OFF, neither built-in
VFO-flip path can run (RADIO_SelectVfos sets RX=TX; DualwatchAlternate is gated on
DUAL_WATCH != OFF at app.c:922) -- so the reconfigure TAIL itself was the culprit, and a
normal (no-lock) spectrum exit never runs it, which is exactly why normal exits work.

v1.4.2 mirrors the normal exit: on exit with a lock, write the locked frequency into the
active VFO's RAM, persist it straight to EEPROM (SETTINGS_SaveChannel -- no flag, no
reconfigure), clear gRxVfoIsActive (show the active/TX VFO), then the SAME SetF +
RestoreRegisters the normal exit uses. Freq mode + same band only; provably never touches a
memory channel. Lock, PTT-in-VOX, MTI removal, PEAK, About restore unchanged.

This direct-write carry-over was never cleanly tested before -- v1.3.7/1.3.8 carried it but
shipped with a stale version string, so those flashes ran the OLD broken build. This is its
first clean build.

**Binary:** 61,380 / 61,440 bytes (.bin, 60 free). SHA-256 packed
75a286be3f3a41beed735ac4209f6ceeb3562098d604bc2a1dab5fa7130fb193. Clean canonical Docker
build; version string verified in-binary; zero warnings.

## v1.4.1 (July 2026)

Removes the VOX-lock frequency carry-over. It was the source of the exit-time VFO flip
AND the deaf radio, and could not be made reliable across four attempts (v1.3.6 -> v1.4.0):
persisting the locked frequency made ProcessKey's exit tail run RADIO_SelectVfos +
RADIO_SetupRegisters, which flipped the active VFO and stranded/muted the receiver.

The lock is now **monitor-only**: you can still lock onto a voice (PTT) and seek (UP/DOWN),
the frequency just no longer follows you out of the spectrum. Exiting a lock is now
byte-identical to a normal spectrum exit -- the known-good path. Everything else from v1.4.0
(MTI removed, PEAK kept, PTT-in-VOX, About restored) is retained.

Recovery note: if the radio is still deaf after flashing, power-cycle it (clears any stuck
runtime state); if still deaf, an earlier build may have saved a bad VFO config -- re-enter
the frequency (or reset the VFO) to clear it.

**Binary:** 61,280 / 61,440 bytes (.bin, 160 free); packed 61,298. SHA-256 packed
335c660e0a1a0ff06d6fed358b612db1761b7ecbe2c85bf982d19371f6aa58f7. Clean canonical Docker
build; version string verified in-binary; zero warnings.

## v1.4.0 (July 2026)

Definitive deaf-radio fix + MTI spectrum mode removed. Supersedes the incomplete
v1.3.7/v1.3.8 (and fixes a build-hygiene slip in those: incremental builds left a stale
"v1.3.6" boot string -- this is a clean build, version verified baked in).

**Deaf radio -- the correct fix.** Locking a voice in the spectrum and exiting could
leave the radio deaf (green RX light, no audio) and switched to the other VFO, with
dual-watch on. Root cause (deep source trace): the multi-second spectrum session freezes
the FUNCTION state, so the dual-watch flip countdown drains and latches an OVERDUE flip. On
exit -- AFTER the save tail's RADIO_SetupRegisters(true) has cleanly reset FOREGROUND
(radio.c:714) so audio can re-open -- that overdue flip fires and calls
RADIO_SetupRegisters(FALSE), which mutes the AF and RE-strands FUNCTION_RECEIVE (the
identical failure was fixed once before, at v1.2.8). Fix: on exit with a lock, cancel the
overdue flip (gScheduleDualWatch = false) so the (true) FOREGROUND reset STICKS, and release
the lock's uniquely-skipped ToggleRX(false) (stuck green LED + forced RX). v1.3.7/1.3.8
wrongly tried a direct save that BYPASSED the (true) reset -- reverted.

**PTT in VOX** now only ever locks a voice (or does nothing if none is heard yet) instead of
dropping into the STILL register/meter editor.

**MTI spectrum mode removed.** Research found MTI (a crude sweep-to-sweep XOR) has no
community usage and is the safe display mode to cut; PEAK (peak-hold, genuinely relied on
across the RF world) stays. The spectrum now cycles NORM / PEAK / VOX. This freed the flash to
restore the About-screen feature list (borrowed in v1.3.7) plus 128 bytes of RAM.

**Binary:** 61,348 / 61,440 bytes (.bin, 92 free); packed 61,366. SHA-256 packed
e26a154932c84d755cf2432f887b755d8790b24087539cd4c7019e435ffc0041. Clean canonical Docker
build; version string verified in-binary; zero warnings.

## v1.3.8 (July 2026)

Completes the v1.3.7 deaf-radio fix. v1.3.7's direct-save change was necessary but NOT
sufficient -- a deeper trace found the real cause: a dual-watch timing latch. During the
multi-second spectrum session the FUNCTION state is frozen at idle, so the dual-watch flip
countdown drains and LATCHES an overdue flip. On exit that flip fires, mutes the AF
(RADIO_SetupRegisters(false)) and strands FUNCTION_RECEIVE -> deaf, on the other VFO. (Only
bites with dual-watch ON; single-VFO is immune. The identical failure was documented in the
code at v1.2.8.)

**Fix:** on spectrum exit with a lock, cancel the overdue dual-watch flip
(gScheduleDualWatch = false) and release the lock's forced RX (ToggleRX(false) -- the locked
exit path uniquely skipped it, leaving a stuck green LED + AF path) before the retune. The
carried voice now settles on the SAME VFO with audio, exactly like a normal exit. The v1.3.7
direct-save (SETTINGS_SaveChannel, no muting reconfigure) + PTT-in-VOX fix are retained.

**Binary:** 61,392 / 61,440 bytes (.bin, 48 free); packed 61,410. SHA-256 packed
110727aa2e840557e9e707e0f5c5d9c113fc3968167a87c9b88e9ef74b6906a3. Canonical Docker
toolchain, zero warnings.

## v1.3.7 (July 2026)

Deaf-radio fix (critical) for the v1.3.6 VOX lock carry-over, plus a PTT fix.

**The bug:** after locking a voice in the spectrum and exiting, the radio was left
DEAF -- green RX light but no audio, on both VFOs, even at squelch 0. Root cause:
the v1.3.6 carry-over persisted the frequency by setting gRequestSaveChannel, which
made ProcessKey's exit tail run the full RADIO_SetupRegisters reconfigure. That call
MUTES the audio path (AUDIO_AudioPathOff + gEnableSpeaker=false) and re-derives the
RX VFO, expecting the normal receive flow to re-arm audio -- which does not happen
coming out of the spectrum. A normal (no-lock) spectrum exit was never affected
because it skips that reconfigure.

**The fix:** the carry-over now persists the locked frequency DIRECTLY
(SETTINGS_SaveChannel + RADIO_ApplyOffset in DeInitSpectrum) instead of via the
deferred flag, so exit stays on the clean SetF + RestoreRegisters path a normal exit
uses -- audio survives exactly like a normal exit, and no VFO switch. Same
freq-mode-only, same-band gating as v1.3.6; still provably cannot touch a memory
channel.

**PTT in VOX:** pressing PTT when NOT yet hearing a voice used to drop into the STILL
register/meter editor screen (confusing). In VOX, PTT now only ever locks a voice; if
nothing is being heard yet it does nothing (seek first). NORM/PEAK/MTI keep the stock
PTT->STILL park-and-monitor behavior.

**Flash:** the direct save costs ~20 bytes over the old flag; reclaimed by dropping the
4-line feature list from the (rarely-seen) About screen -- restored in v1.4.0 when the
MTI-mode removal freed the headroom.

**Binary:** 61,368 / 61,440 bytes (.bin, 72 free); packed 61,386. SHA-256 packed
c1cf7295ddaffa29b9e5099e3397cd8d3e99f950223a946f2a9db639446df1a6. Canonical Docker
toolchain, zero warnings.

## v1.3.6 (July 2026)

VOX Voice Lock + VFO carry-over. Everything else is byte-for-byte unchanged.

You asked to be able to stop the hunt once you find a signal worth staying on,
and to keep that frequency when you leave. Both are now in (and the
frequency-write is fenced so it can never corrupt a memory channel):

- **PTT locks onto the voice you're hearing.** In VOX, while listening to a
  station, tap PTT: the radio stops hopping, a "LOCKED" banner flashes and fades,
  and it holds that frequency -- it no longer drops the lock and resumes hunting
  when the voice pauses. It's an open-squelch monitor, so you hear the whole
  channel (both sides of a QSO, and anyone else who keys up). PTT still means
  "park here" (STILL) in the other spectrum modes and can't transmit inside the
  spectrum, so nothing is lost.
- **UP or DOWN releases** and resumes the directional voice-seek from v1.3.5.
- **Exit while locked and the frequency follows you out.** If you were in
  frequency (VFO) mode, the locked frequency becomes your VFO frequency, so the
  moment you leave the spectrum you're tuned to it and can answer in simplex. It
  goes through the radio's own save path (the same one a keypad frequency entry
  uses), gated so it is written *only* in frequency mode and only when the
  frequency is in the VFO's current band -- in memory-channel mode it is a
  provable no-op (three fences) that cannot touch a channel, and a cross-band
  lock simply exits on your original frequency.

Flash was the binding constraint (the feature is ~2x what was free), so the fit
came from behavior-neutral trims plus one visible change: the brief mode-change
toast now reads "VOX" instead of "MODE VOX" (the mode name is already shown in
the corner). Same-band carry ships now; cross-band carry is a later add once
flash is reclaimed.

**Binary:** 61,424 / 61,440 bytes (.bin, 16 free); packed 61,442. SHA-256:
.bin e6152f3993c2f679835197971f0dc88d6de24690b37db64ae9f133bd6f7281cf, packed
8ed9ddb25d2ef0e188ca27348f5204f8f37f228e1674efac5ac4bd6569e2cf4e. Canonical
Docker toolchain, zero warnings.

## v1.3.5 (July 2026)

Voice-Seeking Spectrum (VOX) navigation fix + hardening. No change to the rest
of the radio.

In VOX mode, UP/DOWN was bound to a wrap-around "voice hop" that could orbit a
single window forever and, on a band with no detected voice, played a descending
chord and did nothing -- so the operator lost band navigation on the arrow keys.
(v1.2.6 had already tried to work around this by moving frequency stepping to the
SIDE keys; it never stuck, because UP/DOWN is the navigation gesture.) Redesigned,
verified against source:

- **UP/DOWN is now a directional voice seek**: UP walks to the next voice bin
  above the cursor, DOWN below, no wrap (UP always raises frequency). When there
  is no voice that way it falls through to real band navigation (the same
  UpdateCurrentFreq NORM/PEAK/MTI use). So UP/DOWN always moves you; a quiet band
  simply tunes, and holding a key patrols the band and auto-stops on voice.
  SIDE1/SIDE2 stay as pure "move without seeking".
- The dead-end no-find chord is removed -- motion is the feedback; the ascending
  found-voice chord stays for successful hops. AUDIO_PALETTE_PlayNoVoice retired.
- Two latent bugs also caught and fixed: entering VOX could
  fabricate fake voice bins for one sweep (noiseHistory seeded to 0 scores as
  voice; now seeded to 255 = reject), and the hop could land on an invisible
  blacklisted bin and silently un-blacklist it (now guarded by
  rssiHistory != RSSI_MAX_VALUE, matching the auto-listen path).

The passive voice-seek (auto-listen + the v1.3.3 noise-gate hold rule) is
byte-identical. Net -32 bytes vs v1.3.4.

**Binary:** 61,280 / 61,440 bytes (.bin, 160 free); packed 61,298. SHA-256:
.bin e445bd1f4ce98ad1988829a5325171fab8f31c45bd3dbdd6332cd91fc601eb88, packed
49a37aaa02d1dfa68d175f907e99c5e5d08816496b82ffef13f83641419f2a1e. Canonical
Docker toolchain, zero warnings.

## v1.3.4 (July 2026)

Display refinement over v1.3.3, no functional change to the radio's behaviour.
The v1.3.3 rework relocated the bottom status line to the centre line; on the
bench that thinned the display and dropped the signature "RX S.. ..dBm Q.. F"
readout that reads like the instrument it is. v1.3.4 restores the status line
and RX readout to their original bottom-row placement (they draw over VFO-B's
mode indicators by design, the established VUURWERK look). The v1.3.3 renderer
clamp stays, so this is the original look with the framebuffer-overrun /
settings-corruption defect still fixed -- not a revert of the safety work. The
bottom readout and the centre RSSI bar were unified onto one shared calibrated
computation (RX_Signal_dBm) so they report the same dBm/S; the readout
previously omitted the per-band correction and disagreed with the bar.

Compatibility unchanged from v1.3.3: no menu / keybinding / EEPROM / CHIRP
surface touched.

**Binary:** 61,312 / 61,440 bytes (.bin, 128 free); packed 61,330. SHA-256:
.bin a9f6f2730ad62269eb4f4a44a2bd781b5a326a8a2260fb483be41cebf276db2f, packed
6d4f249eec864d5db758bd3093f510e72ba8563ca0bd803286802e8a1a42d0ad. Canonical
Docker toolchain, zero warnings. Same feature set as v1.3.3; +68 bytes for the
restored RX readout net of the shared-helper dedup.

## v1.3.3 (July 2026)

"Everything-on" workshop/test build: Scott Savage's (KC7WDG) scan-deafness
fix, a reliability and memory-safety sweep, and a major Voice-Seeking Spectrum
upgrade -- built to bench-tune toward a shelf-stable v1.3.4. Compatibility
doctrine held: no menu names, ordering, or value vocabulary changed, no
keybindings changed, no EEPROM layout changes (v1.3.x settings and channels
survive flashing untouched), CHIRP driver surface untouched.

**Binary:** 61,244 / 61,440 bytes (.bin, 196 free); packed 61,262. SHA-256:
.bin 43e8a69a37831876088ff2ceb55661ef3f4bfca1e015a97b529510a715c7b60e, packed
f6f2468b2ec90200b8e6d96a92006529f827583c69cccb750b2a6a0a864d54da. Canonical
Docker toolchain, zero warnings. +344 bytes vs v1.3.2, spent on the
Voice-Seeking Spectrum upgrade, adaptive scan, and the memory-safety sweep.

### Fixed

- **Scanner went deaf on tone-coded memory channels** (reported by Scott
  Savage, KC7WDG). Tone-coded (CTCSS/DCS) MR channels only received the flat
  200 ms carrier dwell -- too short for the decoder to confirm the tone before
  the scanner stepped on -- so coded channels were effectively skipped. Coded
  channels now get a 500 ms dwell (scan_pause_delay_coded_10ms); non-coded
  channels are unchanged.
- **The RX status line and the About screen overran the framebuffer into saved
  settings** (ui/helper.c, ui/main.c, ui/vuurwerk_about.c). The small-font
  renderer had no upper bound, so a long status string -- or the About screen's
  row-7 draw, past the last framebuffer row -- wrote into the globals linked
  after gFrameBuffer, silently corrupting stored channel and settings bytes.
  This is the most likely cause of the "settings change on their own" reports.
  The renderer now clamps to the framebuffer extent and rejects out-of-range
  rows; the RX status line moved to the centre line so it no longer collides
  with VFO-B's mode indicators; the About screen fits rows 0-6.
- **Tail-tone elimination inverted the DCS code-present test** (squelch_tail.c,
  a v1.3.1 regression). In this firmware g_CDCSS_Lost == true means the code IS
  present; the T4 path used !g_CDCSS_Lost, so it would have muted real DCS
  receptions. Polarity corrected to match the stock detect condition, plus a
  presence-proof latch so a co-channel signal on a non-matching tone can no
  longer walk the tail-elimination machine to the mute path.
- **Voice-Seeking Spectrum: chained hops locked silently** (app/spectrum.c).
  The "found voice" chord mutes the chip AF (REG_47) and the follow-on ToggleRX
  no longer reopened it when hopping from an active listen. Fixed by re-syncing
  audioState so the AF path is physically restored.
- **Voice-Seeking Spectrum: two out-of-bounds accesses.** In >128-step
  scan-range spans the voice gate and noise store indexed the 128-entry arrays
  with a raw peak index; and every sweep measured one step past the span edge
  (rssiHistory[128] at STEPS_128). Both guarded.
- **Voice-Seeking Spectrum: bin 0 was never scannable** (the blacklist was
  zero-initialised, so IsBlacklisted(0) always matched) and the blacklist
  leaked across sessions. The blacklist is now filled with 0xFFFF and reset on
  each spectrum entry.
- Dual watch yields to manual tuning (app/app.c); VFO-split TX-abort /
  hop-launch / dual-watch-crossband edge cases (vfo_split.c); EEPROM
  corrupt-byte validation for the NOAA channel, DUAL_WATCH, battery and RSSI
  calibration (settings.c); UART 051B read clamp (app/uart.c); RSSI-histogram
  int8 clamp (rssi_histogram.c); side-toast VFO/MR readout (side_toast.c); F+8
  reverse (app/main.c); TX-compressor FM-only gate (tx_compressor.c).

### Improved

- **Adaptive scan dwell (~2-3x faster on empty bands, weak-signal-safe;
  ENABLE_ADAPTIVE_SCAN)** (app/chFrScanner.c). Each stepped channel gets a
  short ~30 ms RSSI pre-check; only channels reading at/above the band's
  first-bar calibration point are held for the full 90 ms dwell -- empty
  channels advance immediately. Any channel showing energy always gets the
  identical full evaluation, so weak-signal sensitivity is unchanged by design.
  The scan-rate and Scan+Watch step counters are gated on a real advance.
- **MicDB curve bumped** (misc.c): the middle and upper mic-gain steps drive
  ~1.5-2 dB harder (+6.0/+10.0/+13.5 dB vs +4.0/+8.0/+12.0); the quiet step and
  the +15.5 dB hardware maximum are unchanged. No value exceeds the 5-bit field
  ceiling, so there is no new distortion risk versus the existing top step.
- **Voice-Seeking Spectrum now seeks voice, not the loudest signal**
  (app/spectrum.c, audio_palette.c). When the strongest bin isn't voice, the
  auto-listen seeks the strongest actual voice bin in the span and locks it.
  Locks hold through natural speech gaps -- the noise gate is the hold
  authority, independent of the RSSI trigger, with 2-strike hysteresis to ride
  a lone flutter sample. A readable moderate voice (noise 30-49) now reaches
  the detect threshold instead of scoring just under it. A descending
  1200->800 Hz chord plays synchronously when a hop finds nothing (the old
  queued double-beep never sounded until spectrum exit).

### Honesty section

- **FM transmit deviation (louder TX) is deliberately not changed.** It was
  fully investigated: there is no factory deviation calibration in the UV-K5
  EEPROM to read, and the community deviation forks do not ship a universal
  "safe" REG_40 value -- they add a per-radio measurement menu, because the
  raw-value-to-kHz relationship varies between radios. Adopting a blind number
  would risk illegal over-deviation and adjacent-channel splatter. Parked for
  RTL-SDR measurement toward v1.3.4; MicDB is the safe interim loudness lever.
- **A dead-carrier auto-release was built, verified ineffective, and removed.**
  Review found it re-locked the same birdie within one sweep (a
  dead carrier scores highest in the voice-probability function) and relied on
  an unverified glitch threshold. Removing it makes the voice hold strictly
  more reliable -- a keyed-but-silent carrier stays quieted (noise < 70) and
  holds, so a real station is never dropped on a long pause. Birdies are
  escaped by manual blacklist (MENU), as before.

## v1.3.2 (June 2026)

Boot banner hotfix. One defect, display-only, no settings semantics
changed, no menu/keybinding/EEPROM/CHIRP surface touched.

**Binary:** 60,900 / 61,440 bytes (.bin, 540 free); packed 60,918.
SHA-256: .bin
92f8433c8df1d68328ed7051e8efd4b0b5bdacf7ef40be6712f653583a846cf1,
packed
ef36bfefdff83bb10601d7f1ae0e07739f7cae9d537cd249c2dfcaf601bc2b8c.
Canonical Docker toolchain on the canonical build machine, zero
warnings. -36 bytes vs v1.3.1.

### Fixed

- **POnMsg=FULL booted to a solid black screen instead of the
  VUURWERK banner** (ui/welcome.c). FULL is the factory-default
  byte, so fresh flashes and any radio reset to defaults greeted
  the operator with a black void at every power-on; the documented
  banner was only reachable when POnMsg happened to be MESSAGE.
  Provenance: FULL=black is inherited stock semantics ("full
  screen" = all-pixels-on LCD test) present since the v1.2.4
  baseline import, and the 2026-05-10 welcome dedupe commit folded
  NONE into the same black fill (stock NONE is blank). As of
  v1.3.2: FULL and MESSAGE both render the VUURWERK banner
  (brand, version, Precision RF), NONE blits the cleared buffer
  (genuinely blank, stock semantics restored), VOLTAGE unchanged,
  fault banners unchanged. The welcome-path FillScreen(0xFF) call
  is removed entirely. Stored setting values keep their meanings;
  only what FULL and NONE render changed. USER_MANUAL section 1
  now documents all four modes truthfully, including a note for
  operators who ever saw the black flash.

## v1.3.1 (June 2026)

Operating quality, technical excellence, and core refinement pass.
Compatibility doctrine held throughout: no menu names, ordering, or
value vocabulary changed, no keybindings changed, no EEPROM layout
changes (v1.3.0 settings and channels survive flashing untouched),
CHIRP driver surface untouched. The MR scan stop-on-signal decision
path is untouched.

**Binary:** 60,936 / 61,440 bytes (.bin, 504 free); packed 60,954.
SHA-256: .bin
feb5746a8b903eb1e675441eb579abeab92a9e5f30027d19eaef67f079c3f9a2,
packed
78a90257269c6df95d26f7575a31aa15615b26a1ac829b357a557e52d35e60c5.
Canonical Docker toolchain on the canonical build machine, zero
warnings. +1,452 bytes vs v1.3.0, spent deliberately on operator
clarity and signal-path robustness.

### Added (feedback: the radio now tells you what it is doing)

- **Menu hint line** (U3, ui/vuurwerk_menu.c, ui/menu.c, ui/helper.c,
  +1,200). Forty cryptic menu items show a plain-language description
  in the 3x5 mini font on the menu's bottom line while highlighted:
  RxMode reads DUAL WATCH / CROSSBAND RX, TxODir reads REPEATER
  OFFSET DIRECTION +/-, ScnRev reads SCAN RESUME MODE AFTER SIGNAL,
  and so on. Items with self-explaining names keep the category/
  position line; submenus are unchanged. Nothing renamed, nothing
  moved. The mini-font renderer was relocated verbatim from
  app/spectrum.c to ui/helper.c and is shared by both call sites.
- **VFO Split armed indicator** (U1, ui/status.c, +36). "SP" renders
  in the status bar's permanently-free slot the whole time split is
  armed. Directly answers the v1.3.1 field report: VFO B walking the
  channel list by itself no longer looks like a scanning bug.
- **VFO Split activity alert made real** (U2, vfo_split.c, net -172
  after LTO re-layout). gVfoSplit.alert was computed and displayed
  nowhere; the feature's entire output was invisible. Activity on a
  walked channel now raises an "ACT <frequency>" toast and a single
  880 Hz / 200 ms cue. The cue is deliberately in the
  BEEP_CONTROL-exempt class (like the low-battery warning): arming
  split is opting into an audible alert.
- **Battery sag warning** (U6, battery_sag.c, +68). The per-TX sag
  value (computed since v1.2.7, shown only on the About screen) now
  raises a "SAG 0.NNV" toast after unkey when it reaches 0.20 V, the
  documented "tired pack" threshold.

### Improved (excellence: existing functions, closer to the silicon)

- **Smart squelch parked voice pre-bias** (T1, smart_squelch.c,
  +304). The module previously ran only after the hardware squelch
  had opened, so it could keep a weak voice open but never help it
  open. It now also evaluates RSSI/noise/glitch at 50 ms cadence
  while the radio is parked and, when the voice score is high and
  the channel noise floor has been learned, biases the REG_78 open
  threshold down by at most 2 steps (~1 dB), with falling-edge
  baseline restore in every exit path. Hard-gated off during any
  scan (gScanStateDir, SCANNER_IsScanning, gCssBackgroundScan),
  while VFO Split is armed, and outside FUNCTION_FOREGROUND; the
  gates are in one verifiable block and the scan stop fence is
  untouched. The dead +2/+4 "tighten" branch (clamped to zero at its
  only call site since v1.1.0) is removed.
- **Squelch tail elimination covers DCS** (T4, squelch_tail.c, +52).
  The STE state machine armed only for CTCSS; DCS channels got the
  full tail burst. It now arms for digital codes too, reading the
  stock interrupt-maintained g_CDCSS_Lost flag (no new register
  polling), with a 250 ms DCS mute window matched to the ~180 ms
  DCS turn-off code. Same TONE > STE switch, OFF bypasses both.
- **Dual watch reacts ~4x faster** (T6, dual_watch_mgmt.c, -16).
  Base dwell restored from 500 ms to a stock-class 150 ms (+/-50
  activity bias, was +/-100 on 500). Worst-case detection latency
  drops from ~0.6 s to ~0.2 s: the first word of a call on the
  other VFO is no longer routinely lost. Slightly higher idle drain
  while dual watch is armed; power save behavior unchanged.
- **Spectrum sweep REG_30 cache** (T2, app/spectrum.c, +36). SetF
  re-read REG_30 on every sweep step only to pulse it; the value
  cannot change mid-sweep. One read per RX-toggle now, dirty-marked
  at every REG_30 writer. Register traffic semantics byte-identical;
  measurable sweep-rate gain on the order of 10%.

### Refined (core leanness, measured)

- **BK4819_UpdateRegisterBits read-modify-write helper** (C1,
  driver/bk4819.c/h, app/spectrum.c, -56 measured). Five driver
  RMW sequences (scramble on/off, compander both branches, VOX
  disable, REG_7E DC filter) and both spectrum AF toggles collapse
  onto one out-of-line helper.
- **Linker stack reservation honest** (C2, firmware.ld, 0 flash).
  _Min_Stack_Size 0x80 -> 0x400. Link-time guarantee only -- the
  stack always grew into free RAM at runtime -- but the old value
  made the link check weaker than the measured ~350-byte worst-case
  call depth. The link now fails loudly if static RAM ever grows
  into the stack margin.

### Confirmed and pending (honesty section)

- **TX compressor bench-measured; audit defect claim corrected**
  (T3). The v1.3.0 audit hypothesis -- threshold 18 sitting far
  below speech envelope levels, pegging gain reduction on all
  speech -- was carried forward as "structurally
  confirmed" via an audio-bar scale inference. Bench captures with
  the diagnostic build refute it: normal speech at 30 cm measures
  env 3-16 with syllable peaks to 30 (mostly below threshold 18;
  gain at maximum essentially throughout), loud 10 cm speech
  measures 36-134 with gain stepping 28 -> 26 -> 19. Decoded
  against source, measured behavior is: transparent at
  conversational level, progressive ~2:1-to-3:1 compression above
  threshold, designed 15-step (~7.5 dB) ceiling from env ~65. The
  observed G19 floor is base mic gain 31 minus the 15-step cap
  plus makeup 3 -- by design, not a defect. Verdict: the constants
  fit the measured levels; none ship changed.
- **U5 withdrawn**: the audit claimed locked-keypad digit presses
  fail silently; code review showed they already double-beep with
  the 2 s unlock popup (app/app.c:1804-1841). No fix shipped for a
  bug that does not exist.

### Documentation

- USER_MANUAL: hint line section with mockup, SP/ACT/SAG indicator
  documentation, DCS STE, dual-watch cadence, parked pre-bias, and
  an honest compressor status note. FEATURES: v1.3.1 budget row and
  v1.3.0 canonical numbers backfilled.

## v1.3.0 (June 2026)

LAW 1B fork declaration, the full sixteen-finding audit fix sweep, and a
reclamation pass that leaves v1.3.0 roughly 900 bytes SMALLER than v1.2.8
with every fix included. Every edit to an egzumer-origin file is
marker-tagged in source.

**Binary:** 59,484 / 61,440 bytes (.bin, 1,956 free); packed
59,502. SHA-256: .bin
e1d70aeb3da309a8ed2fa70ebc07220b0679dfd2196b7590bbcfad88e454c701,
packed
610f49530fb69de69cacd26974b1f94deae16eeb2b927ff221bdec54dee2c608. Local
reference toolchain (GCC 13.2.1, runs ~572 bytes heavier than the
canonical Docker image): 60,476 bytes, 964 free, zero warnings.

### LAW 1B

The old LAW 1 (stock egzumer files untouched) is repealed. VUURWERK is
now a true fork: stock files may be edited when it fixes a bug or
measurably improves size, speed, or robustness. Conditions: every edit
carries a `// VUURWERK:` marker, GPL headers and upstream
attribution are never altered, and diffs stay minimal.

### Fixed (VUURWERK-original code)

- **0.4 s deaf dip after every parked FM channel change or VFO flip**
  (F1, `gain_staging.c`). Index 0 of the shared gain table is the stock
  -7 dB reference, not the bottom rung; the attack and release paths
  treated it as the bottom, so the first movement off stock gain landed
  on `gain_table[1]` = -93 dB and climbed back one rung per 10 ms tick.
  Both paths now enter the ladder at rung 39 (-6 dB), guarded by a
  compile-time assert on the table layout. Also fixes the half-second
  signal dropout when tuning onto a -75 to -65 dBm signal.
- **Dual-watch deafness for ~1-2 s after traffic on the other VFO**
  (F2, `gain_staging.c`). REG_13 is one physical register but the
  write gate is per-VFO, and a dual-watch flip never rewrites it
  (FM-to-FM AGC cache skip). A once-per-second heartbeat invalidates
  the write gate so the active VFO's gain is re-asserted. Also covers
  gain edits leaking out of the spectrum analyzer's register editor.
- **AM audio chopped ~5x per second on channels carrying a stray CTCSS
  code with STE on** (F4, `squelch_tail.c`). STE armed off the raw
  channel record; it now arms off `gCurrentCodeType`, which stock
  logic already forces to OFF for non-FM.
- **F+9 calling-frequency jump hit the wrong VFO under dual watch or
  crossband, and the restore was not VFO-tagged** (F5, `app/main.c`).
  Jump and restore now operate on the operator's VFO and the restore
  returns to the VFO it saved from.
- **Squelch stayed ~3 dB easier to open after a voice transmission
  ended** (F7, `smart_squelch.c`). The adaptive loosening written to
  REG_78 during voice RX is now restored to the channel baseline on
  the falling edge of reception.
- **Signal classifier letter (F/N/S/~) carried the previous channel's
  verdict across hops** (F8, `signal_classifier.c`). Per-VFO state now
  resets on frequency change, matching the v1.2.7 resets in
  signal_quality and smart_squelch.
- **CSS scanner NO SIGNAL abort leaked the previous session's save
  prompt and result** (F9, `app/scanner.c`). The abort now resets
  `gScannerSaveState` and the stale CSS result.
- **VFO Split never actually walked the memory list** (F3a,
  `vfo_split.c`). It validated a walking channel index but always
  reloaded VFO B's parked channel, re-checking one frequency forever.
  The walked channel is now committed to VFO B before the reload, so
  SPLIT genuinely patrols every valid memory channel and VFO B's
  display shows the channel being checked.
- **Power save and VFO Split fought each other** (F3b, `app/app.c`).
  Power-save entry mid-hop bounced straight back to FOREGROUND and
  left the battery-save countdown dead at zero (power save never
  re-engaged until a keypress); entry while idle silently stopped the
  hop machine. Power save is now blocked while SPLIT is armed, the
  same policy as an active scan.
- **Scan+Watch was one-shot and corrupted power-save behaviour when
  armed but idle** (F10, `scanwatch.c`, `app/menu.c`,
  `app/chFrScanner.c`, `app/app.c`). The ScnWch setting is now
  persistent: it arms on every scan start and survives scan stop. The
  power-save wakeup condition returns to stock, ending the
  VFO coin-flipping on wakeups with dual watch off.
- **Unterminated gDTMF_String initializer** (F11, `dtmf.c`). A
  15-character literal exactly filled char[15] and legally dropped the
  NUL; only a boot-time memset repaired it. Now 14 dashes, terminated.

### Fixed (formerly-stock egzumer code, under LAW 1B)

- **Spectrum analyzer exit left the receiver tuned half a sweep span
  below the displayed frequency** (F6, `app/spectrum.c`,
  stock-inherited, verified byte-identical upstream). The radio sat
  deaf-on-displayed-frequency until an unrelated event reconfigured
  it. Exit now retunes to the VFO frequency.
- **RADIO_SetupAGC encoded both of its flags into the same bit**
  (S1, `radio.c`). `(listeningAM << 1) | (disable << 1)` aliased
  (AM, false) with (FM, true), so the spectrum gain editor's AGC-lock
  request was swallowed by the change cache in AM mode. Each flag now
  has its own bit. The audit also proved the old collision was
  accidentally load-bearing: it suppressed the `BK4819_InitAGC` table
  re-init that would otherwise clobber a locked manual gain on every
  spectrum listen/scan transition, so both InitAGC calls are now
  guarded with `!disable`. All disable=false paths (scan, dual watch,
  mode changes) are byte-identical in behaviour before and after.
- **AM gain manager dived to -93 dB once after boot** (S2,
  `am_fix.c`). Same index-0 ladder defect as F1, same virtual-rung
  fix, in the stock AM path.
- **Starting an MR scan on an empty scan list showed SCANNING while
  walking nothing** (S4, `app/chFrScanner.c`). Stock fell into a
  quasi-no-op loop through the invalid-channel fallback. The scan now
  refuses cleanly with an EMPTY LIST toast and stays parked.
- **TX status line showed the wrong power letter under crossband**
  (S5, `ui/main.c`). It read gRxVfo, which crossband inverts; it now
  reads the transmitting VFO.

### Improved (approved reclamation pass)

- **A1: unreachable CSS frequency-hunt mode removed, -440 bytes**
  (`app/scanner.c/.h`, `ui/scanner.c`). Every scanner launch path
  (F+STAR, menu R-CTCS/R-DCS, STAR restart) is single-frequency; the
  hunt mode, its tick state, its save-path branches and the
  BK4819 frequency-scan driver calls are gone. The tone scan the
  operator uses is behaviourally identical.
- **A2: VFO Split dead scan sources removed, -96 bytes**
  (`vfo_split.c/.h`). The FREQ_RANGE and LIST sources had no setter
  anywhere; their enum, branches, and range fields are gone.
- **A3: squelch.c deleted** (0 bytes; LTO had already stripped the
  dead `gAdaptiveSquelch` -- source hygiene only).
- **A4: bold small font dropped (ENABLE_SMALL_BOLD=0), -640 bytes.**
  Bold UI text now renders in the regular small font; this is the
  only operator-visible cost in the release.

### Known issues and fences

- The MR scan stop-on-signal report remains under investigation. The
  stop-decision path is deliberately untouched in this release.
- CHIRP driver unchanged (mjwgeek's work).

## v1.2.8 (May 2026)

Scanner deafness fix (front-end gain staging), Scan+Watch and VFO/MR
fixes, and a community-contributed CHIRP driver.

**Binary:** 60,812 / 61,440 bytes (.bin, 628 free); packed 60,830 bytes.
SHA-256: `.bin` 02cfb30792843504f905ea740f8ac7a73e291a85a04f92ef2c75bcbb40c5dd11,
`packed` 0c5c795978c5746411e2836f6d68f32c48fbaa8d4acd8fb4236f427030ee2cb7.

### Fixed (scanner deafness)

- **Receiver went deaf during scan** even though sitting on a
  frequency received normally (reported by Scott, KC7WDG). Root
  cause: the VUURWERK FM gain-staging module (`gain_staging.c`, a
  software front-end AGC over BK4819 `REG_13`) ran during scan with
  no scan guard. On every channel hop it reset to the stock gain slot
  and, one 10 ms tick later, the release path's off-by-one wrote
  `gain_table[1]` = 0x0000 = -93 dB (minimum gain) into the auto-AGC
  max-gain slot, then climbed back only ~2-3 dB per tick. With a
  ~90 ms dwell the front end stayed gain-starved for most of each
  channel's evaluation window, so the post-gain RSSI never crossed
  the squelch threshold and weak signals were walked past. Parked,
  the gain ramped up and the 6 dB hysteresis hold kept it there, so
  static RX was unaffected. Fix: `GAIN_STAGING_10ms` now yields the
  front end to the BK4819 hardware AGC while a scan is active
  (`gScanStateDir != SCAN_OFF`), restoring the stock `REG_13`
  baseline once and not running the adaptive ramp -- matching stock
  egzumer scan behaviour. Covers both the frequency-sweep and
  memory-channel scan paths. +40 bytes.
- **Scan+Watch went deaf on returning to the scan VFO.** When
  Scan+Watch finished dwelling on the watch VFO and returned to the
  scan VFO, `app/app.c` called `RADIO_SetupRegisters(false)`, which
  mutes AF but leaves `gCurrentFunction` in `RECEIVE` so AF was never
  re-enabled. Now uses `RADIO_SetupRegisters(true)` to resume a clean
  FOREGROUND scan (matching `CHFRSCANNER_Stop`); the next found
  signal unmutes via `APP_StartListening`. Byte-neutral.

### Fixed (VFO/MR toggle)

- **F+3 showed a false "NO CHANNELS" on every successful toggle.**
  The v1.2.5 guard compared `gTxVfo->CHANNEL_SAVE` before and after
  `COMMON_SwitchVFOMode`, but that field is reloaded later (deferred
  `VFO_CONFIGURE_RELOAD`), so the pre/post compare always matched and
  fired the empty-memory message even on a real toggle. Now reads the
  synchronously-updated `gEeprom.ScreenChannel[gEeprom.TX_VFO]`, so
  "MR MODE" / "VFO MODE" shows on success and "NO CHANNELS" only on a
  genuine empty-memory no-op. Byte-neutral.

### Added (community CHIRP driver)

- **CHIRP driver** (`chirp/uvk5_vuurwerk.py`) for reading and writing
  channels, side-keys, and settings from CHIRP without flashing back
  to stock egzumer first. It recognises VUURWERK's honest firmware
  identification string (`VUURWERK` / `EGZUMER `), so no firmware
  change is required; it reads and writes channels, programmable
  side-keys, and the settings block, but not the spectrum analyzer
  settings. Contributed by mjwgeek (github.com/mjwgeek), adapted from
  the EGZUMER driver which is based on Jacek Lipkowski SQ5BPF's
  original UV-K5 driver. GPL (the file carries its own header). Zero
  firmware flash (host-side Python). See `chirp/README.md`.

## v1.2.7 (May 2026)

SIDE2 keymap symmetry fix, CSS scanner robustness, and economic
re-engineering.

### Fixed (v1.2.6 keymap correction)

- **SIDE2 long-press incorrectly toggled backlight** instead of
  continuing freq-down auto-repeat. The "clean short / long
  separation" model introduced in v1.2.6 was operationally wrong:
  operators expect SIDE2 hold to mirror SIDE1 hold (freq UP and freq
  DOWN should be symmetric). SIDE2 is now press-dispatched with
  auto-repeat, symmetric with SIDE1. ToggleBacklight is no longer
  accessible from spectrum view; operators wanting to toggle the
  backlight while in spectrum should press EXIT first, toggle from
  the main screen, and re-enter via F+5. This is a deliberate UX
  simplification, not a feature loss.

### Fixed (CSS scanner robustness)

- **Pre-flight RSSI gate** added to the CSS scanner. Before launching
  the BK4819 scan engine, `SCANNER_Start(true)` samples RSSI on the
  current frequency; if it reads below carrier-present threshold
  (about -110 dBm), the scan aborts with a `NO SIGNAL` toast and a
  500 Hz double-beep. Today's behaviour on dead air was to run the
  scanner for 60 seconds and surface SCAN FAIL; new behaviour is
  instant feedback that the operator parked on a quiet frequency.
- **CTCSS confirmation logic** changed from two-consecutive-matches
  to single-match, matching the existing DCS behaviour. CTCSS scans
  no longer feel broken next to DCS scans. The BK4819 detector's
  internal voting plus the new pre-flight RSSI gate together provide
  the confidence the second confirmation used to add.
- **Dwell time reduced** from 210 ms to 120 ms (`scan_delay_10ms` in
  misc.c, 21 ticks to 12 ticks). 120 ms is enough for the BK4819
  CTCSS detector to converge on the lowest tone (67 Hz, ~8 cycles)
  with margin. Affects both RF frequency scan and CSS scan.
- **Soft-timeout tightened** from 60 seconds to 30 seconds. With
  faster dwell, single-match CTCSS, and the pre-flight gate, typical
  successful scans complete in 3-10 seconds; 30 seconds is the new
  "this is fruitless" cap.
- **FOUND validation considered but not implemented.** The BK4819
  detector's internal voting plus the pre-flight RSSI gate plus
  sufficient dwell already provide high confidence on FOUND; post-
  validation cost (extra ~500 ms per scan, ~50-100 bytes flash) was
  unfavourable.

### Changed (Economic re-engineering)

- **`TX_COMPRESSOR_GetGainReduction` removed.** Function had zero
  callers across the tree (verified via grep). Dead code.
- **`RSSI_HISTOGRAM_Init` removed.** Function only zeroed the
  histogram global; the C runtime zero-clears BSS at boot, making
  the Init redundant. The main.c call site removed too.
- **`GAIN_STAGING_Reset` made `static`** to `gain_staging.c`. Zero
  external callers; LTO can now fully inline into the single
  caller. Header prototype removed.
- **`bandscope.c` memmove pair consolidated.** Two consecutive
  memmoves on different buffers replaced with one combined loop.
- **`SIGNAL_CLASSIFIER_Init` removed.** Function only set
  `prev_rssi = -127` (the rest of the struct was BSS-zero-default).
  Now relies on BSS zero-init; the worst case is one extra NOISE
  classification on the first signal post-boot before the algorithm
  self-corrects to FAST / NORMAL / SLOW. The main.c call site
  removed too.
- **CSS scanner CDCSS/CTCSS arms merged** in
  `SCANNER_TimeSlice10ms`. Once CTCSS dropped to single-match (see
  Fixed section above), the two arms became structurally identical
  and collapse into a single result-handling block, parameterised
  only by which DCS table to query and which CodeType to assign.

### Changed (License header enforcement sweep)

- Every VUURWERK-original source module now carries the canonical
  dual-license header including the explicit commercial-licensing
  contact line. 51 files normalised in one sweep: 22 had the v1.2.6
  canonical header (missing the contact line) and were updated;
  another 8 had non-canonical or SPDX-only headers and were rewritten
  with the canonical block (preserving any technical descriptive
  content as a separate comment immediately after); `battery_sag.c`
  needed manual layout adjustment.
- **LICENSE Dual-Licensing Notice section clarified** to make the
  complete-firmware-GPL-v3 / extracted-module-commercial boundary
  unmistakable. The notice now explicitly enumerates what "outside
  the complete VUURWERK firmware" means (extraction, porting,
  repackaging, commercial use) and confirms that marker-block
  contributions in upstream-derived files (app/app.c, app/main.c,
  app/spectrum.c, ui/main.c, ui/status.c, ui/welcome.c, main.c,
  misc.c) are protected by the LICENSE contributions list. The
  contributions list above the Dual-Licensing Notice delimiter is
  byte-identical (LAW 7).
- **README License section** added near the top with the same
  boundary expressed in operator-facing language.
- **Zero flash impact** -- all header sweep changes are comments,
  stripped by the preprocessor before codegen.

### Released

- `vuurwerk-v1.2.7.packed.bin`: 60,790 bytes, SHA-256:
  1e1486c27baefce73d817dec45053914a7a128b8f8af92b01c8760bf270740b0
- Shipped to `release/` on the dev repository.

---

## v1.2.6 (May 2026)

Spectrum analyzer keymap reorganization, voice-hop robustness, and
visual polish.

### Fixed

- **Voice-hop now respects the blacklist.** The spectrum VOX mode's
  UP/DOWN voice-hop loop did not check `IsBlacklisted(idx)` before
  landing on a voice-probable bin, so a blacklisted bin with strong
  voice probability would still be tuned. With Blacklist now moving to
  the more discoverable MENU short-press in spectrum view, the
  blacklist is expected to see more use; the missing check is fixed.
- **Spectrum analyzer VOX mode keymap reorganization.** UP/DOWN in VOX
  mode previously consumed the voice-hop path and never reached the
  frequency-step path, leaving operators with no way to manually step
  frequency while in VOX. Resolved by moving frequency stepping to
  SIDE1 (up) and SIDE2 (down) as universal gestures across all spectrum
  modes, so UP/DOWN remains the voice-hop in VOX and the frequency-step
  in NORM/PEAK/MTI as before.

### Changed

- **Spectrum view keymap reorganized.** Operators upgrading from v1.2.5
  will need to relearn Blacklist and Backlight access in spectrum view.

  | Key | Before (v1.2.5) | After (v1.2.6) |
  |---|---|---|
  | SIDE1 short | Blacklist current bin | Frequency UP (one step) |
  | SIDE2 short | Toggle backlight | Frequency DOWN (one step) |
  | SIDE2 long  | (none) | Toggle backlight |
  | MENU short  | (none) | Blacklist current bin |
  | UP / DOWN   | Freq step (NORM/PEAK/MTI), voice-hop (VOX) | Unchanged |

  SIDE1 short is now press-dispatched with auto-repeat: tap for one
  step, hold for continuous fast tuning up.

  SIDE2 short is now release-dispatched (one step per tap, no
  auto-repeat). SIDE2 long fires once at the 640 ms hold threshold and
  suppresses the release-side short action, so short and long are
  unambiguously separated.

  MENU short is press-dispatched (one-shot).
- **Spectrum mode-change feedback.** STAR now flashes a brief
  centered `MODE <name>` label for ~6 renders on mode change, in
  addition to the permanent small label in the top-right corner.
  Closes the "operators only see NORM" issue where the permanent label
  was too small to register at a glance.
- **VOX bar drawing tier matches hop threshold.** Bins with voice
  probability below 50 now drop to a floor dot (previously 35); bins
  at 50 or above draw as bars. Visible bars now equal hop-eligible
  bins, so the visual matches the navigation behaviour.
- **Voice-probable marker tick in VOX mode.** Every bin with
  `voiceProb >= 50` now gets a 1-pixel marker at the top of the
  spectrum display in addition to its bar, so hop targets are visible
  at a glance even among similarly-tall bars.

### Released

- `vuurwerk-v1.2.6.packed.bin`: 61,002 bytes, SHA-256:
  7a14b55039f2a9de6f22faf858f3c64fcc7d2cf372309d42208f47a375ded884
- Shipped to `release/` on the dev repository.

---

## v1.2.5 (May 2026)

Feature release adding diagnostic, reliability, and UX improvements across
the boot path, battery telemetry, backlight behaviour, scanning, and the
voice-seeking spectrum mode.

### Added

- **Side-Button Toast Feedback** (Feature #28). 1-second on-screen toast
  confirms SIDE1 short, SIDE1 long, SIDE2 short, SIDE2 long, and MENU
  long actions, so every shortcut path -- F+keys, side-buttons, long
  presses -- shares the same visible confirmation contract. Closes the
  UX-parity gap inherited from upstream where side-button actions were
  silent.
- **Toast Notification Subsystem** (Feature #29). Extracted to a dedicated
  toast.c/h module. Powers all F+key shortcuts, side_toast, and any
  future shortcut surface. The extraction restored LAW 1 byte-identity
  on app/app.h and saved 204 bytes via LTO re-inlining gains.
- **Scan Rate Telemetry** (Feature #30). Status line now shows live
  channels-per-second readout during scan ("SCANNING NNc/s"). 100-tick
  rolling window. Operators can see at a glance whether squelch is
  robbing scan throughput or the band is just empty.
- **F-Key UX Hardening** (Feature #31). F-hold now shows a LOCKED toast.
  F+3 emits "NO CHANNELS" when the channel list is empty (was a silent
  no-op). F+\* is gated to FM-mode only (was a silent failure in AM/SSB).
  STAR long-hold now shows a SCAN ON / SCAN OFF toast.
- **Flashlight Auto-Off Watchdog** (Feature #32). Auto-extinguishes the
  flashlight after 30 minutes of continuous ON or BLINK, or 10 minutes
  on a low-battery pack. SOS is preserved indefinitely. Prevents the
  belt-clip / pocket bump from running the pack dead overnight.
- **CSS Scan Soft-Timeout Watchdog** (Feature #33). 60-second cap on CSS
  scans with an audible double-beep on timeout. Both the F+\* and menu
  scan paths are covered. Prevents the unbounded-scan trap inherent to
  FM-with-no-tones.
- **Audio Palette VOX-Hop Cues** (Feature #34). Spectrum VOX mode plays
  a short ascending two-tone chord (800 Hz + 1200 Hz) when UP/DOWN
  finds a voice-probable bin, and a soft 500 Hz double-beep when it
  sweeps the range without finding voice. Tied to the
  Intelligence-Based Squelch voice-probability scorer.
- **Boot-Time Hardware Health Probe** (Feature #35). At every power-on,
  the firmware checks BK4819 SPI liveness and the EEPROM I2C
  battery-calibration page. Faults are surfaced as a "BK4819 FAULT /
  RX/TX disabled" or "EEPROM FAULT / calib lost" welcome banner. First
  UV-K5 firmware to boot with a hardware self-test.
- **Live Battery Voltage During TX** (Feature #36). Battery icon and
  voltage display update live during transmit. Stock firmware froze the
  reading on PTT entry. Operators on long key-down (contests, repeater
  nets) now see real-time sag instead of a stale value.
- **Backlight Fade-Out Tail** (Feature #37). 2-second linear taper to
  off at the end of the idle window, instead of cutting to black
  abruptly.
- **Backlight TX/RX Activity Refresh** (Feature #38). Re-arms the
  backlight countdown during TRANSMIT, RECEIVE, INCOMING, and MONITOR
  when the per-mode `BltTRX` bit is set, so the screen never goes dark
  mid-conversation.
- **TX Battery Sag Delta Tracker** (Feature #39). Per-PTT peak voltage
  sag is captured by comparing the clean-idle baseline against the
  during-TX minimum and is displayed on the About screen as
  "TX sag NNNNmV". Trends across sessions help operators tell a
  healthy pack from a tired one.
- **CSS Scan Status-Bar Glyph** (Feature #40). CSS scans now show "Cs"
  in the status bar instead of the generic "S" used for channel scans.
  Makes CSS-scan flavour unambiguous at a glance.
- **CSS Scan FOUND Beep** (Feature #41). Audible 1 kHz single-tone cue
  when a CSS scan locks onto a tone, paired with Feature #33's FAILED
  double-beep for symmetric audible feedback.
- **Quiet Backlight PWM** (Feature #42). PWM carrier raised from 1 kHz
  to ~6.7 kHz at boot, above the radio's 3 kHz audio passband. Removes
  the 1 kHz whine that stock firmware bleeds into outbound audio when
  the screen is dimmed during TX. Mission-aligned with the existing TX
  intelligence (Soft Start, CTCSS Lead-In, Compressor). Credit:
  kamilsss655 / NUNU (2024) bench-validated and fixed this approach on
  another UV-K5 fork; VUURWERK matches that TX-audio quality result
  via an independent mask-then-OR implementation in main.c.

### Changed

- **Intelligent Dual-Watch** rewritten to weight dwell decisions by
  cumulative RX duration rather than per-tick boolean activity. A 30 s
  QSO now weights 30x more in the dwell cascade than a 1 s carrier
  blip.
- **Signal Quality Metrics** variance ring is now reset on channel or
  VFO change. Closes the stale-variance reading that previously persisted
  for ~8 samples across a VFO flip.
- **Squelch Tail Elimination** CTCSS-loss confirmation is now
  confidence-weighted. Stable signals confirm tail-loss at 20 ms; the
  EWMA-flagged fluttery signals confirm at 30 ms. Reduces false squelch
  closures on marginal signals.

### Removed

- **Activity Log** (Feature #27). The 20-entry ring buffer and uptime
  counter are retired. The 128x64 LCD could not show enough log context
  to be operationally useful in HT form factor. The #27 slot is
  intentionally empty going forward.

### Released

- `vuurwerk-v1.2.5.packed.bin`: 60,672 bytes, SHA-256:
  2695c154a9277fd0393c514f5e969650741facbda2b2031c4e91a6520a09ee72
- Shipped to `release/` on the dev repository.

---

## v1.2.4 (April 2026)

Bugfix release addressing three community-reported issues.

### Fixed

- **Spectrum analyzer freeze on entry.** The `scanStepBWRegValues[]` array was missing three entries for the larger scan steps (25 kHz, 50 kHz, 100 kHz), causing an out-of-bounds read on entry with the default scan step. Combined with a no-timeout wait loop in `GetRssi()`, this hung the radio requiring a battery pull. Fixed the array size, added a 50-iteration timeout to `GetRssi()`, and corrected the return type of `GetBWRegValueForScan()` from uint8_t to uint16_t (a latent bug that was silently truncating filter bandwidth register values). Thanks to JoseSoler (https://github.com/JoseSoler) for the detailed root-cause analysis and exact code fix in issue #5.

- **Channel name drift in Dual Watch.** After 4+ hours of Dual Watch operation, the displayed channel name would switch to Channel 10's name on both VFOs, while RX continued correctly on the saved channels. A defensive guard has been added to the display path with a trip counter visible on the About screen. Root cause investigation continues and user reports of trip counts will inform the v1.2.5 fix. Thanks to rieschrispy (https://github.com/rieschrispy) for the detailed reproduction steps in issue #4.

- **About screen now accessible from the CONFIG menu.** Previously About was hidden behind a long-press gesture in the category picker that wasn't discoverable. About is now a regular menu item under CONFIG: scroll to it and press MENU. Thanks to ErikS-web (https://github.com/ErikS-web) for the feedback in issue #1.

---

## v1.2.3 (2026-02-21)

### Fixes
- Fixed: Activity Log now records RF activity (frequency, RSSI, tone, duration) on every squelch open
- Fixed: Intelligent Dual-Watch dwell times now adapt based on VFO activity
- Fixed: VFO Split now accessible via SCANWATCH menu (option 3: SPLIT)
- Fixed: Signal Classifier F/N/S/~ symbol now displayed on status line during RX
- Fixed: FEATURES.md LTO-stripped list corrected (removed 3 false entries for working functions)
- Updated: LICENSE attribution expanded with all VUURWERK-original contributions

### Optimizations
- Removed snprintf from status_line.c (eliminated stdio.h linkage)
- Replaced modulo operators with comparison+reset (avoided software division library)
- Simplified activity_log Init to memset
- Reused SCANWATCH menu for VFO Split (zero new menu item overhead)

### Binary
- Text: 59,164 bytes (2,276 free, 3.71% headroom)
- vuurwerk-v1.2.3.packed.bin

---

## v1.2.2 (2026-02-20)

### Changes
- Code optimization pass
- Text size: 61,084 bytes (356 free)

---

## v1.2.1 (2026-02-16)

### Bugfix Release
- **Voice-hop fix:** `ToggleRX(true)` instead of `SetState(STILL)`, stays in SPECTRUM state
- **Voice gate:** auto-listen skips non-voice bins (voiceProb<50), `UpdateListening()` checks noise register
- **Mode label:** "VOIC" renamed to "VOX " (clearer on 3x5 font)
- **DrawSpectrum:** cases 0/1 merged (saved 40 bytes)
- **Dead code:** removed sprintf("%s",name), idx==5 guard, redundant bin<128 checks
- **VOIC drawing:** 2-tier confidence (was 3-tier), removed h<2 guard

Text size: 61,192 bytes (44 free, -36 from v1.2.0)

---

## v1.2.0 (2026-02-16)

### Voice-Seeking Spectrum
- **VOX mode** (replaces waterfall): voice probability per spectrum bin via REG_0x65 noise indicator
- `ComputeVoiceProb()` in spectrum.c: noise + RSSI scoring, integer-only, max 95
- VOX drawing: bars scaled by confidence (half/<75, full/>=75), floor dots for non-voice
- **Voice-hop:** UP/DOWN jumps to next bin with VP>=50, uses ToggleRX(true)+TuneToPeak()
- **STE menu toggle wired:** squelch_tail.c checks gEeprom.TAIL_TONE_ELIMINATION
- Smart squelch always-on (no separate toggle; flash too tight for MENU_SQIN)
- Dead API removed: GetVoiceProb(), QuickVoiceProb() from smart_squelch.c/h
- **Waterfall removed:** saved 200 bytes text + 896 bytes BSS

Text size: 61,228 bytes (212 free)

---

## v1.1.0 (2026-02-15)

### Squelch Tail Elimination + Intelligence-Based Squelch
- **squelch_tail.c:** 4-state machine (IDLE/MONITORING/TONE_LOST/MUTED)
  - Reads REG_0C (CTCSS status), writes REG_47 (AF mute)
  - 20ms confirmation, 150ms timeout, handles all STE methods
- **smart_squelch.c:** 3-register hardware VAD (REG_67/65/63), EWMA smoothing, dynamic squelch
  - Voice probability 0-100 from noise, glitch, and SNR scoring
  - Dynamic REG_78 adjustment: -6 to +4 steps based on VP

Text size: 61,040 bytes (400 free)

---

## v1.0.9.7 (2026-02-15)

### Dead Code Cleanup
- Removed orphaned files and modules, archived tx_power_mgmt/battery_model
- Text: 60,452 bytes, BSS dropped 896 bytes

---

## v1.0.9.6 (2026-02-15)

### Gain Staging Rewrite + S-Meter
- gain_staging.c rewritten: FOREGROUND+RX guard, frequency change detect, S-meter compensation
- RSSI bar enabled (ENABLE_RSSI_BAR=1)
- Text: 60,500 bytes

---

## v1.0.9.5 (2026-02-15)

### TX Chain Integration
- TX soft start, CTCSS lead, TX compressor fully wired to app.c and functions.c
- TX_RAMP_STEPS fix (was 3, now 6 for proper 60ms S-curve)
- Removed dead modules from Makefile: alc_learning, preemph_opt, tx_power_mgmt
- Text: ~60,150 bytes

---

## v1.0.9.4 (2026-02-14)

### Major Cleanup
- Dead code removed across all modules
- signal_classifier fix: proper rise-time measurement
- gain_staging rewrite began
- Text: ~59,200 bytes

---

## v1.0.9 (2026-02-14)

### Categorized Menu System
- 6 visible categories + hidden UNLOCK
- 63 menu items organized by function
- Number key category selection

---

## v1.0.8 (2026-02-14)

### Intermediate Bug Fixes
- TX compressor, CTCSS lead, tx_soft_start module creation
- VFO split, bandscope modules created

---

## v1.0.4 (2026-02-14)

### Feature Integration
- Spectrum modes (NORM/PEAK/MTI) wired
- Scan+Watch, bandscope, RX signal chain fully integrated
- Signal classifier, RSSI histogram, dual-watch mgmt, status line added
- About screen, activity log

---

## v1.0.0 (2026-02-13)

### Initial Release

**Based on**: Egzumer 2.0 Firmware

- 22 modules created (TX chain, squelch, scanning, spectrum, UI)
- Most integration pending. Modules compiled but not all wired
- DTMF calling removed for space (~12KB saved)
- Text: ~57,800 bytes

### Credits
- **Base firmware**: Egzumer 2.0
- **Original project**: DualTachyon UV-K5 Firmware
- **Firmware design and architecture**: James Honiball (KC3TFZ)
