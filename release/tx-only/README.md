# UV-K6 outgoing APRS candidate — receive removed

This version removes the unsuccessful APRS receiver entirely at the owner's
request. It is for the **original DP32G030 UV-K6/UV-K5 only**. The existing voice
functions, custom home/overlays and battery percentage remain. Manual APRS message
and static-beacon transmit remain experimental and require independent RF tests.

## Download and install

Download and extract **uvk6-tx-only.zip**, then use
**uvk6-tx-only.packed.bin** with the
[Egzumer updater](https://egzumer.github.io/uvtools/?firmwareURL=) in desktop Chrome.
Use the same two-pin programming cable that worked previously, a charged battery
and the owner's backup/recovery preparation. Hold PTT while powering on to enter
the original bootloader. Select this package's packed image and leave optional
patches/modifications disabled. Keep power/cable connected until the updater
finishes, then power-cycle normally without PTT. The radio's USB-C socket alone
is charging-only.

The raw **uvk6-tx-only.bin** is supplied for reproduction/compatible raw-image
utilities; it is not the browser updater file. Manifest and SHA256SUMS identify
both images, exact corresponding source, build settings and memory limits.

## Removed

- APRS reception, NRZI/HDLC decoding and incoming position/message parsing.
- Personal inbox, station directory, all-traffic log and receive banners/sounds.
- RX interrupt/FIFO ownership, diagnostic screens, raw-capture export and reader.
- Received ACK/reject handling, automatic ACK replies, ACK wait/listening settings
  and automatic message retries.

The old R2/D1–D4 packages are historical; this is a new firmware image.

## Retained outgoing operation

**F then 5 → APRS TX** contains **NEW MESSAGE**, **MY CALLSIGN**, **SETUP**.
The existing recipient → multi-tap/quick message → review → Send flow remains.
Setup retains callsign, degrees/minutes static position, quick-message editing,
separate APRS frequency, transmit-audio level, explicit data-TX enable and manual
beacon review/send. All APRS settings and drafts remain session-only.

One completed transmission follows each explicit Send. **TX SENT / DELIVERY
UNKNOWN** does not imply a peer received or decoded it. The radio immediately
restores voice operation after the burst; it cannot listen for an APRS ACK.
Outgoing messages keep IDs for peer duplicate handling. No automatic beacon
schedule or reply is introduced.

## Verification and limits

Run `./tools/test.sh` and `./tools/build-transport.sh` at the manifest's source
revision to reproduce. Build outputs are in `build/tx-only/`.
Pinned ARM GCC 10.3.1 checks CPU/vectors, actual loadable flash, the updater's
61,439-byte limit, 16 KiB SRAM with 2 KiB reserved stack, packed CRC/payload and
absence of removed receive symbols. The actual Egzumer unpacker/flash loop is
tested with serial mocked. Native tests exercise retained radio/settings/LCD
behavior and outgoing admission, encoding, cancellation and voice restoration.

No cloud tool flashes the radio. This revised image needs local boot/voice checks
and independent RF/TNC validation of APRS TX. Preserve original project credits,
copyright notices, source history and licenses in any redistribution.
