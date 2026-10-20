# D2 — full APRS receive capture export

D1 hardware photos show receive eligible/armed, complete 240-byte captures,
15 flag detections, zero complete frame candidates and zero valid checksums.
This establishes capture activity and a framing-stage failure. It does not
establish that every observed bit came from the beacon or identify the cause.
The eight-byte prefix shown by D1 is insufficient to analyze a complete packet.

D2 keeps the current modem configuration and decoding rules. It stores the last
four complete/timeout captures in RAM and exports them through the existing
programming cable. The browser reader only sends diagnostic RAM-read requests;
it sends no EEPROM, calibration, firmware, tuning or transmit commands. Data
stays local until you explicitly save/upload the JSON file.

**Temporary diagnostic profile:** APRS data TX, composition, session station
directory and personal inbox remain absent, as in D1. Standard voice operation,
packet log and diagnostics remain. The [full candidate](../aprs-candidate/README.md)
is separate. The diagnostic-only changes are compiled out of it; a clean full
build remains byte-for-byte identical to published revision 2.

Download **uvk6-rx-diagnostics-d2.zip** and use Windows **Extract All**. It contains
the firmware, reader, manifest, instructions and file checksums.

## Windows collection steps

1. Use **uvk6-rx-diagnostics-d2.packed.bin** from the extracted ZIP and flash it through the same
   updater, with optional modifications disabled. Boot normally afterward.
2. Enable **APRS ON**, use **MAIN ONLY** with scanning stopped and the active
   FM beacon channel under the conditions where you hear the clear burst.
3. Press **F then 5**. Confirm **APRS RX DIAG D2**. Press **1** to collect fresh
   captures, transmit your beacon, then wait approximately three seconds.
4. Close the updater and CHIRP so they release the programming cable's COM port.
5. Find **read-rx-capture.html** in the extracted folder. Open that file
   in desktop Chrome or Edge, the browser used for flashing. No Python or other
   installation is required. The page is self-contained and works offline.
6. Click **Connect cable**, choose the programming cable, then **Read captures**.
   The radio stays in normal operation, not bootloader mode. Reading freezes
   the stored four-capture ring; live reception/counters continue.
7. Click **Save JSON** and attach `uvk6-d2-captures.json` in the chat.

For another collection, press **1 on the radio** before transmitting the next
beacon. This clears/unfreezes only the diagnostic capture ring. `H1` means that
stored data is frozen; `H0` means it is collecting. MENU still cycles diagnostic,
register/raw and packet-log pages; EXIT returns home. Counters are cumulative
and unrelated to clearing the stored ring. APRS settings reset at power-off.

The reader tries the existing XOR and plain serial modes automatically. It does
not send the ordinary programming-session initialization that suspends APRS.
If there is no D2 reply, check D2 is installed, normal power-on mode, the selected
COM port, and that other serial applications are closed. No captures means the
ring was cleared but no capture completed afterward; repeat step 3.

## Verification and reproduction

Strict ARM/flash/RAM/vector/updater checks pass. Host ASan/UBSan tests cover
real receiver/decoder counters, known valid and corrupted captures, short-capture
timeouts, four-slot ring wrap, frozen exports, bounds and read-only snapshots.
The actual UART parser/handler/serializer passes 1,543 cases of encrypted/plain
requests, partial packets, DMA wrap, CRC rejection and malformed command bounds.
The exact browser protocol passes 2,528 fragmented reply cases plus capture-header
validation; its UI/physical USB connection has not been tested on the owner's PC.
The independent offline analyzer tests byte/bit orders, coding and explicit
post-sync phase hypotheses, requiring valid FCS before reporting a valid frame.
The full software suite passes. D2's physical radio behavior remains untested.

Use the manifest's source revision:

```bash
./tools/test.sh
./tools/build-rx-diagnostics.sh
```

Host tests now also require Node.js for the browser-protocol tests. The pinned
ARM compiler remains 10.3.1. Outputs are in `build/rx-diagnostics-d2/`.
`tools/analyze-rx-capture.py` analyzes the saved JSON offline. The protocol is
versioned, bounded diagnostic RAM only: command 0x0540 / reply 0x0541. Ring format
is a 16-byte header plus four 248-byte records (sequence, length, completion reason,
240 raw bytes). Original copyrights, credits and licenses remain intact.
