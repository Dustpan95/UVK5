# APRS receive investigation — hardware evidence

The owner has flashed the full R2 candidate and D1–D3 diagnostic images locally.
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

The uploaded D2/D3 raw dumps agree with their JSON record lengths, sequence
numbers, completion reasons and, for D3, modem-mode tags. A completed capture
establishes FIFO activity, not that every collected bit came from the beacon.
The mode comparisons have not demonstrated a working receive configuration.
They do not establish that the BK4819 is incapable of Bell 202 reception.
See [D3 settings and instructions](../release/rx-diagnostics-d3/README.md).

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

## Next discriminating test

Record the same beacon through one of the owner's other receivers, using the
same phone and recording method. An independently decodable comparison recording
would establish that this method can preserve the packet, helping isolate the
K6 receive/audio path. If the comparison also fails, improve or bypass the
acoustic recording path before using it to judge the K6 modem.

The current evidence does not justify selecting a new production modem setting
or describing another speculative diagnostic build as a decoding fix. Further
changes should follow evidence that distinguishes the receive/audio path,
synchronization, FIFO servicing and software framing. The complete R2 binary
remains unchanged by D3 instrumentation; hardware RX and TX validation remain
outstanding.
