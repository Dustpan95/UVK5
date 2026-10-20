#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
${CC:-gcc} -std=gnu11 -fshort-enums -funsigned-char -I . \
 -ffunction-sections -fdata-sections -g -Wall -Wextra -Werror \
 -fsanitize=address,undefined -fno-omit-frame-pointer \
 tests/test_bk_words.c tests/reference/bk_words.c driver/bk4819.c \
 -Wl,--gc-sections -o build/test_bk_words
build/test_bk_words
