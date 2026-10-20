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
| Receive and approved home overlays | 58,036 | 7,188 | 3,404 free |
| Editor, profile, inbox, stations, received ACK handling | 63,000 | 8,232 | 1,560 over |
| Same plus retained TX protocol and timing helpers | 65,028 | diagnostic | 3,588 over |
| Same plus frozen outbox, control FIFO, tone backend and radio owner | 67,740 | 8,520 | 6,300 over |

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
state. No BK4819 or GPIO write occurs. The probe adds 2,028 bytes to its exact
same-source candidate; `protocol-size-results.json` records the fingerprint.

This measures the retained protocol/timing paths plus diagnostic scaffolding;
it is not an exact forecast of the final integrated firmware. Actual tone output, MCU clock backend,
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
and frozen outbox. It measures **67,740 bytes**, 6,300 over physical flash. These
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
the approved home and existing radio features. The full editor remains 63,000,
and retained transport/owner remains 67,740. This is a bench checkpoint and no
final TX/feature-cut decision follows from it. Only 52 flash bytes remain, so
this profile is not evidence that the complete requested TX service fits.

## Full-transport option comparisons at the prior checkpoint

The prior source checkpoint (`4ad1423`) diagnostic freezes RF frequency, separates two pending
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
flags, source fingerprint and static RAM are in `transport-option-costs.json`.
The current protocol-only report is `protocol-size-results.json`.

The RX bench rebuilt from the new development source still matches both frozen
published images byte-for-byte. No TX probe is part of that package.

## Optimal resource parsing follow-up

The build-time LZSS parser now chooses minimum-byte token sequences, including
flag-byte overhead, rather than taking each longest match greedily. The firmware
format, decoder, font layout and menu text are unchanged. Packed plain fonts
shrink from 1,784 to 1,765 bytes and hints from 646 to 644. ARM alignment produces
20 bytes of net saving: default RX **58,036**, development RX-session bench
**61,368** (72 free), editor **63,000**, retained full TX prototype **67,740**
(**6,300 over**). Static RAM is unchanged. The frozen downloadable bench remains
61,388 bytes at its own manifest source; it was not silently replaced.

All original font/hint bytes and actual framebuffers still compare exactly.
Additional guarded-decoder tests cover 65 generated streams, empty/short inputs,
overlapping matches, incompressible data and the 4,096-byte window boundary.
Alternate resource transforms and token splits did not justify a decoder change.
A byte-column renderer matched the pixel output but added four flash bytes; it
was not adopted. Nested-loop variants exceeded the RX bench budget. Splitting
code/data sections, alignment sorting and whole-program compilation each saved
zero bytes on the same editor candidate (`section-size-results.json`). The
production compiler flags and radio features remain unchanged. These small
savings do not solve the full firmware budget; no feature-cut decision follows.

## Approved service and model-bound measurement

Automatic personal ACKs, three total attempts, a separate APRS frequency and a
configurable 10-second default ACK wait are now approved. The experimental
service and channel model implement/test those mechanisms with injected radio
operations. The actual hardware enter/restore adapter remains absent.

The old retained-path diagnostic plus the service/model uses **68,560 bytes**,
7,120 over. A second diagnostic instead binds the service into copied UI-send,
receive and 10 ms foreground paths, omits parallel raw protocol probes and removes
the superseded draft ACK callback in that isolated copy. It measures **67,692
bytes**, **6,252 over**, with 8,608 static RAM bytes. The difference includes
changed diagnostic scaffolding; it is not a 868-byte production optimization or
an adopted feature cut. These measurements retain incomplete paths and cannot
establish an exact final shortfall. `service-size-results.json` records the
fingerprint and exclusions; reproduce with `python3 tools/measure-service.py`.

The bound copy still has RAM-only channel entry/restore, placeholder external
message IDs and incomplete delivery-state presentation. Hardware retuning,
position/quick-message editing, ID allocation, relay paths and RF/carrier-sense
validation remain. **Never execute, flash or distribute either diagnostic as
usable firmware.** The normal Makefile/editor/receiver remain unchanged, and
no operational automatic transmitter is enabled by this checkpoint.


## Actual foreground, hardware and setup candidate

`ENABLE_APRS_TRANSPORT=1` retains the real runtime binding rather than injected
measurement probes: on-radio setup, sender IDs, receive callbacks, actual RAM
VFO retuning/restoration, normal radio TX admission, delivery status and the
approved 30-second voice interval. It measures **72,088 / 61,440 bytes**, a
**10,648-byte excess**, with **8,736 static RAM + 2,048 reserved stack**. RAM
fits; flash does not. No linker capacity change was made to production source.

