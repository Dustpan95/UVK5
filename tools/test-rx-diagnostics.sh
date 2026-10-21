#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/rx-diagnostics-tests
compiler=${CC:-gcc}
flags=(-std=gnu11 -Wall -Wextra -Werror -g -I . -DPRINTF_INCLUDE_CONFIG_H -DENABLE_APRS_RX_DIAGNOSTICS)
if [[ ${SANITIZE:-1} == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
"$compiler" "${flags[@]}" tests/test_radio.c app/aprs_rx.c app/aprs_decode.c app/aprs_position.c \
    tx_compressor.c smart_squelch.c ui/aprs.c ui/home.c ui/helper.c font.c external/printf/printf.c \
    ui/display_text.c tests/init_display_text.c dcs.c tests/init_dcs.c helper/packed.c helper/packed_huffman.c \
    -o build/rx-diagnostics-tests/test
build/rx-diagnostics-tests/test build/rx-diagnostics-tests
"$compiler" "${flags[@]}" -ffunction-sections -fdata-sections tests/test_reference_gain.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c gain_staging.c am_fix.c \
    -Wl,--gc-sections -o build/rx-diagnostics-tests/gain
build/rx-diagnostics-tests/gain
