# Shared menu and position implementation

The complete candidate uses **63,832 / 61,440 bytes**, **2,392 over flash**.
This checkpoint recovers another 556 bytes; total recovery from the 67,896-byte
architecture audit is 4,064 bytes. The agreed settings, controls, screens and
radio features remain. Only the three previously selected removals apply.

- One degrees/minutes formatter now supplies setup reviews and AX.25 beacons.
  It preserves hemisphere, rounding and degree carry, with optional UI spacing.
- Callsign comparison shares canonical length handling, preserving equivalence
  between an absent SSID and `-0`. Existing admission checks remain.
- Menu drawing snapshots its foreground-owned ID and selected value once per
  frame. Fixed-width choice rows share a compact lookup; row bytes and indices
  are unchanged. Pointer-based labels and side-button action names now use the
  same lossless resource bundle as other display text.
- GCC may optimize standard memory/string calls using its builtins. The shared
  memory and arithmetic implementation objects retain `-fno-builtin` and
  `-fno-lto`, avoiding self-recursion and duplicated runtime bodies.

The retained original renderer is tested against the actual current renderer:
16,002 complete framebuffer and hardware-preview trace comparisons pass.
Every menu entry is visited; ranges, browsing/editing, confirmation prompts,
category scrolling and keypad entry are exercised. A separate position oracle
passes 2,164,390 comparisons across both geographic ranges and text forms,
including rounding edges, returned lengths, terminators and sentinel bounds.
The complete sanitizer suite and all 56 stored LCD comparisons pass.

The text bundle expands to 4,241 bytes from 2,602 packed bytes. Complete static
RAM is 12,000 bytes, with the unchanged 2,048-byte stack reserve. Default RX
verifies at 52,068 bytes; RX-session development verifies at 54,492 bytes.
Those profiles are distinct from the oversized full transport candidate.

Rejected diagnostic probes included additional parser inlining boundaries,
a different glyph compositor, font-data transforms and GCC 13 `-Oz`. They did
not improve the final candidate enough to adopt. A linker liveness probe found
24 unused text labels, rather than a large unneeded screen. Its build-system
complexity has not been adopted. An old APCS ABI trial failed resource-layout
assertions; the normal ARM EABI and pinned GCC 10.3.1 remain in use.

No hardware was flashed and no RF test was performed. These changes do not
establish real tone timing, deviation, on-air interoperability or recovery.
