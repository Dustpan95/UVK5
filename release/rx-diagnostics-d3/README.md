# D3 — controlled APRS receive modem comparison

The owner's D2 export contains three full 240-byte captures. Their lengths,
sequence numbers and raw dump agree. An independent decoder found no valid FCS
under 12 combinations of byte order, bit order and NRZI/direct/inverted coding.
This rules out those simple interpretations for these captures; it does not
identify the hardware root cause or prove that captured bits represent the beacon.
The uploaded raw captures are not redistributed here.

D3 adds four receive configurations, selectable without reflashing. These are
controlled hypotheses, **not a verified APRS fix**. The BK4819 Registers V1.1
REG_58 definitions specify receive modes and bandwidths for 1200/1800 and
1200/2400 FFSK; neither is expressly documented as Bell 202 1200/2200.
REG_71's 2200 Hz setting alone does not establish receiver compatibility.

| Mode | RX mode bits 12:10 | Bandwidth bits 3:1 | REG_58 |
|---|---|---|---|
| A (default; same as D2) | 7: 1200/1800 | 1: 1200/1800 | 0x3FC3 |
| B | 7: 1200/1800 | 4: 1200/2400 | 0x3FC9 |
| C | 4: 1200/2400 | 4: 1200/2400 | 0x33C9 |
| D | 4: 1200/2400 | 1: 1200/1800 | 0x33C3 |

Key 2 advances A → B → C → D → A, stopping the receiver, dropping its partial
capture, restoring saved radio registers, then rearming if eligible. It does
not change frequency, transmit or write EEPROM. Completed captures remain,
with the mode tagged per record. Reading freezes the last-four capture ring;
live counters/reception continue. Key 1 clears/unfreezes only the stored ring.
COLLECT / HELD replaces the ambiguous H0 / H1 display. Counters remain cumulative.
Settings and mode reset at power-off.

## Windows test

1. Download and Extract All **uvk6-rx-diagnostics-d3.zip**. Flash the extracted
   **uvk6-rx-diagnostics-d3.packed.bin** using the same updater, optional
   modifications disabled. Boot normally.
2. Enable APRS ON; use MAIN ONLY, scanning stopped and the active FM beacon
   channel where the packet burst is audible. Press F then 5. Confirm D3.
3. Press **2 once for B**, then **1**. Confirm **B COLLECT**. Send a beacon
   and wait three seconds. Note whether OK increases or a packet appears.
4. Close the updater/CHIRP. Open the extracted **read-rx-capture.html** in
   desktop Chrome/Edge. Connect the programming cable, Read captures, Save JSON.
   Rename the saved file to **mode-B.json**.
5. If no decode, press **2 for C**, then **1**; repeat the beacon and export,
   saving **mode-C.json**. Send both JSON files and any successful decode result.
   The reader can stay connected. Mode D is available if further comparison is needed.

MENU cycles statistics, registers/raw and packet log. EXIT returns home.
Do not judge reception from COLLECT changing to HELD: only reading the export
sets HELD. Pressing 1 does not reset counters or require an on-screen count to change.

The reader accepts D2 and D3 exports. Wire format v2 retains the 16-byte header
and four 248-byte records, using previously reserved record byte 7 for mode 0–3.
The JSON container remains UVK6_RX_CAPTURE_V1 with added wire_version,
modem_mode and reg58_configured fields. The latter is the selected configuration,
not a hardware readback; the register screen separately shows live REG_58.
Only bounded diagnostic RAM-read commands are sent. Data stays local until saved.

## Scope and verification

This temporary profile omits APRS TX, message composition, session station
directory and personal inbox, like D2. Voice functions and packet log remain.
The full R2 candidate is separate and its binary remains unchanged.

Host ASan/UBSan tests exercise all four register configurations, mode changes
while ineligible, held-key suppression, capture tagging, valid/corrupt packets,
timeouts, wrap, frozen exports, bounds, UART framing and the browser protocol.
Strict pinned ARM builds check vectors, application flash, updater size and RAM
with a reserved 2 KiB stack. Software fixtures cannot establish RF decoding.
D3 requires the owner's physical beacon test before claiming any improvement.

Reproduce from the manifest's source revision:

```sh
./tools/test.sh
./tools/build-rx-diagnostics.sh
```

Outputs: build/rx-diagnostics-d3/. Compiler: ARM GCC 10.3.1.
Original project copyright notices, credits and licenses remain intact.
