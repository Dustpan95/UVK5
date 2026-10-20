#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build/settings
compiler=${CC:-gcc}
flags=(-std=gnu11 -fshort-enums -funsigned-char -Wall -Wextra -Werror -g -I .
    -ffunction-sections -fdata-sections)
if [[ ${SANITIZE:-1} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
for optional in '' '-DENABLE_AM_FIX -DENABLE_RSSI_BAR' \
    '-DENABLE_AM_FIX -DENABLE_RSSI_BAR -DENABLE_NOAA -DENABLE_VOX -DENABLE_VOICE -DENABLE_ALARM -DENABLE_AUDIO_BAR -DENABLE_PWRON_PASSWORD'; do
    # Match the target's enum/char ABI; compare real loaders, including blank
    # and corrupt inputs. The EEPROM mock refuses every attempted write.
    "$compiler" "${flags[@]}" $optional tests/test_settings_init.c \
        tests/reference/settings_init.c settings.c misc.c dtmf.c \
        -Wl,--gc-sections -o build/settings/test
    build/settings/test
done
