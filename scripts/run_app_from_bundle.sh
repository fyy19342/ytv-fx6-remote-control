#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP_PATH="${1:-$ROOT/frontend-pyside/dist/FX6OperationApp.app}"
exec "$APP_PATH/Contents/MacOS/FX6OperationApp"
