#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/compact-memory
compiler=${CC:-gcc}
flags=(-std=c11 -O2 -Wall -Wextra -Werror -fno-builtin -I .)
if [[ ${SANITIZE:-1} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
# Avoid interposing on the sanitizer runtime's own libc calls. The source
# bodies remain identical; the oracle test calls the original libc names.
names=()
for function in memcpy memmove memset memcmp strlen strcmp strcpy strncpy; do
    names+=(-D"$function=FIRMWARE_$function")
done
"$compiler" "${flags[@]}" "${names[@]}" -c helper/compact_memory.c -o build/compact-memory/routines.o
"$compiler" "${flags[@]}" tests/test_compact_memory.c build/compact-memory/routines.o -o build/compact-memory/test
build/compact-memory/test
