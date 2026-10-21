# Receive-path comparison with TA1JS

The owner hears APRS bursts but has not reported a valid packet on the K6.
The comparison below identifies a concrete next experiment, **not a confirmed
fix**. It does not require the owner to be at the radio while software work runs.

## Source and upstream evidence

Reviewed TA1JS main at
[`6320cda`](https://github.com/bcanata/uv-k5-firmware-ta1js/tree/6320cdac7a2dce46f88027d220ff67e47eb0e3fb).
Its [initial receive integration](https://github.com/bcanata/uv-k5-firmware-ta1js/commit/8f27a233dc9da747b0cebfb262dab4eb444ea881)
reports three necessary fixes: disable sleep while listening, preserve FSK
interrupt enables, and decode partial captures after FIFO stalls because
RX_FINISHED often does not arrive for real packets. A separate user reports
receiving a TH-D75 message in
[issue 1](https://github.com/bcanata/uv-k5-firmware-ta1js/issues/1).
These are upstream hardware reports, not validation of this project's radio.
The issue's initial TX-gain explanation was later corrected after tone
measurements; it is not evidence of a receive fix.

## What already matches

Both production receive paths program the same eleven modem-arm writes:
REG_70=0x80E0, 71=22714, 72=12389, 58=0x3FC3, 5A=0xFEFE,
5C=0xAA30, 5D=0xEF00, 5E=0x0204, 3F OR 0x3002, and
59=0x4000 followed by 0x1000. Both use a 240-byte capture, store FIFO words
low byte first, and decode MSB-first NRZI, HDLC stuffing and AX.25 FCS in software.
The upstream decoder's frame buffer is 80 bytes; ours is larger.

Both drain four words at FIFO threshold, eight at completion, and handle a
stalled synchronized capture after three 500 ms ticks. Despite upstream's
"streaming decoder" description, production decoding walks the collected
buffer at completion/stall, rather than decoding each incoming FIFO word.
Both prevent power save while APRS is enabled.

Interrupt ownership differs: TA1JS ORs FSK enables into radio setup and repairs
missing enables in its periodic task. UVK5 releases/snapshots/restores the shared
modem around radio setup, then rearms when eligible. No active interrupt-clobber
path has been established. D4 records REG_3F without adding an independent
interrupt-policy change to its four-profile comparison.

## Differences before the modem

| Receive setting | UVK5 baseline | TA1JS reference |
|---|---|---|
| REG_43 WIDE | 0x49A8 | 0x3628 |
| REG_43 NARROW | 0x4808 | 0x3648 |
| REG_43 NARROWER | 0x3658 | 0x1348 |
| FM gain control | VUURWERK software adjusts REG_13 every 10 ms as needed, alongside hardware AGC | Stock REG_13=0x03BE and hardware AGC |

These filter values use equal strong/weak-signal bandwidth, matching this
build's ENABLE_AM_FIX setup policy. TA1JS inherits the newer values from
[F4HWN's filter change](https://github.com/bcanata/uv-k5-firmware-ta1js/commit/c94ea17943d11c2078c39a041e49c4e0405e435d).
REG_43 controls RF/IF filtering; it is separate from the REG_58 modem bandwidth
combinations already tried in D3. The difference does not prove either filter
choice is faulty. Software gain adjustment during capture is another plausible
interaction, not an established cause.

[D4](../release/rx-diagnostics-d4/README.md) tests both differences together and
separately, retaining the original modem configuration throughout. It is a
controlled reference-settings comparison, not an exact copy of all TA1JS radio
behavior. Other receive/audio processing differences remain possible.

## Actual production-code comparison

`tools/compare-ta1js-rx.py` extracts eight production functions verbatim from the
pinned upstream git object, retains the copyright/license header, and compiles
them beside our actual RX/decoder modules with AddressSanitizer and UBSan.
Only UI, configuration and TX callbacks are stubbed for passive receive.
Register/FIFO fixtures do not simulate demodulation, analog filtering or timing.

Results:

- All eleven production arm writes match in value and order.
- Both paths decode the valid fixture through RX_FINISHED (240 bytes) and
  through the real stall path (72 bytes).
- Both reject the corrupted and truncated fixture.
- **Both reject all five owner-exported 240-byte captures**: three D2/A,
  one D3/B, one D3/D. Swapping in the upstream decoder alone cannot recover
  a valid frame from those captures.

Reproduce using a local clone containing the pinned commit:

```sh
git clone https://github.com/bcanata/uv-k5-firmware-ta1js.git /tmp/ta1js
python3 tools/compare-ta1js-rx.py /tmp/ta1js
# Optional: append private capture JSON paths for the owner-capture comparison.
```

The generated source, executable, function hashes and aggregate results are
written under ignored `build/ta1js-comparison/`. Private raw bytes, callsigns,
positions and audio are not published. Original TA1JS, F4HWN, Egzumer and
VUURWERK credits/licenses remain intact.
