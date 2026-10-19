#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
compiler=${CC:-gcc}
flags=(-std=gnu11 -Wall -Wextra -Werror -g -I .)
if [[ ${SANITIZE:-1} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"$compiler" "${flags[@]}" tests/test_aprs.c app/aprs_decode.c app/aprs_position.c -o build/test_aprs
build/test_aprs
"$compiler" "${flags[@]}" -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_radio
build/test_radio build

"$compiler" "${flags[@]}" tests/test_message.c app/aprs_message.c app/aprs_decode.c app/aprs_position.c -o build/test_message
build/test_message

"$compiler" "${flags[@]}" -DENABLE_APRS_MESSAGING -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_radio_messaging
build/test_radio_messaging build

# Exhaustive exact-arithmetic comparison uses optimized host code; parser/UI
# suites above retain sanitizer coverage. This is 137 million comparisons.
"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -I . tests/test_scale.c -o build/test_scale
build/test_scale
