#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP_PATH="${1:-$ROOT/frontend-pyside/dist/FX6OperationApp.app}"
python3 - "$ROOT" "$APP_PATH" <<'PY'
import json, pathlib, plistlib, sys
root, app = map(pathlib.Path, sys.argv[1:])
expected = json.loads((root / 'build_info.json').read_text())
runtime = app / 'Contents/Resources/backend/build'
for name in ['fx6d', 'libCr_Core.dylib', 'libmonitor_protocol.dylib', 'libmonitor_protocol_pf.dylib',
             'Contents/Frameworks/CrAdapter/libCr_PTP_IP.dylib',
             'Contents/Frameworks/CrAdapter/libCr_PTP_USB.dylib',
             'Contents/Frameworks/CrAdapter/libssh2.dylib',
             'Contents/Frameworks/CrAdapter/libusb-1.0.0.dylib']:
    assert (runtime / name).is_file(), name
for stamp in [app / 'Contents/Resources/build_info.json', runtime / 'build_info.json']:
    assert json.loads(stamp.read_text()) == expected, str(stamp)
info = plistlib.loads((app / 'Contents/Info.plist').read_bytes())
assert info['CFBundleVersion'] == expected['build_id']
assert info['CFBundleShortVersionString'] == expected['version']
assert (app / 'Contents/MacOS/FX6OperationApp').is_file()
for name in ['licenses/LGPL-3.0.txt', 'licenses/LGPL-2.1.txt', 'THIRD_PARTY_NOTICES.md', 'TERMS.md']:
    assert (app / 'Contents/Resources' / name).is_file(), name
qt = app / 'Contents/Frameworks/PySide6/Qt'
frameworks = {p.stem for p in (qt / 'lib').glob('*.framework')}
assert frameworks == {'QtCore', 'QtGui', 'QtWidgets', 'QtDBus', 'QtNetwork'}, frameworks
assert not list(qt.rglob('*virtualkeyboard*')), 'Unexpected optional Qt plugin'
broken = [str(p) for p in app.rglob('*') if p.is_symlink() and not p.exists()]
assert not broken, broken
print('PASS bundle files and build stamps:', expected['build_id'])
PY
python3 "$ROOT/scripts/check_runtime_dependencies.py" "$APP_PATH/Contents/Resources/backend/build/fx6d"
python3 "$ROOT/scripts/check_app_dependencies.py" "$APP_PATH"
codesign --verify --deep --strict "$APP_PATH"
echo "PASS app ad-hoc code signature"
