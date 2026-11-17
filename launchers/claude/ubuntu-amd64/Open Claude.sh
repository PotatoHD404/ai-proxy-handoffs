#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
if ! command -v claude-desktop >/dev/null 2>&1; then
    printf '%s\n' 'Run ./setup.sh first.' >&2
    exit 1
fi
source "$root/bin/proxy-env"
handoff_proxy "$@"
shift "$proxy_shift"
exec claude-desktop --proxy-server="$proxy" '--proxy-bypass-list=<-loopback>' --disable-quic "$@"
