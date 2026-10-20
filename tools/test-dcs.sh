#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
python3 tools/pack-dcs.py --check
${CC:-gcc} -std=gnu11 -Wall -Wextra -Werror -I . -fsanitize=address,undefined \
 -fno-omit-frame-pointer tests/test_dcs.c tests/reference/dcs.c dcs.c -o build/test_dcs
build/test_dcs
