#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
python3 tools/test-firmware-limits.py
bash tools/test-uart-framing.sh
bash tools/test-format.sh
bash tools/test-settings.sh
bash tools/test-menu-render.sh
bash tools/test-menu-settings.sh
bash tools/test-dcs.sh
python3 tools/test-call-bands.py
bash tools/test-bk-words.sh
bash tools/test-i2c-sequence.sh
bash tools/test-compact-memory.sh
python3 tools/test-compact-division.py
python3 tools/pack-display-text.py --test
compiler=${CC:-gcc}
flags=(-DENABLE_VFO_SPLIT -DENABLE_TONE_SEARCH -std=gnu11 -Wall -Wextra -Werror -g -I .)
flags+=(helper/compact_memory.c app/aprs_format.c -fno-builtin)
for function in memcpy memmove memset memcmp strlen strcmp strcpy strncpy; do flags+=(-D"$function=FIRMWARE_$function"); done
if [[ ${SANITIZE:-1} == 1 ]]; then flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer); fi
support=(ui/display_text.c tests/init_display_text.c dcs.c tests/init_dcs.c helper/packed.c helper/packed_huffman.c)
radio=(tests/test_radio.c tx_compressor.c smart_squelch.c ui/home.c ui/helper.c external/printf/printf.c)
"$compiler" "${flags[@]}" -DPRINTF_INCLUDE_CONFIG_H "${radio[@]}" font.c "${support[@]}" -o build/test_radio
build/test_radio build
"$compiler" "${flags[@]}" tests/test_message.c app/aprs_message.c app/aprs_frame.c -o build/test_message
build/test_message
"$compiler" "${flags[@]}" tests/test_aprs_format.c -o build/test_aprs_format
build/test_aprs_format
"$compiler" "${flags[@]}" tests/test_tx.c app/aprs_tx.c app/aprs_message.c -o build/test_tx
build/test_tx
"$compiler" "${flags[@]}" -DENABLE_APRS_SESSION -DENABLE_APRS_TRANSPORT -ffunction-sections -fdata-sections \
 tests/test_aprs_runtime.c app/aprs_runtime.c app/aprs_ui.c experiments/aprs_channel.c experiments/aprs_service.c \
 app/aprs_tx.c app/aprs_message.c app/aprs_session.c app/aprs_setup.c -Wl,--gc-sections -o build/test_aprs_runtime
build/test_aprs_runtime
mkdir -p build/setup-ui
"$compiler" "${flags[@]}" -DENABLE_APRS_TRANSPORT -DPRINTF_INCLUDE_CONFIG_H tests/test_aprs_setup_ui.c \
 ui/aprs.c ui/aprs_setup.c ui/aprs_message.c ui/helper.c font.c external/printf/printf.c "${support[@]}" -o build/test_aprs_setup_ui
build/test_aprs_setup_ui build/setup-ui
"$compiler" "${flags[@]}" -DENABLE_APRS_TRANSPORT tests/test_aprs_setup.c app/aprs_setup.c -o build/test_aprs_setup
build/test_aprs_setup
"$compiler" "${flags[@]}" tests/test_bell202_timing.c experiments/bell202_symbols.c experiments/bell202_timing.c -o build/test_bell202_timing
build/test_bell202_timing
"$compiler" "${flags[@]}" -I tests/clock_mock tests/test_clock.c driver/systick.c -o build/test_clock
build/test_clock
"$compiler" "${flags[@]}" -ffunction-sections -fdata-sections tests/test_scheduler.c misc.c -Wl,--gc-sections -o build/test_scheduler
build/test_scheduler
"$compiler" -I tests/radio_mock "${flags[@]}" tests/test_bell202_radio.c experiments/bell202_radio.c experiments/bell202_symbols.c experiments/bell202_timing.c -o build/test_bell202_radio
build/test_bell202_radio
"$compiler" -O1 -I tests/radio_mock "${flags[@]}" tests/test_radio_owner.c experiments/aprs_radio_owner.c -o build/test_radio_owner
build/test_radio_owner
"$compiler" -O1 -I tests/radio_mock "${flags[@]}" tests/test_aprs_hardware.c experiments/aprs_hardware.c -o build/test_aprs_hardware
build/test_aprs_hardware
"$compiler" -O1 -I tests/radio_mock "${flags[@]}" -UENABLE_VFO_SPLIT -UENABLE_TONE_SEARCH tests/test_aprs_hardware.c experiments/aprs_hardware.c -o build/test_current_hardware
build/test_current_hardware
"$compiler" "${flags[@]}" -UENABLE_VFO_SPLIT -UENABLE_TONE_SEARCH -ffunction-sections -fdata-sections -DPRINTF_INCLUDE_CONFIG_H \
 tests/test_disabled_features.c ui/menu.c -Wl,--gc-sections -o build/test_disabled_features "${support[@]}"
build/test_disabled_features
"$compiler" "${flags[@]}" tests/test_packed_stream.c helper/packed.c helper/packed_huffman.c -o build/test_packed_stream
python3 tools/test-packer.py
"$compiler" "${flags[@]}" -DPACK_HUFFMAN_STREAM tests/test_packed_stream.c helper/packed_huffman.c -o build/test_huffman_stream
python3 tools/test-huffman.py
./tools/test-fonts.sh
python3 tools/test-huffman-cpu.py
mkdir -p build/voice-frames
for name in home-memory home-unnamed home-vfo home-split home-toast home-entry home-warning home-scan; do cp "build/$name.pbm" build/voice-frames/; done
"$compiler" "${flags[@]}" -DENABLE_FONT_PACKING -DPRINTF_INCLUDE_CONFIG_H "${radio[@]}" font_packed.c "${support[@]}" -o build/test_radio_packed
build/test_radio_packed build
for name in home-memory home-unnamed home-vfo home-split home-toast home-entry home-warning home-scan; do cmp "build/$name.pbm" "build/voice-frames/$name.pbm"; done
python3 tools/pack-menu-hints.py --check
"$compiler" "${flags[@]}" tests/test_hints.c helper/packed.c helper/packed_huffman.c -o build/test_hints
build/test_hints
for am_flag in '' '-DENABLE_AM_FIX'; do
 "$compiler" "${flags[@]}" ${am_flag:+$am_flag} -DTEST_HINT_LOOKUP -ffunction-sections -fdata-sections \
 tests/test_hints.c ui/vuurwerk_menu.c -Wl,--gc-sections -o build/test_hint_lookup "${support[@]}"
 build/test_hint_lookup
done
"$compiler" "${flags[@]}" -DENABLE_APRS_TRANSPORT -DENABLE_RSSI_BAR -DPRINTF_INCLUDE_CONFIG_H tests/test_ui_main.c ui/main.c -o build/test_ui_main "${support[@]}"
build/test_ui_main
