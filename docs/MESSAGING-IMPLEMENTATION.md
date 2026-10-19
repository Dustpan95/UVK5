# Approved messaging flow: development checkpoint

The user approved recipient entry → multi-tap composition/quick messages →
review before send → acknowledgment status. Messaging is primary, alongside
a static-position beacon. This checkpoint implements the editor and protocol
helpers; a working on-radio message transmitter is still outstanding.

## Implemented and host-tested

- Phone-style multi-tap uppercase/digit/punctuation entry. # commits a character,
  * deletes; one second commits a pending character. Exact timing/key details
  are provisional. MENU opens quick-message/review/recipient options.
- Editable insertion of three example quick messages without overwriting the
  draft. Insertion refuses overflow and reports a full message. The 67-character
  APRS text limit is enforced; recipient/SSID format is validated.
- Send review shows recipient and all message text. EXIT returns to editing.
- PTT closes the editor, preserves the draft and follows normal voice PTT.
- Send reports NOT SENT / TX UNAVAILABLE, because no radio modem is connected.
- Separate protocol helpers generate bounded AX.25 UI message bodies with
  padded addressees and message IDs. Reserved APRS message characters are
  rejected. No relay path is assumed by this low-level builder.
- ACK/rejection helpers require matching source, local addressee and message ID,
  and a previous explicit successful-handoff state. SSID zero and omitted SSID
  compare equally. ACKs are not inferred from pressing Send or from elapsed time.

The ACK/handoff APIs are exercised by simulated tests, connected to received control parsing, but not to a transmitter. The software editor is integrated into
an optional candidate build; it is not yet a usable messaging release.

## Measured size

ARM GCC 10.3.1, same radio features and real 16 KiB SRAM layout:

| Profile | Flash | Static RAM | Against 61,440-byte limit |
|---|---:|---:|---:|
| Existing receive/home prototype | 58,056 | 7,188 | 3,384 free |
| With editor, session profile and separate inbox/monitor | 63,020 | 8,232 | 1,580 over |

Delta: 4,964 flash bytes, 1,044 static RAM bytes. This measures the reachable
editor, profile, inbox and UI, not the complete TX service. LTO removes unused builder/ACK
helpers from this particular candidate, so their full retained firmware cost
is still unmeasured. Position/quick-message editing screens, modem, relay configuration and retry
processing require additional space. Station directory and received control
parsing are included in the candidate.

Diagnostic measurement enlarges FLASH to 128 KiB only in ignored copies to
measure overflow; **never flash those outputs**. The production linker still
enforces 60 KiB. The default receive/home build fits;
`ENABLE_APRS_MESSAGING=1` is a development candidate, not a feature-cut decision.
No final radio-function removal or optional-feature exclusion is approved.

Reproduce: `python3 tools/measure-features.py --profile messaging_editor`.
Run `tools/test.sh` for sanitizers, model tests and both actual UI integrations.
Actual host-rendered images are `screenshots/message-{compose,review,blocked}.png`.
These differ in spacing from concept mockups and are not hardware photographs.

## Outstanding

- Numeric static-position editing and quick-message customization on the LCD.
  Their validated RAM setters exist; their operator editing screens do not.
  The own-callsign editor is implemented. No persistence is required.
- Connect the tested ACK/reject builder and received control matching to a real
  transport. Parsing, personal-message deduplication and address filtering exist.
- Specify delivery timeout, retries/cancellation, busy-channel handling and relay
  routing before enabling an operational send action.
- Standard 1200/2200 Hz radio output, timer scheduling, radio arbitration and
  independent RF validation. Ideal software audio tests do not establish these.
- Resolve measured flash pressure through optimization and reviewed tradeoffs.

No EEPROM writes, RF transmission or hardware flashing occur in this editor.

## Session profile and separate inbox/monitor

All APRS settings and quick-message changes are deliberately RAM-only. Profile
callsign and coordinates start unset; example quick messages initialize at boot.
No EEPROM address is claimed, read or written by the new profile code.