Reproduce with `python3 tools/measure-transport.py`; it changes only the linker
capacity in an ignored copy and records its source fingerprint. Its binary
must never be flashed. `transport-runtime-size.json` contains this snapshot.
Earlier 6–7 KB deficits above measured incomplete paths and are historical.

For perspective, the default RX remains **58,036 bytes** with identical raw
and packed hashes. Development receive-session remains a distinct fitting
profile; the frozen public bench stays unchanged. The complete requested TX
firmware is not finished merely because these RX profiles fit.

A diagnostic with UART, wide RX, AM fix, adaptive scanning, scan ranges,
flashlight and several smaller optional flags disabled still used **68,412
bytes** at an intermediate source snapshot. This was not a selected profile
and did not fit. The owner requested an itemized current-source feature cost
review before selecting removals. Individual savings interact under LTO and
must be remeasured together. Receiver histories and station tables mainly use
RAM; reducing their counts is not a solution to a 10 KB flash deficit.


## Owner-selected tone-search and flashlight removal

The owner chose automatic CTCSS/DCS tone search and flashlight for removal.
Current defaults exclude both. Tone-search engine/display objects, screen and
key-dispatch entries, F+* and tone-menu launch paths, result handling and ticks
are excluded. Manual configured RX/TX CTCSS/DCS and ordinary channel/frequency
scanning remain. Existing flashlight action IDs remain stable for EEPROM
compatibility and resolve to the existing no-op; no flashlight menu choice is
offered. Original source and notices are retained.

The real complete transport candidate is **68,920 bytes**, saving **3,168**
against 72,088 and leaving **7,480 over**. Static RAM is 8,712 plus the existing
2,048-byte stack reservation. Default RX is **54,680** (3,356 saved); development
RX-session is **57,928** (3,440 saved). This is why isolated savings cannot be
added or transferred between profiles. The historical frozen public RX bench
remains unchanged at its own source; new CI artifacts use current defaults.

ASan/UBSan tests include the current tone-search-free receiver and actual
retune/restore adapter, and inspect actual menu/tone tables to retain manual
CTCSS/DCS while excluding flashlight bindings. The complete candidate ELF
retains ordinary scanning/tone tables and excludes tone-search/flashlight
implementation symbols. The legacy optional tone-search configuration still
has host exclusion tests. RF and physical flashing remain unrun. No further
removals follow from these measurements.


## Third owner-selected removal and shared setup handlers

Remove VUURWERK's secondary-VFO background channel activity scan. Only its alert
mode was wired in this fork; auto-switch/log-only were enum names, not working
mode branches. `ENABLE_VFO_SPLIT=0` excludes its engine, menu choice, SP/status
activity display and activation state. Normal dual watch, A/B VFO selection,
repeater offsets, ordinary scanning and Scan+Watch remain. Menu input bounds
now derive from available choices and reject the removed value. Original
source and notices remain in the complete fork.

The complete candidate goes from **68,920 to 68,104 bytes**: **816 saved** by
this removal. Pinned-GCC LTO shared setup boundaries recover another **208**,
with no screen/control changes, yielding **67,896 bytes**, **6,456 over**.
Static RAM is 8,684 plus 2,048 stack. Default RX is 54,164; development RX-session
is 57,324. Frozen historical releases are unchanged. All host sanitizer checks
pass, including the current feature-disabled receiver/adapter and actual
Scan+Watch choice/tone tables. No RF or physical flashing has occurred.

The boundary report is `transport-inlining-results.json`. A trial table-driven
formatter for 15 simple menu values saved only eight additional bytes and was
not adopted (`menu-table-size-results.json`); it has no claimed behavioral
validation. Menu/setup boundary savings are not additive: their combined trial
increased flash. These results do not establish that ordinary UI factoring can
recover the remaining six kilobytes. No other feature removals are selected.

## Shared implementation follow-up

The full candidate is now **66,796 bytes**, **5,356 over**, after recovering
**1,100 bytes without further feature cuts**. Shared formatting, delay calls,
EEPROM loading/validation and APRS row rendering are documented and tested in
[SHARED-IMPLEMENTATION.md](SHARED-IMPLEMENTATION.md). Static RAM is 8,680 bytes
plus the unchanged 2,048-byte stack reserve. Default RX is 53,172 bytes;
RX-session development is 56,344 bytes. The frozen historical release is unchanged.
