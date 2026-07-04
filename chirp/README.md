# CHIRP Support for VUURWERK

`uvk5_vuurwerk.py` is a community-contributed CHIRP driver that lets you read
and write your VUURWERK radio's channels, side-keys, and settings directly from
[CHIRP](https://chirpmyradio.com) -- no need to flash back to stock egzumer
first.

## What it does

- **Reads and writes:** memory channels (frequency, offset, CTCSS/DCS,
  modulation, power, bandwidth, step, scrambler, channel names, scan-list
  membership, compander), the programmable side-keys (SIDE1/SIDE2 short and
  long press) and the M-key long-press, and the settings block (squelch, VOX,
  mic gain, backlight, dual-watch, crossband, battery save, STE, scan-resume,
  key-lock, DTMF contacts, FM broadcast presets, S-meter levels, power-on
  options, and the F-LOCK/unlock settings).
- **Does NOT handle the spectrum analyzer settings.** VUURWERK's spectrum
  analyzer / bandscope configuration is not part of the CHIRP image and is
  neither read nor written by this driver. (An optional, off-by-default
  calibration upload is available for advanced users.)

It works because VUURWERK identifies itself honestly over UART, and the driver
accepts firmware identification strings beginning with `VUURWERK` (and
`EGZUMER `). No firmware change is needed.

## How to load it

CHIRP loads this as a Developer-mode module:

1. Open CHIRP.
2. Enable **Developer mode** in CHIRP's settings, then **restart CHIRP**.
3. **File > Load Module**, and select `uvk5_vuurwerk.py`.
4. Connect the radio with a Kenwood-style 2-pin programming cable.
5. **Radio > Download from Radio**, then pick **Quansheng**, then **UV-K5** in
   the radio dropdown.

Read your radio, edit channels/settings, and write back.

## Credit and license

CHIRP driver contributed by mjwgeek (github.com/mjwgeek), adapted from the
EGZUMER driver which is based on Jacek Lipkowski SQ5BPF's original UV-K5 driver.

It is GPL -- the file carries its own GPL header (GNU General Public License v2
or, at your option, any later version). It is the contributor's work, shipped
here as-is with credit; improvements will come later with mjwgeek's involvement.
