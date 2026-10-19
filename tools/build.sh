#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
image=${UVK6_BUILD_IMAGE:-uvk6-build:jammy}
if command -v arm-none-eabi-gcc >/dev/null 2>&1 && [[ ${UVK6_USE_DOCKER:-0} != 1 ]]; then
    [[ $(arm-none-eabi-gcc -dumpfullversion) == 10.3.1 ]] || { echo 'Use ARM GCC 10.3.1 or set UVK6_USE_DOCKER=1.' >&2; exit 1; }
    make clean "$@"
    make -j4 "$@"
    python3 tools/verify_firmware.py
else
    tools/setup-toolchain.sh
    docker run --rm --user "$(id -u):$(id -g)" -v "$PWD:/src" -w /src "$image" \
        bash -c 'set -euo pipefail; make clean "$@"; make -j4 "$@"; python3 tools/verify_firmware.py' bash "$@"
fi
