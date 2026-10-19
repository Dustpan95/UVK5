# APRS transmit investigation

Standard-compatible APRS transmit is being investigated before release,
including the first release. No TX release scope has been fixed. The local
firmware currently implements receive only; usable TX and RF validation remain
outstanding.

## Compatibility finding

The reviewed TA1JS revision is
`6320cdac7a2dce46f88027d220ff67e47eb0e3fb`. Its source, README and open
[issue #1](https://github.com/bcanata/uv-k5-firmware-ta1js/issues/1)
document that its FSK TX recipe actually produces **1200/1800 Hz**, despite
programming a 2200 Hz tone reference. Standard APRS Bell 202 uses
**1200/2200 Hz**. TA1JS reports decoding by software modems and UV-K5 peers,
with a known failure to decode on hardware TNCs such as the Kenwood TH-D75.
Those are upstream reports, not RF measurements performed on this fork.

A direct TX port would inherit that limitation. It must not be presented as
interoperable Bell 202 solely because it generates valid AX.25 bytes.

## Earlier flash feasibility experiment

A small TA1JS manual-beacon prototype was linked into an isolated copy of the
combined firmware using ARM GCC 10.3.1 and the unchanged **61,440-byte** flash
limit. The relevant radio features, APRS receiver and partial F4HWN display
remained linked.

| Profile | Flash bytes | Remaining |
|---|---:|---:|
| Pre-custom-home receive prototype | 59,980 | 1,460 |
| Earlier fixed-coordinate prototype, before the last display changes | 61,284 | 156 |
| Pre-custom-home beacon prototype with variable coordinates/SSID and geographic bounds | 61,416 | 24 |

The variable-coordinate prototype added 1,436 bytes. Its input mechanism is
synthetic volatile variables, used to retain the code under LTO; it is not
an operator-usable on-radio configuration implementation. The exact complete
TX feature cost has not been measured. The prototype includes neither callsign/
position editing nor messaging, ACK/retry/scheduling or a corrected tone modem.
These figures therefore establish a narrow feasibility result, rather than a
claim that the complete TX feature set fits or cannot be optimized to fit.

Reproduce the variable-coordinate experiment with `tools/investigate-tx.sh`.
Its source and original notices are in `experiments/aprs_tx_size.c`. It is not
compiled into the default firmware or attached as a release asset.

## Work needed for usable standalone TX

- Implement explicit operator callsign, SSID, position and message configuration
  on the LCD, without assuming a connected phone or computer.
- Keep normal radio TX limits, timeouts and receiver/register handoff intact.
  Refuse unconfigured/placeholder callsigns and invalid positions.
- Investigate a genuine 1200/2200 Hz output method, such as correctly timed
  software tone switching, and measure the RF waveform and deviation.
- Budget the extra UI/modem code through measured optimization or explicit
  feature tradeoffs. Preserve a separately verified default profile.
- Test with independent software and hardware TNCs, then test beacon/message
  behavior, acknowledgments, retries, receive recovery and long-running use.

No APRS RF was transmitted during this investigation. No hardware was flashed.

## Candidate standard tone path

The ordinary BK4819 tone generator accepts frequency-scaled values through
REG71 (1200 Hz: 12389, 2200 Hz: 22714). A possible standard modem would switch
that generator per NRZI symbol instead of using the incompatible hardware FSK
TX mode. At the configured 48 MHz CPU clock, each 1200-baud symbol is exactly
40,000 cycles. Absolute symbol deadlines must include SPI transaction time;
a delay of 833 microseconds *after* each register write would accumulate drift.

The current bit-banged SPI write has about 76 microseconds of explicit delays
plus GPIO/software overhead. Its worst-case timing and interrupt interference
are unmeasured. The current SysTick interrupt is 10 ms, so it cannot directly
schedule each symbol. A dedicated timer or carefully bounded absolute-deadline
loop needs evaluation, along with RF tone purity, phase behavior, deviation,
radio timeout and restoration. Register values alone establish none of these.

The earlier approved home and alert build measured 61,012 bytes, leaving 428. The older
1,436-byte beacon addition cannot simply be assumed to fit. See
`FEATURE-COSTS.md` for independently measured options. No cuts have been selected.

## Independent protocol/audio result

The separate C encoder in `experiments/bell202_symbols.c` generates AX.25 FCS,
HDLC stuffing and NRZI tone symbols. `tools/test-bell202.py` converts its output
into ideal continuous-phase 48 kHz PCM with 40 samples per symbol, then invokes
Dire Wolf `atest -F 0` (bit repair disabled). Position, addressed message, and
67-character stuffing-heavy message fixtures decode. One corrupted tone symbol
inside each frame produces zero decoded packets, confirming CRC rejection.
This code is not in the radio build. See `VALIDATION.md` for the physical limits.

The user selected messaging as the primary TX workflow, alongside a generic
static-position beacon. Message editing, addressing, ACK/retry limits and beacon
configuration are still being designed. The global key-beep setting must also
silence APRS sound; the independent APRS sound toggle defaults ON.

## Experimental original-MCU tone backend and radio owner

`experiments/bell202_radio.c` drives the ordinary BK4819 tone generator with
12389/22714 frequency words for nominal 1200/2200 Hz. `driver/systick.c` combines
the software 10 ms epoch with the downcounter and pending-interrupt state to
supply a wrapping 48 MHz cycle clock. Interrupt-mask restoration, epoch/pending
cases and clock wrap have host register-model tests. A disabled tick interrupt,
wrong period or masked interrupts prevents the tone backend from starting.

`experiments/aprs_radio_owner.c` is the bench transport wrapper. Explicit enable
and an explicitly supplied uncalibrated tone gain are required. It accepts only
stationary simplex FM with no TX code, scrambler or PTT ID and no dual/crossband,
scan or split operation. Active receive, open squelch, a held PTT or a pending
receive interrupt defers transmission. The normal radio TX preparation retains
its frequency, battery and timeout checks; audio remains muted during the
existing PA soft-start. Every acquired-TX return drops PA and restores RX without
roger/DTMF/tail audio, then immediately rearms eligible APRS reception. A refused
normal TX preparation also rearms RX. Rearming does not advance session ages or
notification timers and does not dispatch a sound. Actual owner tests exercise ramp/cleanup on completion,
PTT interruption and modem timing faults. No RF or hardware is present in these
tests. This conservative admission is not validated carrier sense; open-squelch
APRS use and collision avoidance still need design and bench work.

`app/aprs_tx.c` implements a frozen session-only outbox behind an injected link.
Draft changes cannot mutate the stored frame, requested RF frequency or ACK
identity. Frequency is stored in local radio-native 10 Hz units; the transport
refuses a changed channel. Received records in the full messaging candidate also
retain their locally observed frequency. This does not select the pending UI
choice between active-VFO operation and a separate APRS channel. Explicit policy
supports one or up to three attempts, busy deferral, bounded lifetime, cancellation,
ACK/reject matching and wrap-safe deadlines. ACK waiting begins after successful
handoff. Automatic personal ACKs and three total outgoing attempts are now approved.
The ACK-listen interval is configurable with a 10-second default.
Failure/cancellation can follow an earlier successful attempt; attempts remain
recorded and must be shown honestly by the future UI. Matching late ACK/rejects
can still confirm an earlier completed send after cancellation or timeout, until
that frozen job is replaced; an already received ACK/reject is not overturned.

Two provisional control slots hold ACK/reject frames independently of the own
message. They freeze frequency, have bounded lifetimes, deduplicate identical
pending replies, reject contradictory replies and do not replace a full queue.
They take priority for one radio handoff per foreground service call without
changing the outgoing message or its ACK deadline. Own-job expiry continues even
when a control reply is busy. Whole-data-TX cancellation clears pending controls.
These are tested mechanisms, not an adopted automatic-ACK or retry policy; no
received packet currently queues an RF reply in normal firmware.

These modules are deliberately absent from the normal Makefile/send action.
They are host-tested and cross-compiled, not an operational or RF-validated
transmitter. Missing integration includes configuration screens, actual send
binding, automatic ACK dispatch and relay routing. Tone phase continuity, real
frequency/baud accuracy, SPI/interrupt jitter, deviation, decoding by independent
hardware and TX/RX recovery remain unmeasured. No claim of standard-compatible
RF follows from software tests or nominal frequency words.

The actual C scheduler's tone transitions were also resampled into audio with
synthetic +/-6.7 us SPI-duration variation and periodic simulated interrupt
latency. Independent Dire Wolf decoded message, beacon, ACK and reject fixtures
with bit repair disabled. `tools/test-bell202-timing.py` reproduces this optional
test. It tests the chosen software timing budget, not a physical SPI bus or RF.

## Session service and separate-channel orchestration

`experiments/aprs_service.c` applies the approved policy behind explicit session
configuration. It requires a callsign, selected APRS frequency and bounded timing
before enable. It queues ACKs only for valid numbered personal messages received
on that APRS frequency, preserving ID case; controls never ACK other controls.
Duplicate receptions can queue another ACK after an earlier reply was sent, while
identical pending replies share a queue slot. Call the service before personal
mailbox duplicate suppression when the actual receive binding is added.

Own messages use three total attempts. Static beacons require a configured
position and do not request an ACK. Disabling TX or changing callsign clears
pending control replies and stops old-identity retries. Late, frequency-matched
ACK/reject evidence can still update a completed send without enabling any RF.
No APRS configuration or history is written to EEPROM.

`experiments/aprs_channel.c` orchestrates enter/send/ACK-wait/restore using injected
hardware operations. It waits from the completed handoff deadline, restores
voice on ACK/reject, expiry, fault or disable, and lets PTT interrupt a wait.
A timeout restores voice before a later service call can acquire another attempt.
The model preserves retry deadlines after PTT; retry spacing and its display
remain unfinished. Control replies can be sent during the same ACK-listen lease
without overwriting the outgoing frame or deadline. Tests cover time wrap,
refused entry, interruption during acquisition, restoration exactly once, queued
controls while waiting and early lifetime expiry.

**The actual hardware enter/restore adapter is not implemented.** Test callbacks
simulate tuning and restoration. Integration must preserve the actual voice
VFO, offsets/codes, calibration and receiver state, and restore voice before any
ordinary key/PTT, save or serial operation can act on temporary APRS state.
The existing tone owner only accepts an already selected, matching simplex
frequency; it is not itself the requested separate-frequency adapter.

The latest diagnostic additionally retains the service/channel model on top of
the tone/outbox/owner. Its callbacks simulate channel entry/restore in RAM;
retaining these helpers is not a physical retuning test or a flashable image.
Normal UI/RX/foreground binding, hardware adapter, outgoing ID allocation, relay
paths, position/quick-message editing and RF validation remain outstanding.
