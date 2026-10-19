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

The ACK/handoff APIs are exercised by simulated tests, not wired into the
current receive parser or a transmitter. The software editor is integrated into
an optional candidate build; it is not yet a usable messaging release.

## Measured size

ARM GCC 10.3.1, same radio features and real 16 KiB SRAM layout:

| Profile | Flash | Static RAM | Against 61,440-byte limit |
|---|---:|---:|---:|
| Existing receive/home prototype | 61,012 | 3,832 | 428 free |
| With editor candidate | 62,872 | 3,948 | 1,432 over |

Delta: 1,860 flash bytes, 116 static RAM bytes. This measures the reachable
editor and UI, not the complete TX service. LTO removes unused builder/ACK
helpers from this particular candidate, so their full retained firmware cost
is still unmeasured. Persistent settings, inbox/directory replacement, modem,
relay configuration and ACK/retry processing require additional space.

Diagnostic measurement enlarges FLASH to 128 KiB only in ignored copies to
measure overflow; **never flash those outputs**. The production linker still
enforces 60 KiB. The default receive/home build remains unchanged and fitting;
`ENABLE_APRS_MESSAGING=1` is a development candidate, not a feature-cut decision.
No final radio-function removal or optional-feature exclusion is approved.

Reproduce: `python3 tools/measure-features.py --profile messaging_editor`.
Run `tools/test.sh` for sanitizers, model tests and both actual UI integrations.
Actual host-rendered images are `screenshots/message-{compose,review,blocked}.png`.
These differ in spacing from concept mockups and are not hardware photographs.

## Outstanding

- Operator settings, own-callsign validation, position and editable saved quick
  messages; persistence and inbox behavior are awaiting design responses.
- Integrate message ID/ACK/rejection parsing, deduplication and address filtering.
- Specify delivery timeout, retries/cancellation, busy-channel handling and relay
  routing before enabling an operational send action.
- Standard 1200/2200 Hz radio output, timer scheduling, radio arbitration and
  independent RF validation. Ideal software audio tests do not establish these.
- Resolve measured flash pressure through optimization and reviewed tradeoffs.

No EEPROM writes, RF transmission or hardware flashing occur in this editor.
