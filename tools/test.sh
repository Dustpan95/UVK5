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

"$compiler" "${flags[@]}" tests/test_bell202_timing.c experiments/bell202_symbols.c experiments/bell202_timing.c -o build/test_bell202_timing
build/test_bell202_timing

"$compiler" "${flags[@]}" -DENABLE_APRS_MESSAGING -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_radio_messaging
build/test_radio_messaging build

# Exhaustive exact-arithmetic comparison uses optimized host code; parser/UI
# suites above retain sanitizer coverage. This is 137 million comparisons.
"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -I . tests/test_scale.c -o build/test_scale
build/test_scale

./tools/test-fonts.sh
mkdir -p build/font-compression/frames
frame_names=(aprs-position aprs-message home-memory home-unnamed home-vfo message-compose message-review message-blocked personal-inbox personal-message personal-unconfigured)
for name in "${frame_names[@]}"; do cp "build/$name.pbm" "build/font-compression/frames/$name.pbm"; done
"$compiler" "${flags[@]}" -DENABLE_FONT_PACKING -DENABLE_APRS_MESSAGING -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font_packed.c helper/packed.c external/printf/printf.c -o build/test_radio_packed
build/test_radio_packed build

for name in "${frame_names[@]}"; do cmp "build/$name.pbm" "build/font-compression/frames/$name.pbm"; done
printf '%s\n' 'PASS packed-font framebuffers identical in 11 radio/APRS/editor states'

python3 tools/pack-menu-hints.py --check
"$compiler" "${flags[@]}" tests/test_hints.c helper/packed.c -o build/test_hints
build/test_hints

"$compiler" "${flags[@]}" -DENABLE_RSSI_BAR -DPRINTF_INCLUDE_CONFIG_H tests/test_ui_main.c ui/main.c -o build/test_ui_main
build/test_ui_main
