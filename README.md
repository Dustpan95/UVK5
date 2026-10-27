# UVK5 — custom UV-K6 firmware

Custom firmware for the **original Quansheng UV-K6 / UV-K5 with DP32G030 MCU**,
maintained by [Dustpan95](https://github.com/Dustpan95/UVK5). This is a complete
modified VUURWERK fork with an approved custom interface influenced by F4HWN.
Original project credits, source history and licenses are retained in
[NOTICE](NOTICE), [CREDITS](CREDITS.md) and [LICENSE](LICENSE).

**APRS receive has been removed entirely at the owner's request.** The current
firmware has no APRS receiver, packet decoder, inbox, station list, traffic log,
received-message alerts, diagnostic capture export or automatic ACK replies.
The D1–D4 and R2 packages are historical builds and do not represent this version.

The new [outgoing APRS package](release/tx-only/README.md) retains manual message
composition and static beacons. Messages transmit once and report **TX SENT /
DELIVERY UNKNOWN**; the radio cannot receive ACKs or confirm delivery. It restores
voice operation immediately after a burst, without an APRS listening interval.
Physical APRS TX interoperability remains unverified.

## Retained radio features

- Large active channel name or frequency, smaller secondary VFO, battery percent.
- Compact overlays for frequency entry, scanning and temporary radio alerts.
- VUURWERK FM TX compressor, adaptive squelch, channel/frequency scanning and
  Scan+Watch; manual RX/TX CTCSS/DCS settings.
- Original radio settings, channels and calibration layout; no new APRS EEPROM
  storage. APRS callsign, position and edited quick messages remain session-only.

The prior owner-selected removals remain: automatic CTCSS/DCS tone search,
flashlight and secondary-VFO activity scanning. This change does not restore
or remove additional optional voice features.

## Outgoing APRS

Press **F then 5** for **APRS TX**. The hub contains **NEW MESSAGE**, **MY CALLSIGN**
and **SETUP**, preserving the existing recipient → compose/quick message → review
before send flow. Setup retains degrees/minutes static position entry, quick
messages, a separate APRS frequency, audio level, explicit data-TX enable and
manual beacon review/send. ACK-wait settings are gone.

Data TX defaults OFF. A callsign, valid frequency and explicitly set audio level
are needed to enable it. Calibration/band locks, carrier-busy checks, scan/serial
exclusion, muted PA ramp, timeout/PTT cancellation and voice restoration remain.
A successful software handoff does not prove an independently decodable RF packet.

## Build and validation

```sh
./tools/test.sh
./tools/build-transport.sh
```

The package is written to `build/tx-only/`. Pinned compiler: ARM GCC 10.3.1;
original DP32G030 application flash: 61,440 bytes; updater limit: **61,439 bytes**;
SRAM: 16 KiB with a reserved 2 KiB stack. Verification checks the CPU/vector
layout, size, packed CRC/payload and absence of removed receive symbols.

Automated checks cover retained radio settings and LCD rendering, outgoing frame
encoding, explicit send permissions, single-send behavior, cancellation and
voice restoration. The owner confirmed boot/controls/voice RX on the earlier
R2 image; this revised image still needs local boot/voice and RF TX tests.
See [hardware validation](docs/HARDWARE-VALIDATION.md) and
[backup/flashing preparation](FLASHING.md). No cloud tool flashes the radio.

Historical [receive investigation](docs/RX-INVESTIGATION.md) records why receive
was abandoned; it is no longer a testing plan for this firmware.
