# D4 — TA1JS receive filter and gain comparison

This temporary diagnostic build follows a
[production-source comparison with TA1JS](../../docs/TA1JS-RX-COMPARISON.md).
No successful hardware APRS decode has yet been established on the owner's K6.
D4 is **not a verified decoding fix**. There is no need to flash it until the
owner is back at the radio.

## Profiles

All profiles keep the same production modem REG_58=0x3FC3. Key **2** cycles:

| Profile | RF/IF filter | FM gain control |
|---|---|---|
| A:BASE (boot default) | Existing VUURWERK values | Existing software gain staging |
| B:REF | TA1JS/F4HWN values | Stock hardware AGC; software staging yields |
| C:FLT | TA1JS/F4HWN values | Existing software gain staging |
| D:GAIN | Existing VUURWERK values | Stock hardware AGC; software staging yields |

For WIDE, the filter changes REG_43 from 0x49A8 to 0x3628. Reference gain uses
REG_13=0x03BE and clears fixed-gain mode in REG_7E, with gain index 3 and unrelated
bits preserved. APRS saves/restores all changed registers when it releases the
receiver, including around voice TX, tuning and profile changes. This comparison
does not alter the full R2 candidate or permanently remove voice features.

## Later, when the radio is available

1. Extract **uvk6-rx-diagnostics-d4.zip**. Install the extracted
   **uvk6-rx-diagnostics-d4.packed.bin** with the same
   [Egzumer updater](https://egzumer.github.io/uvtools/?firmwareURL=), with
   optional modifications disabled. Boot normally.
2. Enable APRS; use MAIN ONLY, scanning stopped, the active FM beacon channel
   and WIDE. Open F then 5 and confirm **APRS RX DIAG D4**.
3. Press **2 once**, selecting **B:REF**, then **1** to clear/unfreeze storage.
   Send a beacon, wait three seconds, and check whether OK increases.
4. Close the updater/CHIRP. Open the extracted **read-rx-capture-d4.html** in
   desktop Chrome/Edge. Connect the programming cable, Read captures, Save JSON.
5. If needed, repeat with A:BASE, C:FLT and D:GAIN. Each saved record identifies
   its profile. Key 1 clears stored captures; it does not reset counters.

Reading holds the last four stored captures; live reception/counters continue.
Pressing 1 resumes storage. Key 2 discards any partial capture, preserves complete
records and rearms only if eligible. MENU cycles counters, live registers and
packet log; EXIT returns home. Live register values on the LCD are decimal.
A missing capture is not proof that the selected modem setting cannot decode.

## Capture compatibility

Use this package's uniquely named **read-rx-capture-d4.html**. It reads D2, D3
and D4; older readers cannot parse D4. JSON remains UVK6_RX_CAPTURE_V1, with
wire_version=3 for D4. The bounded RAM-read command remains unchanged.

Wire v3 has a 16-byte header and four 272-byte records (1,104 bytes total).
Each record has sequence:u32, length:u16, reason:u8, profile:u8,
12 little-endian register readbacks:u16, and 240 raw bytes. Snapshot order is
58, 59, 43, 13, 7E, 70, 71, 72, 3F, 0B, 67, 0C. Values are read at capture
completion before rearm; REG_67 is not a peak RSSI measurement. No FIFO register
is read by this snapshot. D4 profile letters are distinct from D3 modem modes;
the reader retains that distinction in exported fields.

## Scope and verification

Like D2/D3, this temporary image omits APRS TX, message composition, session
station directory and personal inbox. Voice functions and the packet log remain.
All settings and captures remain session-only. The reader sends only bounded
RAM-read requests, never EEPROM-write, tuning, transmit or firmware commands.

Host tests exercise actual gain staging against reference ownership, all three
filter bandwidths, register restoration, eligibility and held-key guards,
valid/corrupt captures, FIFO stalls, snapshots, ring bounds/freeze behavior,
UART framing and backward-compatible browser decoding. Pinned ARM GCC 10.3.1
build checks enforce the original DP32G030 flash/RAM limits and reserve 2 KiB
for stack. The real updater's unpacker/size checks run with serial mocked.
These software checks cannot establish physical RF reception.

Build from the source revision in manifest.json:

```sh
./tools/test.sh
./tools/build-rx-diagnostics.sh
```

Outputs are in `build/rx-diagnostics-d4/`. See verification.json and manifest.json
for exact sizes, hashes, profile settings and source. Preserve all original
project credits, copyright notices and licenses when redistributing.
