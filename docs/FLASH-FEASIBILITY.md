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

## Current shared-display/resource checkpoint

The single home renderer now covers scan, frequency entry, split, warnings,
toast, DTMF activity and bandscope overlays. Low-battery and key-lock priorities
remain in the real main-screen code. Host tests exercise these modes; physical
radio operation still needs bench validation. Scan and PTT algorithms remain.

Original fonts and all menu hint text are losslessly expanded into RAM. Every
byte is compared with the original resources under sanitizers; eleven actual
LCD framebuffers compare identically between raw and packed fonts. No glyph,
hint or coordinate precision was removed. Original resource source and notices
remain in the complete repository. This saves flash at a RAM cost, which is
included in the totals below.

| Current profile | Flash | Static RAM | Against 61,440 bytes |
|---|---:|---:|---:|
| Receive and approved home overlays | 58,056 | 7,188 | 3,384 free |
| Editor, profile, inbox, stations, received ACK handling | 63,020 | 8,232 | 1,580 over |
| Same plus retained TX protocol and timing helpers | 65,052 | diagnostic | 3,612 over |
| Same plus frozen outbox, control FIFO, tone backend and radio owner | 67,760 | 8,520 | 6,320 over |

The receive profile passes the real 60 KiB linker, ARMv6S-M/Thumb-1 vector and
packed updater/CRC checks. `receive-build-verification.json` records hashes and
RAM bounds. The candidate and probe are diagnostic builds, not flashable output.
The 2 KiB linker stack reservation is additional to static RAM; runtime maximum
stack depth remains unmeasured.

A same-source GCC 13.2.1 comparison did not help: on an earlier frozen shared-UI
snapshot, GCC 10 -Os used 61,712 bytes, GCC 13 -Os and -Oz each used 61,876.
`toolchain-size-results.json` identifies that snapshot. GCC 10 remains pinned;
the comparison is not additive to the current checkpoint.

## What the current TX measurement means

`tools/measure-protocol.py` retains actual message, static beacon and ACK/reject
builders, FCS/HDLC/NRZI, ACK matching and the experimental absolute symbol
scheduler under LTO. Its clock and tone callbacks operate only on synthetic RAM
state. No BK4819 or GPIO write occurs. The probe adds 2,032 bytes to its exact
same-source candidate; `protocol-size-results.json` records the fingerprint.

This is a lower-bound protocol/timing cost. Actual tone output, MCU clock backend,
TX limits/arbitration, busy-channel handling, automatic ACK dispatch, retries,
and position/custom-quick editing screens still need space and implementation.
The station directory and received ACK/reject parser are already in the
candidate. No diagnostic linker enlargement changes physical flash capacity.

Message/beacon/control builders and the actual encoder independently decode as
ideal 1200/2200 Hz audio in Dire Wolf with repair disabled. Corrupted-symbol
fixtures fail decoding. The standalone scheduler is host-tested for absolute
40,000-cycle intervals, wraparound, cancellation and timing faults. These are
software experiments; BK4819 phase continuity, deviation and RF decoding remain
unverified.

## Next engineering target

Continue lossless storage/code consolidation and measure the complete service.
Review flash and RAM together. If optimization cannot fund all requested
features, present measured combinations for the owner's decision before any
radio-feature cuts. Publishing source checkpoints does not establish that the
complete original-MCU release fits or works on hardware.

## Further compiler/code-boundary experiments

Eight additional same-source GCC option changes did not close the gap. Constant
merging saved 32 bytes; most options saved nothing, disabling jump tables added
984 bytes, and zero inline-limit added 252. No option was adopted. Selective
noinline boundaries saved at most four bytes (menu group); the other groups
increased size. Reports are `compiler-size-followup.json` and
`inlining-size-results.json`.

LLVM 14 with -Oz, full LTO, lld and the same newlib/GCC runtime produced a
64,340-byte messaging candidate versus GCC's 62,984. The diagnostic LLVM linker
uses explicit NOLOAD RAM sections and LOADADDR for initialized data because lld
handles those differently from GNU ld. The production linker/toolchain are
unchanged. `llvm-size-results.json` records the experiment and fingerprint.
A larger or different compiler alone has not solved the original-MCU budget.

## Radio/transport checkpoint

The final phase of `tools/measure-protocol.py` also retains the actual original-MCU
clock, BK4819 tone backend, radio admission/muted PA ramp/RX restoration wrapper,
and frozen outbox. It measures **67,760 bytes**, 6,320 over physical flash. These
modules pass host model tests and cross-compile, but are outside the normal
Makefile and are never executed on hardware here. The probe still excludes normal
send/configuration integration, automatic ACK dispatch, custom quick-message and
position editing screens, and relay-path configuration. Hardware validation
adds no compile bytes but remains a separate essential limitation. This is still
an incomplete total; no compiler change or final radio-feature cut is adopted.

AM-fix-off now builds while retaining the shared table used by FM gain staging;
it saves 556 bytes in a separate receive-profile comparison. Default AM fix
remains on, and both default image hashes are byte-identical after the repair.
This option alone does not fund the remaining transport budget.

## Fitting receive-session bench profile

Separating receive-session screens from the unfinished composer gives a fitting
physical-test profile: **61,388 / 61,440 bytes**, 7,996 static RAM. It includes
stations/positions, personal inbox, monitor and own-callsign entry, along with
the approved home and existing radio features. The full editor remains 63,020,
and retained transport/owner remains 67,760. This is a bench checkpoint and no
final TX/feature-cut decision follows from it. Only 52 flash bytes remain, so
this profile is not evidence that the complete requested TX service fits.

## Full-transport option comparisons

The current same-source diagnostic freezes RF frequency, separates two pending
control replies, and rearms RX promptly. It is 67,760 bytes before the remaining
UI/service integration. The following are measured combinations, **not adopted
feature cuts or flashable firmware**:

| Variant | Flash | Saved vs full transport | Still over 61,440 |
|---|---:|---:|---:|
| All current radio features | 67,760 | — | 6,320 |
| Application UART off | 65,664 | 2,096 | 4,224 |
| UART and AM fix off | 65,340 | 2,420 | 3,900 |
| Also flashlight, scan ranges and adaptive scan off | 64,760 | 3,000 | 3,320 |

This demonstrates why isolated RX savings cannot simply be added/subtracted
from a TX budget. Even this combination does not fit the incomplete TX service.
UART removal loses application serial programming, including CHIRP; AM fix and
scan options have real operating consequences. The compressor, adaptive squelch
and other remaining radio features stay present in all these probes. No final
removals follow from the measurement. The source defaults remain unchanged.
Reproduce with `python3 tools/measure-protocol.py --transport-options`; exact
flags, source fingerprint and static RAM are in `protocol-size-results.json`.

The RX bench rebuilt from the new development source still matches both frozen
published images byte-for-byte. No TX probe is part of that package.
