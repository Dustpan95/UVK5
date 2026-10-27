#!/usr/bin/env bash
# Original-DP32G030 outgoing APRS candidate. Builds/packages; never flashes.
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
mkdir -p build/tx-only
cp uvk6-experimental.bin build/tx-only/uvk6-tx-only.bin
cp uvk6-experimental.packed.bin build/tx-only/uvk6-tx-only.packed.bin
cp uvk6-experimental.map build/tx-only/firmware.map
cp build/verification.json build/tx-only/verification.json
python3 - "${settings[@]}" <<'PY'
import json,subprocess,sys
from pathlib import Path
out=Path('build/tx-only')
sha=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
report=json.loads((out/'verification.json').read_text())
report.update({
 'profile':'Outgoing APRS only; standalone APRS receive removed by owner request',
 'source_commit':sha,
 'source_tree_clean':not bool(subprocess.check_output(['git','status','--porcelain'],text=True).strip()),
 'source_url':'https://github.com/Dustpan95/UVK5/tree/'+sha,
 'make_settings':dict(setting.split('=',1) for setting in sys.argv[1:]),
 'includes':['custom home, compact overlays and battery percentage',
             'voice TX compressor, adaptive squelch and scanning',
             'manual APRS message composition, quick messages and review before send',
             'separate APRS transmit frequency with immediate voice restoration',
             'session-only callsign/static position and explicit beacon review/send'],
 'removed':['APRS packet reception/decoder','receive interrupt handling and modem ownership',
            'personal inbox, station directory, all-traffic monitor and message alerts',
            'RX diagnostics and capture export command','automatic ACKs and received delivery confirmations',
            'ACK wait/listening settings'],
 'message_policy':'One completed transmission per explicit send; no delivery confirmation',
 'owner_selected_radio_removals':['automatic CTCSS/DCS search','flashlight','secondary-VFO activity scanning'],
 'physical_checks':'This revised image has not been run on hardware. Prior R2 boot/controls/voice RX passed per owner; APRS TX/RF interoperability remains unverified.',
 'license':'GPL-3.0 complete modified VUURWERK fork; retain upstream licenses/notices and corresponding source at source_url',
})
(out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
print('Packaged',out,'(no hardware was flashed).')
PY
