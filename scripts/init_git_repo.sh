#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
if [ -d .git ]; then
  echo "Existing Git repository; keeping history and ignore rules."
else
  git init -b main
fi
echo "Stage source deliberately, then run: python3 scripts/audit_public_source.py"
echo "Never stage backend/vendor/sony, incoming, dist, or credentials."
