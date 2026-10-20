#!/usr/bin/env bash
set -euo pipefail
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    gcc-arm-none-eabi=15:10.3-2021.07-4 \
    binutils-arm-none-eabi=2.38-3ubuntu1+15build1 \
    libnewlib-arm-none-eabi=3.3.0-1.3 \
    make python3 python3-crcmod gcc libc6-dev git ca-certificates qemu-system-arm
rm -rf /var/lib/apt/lists/*
arm-none-eabi-gcc --version
