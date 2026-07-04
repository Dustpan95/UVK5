# Flashing VUURWERK v1.5.0

Flashing takes about a minute and does not touch your channels or settings --
only the firmware changes. This guide covers the recommended browser flasher,
a command-line alternative, and recovery.

## What you need

- A **Quansheng UV-K5 V1** (DP32G030 processor -- see the hardware check below)
- A standard **Kenwood-style 2-pin programming cable** (the same one used for
  CHIRP or any other UV-K5 programming)
- **Google Chrome** (the browser flasher uses the Web Serial API, which
  Firefox and Safari do not support)
- The firmware file:
  [`release/vuurwerk-v1.5.0.packed.bin`](release/vuurwerk-v1.5.0.packed.bin)

## The flashing flow

```
 +----------------+     +----------------+     +---------------------+
 | 1. DOWNLOAD    |     | 2. RADIO OFF   |     | 3. BOOTLOADER MODE  |
 | vuurwerk-v1.5.0| --> | rotary switch  | --> | hold PTT, then turn |
 | .packed.bin    |     | fully off      |     | the radio ON        |
 +----------------+     +----------------+     +----------+----------+
                                                          |
                                              flashlight LED lights up
                                              = bootloader is waiting
                                                          |
 +----------------+     +----------------+     +----------v----------+
 | 6. FLASH       |     | 5. PICK FILE   |     | 4. CONNECT          |
 | click "Flash   | <-- | "Select        | <-- | cable into radio    |
 | firmware",     |     | firmware file" |     | and computer; open  |
 | wait ~10 s     |     | -> .packed.bin |     | the flasher in      |
 +-------+--------+     +----------------+     | Chrome              |
         |                                     +---------------------+
         v
 +----------------+     +----------------+
 | 7. POWER CYCLE |     | 8. VERIFY      |
 | unplug, off,   | --> | boot banner:   |
 | wait 5 s, on   |     | VUURWERK v1.5.0|
 +----------------+     +----------------+
```

## Step by step (browser flasher -- recommended)

1. Download `vuurwerk-v1.5.0.packed.bin` from the `release/` folder.
2. Turn the radio off.
3. Hold **PTT** and turn the radio on while still holding it. The flashlight
   LED turns on -- that is the bootloader confirming it is ready.
4. Connect the programming cable to the radio and your computer.
5. Open **https://egzumer.github.io/uvtools/** in Chrome.
6. Click **"Select firmware file"** and choose the `.packed.bin` you
   downloaded.
7. Click **"Flash firmware"**. The transfer takes about 10 seconds.
8. Disconnect the cable, power the radio off and on.

You should see the boot banner:

```
+----------------------------+
|         VUURWERK           |
|                            |
|                            |
|          v1.5.0            |
|                            |
|        Precision RF        |
+----------------------------+
```

You're done.

## Command-line alternative (k5prog)

If you prefer a terminal or cannot use Chrome,
[k5prog](https://github.com/sq5bpf/k5prog) works with the same bootloader
mode (steps 2-4 above). Note the file formats:

- `vuurwerk-v1.5.0.packed.bin` (what `release/` ships) is the Quansheng
  updater format -- obfuscated, with an embedded version header and CRC. This
  is what the **browser flasher** wants.
- `vuurwerk-v1.5.0.bin` is the raw flash image, produced when you build from
  source (see [BUILD.md](BUILD.md)). This is what **k5prog** wants.

```bash
# back up the current firmware first (recommended)
k5prog -r backup.bin

# flash the RAW image from a source build
k5prog -F -YYY -b vuurwerk-v1.5.0.bin
```

If you only have the packed release file, use the browser flasher -- it is
the supported path for release binaries.

## Hardware check -- V1 only

VUURWERK supports only the original **UV-K5 V1** (DP32G030). The V2
(PY32F030) and V3 (PY32F071) revisions will not boot this firmware. To check:
remove the battery and read the board label under it. V2/V3 boards are marked
with their revision; **no marking means V1**, which is the supported radio.

Flashing the wrong hardware does not physically damage it, but you will need
recovery firmware for your specific board:
[K5TOOL](https://github.com/qrp73/K5TOOL) supports all UV-K5 versions.

## Troubleshooting

**The flashlight LED does not light in step 3.**
Make sure the radio was fully off first, and that you press and hold PTT
*before* turning the rotary switch. Try again slowly.

**Chrome does not show a serial port.**
Use a different USB port or cable -- charge-only cables and marginal clones
are the usual cause. Close any other program that may hold the port open
(CHIRP, a terminal). On Linux, ensure your user can access serial devices
(usually the `dialout` group).

**Flashing fails partway.**
Power the radio off, re-enter bootloader mode, and flash again. The
bootloader itself is never overwritten, so a failed application flash is
always retryable -- do not panic, just repeat the steps.

**The radio boots but shows a fault banner (BK4819 FAULT / EEPROM FAULT).**
That is VUURWERK's boot health probe reporting a hardware condition, not a
bad flash. See the Boot-Time Diagnostics section of
[USER_MANUAL.md](USER_MANUAL.md).

**One VFO seems deaf after flashing an older build previously.**
Press **F then EXIT** on that VFO (frequency mode). This resets it to a clean
simplex state and clears any leftover tone, offset, reverse, or scrambler
configuration. Toast: `SIMPLEX`.

## Reverting to stock

Flash your `backup.bin` (if you made one with k5prog), or download the stock
Quansheng firmware / an egzumer release and flash it with the same procedure.
The browser flasher accepts those packed images too.

## Notes

- Do not disconnect the cable while the progress bar is moving.
- Your channels, settings, and calibration are preserved across the flash.
- Every release file in `release/` has its SHA-256 recorded in
  [CHANGELOG.md](CHANGELOG.md) if you want to verify your download.
