#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
./bin/setup
exec "./.runtime/ChatGPT.app/Contents/MacOS/ChatGPT" "$@"
