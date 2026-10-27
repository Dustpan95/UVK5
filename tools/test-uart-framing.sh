#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/uart-framing
flags=(-std=gnu11 -O1 -Wall -Wextra -Werror -g -I . -ffunction-sections -fdata-sections)
if [[ ${SANITIZE:-1} == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
"${CC:-gcc}" "${flags[@]}" tests/test_uart_framing.c -Wl,--gc-sections -o build/uart-framing/test
build/uart-framing/test
