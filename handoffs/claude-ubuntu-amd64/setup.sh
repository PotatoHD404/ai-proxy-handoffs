#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
if [[ "$(dpkg --print-architecture)" != amd64 ]]; then
    printf '%s\n' 'This installer requires Ubuntu amd64.' >&2
    exit 1
fi
sudo apt-get update
sudo apt-get install --no-install-recommends "$root/packages/claude-desktop.deb"
