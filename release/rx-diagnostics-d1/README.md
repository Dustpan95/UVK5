# D1 — temporary K6 APRS receive diagnostics

This image instruments the current receiver to locate the reported decode failure.
The owner reports that revision 2 boots, its home/controls/channels/voice receive
checks pass, APRS is enabled and a clear beacon burst is audible, but no packet
appears. These are owner observations, not independently measured RF results.

Use **uvk6-rx-diagnostics-d1.packed.bin** with the same updater procedure, with all
optional updater patches disabled. The supported target remains original
DP32G030. The [manifest](manifest.json) records the exact source/options/hashes.
The cloud tools never flash a radio.

**This is a temporary receive-diagnostic profile.** Ordinary voice operation and
the packet browser remain. APRS data transmit, message composition, session
station directory and personal inbox are absent from this image to leave room
for instrumentation. The [full revision 2 candidate](../aprs-candidate/README.md)
remains separately available. This is not a feature-cut decision for the project.
No modem tuning, FIFO ordering, capture length or decoding rule is changed in D1.
The instrumentation reads registers and counts existing receive events.

## Collect the K6 evidence

1. Flash D1, then boot normally. APRS settings reset at power-off.
2. Set **MENU → RECEIVE → APRS → ON** and **RxMode → MAIN ONLY**. Stop scanning.
   Use the active VFO on your beacon channel in FM. Keep the conditions under
   which you already hear the clear packet burst.
3. Press **F, then 5**. The first page is **APRS RX DIAG D1**. It refreshes every
   half second. Record its values before a beacon, send a beacon, then record
   them again after about three seconds. Counters are cumulative for this boot.
4. Press **MENU** once for **APRS REG / RAW**. Record this page too. All displayed
   register values are decimal; the RAW bytes are hexadecimal.
5. MENU again opens the ordinary packet log; MENU again returns to diagnostics.
   EXIT returns home. PTT retains the existing voice-transmit path.

Send the two pages' values or photos after the beacon. Sender model/software is
not required for this first capture-stage diagnosis. If the K6 still hears the
burst but counters do not advance, those unchanged readings are useful evidence.

## Readouts

| Field | Meaning |
|---|---|
| EN | APRS receive switch enabled |
| RD | Current radio mode permits APRS receive |
| AR | Firmware considers the modem armed |
| CP | Capture currently in progress |
| S / F / E | Sync / FIFO threshold / RX-finished interrupts seen by the APRS handler |
| B / L / X | Current capture bytes / last decoded capture bytes / full 240-byte captures |
| OK / BAD / TO | Accepted packets / failed decode attempts / timeout-ended captures |
| FL / FR / CRC | HDLC flags / complete frame candidates / candidates passing the FCS |
| 58 / 59 / 3F | Live BK4819 receive-mode, engine-control and interrupt-mask registers |
| I | Last interrupt bits seen while the modem was armed and eligible |
| RAW (two rows) | First eight bytes of the last decode attempt |

Counters are unsigned 16-bit and wrap; compare nearby before/after observations.
AR is firmware state; register readbacks provide the independent chip-state view.
A label or readback alone does not prove that the chip demodulated Bell 202.

## Software checks and reproduction

The actual receiver/decoder passes simulated valid FIFO captures, a corrupted
frame rejected by CRC, timeout/short-capture handling, mode-gate changes and
read-only register snapshots. All three UI pages and MENU/EXIT/PTT routing pass
sanitizer tests. The complete normal candidate rebuilds unchanged when diagnostics
are disabled. The full software suite passes; D1 has not been run on a radio.

Run `./tools/test-rx-diagnostics.sh` and `./tools/build-rx-diagnostics.sh` from the
manifest's source revision. The image retains upstream notices/licenses and the
same original project credits: see [NOTICE](../../NOTICE) and [LICENSES](../../LICENSES).
