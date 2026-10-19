# Custom interface design review

## User-selected direction

- Design a custom interface and review screen mockups before further UI implementation.
- Use a mix of the earlier A and C home layouts: a prominent channel name for
  named memories, and a large frequency for VFO operation, with a smaller
  secondary VFO in both cases.
- For an unnamed memory channel, show the frequency large and the channel
  number smaller.
- Display battery level as a percentage.
- Use brief APRS banners rather than automatically opening incoming messages.
- Retain the APRS station and message screen layouts the user liked.
- APRS message sound is configurable and defaults ON. General key-beep OFF
  also silences APRS notifications.
- Everything APRS is session-only for now: callsign, static position,
  quick-message edits and received history reset on power-off.
- Separate personal inbox and all-traffic monitor.
- All received APRS messages trigger the brief banner and configured sound,
  including those addressed to other stations.
- The revised home mockups were explicitly approved ("Yes. Perfect.").
- Investigate standard-compatible APRS transmit, even if release takes longer.
- Use phone-style multi-tap typing plus saved quick messages for composition.
- Messaging is the primary transmit workflow, plus a generic beacon with a
  manually configured static position.
- Weigh all feature options and measured tradeoffs before making cuts. No fixed
  list of features to sacrifice has been approved.

## Review artifacts

- `review.html`: offline interactive concept with proposed button navigation,
  layout comparisons and synthetic incoming-message/position events.
- `home-comparison.png`: revised named-memory, VFO and brief-banner views.
- `home-review.png`: all four revised cases, including an unnamed memory.
- `home-alternatives.png`: earlier A/B/C layout alternatives.
- `messaging-review.png`: proposed recipient, multi-tap composer, quick messages,
  review-before-send and ACK status screens; flow explicitly approved.
- `aprs-comparison.png`: notification, message, station and setup concepts.
- `mockups/`: individual 128x64 screen concepts enlarged for readability.

The VFO and unnamed-memory designs include smaller final digits to preserve
frequency precision. Its abbreviated secondary VFO strip is a proposed layout,
not a statement that continuous dual-watch and APRS capture can share one receiver.

The displays use the existing radio font tables and actual 128x64 bounds.
The revised home layout is approved and implemented in the local experimental
build. Host-rendered captures are in `docs/screenshots/home-*.png`; the accepted
concepts remain separate in this directory. The actual battery percentage uses
the existing voltage-to-percentage conversion and still needs hardware validation.
The station directory and separate inbox are implemented in the optional
messaging candidate; the fitting receive profile retains the packet browser. Example traffic and 76% mockup battery are synthetic.

## Still open

- Exact home-screen fields and status icons.
- Banner duration and dismissal rules; sound ON by default is approved.
- APRS directory/history/inbox depth and duplicate handling (all session-only approved).
- Callsign/SSID/position entry and on-radio text entry/composition.
- Menu grouping and short/long-press behavior for the physical keys.
- Beacon/message send confirmation, scheduling and ACK/retry presentation.
- Final feature tradeoffs after independent cost measurements and modem work.

Implementation of the approved design is proceeding. Source checkpoints are published as work proceeds. A complete firmware release
awaits the remaining transmit work and measured feature tradeoffs. The four-packet browser remains a technical prototype.

Regenerate with `python3 tools/design_mockups.py` (requires host GCC and Pillow).
The HTML is self-contained; no server or network is required. The generator
uses the included template so the review page can be reproduced without /tmp
files. No mockup control connects to radio hardware or sends RF.

## Approved messaging flow: local implementation status

Recipient → compose/insert quick message → review → delivery status is approved.
`app/aprs_message.c` and `ui/aprs_message.c` implement the initial editor behind
`ENABLE_APRS_MESSAGING=1`. MENU from the prototype packet browser starts a new
message or replies to the displayed sender. PTT closes the editor and preserves
the draft while passing through the normal voice path. The three example quick
messages and one-second multi-tap timeout are prototype values, not final
capacity/key-timing decisions. General key-beep OFF silences APRS sound.

Send currently reports **NOT SENT / TX UNAVAILABLE**. It never pretends a modem
handoff or acknowledgment occurred. The tested message builder and ACK matching
APIs are not yet connected to a working TX/RX message service. There is no
automatic ACK generation, retry scheduling or operational transport yet.

The measured editor candidate with the separate inbox/monitor exceeds application flash by 1,580 bytes. It is
not enabled in the fitting receive prototype and is not a flashable release.
See `MESSAGING-IMPLEMENTATION.md` one directory above for costs and test limits.

The local candidate now implements the separate personal inbox and monitor,
including own-callsign entry and unread tracking. All APRS text messages still
produce notifications regardless of addressee. Four personal entries and four
monitor packets are provisional capacities. Static-position and quick-message
RAM setters exist; their configuration screens remain future work. No EEPROM
storage is used for APRS.

## Approved compact mode overlays

Scanning, frequency entry and temporary alerts keep the home readout visible
with compact overlays. The implementation also covers split, DTMF activity and
bandscope states. `mode-overlays-review.png` records the proposed direction;
the real scan footer says MENU/EXIT:STOP because MENU stops the inherited scan.
The numeric-entry overlay replaces the main readout only while digits are being
entered, keeping the home context and secondary VFO visible.

A separate receive-session bench profile now contains the approved station/inbox
screens and callsign entry, without unfinished composition controls. The full
messaging candidate retains the approved flow. `screenshots/receiver-bench/`
one directory above contains actual host-rendered receive-profile examples.

## Setup proposals awaiting review

`setup-entry-review.png` compares decimal-degree and degrees/minutes entry and
proposes setup/position review/quick-message customization screens. These screens
are not implemented or approved yet. They retain session-only configuration.
Saving a position is distinct from sending a beacon; editing a quick message
would leave both the saved value and composition draft intact until Save.

The owner is also reviewing active-VFO versus separate-frequency APRS routing.
Core prototypes freeze requested frequency, which supports enforcing either
policy later without silently sending queued frames on a changed channel.
Retry and automatic-ACK policies remain open. No final feature cuts are chosen.
