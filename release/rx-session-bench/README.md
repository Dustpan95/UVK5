# Original-DP32G030 UV-K6 receive-session bench checkpoint

[Download the packed updater image](https://raw.githubusercontent.com/Dustpan95/UVK5/main/release/rx-session-bench/uvk6-rx-session-bench.packed.bin) ·
[Build manifest and exact source](manifest.json) · [Verification](verification.json)

**Experimental physical-test firmware, not a completed APRS TX release.**
Target: original **DP32G030** Quansheng UV-K6/UV-K5, Cortex-M0, 16 KiB SRAM.
Raw image: **61,388 / 61,440 bytes**, 52 free. Packed image: 61,406 bytes.
Static RAM: 7,996 bytes; linker stack reservation: 2,048 bytes.

This image includes the approved custom home and compact scan/input/alert
overlays, percentage battery, standalone APRS station directory/positions,
personal inbox, all-traffic monitor and on-radio callsign entry. The selected
VUURWERK radio features, including voice TX compressor, adaptive squelch and
scanning, remain. APRS settings and received history are session-only. Provisional
capacities are eight stations, four personal messages and four monitor packets.

APRS transmit, composition and beaconing remain active development goals.
Their unfinished implementation is outside this particular bench image; no
final feature-cut decision has been made. The complete retained transport still
exceeds original-MCU flash. This file does not transmit APRS packets.

## Before owner-controlled installation

Confirm the physical MCU/revision is DP32G030, preserve the existing firmware and
full EEPROM/calibration/channel data, and establish recovery access. An Egzumer
version string alone is not physical revision confirmation. Compatibility with
the owner's existing EEPROM configuration, boot and RF behavior remain untested.
Do not use this image on another MCU revision.

Use a compatible local updater and its documented bootloader/recovery procedure.
The `.packed.bin` file is the updater image; the raw `.bin` is provided for
inspection and compatible advanced tools. No build or packaging command here
flashes a radio. See [the physical test procedure](../../docs/HARDWARE-VALIDATION.md).

## Receive test

1. Tune the selected VFO to the local APRS frequency in FM. Disable scanning,
   dual watch, cross-band receive and VFO split for continuous APRS capture.
2. Enable **MENU → RECEIVE → APRS → ON**. It resets to OFF after power-off.
3. Open **F + 5** or **MENU → RECEIVE → RxLog**. Select **STATIONS**, **ALL
   TRAFFIC**, **PERSONAL INBOX**, or **MY CALLSIGN**.
4. Callsign entry uses phone-style multi-tap; `#` commits a character, `*` deletes,
   MENU saves and EXIT cancels. Set it to filter the personal inbox.
5. All received APRS text messages trigger brief banners and the configured
   sound, including other addressees. General beep OFF silences APRS too.
   PTT closes the browser and retains the normal voice-transmit path.

Build/ELF/vector/flash/RAM/packed-payload/CRC checks and host sanitizer/register/
framebuffer tests passed. No physical boot, RF reception, sensitivity, audio,
power consumption or maximum stack-depth measurements have been performed.
`manifest.json` records the exact clean source commit and hashes; `SHA256SUMS`
checks the distributed images and metadata. Remote CI status is separate.

## Reproduce and credits

At the manifest's source commit, run `./tools/test.sh` and
`./tools/build-receiver-bench.sh` using the documented pinned toolchain. Outputs
are under `build/receiver-bench/`. Corresponding complete source, build scripts,
licenses, notices and original history are available at the manifest's source URL.

Maintained in **Dustpan95/UVK5**. Original project credit remains prominent:
**James Honiball/KC3TFZ (Tokeloshe), VUURWERK**; **Armel/F4HWN**;
**TA1JS/bcanata**; **DualTachyon, Egzumer, OneOfEleven, fagci**, and the other
upstream authors. See [NOTICE](../../NOTICE), [credits](../../CREDITS.md) and
[licensing](../../LICENSE). This is a complete modified GPLv3 VUURWERK fork;
original copyright notices and compatible upstream licenses remain in source.
