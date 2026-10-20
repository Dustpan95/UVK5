#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/uart-diagnostics
flags=(-std=gnu11 -O1 -Wall -Wextra -Werror -g -I . -DENABLE_APRS_RX_DIAGNOSTICS -ffunction-sections -fdata-sections)
if [[ ${SANITIZE:-1} == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
"${CC:-gcc}" "${flags[@]}" tests/test_uart_diagnostics.c -Wl,--gc-sections -o build/uart-diagnostics/test
build/uart-diagnostics/test
