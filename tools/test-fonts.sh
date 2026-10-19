#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/font-compression
compiler=${CC:-gcc}
flags=(-std=c11 -Wall -Wextra -Werror -g -I .)
if [[ ${SANITIZE:-1} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
python3 tools/pack-fonts.py --check
for bold in plain bold; do
    extra=()
    if [[ $bold == bold ]]; then extra+=(-DENABLE_SMALL_BOLD); fi
    "$compiler" "${flags[@]}" "${extra[@]}" tests/test_fonts.c font.c -o build/font-compression/original-test
    "$compiler" "${flags[@]}" "${extra[@]}" -DENABLE_FONT_PACKING tests/test_fonts.c font_packed.c helper/packed.c -o build/font-compression/packed-test
    build/font-compression/original-test > "build/font-compression/original-$bold.bin"
    build/font-compression/packed-test > "build/font-compression/packed-$bold.bin"
    cmp "build/font-compression/original-$bold.bin" "build/font-compression/packed-$bold.bin"
done
printf '%s\n' 'PASS packed fonts: every original glyph byte identical, plain and bold, sanitized'
