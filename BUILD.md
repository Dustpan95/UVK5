# Building VUURWERK v1.5.0 from Source

VUURWERK builds like its egzumer parent: an ARM GCC cross-toolchain and GNU
Make. The **canonical build** -- the one release binaries and byte counts are
quoted from -- runs inside the repo's Docker image, so every machine produces
the same bytes.

## The build flow

```
 +-------------+     +----------------+     +----------------------+
 | source tree |     | docker build   |     | make (inside image)  |
 | (this repo) | --> | -t uvk5 .      | --> | arm-none-eabi-gcc    |
 +-------------+     | Arch + ARM GCC |     | -Os, LTO, -Werror    |
                     +----------------+     +----------+-----------+
                                                       |
        +-------------------------+--------------------+
        v                         v                    v
  vuurwerk-v1.5.0        vuurwerk-v1.5.0.bin    vuurwerk-v1.5.0.packed.bin
  (ELF: size, debug)     (raw image: k5prog)    (for the web flasher)
```

The `.packed.bin` is produced by `fw-pack.py` (obfuscation + version header +
CRC, the Quansheng updater format) and requires Python with the `crcmod`
module -- the Docker image already has it.

## Canonical build (Docker)

Requires Docker (on Windows, Docker inside WSL Ubuntu works well -- run these
commands from the WSL shell).

```bash
# 1. IMPORTANT: purge stale objects first. The image COPYs the whole tree,
#    and leftover host-built .o/.d files would poison the container build.
make clean
# (or manually: rm -f *.o *.d and the same in app/ driver/ ui/ helper/)

# 2. Build the toolchain image (first time, and after toolchain changes)
docker build -t uvk5 .

# 3. Compile and copy the artifacts out
mkdir -p compiled-firmware
docker run --rm -v "$PWD/compiled-firmware:/app/compiled-firmware" uvk5 \
  /bin/bash -c "cd /app && make && cp vuurwerk-v1.5.0* compiled-firmware/"
```

Your firmware lands in `compiled-firmware/`. Note: the convenience scripts
`compile-with-docker.sh` / `compile-with-docker.bat` still copy the upstream
`firmware*` name -- use the command above (or adjust the copy pattern to
`vuurwerk-v1.5.0*`).

## Native build (Linux / macOS)

```bash
# Ubuntu / Debian
sudo apt-get install gcc-arm-none-eabi make python3 python3-pip
pip install crcmod

# macOS
brew install --cask gcc-arm-embedded
brew install make

make clean
make
```

A native toolchain works fine for development, but different GCC versions
produce slightly different code sizes -- release byte counts always refer to
the Docker build.

## Native build (Windows)

`win_make.bat` documents the direct-on-Windows path: install ARM GCC
(`gcc-arm-none-eabi`, from the ARM developer downloads) and GNU Make 3.81
(gnuwin32), make sure both are on `PATH`, then run the batch file from the
repo root. Python 3 with `crcmod` is optional -- without it you get the raw
`.bin` but no `.packed.bin`.

## Build output

A successful `make` prints the size summary and leaves three artifacts:

| File | What it is |
|---|---|
| `vuurwerk-v1.5.0` | ELF executable (for `arm-none-eabi-size`, debugging) |
| `vuurwerk-v1.5.0.bin` | Raw flash image (k5prog and similar tools) |
| `vuurwerk-v1.5.0.packed.bin` | Packed image for the browser flasher |

## Size check

Flash is 60 KiB (61,440 bytes, set in `firmware.ld`) and VUURWERK v1.5.0
fills 61,396 of them -- **44 bytes free**. The linker fails loudly if the
image no longer fits, so if your build links, it fits.

```bash
arm-none-eabi-size vuurwerk-v1.5.0
```

Watch the `text + data` total against 61,440. There is no headroom for casual
additions; if you are hacking on the firmware, expect to pay for new code by
trimming elsewhere.

## Build options

Feature switches live at the top of the `Makefile` (`ENABLE_...` variables).
VUURWERK's shipped configuration is the supported one; switching features off
may orphan code paths that expect them. The version string is also set there:

```makefile
VERSION_STRING ?= v1.5.0
```

Compiler configuration (already set, listed for reference):

- **Optimization**: `-Os` (size)
- **LTO**: enabled (`-flto=auto`)
- **Standard**: C2x (`-std=c2x`)
- **Warnings**: `-Wall -Wextra -Werror` -- warnings are errors, always

## Troubleshooting

**`arm-none-eabi-gcc: command not found`** -- the toolchain is not on `PATH`.
Use the Docker build, or fix your `PATH`.

**Link fails with a region-overflow error** -- your change made the image
bigger than 61,440 bytes. Trim something.

**Docker build succeeds but `make` inside fails strangely** -- almost always
stale `.o`/`.d` files copied into the image from a previous host build. Run
`make clean` on the host, rebuild the image, retry.

**`*.packed.bin` was not built** -- Python or `crcmod` is missing. The raw
`.bin` is still valid; install `crcmod` (`pip install crcmod`) to get the
packed file.

## Flashing what you built

See [FLASHING.md](FLASHING.md). Browser flasher takes the `.packed.bin`;
k5prog takes the raw `.bin`.
