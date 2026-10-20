# Lossless display resources

This checkpoint used **65,752 / 61,440 flash bytes**, **4,312
bytes over**. This follows the shared-implementation checkpoint at 66,796
bytes and preserves its retained radio features, screen layouts and controls.
The original audit baseline was 67,896 bytes: **2,144 bytes recovered** so far.

## What changed

234 readable `DISPLAY_TEXT("literal")` labels/format strings, the original
921 hint bytes and 15 fixed-width choice tables share a losslessly compressed
resource stream. Its **3,883 raw bytes** occupy **2,370 flash bytes**. The
measured complete-build saving is **1,016 bytes**, accounting for the original
linker's string sharing and the new initialization code. Immutable modem and
channel-operation configuration saves another **28 bytes**.

`Main` expands the resources before any screen or formatter uses them.
Explicit compiler-builtin literal comparisons resolve text offsets at build
time, including constant pointer-table initializers. They do not search text
at runtime. An unknown literal fails compilation; regeneration is required.
The radio's original menu tables remain readable source input in
`ui/menu_choices.inc`, with their copyright/license notice retained.

Public choice-table aliases retain the original const array dimensions, so
the existing setting limits and indexing behavior continue to work. Original
hint text/order, fonts, positions and button behavior remain unchanged.

The full candidate uses **11,644 bytes of static RAM**, plus the unchanged
**2,048-byte stack reserve**, leaving **2,692 bytes beyond that reserve**.
RAM use increased because display data expands once at startup. Executable
code remains in flash; no code overlay or EEPROM storage scheme was added.

## Reproduction and checks

Run `python3 tools/pack-display-text.py` after adding or changing tagged UI
text or choice-table input. `--check` verifies generated files; `--test` also
checks every expanded byte, original compiled choice-table padding/dimension,
hint byte, text literal and constant offset with ASan/UBSan. It verifies that
an unregistered literal is rejected by the C compiler.

The complete host suite passes. All **56 stored LCD framebuffer comparisons**
remain identical to the pre-optimization baseline. Runtime and actual tone
backend tests also pass after making their operation tables immutable.

The default RX image verifies at **53,632 bytes**; the RX-session development
image verifies at **56,308 bytes**. These profiles were built sequentially.
The complete transport image is still oversized. Historical frozen bench
binaries remain unchanged, and no radio was flashed. Physical RF timing,
deviation, external-TNC interoperability and actual hardware operation remain
unverified.

The subsequent [compact runtime](COMPACT-RUNTIME.md) reduces the full
candidate to 64,388 bytes, still 2,948 over.
