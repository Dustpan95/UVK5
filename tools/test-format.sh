#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/format
compiler=${CC:-gcc}
flags=(-std=c11 -Wall -Wextra -Werror -g -I . -DPRINTF_INCLUDE_CONFIG_H)
if [[ ${SANITIZE:-1} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"$compiler" "${flags[@]}" -DPRINTF_LEGACY_BUFFER \
    -Dsprintf_=reference_sprintf -Dsnprintf_=reference_snprintf \
    -Dvsnprintf_=reference_vsnprintf -Dprintf_=reference_printf \
    -Dvprintf_=reference_vprintf -Dfctprintf=reference_fctprintf \
    -c external/printf/printf.c -o build/format/reference.o
"$compiler" "${flags[@]}" -UPRINTF_LEGACY_BUFFER -DPRINTF_COMPACT_EXTENDED tests/test_format_buffer.c external/printf/printf.c \
    build/format/reference.o -o build/format/test
build/format/test

python3 tools/test-radio-format.py
