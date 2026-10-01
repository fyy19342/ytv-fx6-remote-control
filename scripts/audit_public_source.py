#!/usr/bin/env python3
"""Fail if the public source tree includes SDKs, generated binaries or secrets."""
import argparse
import pathlib
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('--directory', type=pathlib.Path, help='Audit an exported source tree; default: git tracked files')
args = parser.parse_args()
root = args.directory.resolve() if args.directory else pathlib.Path(__file__).resolve().parents[1]
if args.directory:
    files = [p for p in root.rglob('*') if p.is_file() or p.is_symlink()]
else:
    files = [root / p for p in subprocess.check_output(['git', 'ls-files', '-z'], cwd=root).decode().split('\0') if p]
forbidden_parts = {'vendor', 'incoming', 'streamdeck-plugin', 'node_modules', '.venv', 'build', 'dist', '__pycache__', 'archive'}
secret = re.compile(rb'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----|ghp_[A-Za-z0-9]{36}|github_pat_[A-Za-z0-9_]{70,}')
failures = []
for path in files:
    rel = path.relative_to(root)
    if (set(rel.parts) & forbidden_parts or path.suffix in {'.dylib', '.so', '.zip', '.p12', '.pfx', '.pyc'}
            or path.name in {'CrDebugString.cpp', 'CrDebugString.h', 'CameraRemote_SDK.h', '.env'}):
        failures.append(f'forbidden public path: {rel}')
    if path.is_symlink():
        failures.append(f'unexpected source symlink: {rel}')
    elif path.is_file():
        if path.stat().st_size > 10 * 1024 * 1024:
            failures.append(f'large source file: {rel}')
        elif secret.search(path.read_bytes()):
            failures.append(f'credential pattern: {rel}')
if not files:
    failures.append('no source files found')
if failures:
    raise SystemExit('\n'.join(failures))
print(f'PASS: {len(files)} public source files; no SDK/runtime/plugin/credential patterns')
