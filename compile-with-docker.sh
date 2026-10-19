#!/usr/bin/env bash
# Modified for UVK5: use the verified toolchain/build/size-check entry point.
set -euo pipefail
cd "$(dirname "$0")"
export UVK6_USE_DOCKER=1
exec ./tools/build.sh "$@"
