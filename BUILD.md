# Building the UVK5 experimental VUURWERK APRS fork

Run commands from the checkout root. The default build targets the original
DP32G030's Cortex-M0, 60 KiB application flash and 16 KiB RAM. It does not target
newer MCU variants. No firmware flashing is performed by the build scripts.

The complete APRS candidate is selected by `tools/build-transport.sh`; the
minimal receive-only build remains `tools/build.sh`. The complete profile now
fits 61,440 bytes with no growth margin. See [FINAL-FIT.md](docs/FINAL-FIT.md)
and the [candidate package](release/aprs-candidate/README.md).

## Reusable container toolchain

```bash
./tools/setup-toolchain.sh
./tools/test.sh
./tools/build-transport.sh
```

The setup script uses a digest-pinned Ubuntu 22.04 image and signed Ubuntu
packages. ARM GCC is pinned to **10.3.1**, with pinned ARM binutils and newlib.
QEMU supplies the Cortex-M0 arithmetic/ABI checks; it does not model the radio.
The container image is local, contains no project checkout, and is never pushed
to a registry. Existing matching toolchain images are reused. `UVK6_BUILD_IMAGE`
overrides the local image tag, default `uvk6-build:jammy`.

Setup deliberately uses a normal container followed by a local Docker commit.
This also works in the cloud environment where BuildKit's network could not
resolve Ubuntu package hosts. On a host with working BuildKit networking, the
same installation instructions are available as:

```bash
docker build -t uvk6-build:jammy -f tools/Dockerfile .
```

Host GCC is required for `tools/test.sh`; tests run with AddressSanitizer and
UndefinedBehaviorSanitizer by default. `SANITIZE=0` is available for hosts without
sanitizer support, but those runs do not validate sanitizer coverage.

## Native toolchain

ARM GCC 10.3.1, ARM binutils, newlib-nano, Make, Python 3 and `crcmod` must be on
PATH, along with `qemu-system-arm` for CPU-helper tests. Then run the same
test/build scripts. `UVK6_USE_DOCKER=1` forces Docker even
when a native ARM compiler is present. The script builds with four jobs and
always cleans first, since upstream feature flags are not encoded in object
filenames.

## Outputs and enforced checks

- `uvk6-experimental`: ELF with symbols.
- `uvk6-experimental.bin`: raw application image, at most **61,440 bytes**.
- `uvk6-experimental.packed.bin`: updater image with version header and CRC.
- `uvk6-experimental.map`: linker map.
- `build/verification.json`: exact size, RAM reservations, tool version and SHA-256 hashes.

The linker reserves at least **2 KiB** for stack; this is a static reservation,
not a measurement of maximum runtime stack usage. The verifier checks the
initial SRAM stack pointer, Thumb reset vector, ARMv6S-M / Thumb-1 attributes,
flash budget, RAM budget and updater CRC, and confirms that unpacking recovers
the exact raw application image. Packing errors fail the build.

Use `./tools/build.sh ENABLE_...=...` to experiment with feature flags. Those
variants need separate validation and may overflow flash; default-profile test
results do not apply to arbitrary flag combinations. The archived upstream
Arch Dockerfile and build instructions are in `docs/upstream/`; their byte
counts describe upstream, not this fork.

## Messaging development candidate

`ENABLE_APRS_MESSAGING=1` adds the approved editor flow without the full data-TX
runtime. It is not the requested complete firmware; historical editor-only
overflow measurements are not current fit bounds after the size work.
`tools/test.sh` nevertheless exercises its actual sources and framebuffer paths
on the host with sanitizers. To measure its current overflow in an isolated
nonflashable diagnostic copy, run:

```bash
python3 tools/measure-features.py --profile messaging_editor
```

See `docs/MESSAGING-IMPLEMENTATION.md`. Do not enlarge the production linker
region to bypass the physical flash limit.

## Receive-session bench package

`./tools/build-receiver-bench.sh` builds `ENABLE_APRS_SESSION=1` with composition
still in development (`ENABLE_APRS_MESSAGING=0`). It pins all other boolean flags
to the source revision's defaults and creates `build/receiver-bench/` with raw and
packed images, map, verification and source/feature/hash manifest. This command
never flashes hardware. An owner-controlled bench installation still requires
identifying the original DP32G030 and preserving EEPROM/calibration/firmware and
recovery access. See `release/rx-session-bench/README.md`.

Current raw size: **61,388 / 61,440 bytes**, 52 free. Static RAM: **7,996 bytes**;
2,048-byte linker stack reservation. This profile adds station directory/details,
personal inbox, all-traffic monitor and own-callsign entry to the approved home.
The normal default `tools/build.sh` remains the 58,056-byte four-packet receiver.
Full `ENABLE_APRS_MESSAGING=1` remains an oversized development candidate. Session
UI is separated from the unfinished composer so real receive testing can proceed
without changing the final transmit goal or cutting inherited radio features.

Verify the distributed bench binaries against a fresh archive of the manifest's
exact source commit with `python3 tools/verify-bench-release.py`. The source
commit must exist locally (CI fetches full history). This rebuild compares both
raw and packed images byte-for-byte, leaves the working checkout intact, and
never creates a Git worktree or flashes hardware. It uses the manifest's flags
rather than the current development profile.
