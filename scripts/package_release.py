#!/usr/bin/env python3
import datetime
import json
import os
import pathlib
import platform
import plistlib
import shutil
import subprocess
import sys
import zipfile

from release_support import fingerprints, sha256

root = pathlib.Path(__file__).resolve().parents[1]
info = json.loads((root / 'build_info.json').read_text())
build_id = info['build_id']
checks = root / 'dist/checks' / build_id
verification = json.loads((checks / 'results.json').read_text())
assert verification['build_id'] == build_id
assert not any(r['status'] == 'FAIL' for r in verification['results']), 'self check has FAIL results'
required = {'python-unit-and-qt-shortcuts', 'standalone', 'bundle-runtime', 'app-cocoa', 'bundle-files-stamps-dependencies-signature'}
passed = {r['check'] for r in verification['results'] if r['status'] == 'PASS'}
assert required <= passed, f'missing required checks: {required - passed}'
assert verification['fingerprints'] == fingerprints(root), 'Verified source/artifacts changed. Run build and self_check again.'
name = f'{info["release_zip_prefix"]}-{build_id}'
stage = root / 'dist/release' / name
zip_path = root / 'dist' / (name + '.zip')
if stage.exists(): shutil.rmtree(stage)
(stage / 'artifacts').mkdir(parents=True)
source = stage / 'source'
source.mkdir()
excludes = ['.DS_Store', '__pycache__', '*.pyc', '.venv', '.pyinstaller', 'build', 'dist', 'archive', 'RELEASE_MANIFEST_*.md']
for folder in ['backend', 'frontend-pyside', 'scripts', 'docs']:
    subprocess.run(['rsync', '-a', *[arg for x in excludes for arg in ['--exclude', x]], str(root / folder), str(source) + '/'], check=True)
for file in ['README.md', 'build_info.json', '.gitignore']:
    shutil.copy2(root / file, source / file)
shutil.copy2(root / 'build_info.json', stage / 'build_info.json')
app = stage / 'artifacts/FX6OperationApp.app'
subprocess.run(['ditto', str(root / 'frontend-pyside/dist/FX6OperationApp.app'), str(app)], check=True)
runtime = stage / 'artifacts/backend-build'
runtime.mkdir()
for path in [root / 'backend/build/fx6d', root / 'backend/build/build_info.json', *(root / 'backend/build').glob('*.dylib')]:
    shutil.copy2(path, runtime / path.name)
shutil.copytree(root / 'backend/build/Contents', runtime / 'Contents')
shutil.copytree(checks, stage / 'verification', ignore=shutil.ignore_patterns('__pycache__'))
shutil.copy2(root / 'docs' / f'SELF_CHECK_{build_id}.md', stage / f'SELF_CHECK_{build_id}.md')
subprocess.run(['bash', root / 'scripts/verify_app_bundle.sh', app], check=True)
packages = subprocess.check_output([root / 'frontend-pyside/.venv/bin/python', '-c',
                                  'import sys,PySide6,PyInstaller; print(sys.version.split()[0], PySide6.__version__, PyInstaller.__version__)'], text=True).strip()
manifest = f'''# RELEASE MANIFEST {build_id}

- Build ID: {build_id}
- Version: {info['version']}
- App: FX6 Operation App
- Archive: {zip_path.name}
- Built/packaged: {datetime.datetime.now().astimezone().isoformat()}
- Host: macOS {platform.mac_ver()[0]} / {platform.machine()}
- Python / PySide6 / PyInstaller: {packages}
- Signature: ad-hoc, not notarized
- GUI controls: i / g / n / u / d / b / m; Gain (ISO) is ISO sensitivity
- Source input digest: {verification['fingerprints']['inputs_sha256']}
- Verified artifacts digest: {verification['fingerprints']['artifacts_sha256']}

## Included

- source/: backend/frontend source, current docs, scripts, imported Sony SDK headers/runtime
- artifacts/FX6OperationApp.app: GUI and bundled backend runtime
- artifacts/backend-build/: standalone backend and all SDK runtime libraries
- build_info.json
- SELF_CHECK_{build_id}.md and verification/: actual execution results/logs

No Stream Deck plugin, npm dependencies, incoming archives, old docs, virtual environment or build intermediates are included.

## Executable SHA256

- GUI executable: {sha256(app / 'Contents/MacOS/FX6OperationApp')}
- Standalone backend: {sha256(runtime / 'fx6d')}
- Bundled backend: {sha256(app / 'Contents/Resources/backend/build/fx6d')}
- build_info.json: {sha256(stage / 'build_info.json')}

## Verification limits

PASS / WARN / FAIL / NOT RUN are recorded in SELF_CHECK_{build_id}.md. FX6 physical authentication, lens/ISO/ND response and physical keyboard operation with a camera are NOT RUN. Camera discovery returning zero is WARN, not hardware success. Qt tests use real QShortcut events and an injected API.

The final archive SHA256 is in the external .zip.sha256 file, avoiding a circular checksum inside this archive. Verification of zip contents is recorded externally in dist/checks/{build_id}/archive-verification.json.
'''
for path in [root / 'docs' / f'RELEASE_MANIFEST_{build_id}.md', source / 'docs' / f'RELEASE_MANIFEST_{build_id}.md', stage / f'RELEASE_MANIFEST_{build_id}.md']:
    path.write_text(manifest)
# Hash each regular staged file; preserve bundle symlinks with ditto.
inventory = {str(p.relative_to(stage)): sha256(p) for p in sorted(stage.rglob('*')) if p.is_file() and not p.is_symlink()}
(stage / 'FILE_SHA256.json').write_text(json.dumps(inventory, indent=2) + '\n')
if zip_path.exists(): zip_path.unlink()
env = os.environ.copy()
env['COPYFILE_DISABLE'] = '1'
subprocess.run(['ditto', '-c', '-k', '--norsrc', '--keepParent', str(stage), str(zip_path)], check=True, env=env)
with zipfile.ZipFile(zip_path) as archive:
    assert archive.testzip() is None, 'CRC failure'
    names = archive.namelist()
    assert all('streamdeck-plugin/' not in n.lower() and '.sdplugin/' not in n.lower() and '/node_modules/' not in n for n in names)
    for relative in ['build_info.json', 'source/build_info.json', 'artifacts/backend-build/build_info.json',
                     'artifacts/FX6OperationApp.app/Contents/Resources/build_info.json',
                     'artifacts/FX6OperationApp.app/Contents/Resources/backend/build/build_info.json']:
        assert json.loads(archive.read(name + '/' + relative)) == info, relative
    plist = plistlib.loads(archive.read(name + '/artifacts/FX6OperationApp.app/Contents/Info.plist'))
    assert plist['CFBundleVersion'] == build_id and plist['CFBundleShortVersionString'] == info['version']
    for relative, expected_hash in inventory.items():
        import hashlib
        assert hashlib.sha256(archive.read(name + '/' + relative)).hexdigest() == expected_hash, relative
checksum = sha256(zip_path)
zip_path.with_suffix('.zip.sha256').write_text(checksum + '  ' + zip_path.name + '\n')
(checks / 'archive-verification.json').write_text(json.dumps({'status': 'PASS', 'build_id': build_id,
        'zip': str(zip_path), 'sha256': checksum, 'bytes': zip_path.stat().st_size, 'entries': len(names),
        'hashed_files': len(inventory), 'crc': 'PASS', 'build_stamps': 'PASS', 'plugin_absent': 'PASS'}, indent=2) + '\n')
print('Release ready:', zip_path)
print('SHA256:', checksum)
