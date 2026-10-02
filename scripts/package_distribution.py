#!/usr/bin/env python3
"""Export verified app + SDK-free source for GitHub distribution."""
import datetime
import json
import os
import pathlib
import platform
import shutil
import subprocess

from release_support import fingerprints, sha256
from verify_distribution import verify

root = pathlib.Path(__file__).resolve().parents[1]
info = json.loads((root / 'build_info.json').read_text())
build_id = info['build_id']
checks = root / 'dist/checks' / build_id
report = json.loads((checks / 'results.json').read_text())
assert report['build_id'] == build_id
assert not any(r['status'] == 'FAIL' for r in report['results'])
passed = {r['check'] for r in report['results'] if r['status'] == 'PASS'}
required = {'python-syntax', 'python-unit-and-qt-shortcuts', 'standalone', 'bundle-runtime', 'app-cocoa',
            'bundle-files-stamps-dependencies-signature'}
assert required <= passed, f'Missing required verification: {required - passed}'
assert {'cpp-build-and-ctest', 'ctest'} & passed, 'C++ tests have not passed'
assert report['fingerprints'] == fingerprints(root), 'Build inputs/artifacts changed since self_check'
name = f'FX6OperationApp-{build_id}-macos-arm64'
stage = root / 'dist/distribution' / name
if stage.exists():
    shutil.rmtree(stage)
source = stage / 'source'
source.mkdir(parents=True)
excludes = ['.DS_Store', '__pycache__', '*.pyc', '.venv', '.pyinstaller', 'build', 'build-*', 'dist',
            'archive', 'vendor', 'RELEASE_MANIFEST_*.md', 'DISTRIBUTION_MANIFEST_*.md']
for folder in ['backend', 'frontend-pyside', 'scripts', 'docs', 'licenses', '.github']:
    subprocess.run(['rsync', '-a', *[arg for x in excludes for arg in ['--exclude', x]],
                    str(root / folder), str(source) + '/'], check=True)
for file in ['README.md', 'build_info.json', '.gitignore', 'LICENSE.md', 'THIRD_PARTY_NOTICES.md']:
    shutil.copy2(root / file, source / file)
subprocess.run(['python3', root / 'scripts/audit_public_source.py', '--directory', source], check=True)
for file in ['build_info.json', 'LICENSE.md', 'THIRD_PARTY_NOTICES.md']:
    shutil.copy2(root / file, stage / file)
for file in ['INSTALL.md', 'TERMS.md', f'SELF_CHECK_{build_id}.md']:
    shutil.copy2(root / 'docs' / file, stage / file)
app = stage / 'FX6OperationApp.app'
subprocess.run(['ditto', root / 'frontend-pyside/dist/FX6OperationApp.app', app], check=True)
subprocess.run(['bash', root / 'scripts/verify_app_bundle.sh', app], check=True)
# Public evidence excludes raw logs, user directory names and camera credentials.
verification = stage / 'verification'
verification.mkdir()
public_report = json.loads(json.dumps(report).replace(str(root), '$WORKSPACE').replace(str(pathlib.Path.home()), '$HOME'))
(verification / 'results.json').write_text(json.dumps(public_report, ensure_ascii=False, indent=2) + '\n')
manifest = f'''# Distribution manifest {build_id}

- Version: {info['version']}
- Build ID: {build_id}
- Platform: macOS / arm64; host macOS {platform.mac_ver()[0]}
- Packaged: {datetime.datetime.now().astimezone().isoformat()}
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: {report['fingerprints']['inputs_sha256']}
- Verified artifacts SHA256: {report['fingerprints']['artifacts_sha256']}
- GUI executable SHA256: {sha256(app / 'Contents/MacOS/FX6OperationApp')}
- Embedded backend SHA256: {sha256(app / 'Contents/Resources/backend/build/fx6d')}

The ZIP includes FX6OperationApp.app, SDK-free source/docs/scripts, licenses, build_info.json,
install instructions, terms, SELF_CHECK and a sanitized verification/results.json.
Sony runtime is embedded inside the app only. SDK headers/sample source, standalone SDK libraries,
Stream Deck plugins, incoming files, credentials and raw diagnostic logs are excluded.

FILE_SHA256.json records every regular file (symlinks are retained by ditto).
The archive checksum is in the external .zip.sha256 sidecar. The separate open-source-dependencies.tar
release asset supplies LGPL source archives; it contains no Sony proprietary SDK.

PASS/WARN/FAIL/NOT RUN are separate in SELF_CHECK and verification/results.json.
Camera discovery, authentication, SDK readback after exposure commands, and optical image changes
are separate checks. CI covers SDK-independent tests. Local runtime checks exercise SDK
initialization and Cocoa app launch; any live camera evidence is tied to the verified artifact digest.
'''
for path in [stage / f'DISTRIBUTION_MANIFEST_{build_id}.md', root / 'docs' / f'DISTRIBUTION_MANIFEST_{build_id}.md']:
    path.write_text(manifest)
inventory = {str(p.relative_to(stage)): sha256(p) for p in sorted(stage.rglob('*')) if p.is_file() and not p.is_symlink()}
(stage / 'FILE_SHA256.json').write_text(json.dumps(inventory, indent=2) + '\n')
archive = root / 'dist' / (name + '.zip')
if archive.exists():
    archive.unlink()
subprocess.run(['ditto', '-c', '-k', '--norsrc', '--keepParent', stage, archive], check=True,
               env={**os.environ, 'COPYFILE_DISABLE': '1'})
result = verify(archive)
archive.with_suffix('.zip.sha256').write_text(result['sha256'] + '  ' + archive.name + '\n')
(checks / 'distribution-verification.json').write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps(result, indent=2))
