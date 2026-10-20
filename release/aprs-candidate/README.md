# Complete APRS candidate — original DP32G030

This is a **flash-size-verified experimental firmware candidate**, including the
approved UI, retained radio features and complete APRS receive/message/beacon
paths. Physical boot and RF interoperability have not been tested. The intended
Bell 202 tones are 1200/2200 Hz; on-air timing, deviation and independent TNC
results are still required. No cloud command flashes your radio.

- **Updater file:** [uvk6-aprs-candidate.packed.bin](uvk6-aprs-candidate.packed.bin).
- [Raw image](uvk6-aprs-candidate.bin) is for reproduction/inspection; use the
  packed file with the compatible updater.
- [Manifest](manifest.json) identifies exact source, build options, hashes and
  remaining physical checks. [SHA256SUMS](SHA256SUMS) lists file hashes.
- **Flash:** 61,440 / 61,440 bytes. **Static RAM:** 12,268 bytes; **stack reserve:**
  2,048 bytes. No flash growth margin remains. Original 60 KiB linker enforced.
- Source retains all upstream history/notices. See [project credits](../../NOTICE)
  and [licenses](../../LICENSES). No additional feature cuts were made to fit.

## Before local installation on Windows

The owner's radio has not been positively identified as DP32G030 and no backups
have been supplied. Its charging USB-C socket does not program firmware.
Obtain a UV-K5/UV-K6-compatible USB serial **two-pin programming cable**; confirm
its Windows COM port/driver. Establish DP32G030 hardware compatibility and save
two independent full EEPROM/calibration downloads plus channels/settings and a
matching recovery firmware/tool. Follow [backup preparation](../../FLASHING.md).
Do not use this original-MCU image on a different MCU revision.

After those preparations, with a charged battery and firmly seated programming
plug, enter the original radio's bootloader by holding PTT while powering on.
Use a supported Chrome/Edge Web Serial updater such as
[Egzumer UV Tools](https://egzumer.github.io/uvtools/), choose the correct COM port
and **this package's packed image**, and complete the local update without
interrupting power. Power-cycle normally afterward. Preserve the original
calibration; do not upload another radio's EEPROM. Installation is experimental;
bootloader availability alone does not establish that recovery has been tested.

## First use and bench checks

1. Verify boot, display, battery percentage, keys, receive, saved channels and
   ordinary PTT operation on appropriate bench equipment before relying on it.
   Follow the [hardware validation plan](../../docs/HARDWARE-VALIDATION.md).
2. Enable APRS in **MENU → RECEIVE → APRS**. For continuous APRS receive, tune the
   active VFO to the appropriate local APRS frequency in FM and disable scanning,
   dual watch and crossband. The radio has one receiver.
3. **F + 5** / **RxLog** opens the APRS hub: personal inbox, all traffic, stations,
   new message, own callsign and setup. UP/DOWN selects; MENU opens; EXIT backs out.
4. Enter your own callsign/SSID. Setup offers position, saved quick messages and
   data TX. Enter latitude as DD MM.mm and longitude as DDD MM.mm; use UP/DOWN
   for hemisphere. Position Save does not send a beacon.
5. Data TX requires an explicitly configured APRS frequency and audio level,
   plus explicit enable (default OFF). Audio level 0..127 is an uncalibrated
   register setting; no universally valid RF deviation value is claimed.
   Adjust/measure with your receiver/TNC/test equipment. Do not enable automatic
   ACKs until the transmitter is ready for bench testing.
6. New Message uses recipient → multi-tap text/quick message → review → Send →
   status. `#` commits the current multi-tap character; `*` deletes. Maximum text
   is 67 characters. Only Send queues transmission. Beacon has a separate review.
7. ACK wait defaults to 10 seconds and is configurable. There are up to three
   total attempts, with 30 seconds on voice between attempts. Personal numbered
   incoming messages may queue automatic ACKs while data TX is enabled. PTT
   interrupts data ownership and restores voice. Voice RX pauses on the APRS
   frequency; this is not simultaneous dual receive.

All APRS settings, position, quick-message edits and received history reset at
power-off. The receive gate excludes incompatible operating modes. Verify actual
RF decode, tone frequency/deviation, ACK exchange, retries, PTT interruption and
voice/register restoration with an independent receiver/TNC before everyday use.
Record results against this package's source revision and hashes.

## Reproduce

From the manifest's exact source revision, run:

```bash
./tools/setup-toolchain.sh
./tools/test.sh
./tools/build-transport.sh
```

The retained development banner/version is `VUURWERK rx-dev` in both profiles;
identify this complete candidate by its manifest and file hashes.
The pinned compiler is ARM GCC 10.3.1. The package is generated in
`build/transport-candidate/`; strict flash/RAM, vectors, payload and CRC checks
must pass. The historical RX-only bench package remains separate and unchanged.
