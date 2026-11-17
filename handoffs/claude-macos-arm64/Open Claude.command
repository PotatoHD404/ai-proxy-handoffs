#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
./bin/setup
exec "./Claude.app/Contents/MacOS/Claude" "$@"
