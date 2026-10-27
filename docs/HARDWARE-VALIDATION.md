# Physical validation — current outgoing APRS firmware

The owner reports boot, controls, channels and voice reception passed on the
earlier full R2 image. APRS receive never successfully decoded and has now been
removed at the owner's request. D1–D4 receive experiments are historical; do not
continue their receive tests for this version.

This revised image is software-verified but has not yet run on the owner's radio.
No cloud tool has connected to or flashed hardware. Confirm the original DP32G030
hardware and retain the owner's EEPROM/calibration/channel backups and recovery
preparation as described in [FLASHING.md](../FLASHING.md).

Local checks for the revised image:

1. Boot, keypad/LCD, battery percent, saved channels, VFO tuning, FM receive,
   voice transmit, compressor, adaptive squelch, normal scan and Scan+Watch.
2. Confirm APRS RX/RxLog/RxBeep menus and received-message/history screens are
   absent. F+5 opens only the outgoing message/callsign/setup hub. Ordinary voice
   receive and power-save behavior remain available.
3. On suitable RF bench equipment, measure actual 1200/2200 Hz tones, 1200-baud
   timing, deviation, PA ramp and independent TNC decoding. Check messages and
   static beacons. Outgoing messages are sent once without delivery confirmation.
4. Verify carrier-busy, band/power/battery/timeout and serial/scan exclusions,
   PTT interruption, failure handling, and immediate restoration of the complete
   voice VFO after each APRS burst. No APRS receiver should be armed afterward.
5. Compare EEPROM/calibration before/after, power consumption and long-running
   stability. Verify peak stack use on hardware; the linker reserves 2 KiB but
   software budgets alone do not measure runtime peak use.

Record the exact source/binary hashes and expected/observed results. Do not
label outgoing APRS as RF-validated until an independent decoder accepts the
actual radio's transmitted waveform. Upstream credits/licenses are unchanged.
