# APRS receive investigation — hardware evidence

The owner has flashed the full R2 candidate and D1–D4 diagnostic images locally.
Voice reception works and a packet burst is audible, but no successful on-radio
APRS decode has been reported. This remains an unresolved receive failure.
No cloud tool has connected to or flashed the radio.

## Capture evidence

| Image / setting | Owner evidence | Offline result |
|---|---|---|
| D2 / A | Three complete 240-byte captures | No valid FCS under 12 byte/bit/coding interpretations per capture |
| D3 / B | One complete 240-byte capture | No valid FCS under the same 12 interpretations |
| D3 / C | Owner reports no captures stored | Eligibility indicators were not supplied with this report; cause unknown |
| D3 / D | One complete 240-byte capture | No valid FCS under the same 12 interpretations |
| D4 / B:REF | Owner reports BAD=4, L240, FL23, FR0, CRC0; live register settings match reference | No B-profile raw captures in the supplied D4 file |
| D4 / C:FLT | Owner reports BAD increased to 5 after a beacon | No C-profile raw captures in the supplied D4 file |
| D4 / D:GAIN | Owner reports BAD increased to 6; four full 240-byte captures exported | Both production decoders reject all four; no valid FCS under the same 12 interpretations |

The uploaded D2/D3 raw dumps agree with their JSON record lengths, sequence
numbers, completion reasons and, for D3, modem-mode tags. A completed capture
establishes FIFO activity, not that every collected bit came from the beacon.
The mode comparisons have not demonstrated a working receive configuration.
They do not establish that the BK4819 is incapable of Bell 202 reception.
See [D3 settings and instructions](../release/rx-diagnostics-d3/README.md).

## D4 owner capture analysis

The D4 export is a valid 1,104-byte wire-v3 dump with four records, sequences
1–4, all tagged D:GAIN and completed by RX_FINISHED. JSON records agree with
the binary dump, including lengths, profile tags, reasons and actual register
snapshots. This file contains no B:REF or C:FLT raw captures; counter reports
for those profiles are separate evidence.

All four snapshots show REG_58=0x3FC3, REG_59=0x1000, REG_43=0x49A8,
REG_13=0x03BE, REG_7E=0x37EE, REG_70=0x80E0, REG_71=22714,
REG_72=12389 and REG_3F=0x3C0E. These match D:GAIN: baseline filtering,
stock gain, hardware AGC and enabled FSK interrupts. Earlier live B:REF
readings independently confirm its reference filter REG_43=0x3628, stock
gain and hardware AGC; applied reference settings did not yield a decode.

The offline analyzer finds no valid FCS under 12 byte/bit/coding interpretations
per capture. Both the actual UVK5 and pinned TA1JS production decoders reject
all four. Known-good completion/stall fixtures pass and corrupt/truncated
fixtures fail in the same ASan/UBSan run. This adds four unsuccessful captures
to the original five; private raw data, station information and audio are not
published.

Completion-time REG_67 readings are 114–120, not peak measurements during
a beacon. Captures have no timestamps or signal-presence annotations. The
four-slot ring overwrites older records until a read freezes storage, while
live counters continue even when storage is held. Full buffers and flag-like
patterns therefore do not establish that the saved records contain the intended
beacon. Background false captures and overwrite remain unconfirmed hypotheses.
The next observation is whether BAD increases with the test transmitter quiet,
followed if needed by a promptly frozen/exported controlled beacon capture.
This export alone does not justify another modem/filter change.

## Independent test of the K6 speaker recording

The owner supplied an approximately 3.3-second stereo AAC recording containing
one packet burst from the K6 speaker. It was decoded to 48 kHz, signed 16-bit
PCM without normalization, filtering or noise reduction. A canonical WAV header
was written because Dire Wolf's test reader rejects FFmpeg's extra LIST chunk.

Dire Wolf `atest -F 0` reports **zero valid packets** for each of:

- Left channel, right channel and their mono downmix.
- Standard 1200-baud / 1200–2200 Hz profiles E, E+ and E-.

Thus all nine channel/profile combinations fail with bit repair disabled. A
known-good synthetic Bell 202 position waveform decodes correctly with the same
binary, `-F 0`, and default profile. A separate one-bit-repair diagnostic also
fails on both original channels; repaired data is never accepted as validation.

The decoded source channels contain no samples at digital clipping level.
Short-time frequency analysis is consistent with energy near 1200 and 2200 Hz;
this acoustic recording is not a calibrated tone/deviation measurement. Lack of
digital clipping does not exclude RF, analog/audio or microphone processing
distortion. Stereo channels have different acoustic responses; testing both
avoids relying on a possibly destructive downmix.

This result does **not** prove the transmitted packet is invalid or identify a
firmware root cause. It also does not demonstrate that clean, independently
decodable audio reached the K6's internal modem. The K6 receive path, speaker,
room, phone microphone and AAC processing are all part of the recorded path.
The owner's audio, raw captures and any station information are not published.

## Source-informed receive comparison

The [TA1JS production-source comparison](TA1JS-RX-COMPARISON.md) reproduces
both actual receive paths: modem-arm writes match, valid/stalled fixtures pass,
and both decoders reject all five saved captures. Differences in REG_43 RF/IF
filtering and software FM gain staging motivate the controlled
[D4 reference profiles](../release/rx-diagnostics-d4/README.md). The owner has now tested the D4 profiles without a successful decode;
the reference settings remain an unsuccessful hardware-test hypothesis.

## Additional discriminating test

Record the same beacon through one of the owner's other receivers, using the
same phone and recording method. An independently decodable comparison recording
would establish that this method can preserve the packet, helping isolate the
K6 receive/audio path. If the comparison also fails, improve or bypass the
acoustic recording path before using it to judge the K6 modem.

The current evidence does not justify selecting a new production modem setting
or describing D4 as a decoding fix. Further
changes should follow evidence that distinguishes the receive/audio path,
synchronization, FIFO servicing and software framing. The complete R2 binary
remains unchanged by D3 instrumentation; hardware RX and TX validation remain
outstanding.
