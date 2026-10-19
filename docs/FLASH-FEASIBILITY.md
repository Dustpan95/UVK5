# Original-MCU flash feasibility

The requested complete build has not been shown to fit. The original DP32G030
application limit remains **61,440 bytes**; diagnostic linker enlargement does
not increase the radio's physical capacity. No feature cuts are approved.

## Verified saving without feature removal

Compressed APRS positions previously multiplied and divided in 64 bits, pulling
in runtime arithmetic helpers. `app/aprs_scale.h` now computes the exact same
integer result using bounded 32-bit quotient/remainder arithmetic. All
137,133,362 valid input/divisor pairs were exhaustively compared against the old
64-bit formula. The normal parser, register, editor and framebuffer sanitizer
tests pass, as do the production-size linker and packed-image checks.

The receive/home build fell from **61,024 to 60,412 bytes**: **612 bytes saved**,
with static RAM unchanged at 3,832 bytes. It now has 1,028 bytes of flash free.
This is a code optimization, not a reduction in coordinate precision or format
support. MCU execution time and physical radio operation still need bench tests.

## Compiler experiments

Eleven otherwise identical messaging builds used ARM GCC 10.3.1. Their frozen
baseline predates the coordinate optimization and used 65,092 bytes. Results
are in `compiler-size-results.json`, including the exact source fingerprint.

| Diagnostic option | Flash change |
|---|---:|
| Allow GCC builtins | -140 bytes |
| Disable small-function inlining | -40 bytes |
| Disable unwind tables | 0 |
| Disable caller saves | 0 |
| Disable splitting wide types | +24 bytes |
| Disable switch conversion | +64 bytes |
| Inline limit 3 | +176 bytes |
| Disable interprocedural constant propagation | +344 bytes |
| Disable functions-called-once inlining | +1,016 bytes |
| Disable both inlining options | +3,872 bytes |

None is adopted. Size alone does not validate timing or behavior, and results
are not additive across code revisions. These options do not solve the gap.
Reproduce with `python3 tools/investigate-size.py`. Outputs under
`build/size-investigation` use 128 KiB diagnostic regions and must never be
flashed or released. The production linker still enforces 60 KiB.

## What TX measurement means

`tools/measure-protocol.py` retains the actual message builder, new static
beacon builder, HDLC/FCS/NRZI encoder and ACK-state matching helpers under LTO.
Its callback writes synthetic tone selections to volatile RAM, not a radio.
The same frozen source is built before/after adding the probe so the delta is
measured rather than guessed. This is a lower bound on protocol code, not a
complete working transmitter or a trustworthy total release budget.

The current measurements are:

| Profile | Flash | Against 61,440-byte application region |
|---|---:|---:|
| Receive and approved home | 60,412 | 1,028 free |
| Editor, RAM profile, inbox and monitor | 64,480 | 3,040 over |
| Same plus retained TX protocol helpers | 65,884 | 4,444 over |

Retaining these TX helpers adds **1,404 bytes** to the exact same candidate.
`protocol-size-results.json` records the frozen source fingerprint. This is not
an estimate for the missing hardware driver and service. At least 4,444 bytes
must be recovered *before* those parts can be included. The probe does not call
any BK4819 or GPIO write and is not compiled into the normal Makefile.

Missing parts include radio tone output and absolute symbol scheduling,
TX limits/arbitration, busy-channel handling, on-air ACK parsing/generation,
retry/cancel behavior, static-position/custom-quick-message editing screens,
and the approved station directory. Their costs remain unmeasured.

The static beacon builder is host-tested for geographic limits, hemispheres,
rounding to the nearest hundredth minute and carry into the next degree. Its
actual C output, followed by the actual C symbol encoder, decodes in independent
Dire Wolf with repair disabled. Deliberately corrupted audio is rejected. This
establishes ideal software protocol/audio encoding, not BK4819 RF output.

## Next engineering target

Consolidate the inherited and custom screen paths, then measure the resulting
complete UI before asking for radio-feature sacrifices. In the frozen messaging
build, `UI_DisplayMain` occupies 4,688 bytes and `UI_DisplayMenu` 2,672 bytes;
LTO inlines helpers into these functions, so these are size drivers, not amounts
that can simply be deleted. Scanning, numeric input, split operation, warnings,
key lock and toast states still need useful displays after any consolidation.

Other candidates include lossless font/string compression and eliminating
repeated formatting code. Savings must be measured with unchanged feature
behavior. If these cannot fund the full requested service, present measured
feature combinations for the owner's decision. A working fit is not guaranteed;
no feature removal or promise of a complete original-MCU build follows from
publishing the source.
