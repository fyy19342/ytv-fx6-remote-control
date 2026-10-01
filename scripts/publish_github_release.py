#!/usr/bin/env python3
"""Publish verified distribution assets after the target commit passes CI."""
import argparse
import hashlib
import json
import pathlib
import subprocess
import zipfile

from verify_distribution import verify

parser = argparse.ArgumentParser()
parser.add_argument('--repo', default='fyy19342/ytv-fx6-remote-control')
parser.add_argument('--target', required=True, help='Full commit SHA to release')
parser.add_argument('--publish', action='store_true', help='Publish a prerelease; otherwise make a draft')
args = parser.parse_args()
root = pathlib.Path(__file__).resolve().parents[1]
info = json.loads((root / 'build_info.json').read_text())
build = info['build_id']; tag = f'v{info["version"]}-{build}'
if len(args.target) != 40 or any(c not in '0123456789abcdef' for c in args.target):
    raise SystemExit('--target must be a full commit SHA')
archive = root / 'dist' / f'FX6OperationApp-{build}-macos-arm64.zip'
result = verify(archive)
if result['build_id'] != build:
    raise SystemExit('Wrong archive build ID')
assets = [archive, archive.with_suffix('.zip.sha256'),
          root / 'dist' / f'FX6OperationApp-{build}-open-source-dependencies.tar']
assets.append(assets[2].with_suffix('.tar.sha256'))
for binary, sidecar in [(assets[0], assets[1]), (assets[2], assets[3])]:
    if sidecar.read_text().split()[0] != hashlib.sha256(binary.read_bytes()).hexdigest():
        raise SystemExit(f'Asset checksum mismatch: {binary.name}')
tree = json.loads(subprocess.check_output(['gh', 'api', f'repos/{args.repo}/git/trees/{args.target}?recursive=1']))
if tree.get('truncated'):
    raise SystemExit('Target source tree could not be fully verified')
blobs = {item['path']: item['sha'] for item in tree['tree'] if item['type'] == 'blob'}
with zipfile.ZipFile(archive) as bundle:
    prefix = bundle.namelist()[0].split('/')[0] + '/source/'
    for name in bundle.namelist():
        if not name.startswith(prefix) or name.endswith('/'):
            continue
        content = bundle.read(name)
        blob = hashlib.sha1(b'blob ' + str(len(content)).encode() + b'\0' + content).hexdigest()
        if blobs.get(name.removeprefix(prefix)) != blob:
            raise SystemExit(f'Archive source differs from target commit: {name}')
runs = json.loads(subprocess.check_output(['gh', 'run', 'list', '--repo', args.repo, '--commit', args.target,
                   '--workflow', 'ci.yml', '--json', 'conclusion,headSha,status', '--limit', '10']))
if not any(r['status'] == 'completed' and r['conclusion'] == 'success' for r in runs):
    raise SystemExit('Target commit has no successful CI run')
notes = root / 'docs' / f'RELEASE_NOTES_{build}.md'
cmd = ['gh', 'release', 'create', tag, *map(str, assets), '--repo', args.repo, '--target', args.target,
       '--title', f'FX6 Operation App {info["version"]} ({build})', '--prerelease', '--notes-file', str(notes)]
if not args.publish:
    cmd.append('--draft')
subprocess.run(cmd, check=True)
