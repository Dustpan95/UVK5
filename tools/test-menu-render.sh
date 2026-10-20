#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
${CC:-gcc} -std=gnu11 -Wall -Wextra -Werror -Wno-unused-parameter -g -I . \
 -fsanitize=address,undefined -fno-omit-frame-pointer -ffunction-sections -fdata-sections \
 -funsigned-char -include dtmf.h -DENABLE_OVERLAY -DENABLE_AM_FIX -DPRINTF_INCLUDE_CONFIG_H \
 tests/test_menu_render.c tests/reference/menu_render.c ui/menu.c app/menu.c \
 ui/helper.c ui/inputbox.c ui/vuurwerk_menu.c ui/display_text.c tests/init_display_text.c \
 helper/packed.c font.c bitmaps.c dcs.c frequencies.c misc.c helper/battery.c external/printf/printf.c \
 -Wl,--gc-sections -o build/test_menu_render
build/test_menu_render
