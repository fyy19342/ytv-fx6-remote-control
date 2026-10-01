#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ "${1:-}" == "--reuse-verified-build" && "$#" -eq 1 ]]; then
  :
elif [[ "$#" -eq 0 ]]; then
  bash "$ROOT/scripts/build_app_macos.sh"
  bash "$ROOT/scripts/self_check.sh" --skip-build
else
  echo "Usage: $0 [--reuse-verified-build]" >&2
  exit 2
fi
python3 "$ROOT/scripts/collect_open_source.py"
exec python3 "$ROOT/scripts/package_distribution.py"
