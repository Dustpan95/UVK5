# Compact runtime, retained firmware behavior

The full transport candidate uses **64,388 / 61,440 bytes**, **2,948 bytes
over flash**. This recovers **3,508 bytes** against the 67,896-byte architecture
audit baseline, without further radio-feature removals.

| Successive complete builds | Flash bytes | Change |
|---|---:|---:|
| Packed display resources and immutable operation tables | 65,752 | — |
| Compact memory/string routines | 65,000 | −752 |
| Compact division and division-free interrupt cadence | 64,388 | −612 |

Static RAM remains **11,644 bytes**, plus the existing **2,048-byte stack
reserve**. The production linker still enforces the original 60 KiB/16 KiB
limits. This candidate remains too large to flash.

## Memory and strings

Eight small, alignment-independent byte routines replace the speed-oriented
library implementations of `memcpy`, `memmove`, `memset`, `memcmp`, `strlen`,
`strcmp`, `strcpy` and `strncpy`. Their standard contracts are retained,
including overlap handling for memmove, unsigned-byte comparisons, terminators,
return pointers and strncpy zero padding. They are compiled separately from
LTO to keep a shared implementation.

ASan/UBSan tests compare **1,374,283 counted cases** with libc across overlapping
ranges, alignments, lengths, byte values, padding and transfers up to 8 KiB.
The application suite also runs these same source bodies under test-prefixed
names, so sanitizer internals retain their own libc routines. All **56 stored
LCD framebuffer comparisons** remain identical.

## Arithmetic and ARM calling convention

The Cortex-M0 has no integer divide instruction. A small shift/subtract kernel
replaces the larger unrolled library code. Unsigned magnitudes avoid signed
overflow. Existing divide-zero hook behavior and the old runtime's
INT_MIN/−1 wrapped result are retained; these are runtime compatibility cases,
not a claim that C division by zero or signed overflow is defined.

Assembly entry aliases supply the ARM EABI/GCC runtime names, with quotient
in r0 and remainder in r1. The C kernels return a fundamental uint64_t value
using those same registers. Separate compilation avoids LTO interpreting the
special runtime entry points as ordinary C function signatures.

Native tests pass **10,390,468** signed/unsigned comparisons, boundary cases
and divide-zero-hook checks. A separate bare-metal Cortex-M0 QEMU harness
executes **1,924 vectors** through both the original and replacement runtime,
including all six exported entry names and compiler-generated `/` and `%`
consumers. Source and ELF hashes are recorded in
[`runtime-division-verification.json`](runtime-division-verification.json).
The harness is a CPU test, not a firmware image or radio simulation.

Smaller division trades extra arithmetic iterations for flash. The 10 ms
interrupt no longer divides the global epoch by 50; a cached byte phase keeps
the same 500 ms cadence, including uint32 epoch wrap. The actual ISR passes
**1,033,250 ticks** against the original modulo predicate and timeout behavior.
The Bell 202 symbol polling and SPI tone-write paths do not perform division.

## Build and remaining verification

The complete host suite passes, including the actual APRS runtime, hardware
adapter, tone backend, register restoration and timing-fault tests. The default
RX image verifies at **52,380 bytes**; the RX-session development image verifies
at **54,936 bytes**. These remain separate profiles from the oversized full
transport candidate. Historical frozen release files are unchanged.

The reusable signed-package SDK now includes QEMU alongside the pinned ARM
compiler. No radio was flashed. Physical RF timing, deviation and independent
TNC interoperability remain unverified.
