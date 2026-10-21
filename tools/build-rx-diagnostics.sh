#!/usr/bin/env bash
# Temporary original-MCU RX instrumentation profile; no APRS data transmitter.
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
settings+=(ENABLE_APRS_TRANSPORT=0 ENABLE_APRS_MESSAGING=0 ENABLE_APRS_SESSION=0 ENABLE_APRS_RX_DIAGNOSTICS=1)
./tools/build.sh "${settings[@]}"
mkdir -p build/rx-diagnostics-d4
cp uvk6-experimental.bin build/rx-diagnostics-d4/uvk6-rx-diagnostics-d4.bin
cp uvk6-experimental.packed.bin build/rx-diagnostics-d4/uvk6-rx-diagnostics-d4.packed.bin
cp uvk6-experimental.map build/rx-diagnostics-d4/firmware.map
cp build/verification.json build/rx-diagnostics-d4/verification.json
cp tools/read-rx-capture.html build/rx-diagnostics-d4/read-rx-capture-d4.html
python3 - "${settings[@]}" <<'PY'
import json,subprocess,sys
from pathlib import Path
out=Path('build/rx-diagnostics-d4')
assert b'APRS RX DIAG D4\0' in (out/'uvk6-rx-diagnostics-d4.bin').read_bytes(), 'diagnostic feature missing from image'
report=json.loads((out/'verification.json').read_text())
sha=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
report.update({
 'profile':'D4 temporary APRS receive diagnostics; standard voice features and packet browser',
 'source_commit':sha,'source_tree_clean':not bool(subprocess.check_output(['git','status','--porcelain'],text=True).strip()),
 'source_url':'https://github.com/Dustpan95/UVK5/tree/'+sha,
 'make_settings':dict(setting.split('=',1) for setting in sys.argv[1:]),
 'not_in_this_diagnostic_image':['APRS transmit','message composition','session station directory and personal inbox'],
 'purpose':'D4 compares baseline, TA1JS filter plus hardware AGC, filter only and hardware AGC only at the unchanged production modem setting; records actual register readbacks and exports four frozen complete/timeout raw captures through bounded diagnostic RAM reads',
 'physical_evidence':'Owner reports revision 2 boots, controls/channels/voice receive work and a clear beacon burst is audible, but no APRS decode. This diagnostic image has not been run on hardware.',
 'license':'GPL-3.0 complete modified VUURWERK fork; upstream notices/licenses and corresponding source at source_url',
})
(out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
print('Packaged',out,'(no hardware was flashed).')
PY
