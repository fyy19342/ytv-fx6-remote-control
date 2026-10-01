#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP_DIR="$ROOT/frontend-pyside"
APP_PATH="$APP_DIR/dist/FX6OperationApp.app"
export PYINSTALLER_CONFIG_DIR="$APP_DIR/.pyinstaller"
export PIP_DISABLE_PIP_VERSION_CHECK=1

bash "$ROOT/scripts/build_backend_macos.sh"

cd "$APP_DIR"
python3 -m venv .venv
source .venv/bin/activate
if ! python - <<'PY'
from importlib.metadata import version, PackageNotFoundError
from pathlib import Path
try:
    for line in Path('requirements.txt').read_text().splitlines():
        if line and not line.startswith('#'):
            name, expected = line.split('==')
            assert version(name) == expected
except (PackageNotFoundError, AssertionError):
    raise SystemExit(1)
PY
then
  python -m pip install -r requirements.txt
fi

rm -rf build dist "$PYINSTALLER_CONFIG_DIR"
pyinstaller --clean --noconfirm FX6OperationApp.spec

bash "$ROOT/scripts/package_mac_app.sh" "$APP_PATH" "$ROOT/backend/build"
bash "$ROOT/scripts/verify_app_bundle.sh" "$APP_PATH"

echo "app ready: $APP_PATH"
