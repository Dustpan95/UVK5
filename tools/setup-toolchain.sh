#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
image=${UVK6_BUILD_IMAGE:-uvk6-build:jammy}
base=ubuntu@sha256:0a9887931b0e38af2e3487ed39e29436b0535c4f3f52e57745baaf79d1898956
if docker image inspect "$image" >/dev/null 2>&1; then
    version=$(docker run --rm "$image" arm-none-eabi-gcc -dumpfullversion)
    [[ "$version" == 10.3.1 ]] || { echo "Unexpected compiler in $image: $version" >&2; exit 1; }
    if docker run --rm "$image" bash -c 'command -v qemu-system-arm >/dev/null'; then
        exit 0
    fi
fi
# A normal container also works on cloud hosts where BuildKit has no egress.
# No project checkout, credentials or runtime authentication is copied in.
name="uvk6-setup-$$"
trap 'docker rm -f "$name" >/dev/null 2>&1 || true' EXIT
docker run --name "$name" -v "$PWD/tools:/setup:ro" "$base" bash /setup/install-container.sh
docker commit --change 'WORKDIR /src' "$name" "$image" >/dev/null
version=$(docker run --rm "$image" arm-none-eabi-gcc -dumpfullversion)
[[ "$version" == 10.3.1 ]] || { echo "Unexpected compiler: $version" >&2; exit 1; }
