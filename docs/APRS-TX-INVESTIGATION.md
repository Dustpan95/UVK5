# APRS transmit investigation

Standard-compatible APRS transmit is being investigated before publication,
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

The approved home and alert build is now 61,012 bytes, leaving 428. The older
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
