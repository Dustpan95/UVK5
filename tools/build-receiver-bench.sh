#!/usr/bin/env bash
# Original-DP32G030 receive-session bench package. Never flashes hardware.
set -euo pipefail
cd "$(dirname "$0")/.."
# Pin every boolean option to this source revision's defaults, then enable the
# receive-session UI. Native and Docker builds must use the same options even
# when a caller has exported other firmware flags in their shell.
mapfile -t settings < <(python3 - <<'PY'
from pathlib import Path
import re
for name,value in re.findall(r'^(ENABLE_\w+)\s*\?=\s*([01])\b',Path('Makefile').read_text(),re.M):
 print(name+'='+value)
PY
)
((${#settings[@]})) || { echo "No boolean build options found in Makefile." >&2; exit 1; }
settings+=(ENABLE_APRS_SESSION=1 ENABLE_APRS_MESSAGING=0)
./tools/build.sh "${settings[@]}"
mkdir -p build/receiver-bench
cp uvk6-experimental.bin build/receiver-bench/uvk6-rx-session-bench.bin
cp uvk6-experimental.packed.bin build/receiver-bench/uvk6-rx-session-bench.packed.bin
cp uvk6-experimental.map build/receiver-bench/firmware.map
cp build/verification.json build/receiver-bench/verification.json
python3 - "${settings[@]}" <<'PY'
import hashlib
import json
from pathlib import Path
import subprocess
import sys
out=Path('build/receiver-bench')
sha=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
dirty=bool(subprocess.check_output(['git','status','--porcelain'],text=True).strip())
report=json.loads((out/'verification.json').read_text())
report.update({
 'profile':'RX session bench checkpoint; complete APRS TX firmware remains in development',
 'source_commit':sha,'source_tree_clean':not dirty,
 'source_url':'https://github.com/Dustpan95/UVK5/tree/'+sha,
 'make_settings':dict(setting.split('=',1) for setting in sys.argv[1:]),
 'includes':['custom home and compact mode overlays','percentage battery',
             'APRS stations and positions','personal inbox and all-traffic monitor',
             'on-radio callsign entry','brief all-message alerts with global beep mute',
             'existing voice compressor, adaptive squelch and scanning'],
 'not_in_this_image':['APRS transmit and message composition'],
 'physical_checks':'unrun; identify DP32G030, preserve calibration/EEPROM/firmware and verify recovery before owner-controlled bench installation',
 'license':'GPL-3.0 complete modified VUURWERK fork; upstream notices/licenses and corresponding source at source_url',
})
(out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
for name in ('uvk6-rx-session-bench.bin','uvk6-rx-session-bench.packed.bin'):
 print(hashlib.sha256((out/name).read_bytes()).hexdigest()+'  '+name)
print('Packaged',out,'(no hardware was flashed).')
PY
