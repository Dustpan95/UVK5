#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
./tools/setup-toolchain.sh
python3 - <<'PY'
from pathlib import Path
import shutil
root = Path.cwd()
out = root / 'build' / 'tx-investigation'
out.mkdir(parents=True, exist_ok=True)
for source in root.iterdir():
    if source.name in {'.git', 'build'} or source.name.startswith('uvk6-experimental'):
        continue
    if source.is_dir():
        shutil.copytree(source, out / source.name, dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns('*.o', '*.d'))
    elif source.suffix not in {'.o', '.d'}:
        shutil.copy2(source, out / source.name)
shutil.copy2(root / 'experiments/aprs_tx_size.c', out / 'app/aprs_tx_trial.c')
makefile = out / 'Makefile'
makefile.write_text(makefile.read_text().replace('OBJS = app/aprs_position.o',
                    'OBJS = app/aprs_tx_trial.o app/aprs_position.o'))
menu = out / 'app/menu.c'
text = menu.read_text().replace('/* UVK5 integration: passive APRS hooks. */',
       'void APRS_TxTrial(void);\n/* UVK5 integration: passive APRS hooks. */')
text = text.replace('if (UI_MENU_GetCurrentMenuId() == MENU_APRS_LOG) {\n        APRS_Open();',
       'if (UI_MENU_GetCurrentMenuId() == MENU_APRS_LOG) {\n        APRS_TxTrial();\n        APRS_Open();')
menu.write_text(text)
PY
docker run --rm --user "$(id -u):$(id -g)" \
    -v "$PWD/build/tx-investigation:/src" -w /src "${UVK6_BUILD_IMAGE:-uvk6-build:jammy}" \
    bash -c 'set -euo pipefail; make clean; make -j4; wc -c uvk6-experimental.bin'