The candidate APRS hub offers PERSONAL INBOX, ALL TRAFFIC, STATIONS, NEW MESSAGE and
MY CALLSIGN. Callsign entry uses the same multi-tap editor, with save/cancel and
format validation. Omitted SSID and `-0` refer to the same personal identity.
Without a configured callsign, the inbox shows a setup prompt; the monitor and
notifications still work.

A separate four-message personal buffer tracks unread state. The existing
four-packet all-traffic log includes messages for every addressee and positions.
Unrelated traffic cannot evict personal messages. Both capacities are prototype
values, not final limits. Personal messages are deduplicated by source, nonempty ID and exact text.
The monitor still logs every decoded packet, and every received text message
still triggers the configured notification.

Changing the own callsign clears the old personal inbox and unread state,
while leaving monitor traffic and the composition draft intact. Saving the same
canonical callsign leaves the inbox intact. An arriving packet preserves the
currently open personal message while it remains stored; if evicted, the UI
returns to the list instead of silently replacing the displayed body.

Quick-message lists show at most 16 preview characters while preserving all
67 characters for insertion. Full-length custom-text rendering and insertion
are sanitizer-tested.

All APRS text messages, including those for other stations, trigger the brief
banner and configured sound. General beep OFF still silences the sound.
Tests exercise decoded FIFO delivery into the mailbox, other-address alerts,
filtering, SSID-zero equivalence, unread state, bounded eviction, view stability,
identity changes, save/cancel, full keypad callsign entry and setter validation.
These are host simulations, not RF or physical power-cycle tests.

The exact coordinate-decoding optimization saved 612 bytes in both profiles.
See `FLASH-FEASIBILITY.md` for compiler experiments, the protocol lower-bound
measurement and remaining work. `messaging-costs.json` retains the earlier
65,092-byte checkpoint for provenance; current candidate totals are measured
alongside the protocol probe in `protocol-size-results.json`.

## Station directory and transmit investigation

Eight provisional station slots track the latest heard time and last valid
position per callsign. A later message retains that station's known position.
The list shows age; details show callsign, position and time since last heard.
An open detail remains attached to its station as new packets arrive; eviction
returns to the list. Entries and their positions disappear on power-off.

The retained protocol/timing probe now uses 65,052 bytes, 3,612 over flash. It
includes message/beacon/control builders, FCS/HDLC/NRZI and the experimental
absolute-deadline scheduler, with a RAM-only callback. It excludes actual radio
output, MCU clock backend, arbitration, automatic ACK dispatch and retries.
ACK/reject builder output independently decodes as ideal audio in Dire Wolf.

The further frozen-outbox/tone/owner diagnostic uses 67,760 bytes (6,320 over).
It adds tested policy-driven retries, immutable pending frames, conservative
radio admission, muted PA soft-start and RX restoration. These parts are not
connected to the normal editor or default firmware. User policy choices,
automatic ACK policy/dispatch, settings
screens and relay routing remain to be integrated. No RF success is claimed.

## Receive-session bench separation

`ENABLE_APRS_SESSION=1 ENABLE_APRS_MESSAGING=0` compiles the station/inbox/monitor
and callsign editor independently of unfinished composition. The four-item hub
has PERSONAL INBOX, ALL TRAFFIC, STATIONS and MY CALLSIGN. Monitor/detail footers
show their implemented controls. This bench profile is 61,388 bytes, fitting the
real original-MCU limit by 52 bytes. The full editor profile is retained without
changes to its approved flow. This separation supports physical receive tests;
it does not remove transmit from the project or select final feature cuts.

## Setup design review

`design/setup-entry-review.png` proposes the APRS setup hub, decimal or
degrees/minutes position entry, position review, quick-message selection and a
separate multi-tap quick-message editor. These are concept mockups pending owner
review, not implemented screens. Cancel would leave the saved quick message and
composition draft intact. Saving a static position would not send a beacon.
The all-session-only requirement remains in effect.

The latest outbox investigation additionally freezes requested RF frequency,
records the local receive frequency in the full messaging candidate, handles
late matching acknowledgments and separates bounded priority control replies.
Active VFO versus separate APRS-frequency routing, retry policy, automatic ACK
policy and relay configuration remain pending choices. The control mechanism
is not an automatic transmitter in the normal image.
