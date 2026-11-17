#!/bin/bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
./bin/setup --import-config
printf '%s\n' 'Imported the local Codex config into both proxy profiles. Restart the app and use its normal permission settings.'
