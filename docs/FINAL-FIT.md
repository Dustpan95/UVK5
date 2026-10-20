# Complete original-MCU candidate fits

The full APRS transport profile uses **61,440 / 61,440 bytes** with the unchanged
60 KiB production linker. Static RAM is **12,268 bytes**, with **2,048 bytes
reserved for stack** in the original 16 KiB SRAM. There is no flash growth margin;
every subsequent change must be rebuilt and checked. No flash extension, RAM
code overlay, calibration storage or different MCU is assumed.

The 67,896-byte architecture-audit baseline is reduced by **6,456 bytes**, with
no additional feature removals. The previously selected removals remain automatic
CTCSS/DCS search, flashlight and secondary-VFO background activity scanning.
Manual tones, A/B VFOs, dual watch, crossband, ordinary scanning, Scan+Watch,
compressor, adaptive squelch and the agreed APRS controls remain.

## Changes after the 63,832-byte checkpoint

- Menu bounds and scalar settings share metadata and handlers. Global fields,
  save/reconfigure flags and special menu operations preserve their old behavior.
  Fixed choices and repeated value rendering share code.
- A radio buffer formatter handles every buffer format used in every source
  configuration. A source checker rejects unsupported syntax. Device/callback
  printf retains the original implementation; the previous broader buffer
  formatter remains selectable for testing. Signed minima use unsigned magnitude.
- Fonts and display text use lossless LZ/Huffman streams, expanded before use.
  The original resources and notices remain; no labels or glyphs are removed.
- Exact manual CTCSS/DCS tables use byte deltas and expand into 308 bytes of RAM
  before radio setup. Values, lookup order and Golay encoding are unchanged.
- Common BK4819 bit writes and I2C START/STOP transitions share helpers, preserving
  pin order and explicit delay calls. Real CPU/wire timing remains a bench check.
- Frequency and fixed-width coordinate rendering share code; keypad character
  groups use one pool. Calling-band edges use exact 10 kHz units in storage and
  full-resolution inclusive comparisons; channel parameters and priority remain.
- Pinned GCC 10.3.1 uses `-fno-early-inlining`; immutable linker sections are
  grouped by alignment to reduce padding. CPU ABI, vectors and flash limits remain.

## Verification and limits

The full host ASan/UBSan suite exercises the actual APRS runtime, adapter, radio
owner, tone backend, restoration, ACK/retry schedule and UI paths. Differential
checks compare original menu framebuffers, settings/side effects, tones, driver
pin traces, source formats, resource bytes and geographic formatting. Codec and
arithmetic harnesses also execute on a Cortex-M0 CPU model with 16 KiB SRAM.
These are software tests, not a radio emulator or physical RF measurements.

`tools/build-transport.sh` performs a clean strict build and verifies ARMv6S-M /
Thumb-1, initial stack/reset vector, ELF flash/RAM ranges, raw/updater payload
identity and updater CRC. Its manifest records source revision, exact build
settings and SHA-256 hashes. The diagnostic enlarged-linker builds are not
published as firmware.

The complete package is an **experimental candidate**. Physical DP32G030 identity,
boot, EEPROM compatibility, recovery, tone timing/deviation, receive sensitivity
and independent TNC interoperability remain unverified. See
[HARDWARE-VALIDATION.md](HARDWARE-VALIDATION.md) and
[backup/installation preparation](../FLASHING.md). No hardware was flashed here.
