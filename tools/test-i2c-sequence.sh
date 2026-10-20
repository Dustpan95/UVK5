#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
${CC:-gcc} -std=gnu11 -I . -ffunction-sections -fdata-sections -g \
 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer \
 tests/test_i2c_sequence.c tests/reference/i2c_sequence.c driver/i2c.c \
 -Wl,--gc-sections -o build/test_i2c_sequence
build/test_i2c_sequence
