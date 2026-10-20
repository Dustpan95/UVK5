#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
for voice in 0 1; do
optional=(); if [[ $voice == 1 ]]; then optional+=(-DENABLE_VOICE); fi
${CC:-gcc} "${optional[@]}" -std=gnu11 -fshort-enums -funsigned-char -include dtmf.h \
 -DENABLE_OVERLAY -DENABLE_AM_FIX -I . -ffunction-sections -fdata-sections \
 -g -Wall -Wextra -Werror -Wno-unused-parameter \
 -fsanitize=address,undefined -fno-omit-frame-pointer \
 tests/test_menu_settings.c tests/reference/menu_settings.c app/menu.c ui/menu.c \
 ui/display_text.c tests/init_display_text.c helper/packed.c helper/packed_huffman.c misc.c dcs.c tests/init_dcs.c frequencies.c \
 -Wl,--gc-sections -o build/test_menu_settings
build/test_menu_settings
done
