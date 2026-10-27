# UVK5 backup and installation preparation

The current [outgoing APRS candidate](release/tx-only/README.md) removes APRS
receive. It fits the original MCU; physical validation of the revised image is
pending. Earlier R2/D1–D4 packages are historical. This guide does not flash a
radio. The [inherited VUURWERK v1.5.0 instructions](docs/upstream/FLASHING-vuurwerk-v1.5.0.md)
are retained for provenance and are not the installation procedure for this project.

## Connection and hardware

The original UV-K6's USB-C socket supplies charging power, not the firmware
programming interface. Use a USB serial programming adapter explicitly compatible
with the UV-K5/UV-K6 and its Kenwood-style two-pin speaker/microphone sockets.
A passive USB audio cable is not a substitute. Seat the two-pin plug fully;
install the driver appropriate to the adapter's actual USB serial chip.

Confirm reliable MCU/board-revision evidence for **DP32G030** before considering
an original-MCU image. A UV-K6 label, Egzumer v0.21 version string, or absence of
a revision sticker does not by itself establish the MCU. No connected device's
identity has been checked here.

## Preserve the existing radio before installation

1. With the existing Egzumer firmware still running normally, save a channel and
   settings image using its matching CHIRP driver. Keep the original file untouched.
2. Read the **full 8,192-byte EEPROM**, including the device-specific calibration
   area at `0x1E00..0x1FFF`, twice into distinct files. A channel CSV export is not
   a calibration backup. Keep copies outside the computer used for programming.
3. Compare both raw dumps and record their SHA-256 hashes. Keep the firmware
   version, radio/board evidence and cable details with them. Confirm a recovery
   method and matching known firmware image; retain a current firmware dump if
   the chosen tool actually supports reading it.

For a Linux/POSIX host, [SQ5BPF's k5prog](https://github.com/sq5bpf/k5prog)
provides a read-only EEPROM operation. Its documented `-r` operation reads EEPROM,
**not firmware flash**. Select the actual serial port, use unique output names,
and run these with the radio in normal operation, not bootloader mode:

```bash
./k5prog -p /dev/ttyUSB0 -r -f uvk6-before-1.eeprom.bin
./k5prog -p /dev/ttyUSB0 -r -f uvk6-before-2.eeprom.bin
python3 tools/check-eeprom-backups.py uvk6-before-1.eeprom.bin uvk6-before-2.eeprom.bin
```

These commands contain no EEPROM-write or firmware-flash operation. The upstream
README and source were checked: `-f` selects the EEPROM file and `-r` reads all
`0x2000` bytes. Windows/macOS port names and installation differ; choose the host
instructions for the actual computer. The checker only reads local files. Equal
size/hash and nonuniform calibration bytes do not prove every value is valid or
that recovery has been tested. No backup has yet been obtained from the owner's radio.

## Windows PC: CHIRP download procedure

Use the current Windows build from [the official CHIRP download page](https://chirpmyradio.com/projects/chirp/wiki/Download).
With a compatible two-pin cable attached, check Windows Device Manager for its
actual COM port and USB serial driver. The radio's charging USB-C connection will
not provide this programming port.

1. Keep the existing Egzumer firmware running normally; this is not bootloader mode.
2. In CHIRP choose **Radio → Download from Radio**, select the actual COM port,
   Quansheng and the UV-K5 Egzumer variant (or let compatible auto-detection select
   it). Confirm the detected firmware matches the radio. An original UV-K6 uses
   this driver family; this selection alone does not identify its physical MCU.
3. Use **File → Save As** to save `uvk6-before-1.img`. Save the actual radio image,
   not a channel CSV export, and do not edit the downloaded settings first.
4. Start a new **Download from Radio** and save `uvk6-before-2.img`. Two copies of
   one download do not test read consistency. Preserve both files elsewhere too.
5. If Python 3 is available on Windows, run the local file checker:

   ```powershell
   py tools/check-eeprom-backups.py --chirp uvk6-before-1.img uvk6-before-2.img
   ```

The official CHIRP UV-K5 download path reads all `0x2000` EEPROM bytes, including
calibration. The Egzumer driver uses that full download. The **Upload calibration**
setting governs writes; enabling it is not needed for downloading calibration.
The `.img` file appends CHIRP metadata after those 8192 bytes, so its total size can
be larger. `--chirp` validates the known trailer and compares only the EEPROM
payload; it never strips or overwrites the originals. This was checked against
[CHIRP source](https://github.com/kk7ds/chirp/blob/ec452b25de84be125ce933a79115d5428f7f2ba8/chirp/drivers/uvk5.py)
and its image serializer, not against the owner's physical radio.

These are download/save operations. Keep uploading and firmware flashing for the
later installation step after backup, hardware and recovery checks are complete.

## Owner-controlled experimental installation

After hardware identification, backups and recovery preparation, follow the
specific [current candidate instructions](release/tx-only/README.md) and
[physical test plan](docs/HARDWARE-VALIDATION.md). Match the image's manifest and
hashes. Use its packed updater image only with a compatible local updater.
The old VUURWERK filename/banner in the archived guide is a different build.

The cloud build/test/package commands never flash hardware. Physical boot,
EEPROM compatibility and RF behavior still require owner-controlled testing.
Preserving the bootloader is not a guarantee of recovery from every failure.
