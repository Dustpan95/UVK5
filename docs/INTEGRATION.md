# Integration design and current scope

The source base and all its history are the complete VUURWERK firmware at
`2d560898cd02da0fcb94975240209a72f53a62cd`. The project is a modified complete
fork, rather than a collection of extracted VUURWERK modules. Original source,
credits, notices, donation files and the upstream release directory remain.
The `release/` directory contains historical upstream VUURWERK artifacts, not
UVK5 APRS releases. No custom prerelease has been published yet.

## APRS receive

`app/aprs_rx.c` adapts TA1JS's BK4819 FSK receive setup and FIFO capture flow.
`app/aprs_decode.c` adapts TA1JS's CRC, NRZI and HDLC deframing. The parser checks
FCS, address boundaries, UI control/PID and coordinate limits before adding a
packet to the four-entry RAM log. The receiver does not call any APRS transmit
code and introduces no EEPROM layout changes or APRS EEPROM writes.

Coordinate decoding is implemented in `app/aprs_position.c` using the APRS
coordinate formats: decimal degrees/minutes, timestamped position payloads,
base-91 compression and Mic-E. TA1JS credits F4JTV's `aprs_decoder` for its own
position routines. That repository had no clear code license at inspection, so
those routines were excluded. This is not a clean-room provenance claim: the
TA1JS source and its credit were reviewed during development.

The receiver saves and restores its shared BK4819 registers. Radio retuning,
TX setup, scanner start and audible beeps release the modem before changing
those registers. A 500 ms task maintains listening when the radio is stationary
in FM. Scan, CSS scan, split, dual-watch, cross-band receive, serial programming,
TX and power save exclude capture. Battery save does not engage while APRS is
enabled. This is a single-receiver radio; continuous APRS and active scanning
are not simultaneous operations.

RX capture is bounded to 240 raw bytes / 192 decoded frame bytes. One valid
frame is returned per capture. The history holds recent packets, not a deduplicated
station directory. The minimal default receive profile exposes all addressees in one
packet browser. The fitting receive-session bench and full messaging candidate add a separate RAM-only
personal inbox, deduplicated station directory and all-traffic monitor with
an own-callsign editor; see
`MESSAGING-IMPLEMENTATION.md`. Received history, callsign, position and quick
messages are session-only; no power-off persistence is required. Received message-ID/ACK/reject parsing and correlation are implemented in the
candidate; actual APRS transmit, beaconing and automatic ACK dispatch remain
integration goals; see `APRS-TX-INVESTIGATION.md` for the prototype
measurement and known upstream interoperability issue. Position ambiguity,
objects/items and specialized weather/telemetry fields remain future work.

## Display and controls

`ui/home.c` implements the approved custom home: prominent named memory or
frequency, smaller secondary VFO, percentage battery and brief APRS banners.
It uses inherited bitmap fonts and the existing voltage-to-percentage conversion.
Scanning, input, warning, split and toast states use compact home overlays.
The scan/PTT algorithms are preserved; key lock and low-battery popups keep
their existing priority.
This is a custom interface, not a complete F4HWN menu/settings port.

`ui/aprs.c` renders source, destination, positions or up to 67 message characters
on the radio. RECEIVE-category APRS and RxLog entries control listening and the
browser. RxBeep is a session-only toggle, default ON, and respects global beep
OFF. Incoming messages queue a beep for foreground operation; the mocked-audio
tests verify RX handoff and rearming. F+5 opens the browser in the default spectrum-disabled profile. PTT
passes through the normal voice-transmit path. Key lock and low-battery warnings
retain their existing priority.

Two upstream build assumptions were corrected to support disabling spectrum:
the F+5 analyzer call is now conditional, and the shared 3x5 UI font remains
available without the spectrum module. The linker script reserves 2 KiB for
stack. The packer now fails the build on error. Build tools clean before changing
flags and validate the packed artifact independently.

## Remaining hardware work

The source-level hooks and simulated BK4819 tests do not establish RF
sensitivity, packet-loss rate, audio quality, interrupt timing, real register
side effects, power draw or maximum stack depth. None of those have been
measured on a physical radio for this fork. Compressor defaults are retained
from VUURWERK; the upstream source itself notes that its threshold needs bench
calibration. A passing gain-limit test does not establish good sounding audio.

See `HARDWARE-VALIDATION.md` before any physical installation. Future additions
should preserve a reproducible passing profile and remeasure the flash budget.

The receive-session bench uses `ENABLE_APRS_SESSION=1` without the unfinished
composer. It fits at 61,388 bytes and exposes the approved station/inbox/monitor
and callsign screens. The full messaging profile implies session UI. Build
cleanup now removes prior optional objects and passes the selected flags to
`make clean`; the bench packager pins every boolean option to the source
revision's defaults before selecting its profile. Cross-profile stale objects
cannot be reused by those clean build scripts.

The experimental radio owner also defers while APRS capture is active, even if
no BK4819 interrupt is pending. The receive module exposes that state without
changing capture behavior. Host tests verify capture transitions and the owner
admission guard; the normal receive image does not include a transmitter.
