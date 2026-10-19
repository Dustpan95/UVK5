# VUURWERK UV-K6 APRS — experimental UVK5 fork

**Experimental development project:** the custom home layout is approved. APRS
message composition, transmit implementation and feature tradeoffs are still
in development. A [receive-session bench image](release/rx-session-bench/README.md) is available
for owner-controlled physical testing. This is a development checkpoint, not a
completed firmware release.
See the [custom interface review](docs/design/DECISIONS.md) and
[interactive mockup](docs/design/review.html).

Custom firmware for the **original Quansheng UV-K6 / UV-K5 with the DP32G030 MCU**,
maintained in [Dustpan95/UVK5](https://github.com/Dustpan95/UVK5).

This is a complete modified VUURWERK firmware fork, with an approved custom home
layout influenced by F4HWN and standalone APRS receive adapted from TA1JS. It builds within the
original radio's 60 KiB application flash. **It has not been flashed or RF-tested
on a physical radio.** A UV-K6 model label or an Egzumer v0.21 boot screen alone
does not establish the MCU revision.

## Included in this experimental build

- VUURWERK's FM TX compressor, adaptive squelch, adaptive channel scanning,
  Scan+Watch, VFO split, and existing radio functions remain in the source/build.
- Approved custom home: large channel name for named memories, large frequency
  for VFOs and unnamed memories, smaller secondary VFO and battery percentage.
  Scanning, frequency entry, split and temporary alerts use compact overlays
  while keeping the home readout visible.
- Brief incoming-message banners, plus a configurable sound defaulting ON;
  general beep OFF also silences APRS. Five-second timing is provisional.
- On-radio APRS receiver: source callsign/SSID, uncompressed, timestamped,
  compressed and Mic-E positions; addressed text messages and other packet text.
- A browser for the **four most recent packets**, including up to 67 message
  characters. Callsigns, positions and text appear on the radio's LCD. A phone,
  computer, external MCU or TNC is not required by the firmware's receive path.

The approved [messaging editor and inbox](docs/MESSAGING-IMPLEMENTATION.md) is implemented
as a separate candidate (`ENABLE_APRS_MESSAGING=1`), with multi-tap entry, quick
messages, send review and truthful blocked-send status. It exceeds flash by
1,580 bytes before adding a working transmitter, so it is not enabled in the
minimal default receive profile. No final feature cuts have been selected. See the [flash feasibility work](docs/FLASH-FEASIBILITY.md). The message
codec/ACK helpers have host tests. Experimental tone, radio-control and outbox
modules also pass register/model tests, but normal on-radio send/configuration
integration and RF validation remain unfinished; retaining those parts measures
67,760 bytes, still 6,320 over flash.

All APRS settings, static position, quick-message edits and received history
are session-only and reset at power-off. The minimal default receive profile remains
an all-traffic packet browser. The receive-session bench and full messaging candidate add a separate
personal inbox filtered by an operator-entered callsign/SSID, unread markers,
a deduplicated station directory, and an all-traffic monitor. All received APRS messages still trigger brief
banners and configured sound, regardless of addressee. **Currently implemented on-radio: receive.** Standard-compatible transmit is
being investigated before release, with messaging primary and a simple
static-position beacon also requested. TX release scope is not yet fixed; see the measured
[transmit investigation](docs/APRS-TX-INVESTIGATION.md).

The provisional prototype disables the spectrum analyzer to measure APRS fit;
final feature cuts have not been approved. See [measured options](docs/FEATURE-COSTS.md). FM broadcast, VOX,
voice prompts, NOAA and several other optional features were already disabled
in the selected VUURWERK configuration and remain disabled. No claim is made
that every feature in all three upstream projects fits simultaneously.

## APRS on the radio

1. Confirm the radio has the supported MCU before considering installation.
   Back up your current firmware, EEPROM, channel data and calibration first.
2. Tune the active VFO to your local APRS frequency in FM. Turn off dual watch,
   cross-band receive, channel/CSS scanning and VFO split for continuous receive.
3. In **MENU → RECEIVE → APRS**, choose **ON**. Enable it again after power-off.
4. Use **F + 5**, or **MENU → RECEIVE → RxLog**, to open packet history.
5. Use **UP/DOWN** to browse, and **EXIT** to return to the radio. PTT closes the
   browser and follows the existing voice-transmit path.

`APRS RX` means the firmware permits listening; it does not prove an RF packet
was received. `APRS PAUSE` means another mode owns the receiver. Scanning,
transmission, dual watch, cross-band receive, power save, serial programming and
non-FM modes exclude APRS capture. Beeps temporarily release the modem. Battery
save is suppressed while APRS is enabled, so power use increases.

Packets must fit a 240-byte raw-bit capture and a 192-byte AX.25 frame. The
capture decoder records the first valid packet per capture. Position-ambiguity
encodings, objects/items, weather decoding and a power-off inbox persistence are not implemented (session-only history is the
approved requirement). APRS RF decoding sensitivity and interoperability still need
physical testing.

## Build and validate

Requires Linux, Docker, and host GCC for the sanitizer tests. A direct ARM GCC
10.3.1 toolchain plus Python 3 / `crcmod` is also supported.

```bash
./tools/setup-toolchain.sh
./tools/test.sh
./tools/build.sh
```

`tools/build.sh` performs a clean build, rejects flash/RAM overflow, checks the
Cortex-M0 ELF architecture and reset vector, and validates the packed updater
payload and CRC. Outputs are `uvk6-experimental.bin` and
`uvk6-experimental.packed.bin`; the verification report is
`build/verification.json`. Nothing in these commands flashes hardware.

To build the receive-session bench package (stations, positions and inbox), run
`./tools/build-receiver-bench.sh`. It uses the same original-MCU limits and
packages the matching manifest; it never flashes a radio.

See [BUILD.md](BUILD.md), [validation results](docs/VALIDATION.md),
[integration and remaining work](docs/INTEGRATION.md), and
[hardware test procedure](docs/HARDWARE-VALIDATION.md).

## Original projects and licensing

**Original authors retain credit and their copyright notices.**

- **James Honiball, KC3TFZ / Tokeloshe — [VUURWERK](https://github.com/Tokeloshe/vuurwerk-firmware)**:
  complete base firmware, radio features, compressor, adaptive squelch/scanning.
- **Armel, F4HWN — [F4HWN firmware](https://github.com/armel/uv-k5-firmware-custom)**:
  display and channel/frequency presentation used in the earlier prototype,
  informing the current custom home layout.
- **TA1JS / bcanata — [TA1JS APRS firmware](https://github.com/bcanata/uv-k5-firmware-ta1js)**:
  standalone BK4819 receive setup and HDLC/NRZI/FCS processing adapted here.
- **DualTachyon, Egzumer, OneOfEleven and fagci**, and the other upstream authors:
  original open firmware, radio drivers, UI, AM reception and spectrum heritage.
- **F4JTV**: credited by TA1JS for its position decoders. This fork uses its own
  coordinate-parser implementation; F4JTV's parser source is excluded because
  a clear code license was not found during the audit.

The complete fork follows VUURWERK's explicit permission to fork, modify and
redistribute its **complete firmware package under GPL v3**. VUURWERK also states
additional restrictions on extracting its original modules. Its original
[LICENSE](LICENSE) is preserved unchanged. Imported Apache-2.0 notices,
third-party licenses, original credits and base Git history are retained.
See [NOTICE](NOTICE), [LICENSES](LICENSES), and [upstream revisions](docs/upstream/sources.json).

The project owner is Dustpan95. No collaborators or agents have been invited or
added. GitHub may list legitimate upstream commit authors in its contributor
statistics because their history is preserved.
