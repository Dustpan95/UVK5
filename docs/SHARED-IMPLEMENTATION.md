# Shared implementation, retained radio features

This records an earlier checkpoint; see [FINAL-FIT.md](FINAL-FIT.md) for
the current formatter, lossless codec and complete fitting build.

This checkpoint used **66,796 / 61,440 bytes**,
**5,356 bytes over flash**. Four feature-preserving changes recover **1,100
bytes** against the architecture-audit baseline. This is still an oversized
diagnostic candidate, not the requested flashable final firmware.

| Change, measured successively in the complete build | Flash bytes | Saved |
|---|---:|---:|
| Audit baseline | 67,896 | — |
| Compact buffer formatter | 67,340 | 556 |
| Shared millisecond-delay wrapper | 67,244 | 96 |
| Shared EEPROM read/validation | 66,936 | 308 |
| Shared left-aligned APRS row rendering | 66,796 | 140 |

Static RAM is **8,680 bytes**, with the existing **2,048-byte stack reserve**.
The production linker still enforces 60 KiB flash and 16 KiB RAM. The new
EEPROM loader's individual stack frame is 232 bytes in the diagnostic GCC
stack report. Individual frame sizes are not a whole-program stack proof.

No additional radio features were removed. The three previously approved
removals remain tone search, flashlight and secondary-VFO activity scanning.

## Formatting and rendering

The buffer formatter retains the integer/string formats consumed by the
firmware: decimal/octal/hex/binary integers, characters, strings, widths,
padding and precision, including short and long integer arguments. Floating
point and long-long formatting were already disabled. Device/callback output
keeps the original MIT-licensed formatter; `PRINTF_LEGACY_BUFFER` selects the
original buffer path for comparisons. This is a firmware buffer formatter,
not a claim of full libc printf compatibility.

The differential formatter test performs **158,208** output/length/buffer-bound
comparisons against the original. INT_MIN uses libc as its oracle because
the original formatter's signed negation overflows at that boundary.

`UI_PrintLine` shares the repeated left-aligned APRS drawing calls. Layout,
fonts, labels and navigation remain the same. All **56 stored radio/APRS
framebuffer comparisons** against the pre-change baseline were identical.

## EEPROM loading

One sequential 216-byte read replaces scattered settings reads in
`0x0e70..0x0f47`. A shared table validates 22 scalar fields with their existing
limits and corrupt-value fallbacks. Channel attributes, calibration layout,
settings save behavior and EEPROM addresses are retained. This path performs
no EEPROM writes. The table selects wider structure offsets for host tests
and optional configurations whose RAM structure exceeds 256 bytes.

The original loader is retained as a test oracle with its copyright notice.
Each of three test configurations compares **56,576** uniform, individually
mutated and randomized EEPROM images. Loaded settings, relevant globals,
channel attributes and custom-key state match byte-for-byte. The mock rejects
any attempted EEPROM write. All three configurations pass with ASan/UBSan.

## Validation and limits

The complete host suite passes, including the real APRS service/channel/
hardware-adapter tests and packed-font/hint equivalence checks. The default
RX production build passes the original-MCU linker and image verification;
it uses **53,172 bytes**. The RX-session development build uses **56,344 bytes**.
Frozen published RX-session bench binaries remain unchanged.

The complete transport measurement is reproducible with
`python3 tools/measure-transport.py`. RF deviation, waveform timing and external
TNC interoperability remain untested on physical hardware. No hardware was
flashed during this work.

The subsequent [display-resource optimization](DISPLAY-RESOURCES.md) reduces
the full candidate to 65,752 bytes, still 4,312 over.
