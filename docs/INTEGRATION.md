# Current integration — outgoing APRS only

The complete VUURWERK firmware base and its history are retained, together with
original copyright/license notices. This is a modified complete fork under the
upstream GPL v3 permission, not extracted VUURWERK modules relicensed separately.
See [NOTICE](../NOTICE), [CREDITS](../CREDITS.md) and [LICENSE](../LICENSE).

The owner's approved custom home/UI remains influenced by F4HWN. APRS receive
was abandoned after unsuccessful hardware tests and has been removed from the
active source/build: no FIFO/modem receive owner, NRZI/HDLC receiver, position
parser, incoming message/ACK handling, inbox, stations, traffic monitor, received
banner/sound or diagnostic RAM export command remains. Receive-specific build
profiles and tools are retired. Prior release packages and investigation documents
are historical; their exact corresponding source remains available in git.

The TA1JS-derived shared AX.25 checksum survives in `app/aprs_frame.c`, with the
original Apache-2.0 copyright/license notice. TA1JS's receive work remains credited
as project history. No F4JTV parser source or symbol artwork is included.

`app/aprs_ui.c` routes only the outgoing composer, profile and setup. The three
remaining hub entries are the previously approved NEW MESSAGE, MY CALLSIGN and
SETUP. `app/aprs_session.c` now stores only the local RAM profile/quick messages.

`app/aprs_runtime.c` binds manually requested messages/beacons to the existing
Bell 202 transmit backend through service/channel/hardware adapters. The runtime
restores the saved voice VFO immediately after each burst; no modem is armed and
no ACK-listening lease remains. One complete transmission follows each explicit
Send, with no automatic reply, delivery confirmation or timed retry. Numbered
outgoing message IDs remain for peer-side duplicate handling; received ACKs have
no local handler. TX SENT means the radio handoff completed, not proven delivery.

Radio EEPROM layout, calibration, band/battery/TX checks, PTT and timing-fault
cancellation, PA ramp and voice restoration remain. Flash/RAM/vector/payload and
receive-symbol absence checks run on the actual linked DP32G030 image. Native
sanitizer tests and mocked serial/RF cannot establish physical interoperability.
