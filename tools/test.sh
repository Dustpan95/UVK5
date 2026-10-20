#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
bash tools/test-format.sh
bash tools/test-settings.sh
python3 tools/pack-display-text.py --test
text_support=(ui/display_text.c tests/init_display_text.c)
compiler=${CC:-gcc}
flags=(-DENABLE_VFO_SPLIT -DENABLE_TONE_SEARCH -std=gnu11 -Wall -Wextra -Werror -g -I .)
if [[ ${SANITIZE:-1} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"$compiler" "${flags[@]}" tests/test_aprs.c app/aprs_decode.c app/aprs_position.c -o build/test_aprs
build/test_aprs
"$compiler" "${flags[@]}" -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_radio "${text_support[@]}" helper/packed.c
build/test_radio build

"$compiler" "${flags[@]}" tests/test_message.c app/aprs_message.c app/aprs_decode.c app/aprs_position.c -o build/test_message
build/test_message

"$compiler" "${flags[@]}" tests/test_tx.c app/aprs_tx.c app/aprs_message.c app/aprs_decode.c app/aprs_position.c -o build/test_tx
build/test_tx

"$compiler" "${flags[@]}" -DENABLE_APRS_SESSION -DENABLE_APRS_MESSAGING -ffunction-sections -fdata-sections \
    tests/test_aprs_service.c experiments/aprs_service.c app/aprs_tx.c app/aprs_message.c \
    app/aprs_session.c app/aprs_decode.c app/aprs_position.c -Wl,--gc-sections -o build/test_aprs_service
build/test_aprs_service

"$compiler" "${flags[@]}" -DENABLE_APRS_SESSION -DENABLE_APRS_MESSAGING -ffunction-sections -fdata-sections \
    tests/test_aprs_channel.c experiments/aprs_channel.c experiments/aprs_service.c app/aprs_tx.c app/aprs_message.c \
    app/aprs_session.c app/aprs_decode.c app/aprs_position.c -Wl,--gc-sections -o build/test_aprs_channel
build/test_aprs_channel

"$compiler" "${flags[@]}" -DENABLE_APRS_SESSION -DENABLE_APRS_MESSAGING -DENABLE_APRS_TRANSPORT -ffunction-sections -fdata-sections \
    tests/test_aprs_runtime.c app/aprs_runtime.c experiments/aprs_channel.c experiments/aprs_service.c app/aprs_tx.c \
    app/aprs_message.c app/aprs_session.c app/aprs_decode.c app/aprs_position.c -Wl,--gc-sections -o build/test_aprs_runtime
build/test_aprs_runtime

mkdir -p build/setup-ui
"$compiler" "${flags[@]}" -DENABLE_APRS_TRANSPORT -DPRINTF_INCLUDE_CONFIG_H tests/test_aprs_setup_ui.c \
    ui/aprs_setup.c ui/aprs_message.c ui/helper.c font.c external/printf/printf.c -o build/test_aprs_setup_ui "${text_support[@]}" helper/packed.c
build/test_aprs_setup_ui build/setup-ui

"$compiler" "${flags[@]}" -DENABLE_APRS_TRANSPORT tests/test_aprs_setup.c app/aprs_setup.c -o build/test_aprs_setup
build/test_aprs_setup

"$compiler" "${flags[@]}" tests/test_bell202_timing.c experiments/bell202_symbols.c experiments/bell202_timing.c -o build/test_bell202_timing
build/test_bell202_timing

"$compiler" "${flags[@]}" -I tests/clock_mock tests/test_clock.c driver/systick.c -o build/test_clock
build/test_clock

"$compiler" -I tests/radio_mock "${flags[@]}" tests/test_bell202_radio.c experiments/bell202_radio.c experiments/bell202_symbols.c experiments/bell202_timing.c -o build/test_bell202_radio
build/test_bell202_radio

"$compiler" -O1 -I tests/radio_mock "${flags[@]}" tests/test_radio_owner.c experiments/aprs_radio_owner.c -o build/test_radio_owner
build/test_radio_owner

"$compiler" -O1 -I tests/radio_mock "${flags[@]}" tests/test_aprs_hardware.c experiments/aprs_hardware.c -o build/test_aprs_hardware
build/test_aprs_hardware

"$compiler" "${flags[@]}" -DENABLE_APRS_MESSAGING -DENABLE_APRS_SESSION -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_radio_messaging "${text_support[@]}" helper/packed.c
build/test_radio_messaging build

mkdir -p build/session-receiver
"$compiler" "${flags[@]}" -DENABLE_APRS_SESSION -DENABLE_APRS_CALLSIGN_ONLY -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_session_receiver "${text_support[@]}" helper/packed.c
build/test_session_receiver build/session-receiver

# Current profile excludes tone search, flashlight and VFO activity scanning.
# Keep optional legacy exclusion tests above and exercise the current profile too.
mkdir -p build/current-profile
"$compiler" "${flags[@]}" -UENABLE_VFO_SPLIT -UENABLE_TONE_SEARCH -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/home.c ui/helper.c font.c external/printf/printf.c -o build/test_current_profile "${text_support[@]}" helper/packed.c
build/test_current_profile build/current-profile
"$compiler" -O1 -I tests/radio_mock "${flags[@]}" -UENABLE_VFO_SPLIT -UENABLE_TONE_SEARCH \
    tests/test_aprs_hardware.c experiments/aprs_hardware.c -o build/test_current_hardware
build/test_current_hardware
"$compiler" "${flags[@]}" -UENABLE_VFO_SPLIT -UENABLE_TONE_SEARCH -ffunction-sections -fdata-sections -DPRINTF_INCLUDE_CONFIG_H \
    tests/test_disabled_features.c ui/menu.c dcs.c -Wl,--gc-sections -o build/test_disabled_features "${text_support[@]}" helper/packed.c
build/test_disabled_features

# Exhaustive exact-arithmetic comparison uses optimized host code; parser/UI
# suites above retain sanitizer coverage. This is 137 million comparisons.
"$compiler" -std=c11 -O2 -Wall -Wextra -Werror -I . tests/test_scale.c -o build/test_scale
build/test_scale

"$compiler" "${flags[@]}" tests/test_packed_stream.c helper/packed.c -o build/test_packed_stream
python3 tools/test-packer.py

./tools/test-fonts.sh
mkdir -p build/font-compression/frames
frame_names=(aprs-position aprs-message home-memory home-unnamed home-vfo message-compose message-review message-blocked personal-inbox personal-message personal-unconfigured own-callsign)
for name in "${frame_names[@]}"; do cp "build/$name.pbm" "build/font-compression/frames/$name.pbm"; done
"$compiler" "${flags[@]}" -DENABLE_FONT_PACKING -DENABLE_APRS_MESSAGING -DENABLE_APRS_SESSION -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font_packed.c helper/packed.c external/printf/printf.c -o build/test_radio_packed "${text_support[@]}"
build/test_radio_packed build

for name in "${frame_names[@]}"; do cmp "build/$name.pbm" "build/font-compression/frames/$name.pbm"; done
printf '%s\n' 'PASS packed-font framebuffers identical in 12 radio/APRS/editor states'

mkdir -p build/session-receiver-raw
receiver_frames=(aprs-position aprs-message home-memory home-unnamed home-vfo personal-inbox personal-message personal-unconfigured station-directory station-detail own-callsign)
for name in "${receiver_frames[@]}"; do cp "build/session-receiver/$name.pbm" "build/session-receiver-raw/$name.pbm"; done
"$compiler" "${flags[@]}" -DENABLE_FONT_PACKING -DENABLE_APRS_SESSION -DENABLE_APRS_CALLSIGN_ONLY -DPRINTF_INCLUDE_CONFIG_H tests/test_radio.c \
    app/aprs_rx.c app/aprs_decode.c app/aprs_position.c app/aprs_message.c app/aprs_session.c tx_compressor.c smart_squelch.c \
    ui/aprs.c ui/aprs_message.c ui/aprs_session.c ui/home.c ui/helper.c font_packed.c helper/packed.c external/printf/printf.c -o build/test_session_receiver_packed "${text_support[@]}"
build/test_session_receiver_packed build/session-receiver
for name in "${receiver_frames[@]}"; do cmp "build/session-receiver/$name.pbm" "build/session-receiver-raw/$name.pbm"; done
printf '%s\n' 'PASS packed-font framebuffers identical in 11 receive-session states'

python3 tools/pack-menu-hints.py --check
"$compiler" "${flags[@]}" tests/test_hints.c helper/packed.c -o build/test_hints
build/test_hints
for am_flag in '' '-DENABLE_AM_FIX'; do
    "$compiler" "${flags[@]}" ${am_flag:+$am_flag} -DTEST_HINT_LOOKUP -ffunction-sections -fdata-sections \
        tests/test_hints.c helper/packed.c ui/vuurwerk_menu.c -Wl,--gc-sections -o build/test_hint_lookup "${text_support[@]}"
    build/test_hint_lookup
done

"$compiler" "${flags[@]}" -DENABLE_RSSI_BAR -DPRINTF_INCLUDE_CONFIG_H tests/test_ui_main.c ui/main.c -o build/test_ui_main "${text_support[@]}" helper/packed.c
build/test_ui_main
