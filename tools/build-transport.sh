#!/usr/bin/env bash
# Complete original-DP32G030 APRS candidate. Builds/packages; never flashes.
set -euo pipefail
cd "$(dirname "$0")/.."
mapfile -t settings < <(python3 - <<'PY'
from pathlib import Path
import re
for name,value in re.findall(r'^(ENABLE_\w+)\s*\?=\s*([01])\b',Path('Makefile').read_text(),re.M):
 print(name+'='+value)
PY
)
((${#settings[@]})) || { echo 'No boolean build settings found.' >&2; exit 1; }
settings+=(ENABLE_APRS_TRANSPORT=1)
./tools/build.sh "${settings[@]}"
mkdir -p build/transport-candidate
cp uvk6-experimental.bin build/transport-candidate/uvk6-aprs-candidate-r2.bin
cp uvk6-experimental.packed.bin build/transport-candidate/uvk6-aprs-candidate-r2.packed.bin
cp uvk6-experimental.map build/transport-candidate/firmware.map
cp build/verification.json build/transport-candidate/verification.json
python3 - "${settings[@]}" <<'PY'
import json,subprocess,sys
from pathlib import Path
out=Path('build/transport-candidate')
sha=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
report=json.loads((out/'verification.json').read_text())
report.update({
 'profile':'Complete APRS transport experimental candidate; physical/RF validation pending',
 'source_commit':sha,
 'source_tree_clean':not bool(subprocess.check_output(['git','status','--porcelain'],text=True).strip()),
 'source_url':'https://github.com/Dustpan95/UVK5/tree/'+sha,
 'make_settings':dict(setting.split('=',1) for setting in sys.argv[1:]),
 'includes':['custom home, overlays and battery percentage','voice TX compressor, adaptive squelch and scanning',
             'standalone APRS stations, positions, personal inbox and all-traffic monitor',
             'multi-tap composition, quick messages and review before sending',
             'separate APRS frequency and voice restoration','automatic personal ACKs and three total attempts',
             '10-second configurable ACK wait and 30 seconds on voice between attempts',
             'degrees/minutes static position and explicit beacon review/send'],
 'owner_selected_removals':['automatic CTCSS/DCS search','flashlight','secondary-VFO activity scanning'],
 'physical_checks':'unrun; DP32G030 identity, EEPROM/calibration backups, recovery, boot, RF timing/deviation and independent TNC interoperability require local verification',
 'license':'GPL-3.0 complete modified VUURWERK fork; retain upstream licenses/notices and corresponding source at source_url',
})
(out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
print('Packaged',out,'(no hardware was flashed).')
PY
