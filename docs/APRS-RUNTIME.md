# Complete APRS runtime candidate

`ENABLE_APRS_TRANSPORT=1` selects the actual foreground/UI/RX/hardware binding.
It forces messaging and session support. It is disabled by default. The complete
candidate now fits **61,432 / 61,440 bytes**, with 12,268 static RAM bytes and
2,048 reserved stack bytes. `tools/build-transport.sh` selects this full profile,
uses the unchanged production linker and packages verified raw/updater images.
Physical boot and RF interoperability remain untested. The historical RX bench
is unchanged. See [final fit](FINAL-FIT.md) and the
[candidate package](../release/aprs-candidate/README.md).

## Implemented source paths

- RAM-only setup: callsign, latitude/longitude in degrees and hundredths of a
  minute, three editable quick messages, explicit APRS frequency, audio level,
  configurable ACK wait (default 10 seconds), and data TX enable (default OFF).
  No regional frequency is guessed. TX uses normal radio band admission.
- Position review/save never transmits. A separate beacon review/Send queues a
  static uncompressed position. Messaging remains recipient → compose/quick
  message → review → queue → delivery status. Failed queueing stays in review.
- Quick edits preserve the draft; Save commits the quick message, Exit cancels.
  All APRS data resets at power-off; no EEPROM layout or calibration is written.
- Each accepted message gets a five-digit ID. Up to 100,000 unique IDs are
  allocated per boot, with a timer-derived starting point. These are delivery
  correlation IDs, not authentication; uniqueness across reboots is not claimed.
- The hardware adapter snapshots the complete active VFO, including reverse
  pointers, tones, offsets, modulation, calibration-derived settings and state.
  It configures a temporary simplex FM VFO in RAM at the APRS frequency, uses
  the existing TX admission and radio-owner/tone path, then restores voice.
- While borrowed for ACK listening, unrelated scanner/radio update paths pause.
  Keys and serial handling restore voice first. Pending EEPROM saves, active
  RX/capture, scan, dual watch and split modes prevent a new acquisition.
- Numbered personal messages on the configured APRS frequency can queue an
  automatic ACK only while data TX is enabled. Duplicates can be ACKed again;
  ACK/reject controls are never themselves acknowledged.
- There are at most three total message sends. The ACK wait begins after the
  complete modem handoff. ACK/reject ends the wait early; PTT/keys restore voice.
  A missed/interrupted wait leaves 30 seconds on voice before another attempt.
  Queue lifetime is bounded to ten minutes. A late matching ACK can still
  update delivery after cancellation/expiry if a full send previously occurred.
- Status distinguishes queued, waiting, ACKed, rejected, unconfirmed, blocked,
  failed, cancelled, expired and beacon handoff. Handoff alone is not an ACK.
  The custom home uses the saved voice readout while APRS has the temporary VFO.

## Software verification

`tools/test.sh` exercises the real service, runtime and adapter against injected
clock/radio operations under ASan/UBSan. Tests include timer wrap, both VFO
slots, reversed frequency pointers, exact restoration, admission/interrupt
rollback, personal ACK routing, retries/rest intervals, unique per-session IDs,
menu/editor exclusion, quick-message scratch editing, and setup boundaries.
Every hundredth-minute value round-trips through the input conversion and
beacon encoder's rounding rule. Saving/cancelling a position never queues RF.

All actual setup/editor/status renderers are exercised against the real bitmap
fonts/framebuffer. Generate captures with `tools/test.sh`, then
`python3 tools/render-previews.py --setup`. The normal RX build remains
byte-for-byte unchanged. No test flashes or accesses a physical radio.

## Still required

The flash budget needs a user-selected resolution and a fresh fitting build.
Physical original-MCU confirmation, programming/recovery backups, radio retune
and restore checks, TX audio calibration, actual symbol timing/jitter under
load, RF carrier sensing and independent TNC interoperability remain unrun.
The UI audio-level value is a gain setting, not a calibrated deviation claim.

The sender currently uses a direct AX.25 path; configurable digipeater paths
are future work and cannot be assumed from a region. There is one receiver:
it hears APRS on a voice VFO tuned there or during the selected ACK listen
interval. It does not simultaneously monitor a different APRS frequency while
receiving voice. No automatic background polling schedule is implemented.


## Owner-selected removals

Automatic CTCSS/DCS tone search and flashlight are excluded from current
profiles (`ENABLE_TONE_SEARCH=0`, `ENABLE_FLASHLIGHT=0`). The tone-search engine
and display are not linked. F+* and the tone-menu search shortcut no longer
start searching; configured RX/TX CTCSS and DCS operation remains. Ordinary
channel/frequency scanning remains. Source is retained with upstream notices;
no other feature removals are selected. The combined complete-candidate saving
is 3,168 bytes; 7,480 bytes still exceed original-MCU application flash.


Secondary-VFO background activity scanning is now also excluded
(`ENABLE_VFO_SPLIT=0`). Normal dual watch, A/B VFOs, Scan+Watch, memory/frequency
scanning and repeater offsets remain. This removal saved 816 bytes in the
complete candidate; shared setup handlers recovered another 208 with unchanged
screens and controls. That checkpoint used 63,832 bytes, still 2,392 over. The later
[feature-preserving implementation](FINAL-FIT.md) fits at 61,440 bytes. Earlier
figures above describe previous checkpoints, not additional available savings.
