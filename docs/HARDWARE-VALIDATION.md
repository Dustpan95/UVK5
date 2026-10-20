# Physical validation — outstanding

No radio has been connected or flashed by the cloud tools. The owner reports
that revision 2 was locally flashed and boots; home display, controls, saved
channels and voice receive checks pass. A clear APRS beacon burst is audible,
but no packet decodes despite enabling APRS. MCU identity, voice TX, data TX,
RF timing/deviation and recovery remain unverified. The temporary
[D2 capture export](../release/rx-diagnostics-d2/README.md) collects
receiver/modem evidence. D1 photos show eligible/armed state, full 240-byte
captures, 15 flags, zero complete frame candidates and zero valid checksums.
This narrows the failure to capture/framing; D2 exports the full bitstreams.
The following checklist is a validation plan; do not
interpret every item as completed.

1. Identify the MCU as **DP32G030** using reliable board/revision evidence.
   Do not infer it solely from a UV-K6 label or Egzumer v0.21 version string.
   This build is not for replacement MCU revisions.
2. Read and preserve the current EEPROM/calibration/channel data and original
   firmware with a known compatible method. Confirm recovery access before
   deciding to install an experimental image. APRS introduces no new EEPROM
   mapping, but cross-firmware compatibility has not been physically verified.
3. After owner-controlled installation, verify boot, keypad, LCD, memory/VFO
   tuning, normal FM receive and voice TX into a suitable dummy load.
4. Feed known Bell 202 / AX.25 test signals through suitable RF test equipment.
   Check real stations, uncompressed/compressed/Mic-E positions, SSIDs, addressed
   messages (including 67-character text), corrupt frames and repeated packets.
   Compare the displayed result with an independent decoder. Measure sensitivity,
   frequency offset tolerance, missed packets and capture/rearm behavior.
5. Test APRS enable/disable, F+5/RxLog, history navigation, PTT press/release,
   beeps, channel changes, scanning, CSS scanning, dual watch, VFO split,
   serial programming, battery save, low battery and key lock. Check that the
   modem releases and recovers without corrupting normal radio behavior.
6. With RF/audio measurement equipment, measure mic gain, FM deviation,
   compressor attack/release, squelch behavior, scan dwell and tone-coded scan
   acquisition. Calibrate the upstream compressor threshold before making
   audio-quality claims. Restore normal mic gain after every TX and confirm
   no unintended transmitter activation.
7. Measure current consumption, long-running stability and peak stack use.
   Verify EEPROM/channel/calibration integrity before and after testing.

Record MCU marking, board revision, firmware commit and artifact hashes,
measurement setup, expected/observed outcomes and failures. Do not promote this
experimental build to a hardware-validated release until those results exist.

For a future TX bench image, additionally measure actual 1200/2200 Hz tones,
1200-baud edge timing across SysTick reloads, write/interrupt jitter, tone phase
continuity and FM deviation at both frequencies. Decode real output with an
independent TNC; verify messages, ACK/rejects and beacons, busy-channel behavior,
PA ramp, timeout/PTT abort and receive restoration. Start with explicit TX enable
and a calibrated tone gain. The complete transport candidate includes these paths; they still require
physical RF tests. The D1 receive-diagnostic image omits APRS transmit.

## Owner preparation

The original radio USB-C socket is charging-only. A compatible USB serial adapter
using the two-pin programming sockets is needed before any local backup or
installation. See `../FLASHING.md` for a read-only full-EEPROM backup example and
`../tools/check-eeprom-backups.py` for file consistency checks. Two matching
8,192-byte files include the calibration region, but file checks alone do not
establish calibration correctness or recovery. `k5prog -r` reads EEPROM, not a
firmware dump. Physical MCU identification and usable recovery remain required.
Independent receivers/TNCs can validate RF once hardware preparation is complete.
