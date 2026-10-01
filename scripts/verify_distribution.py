#!/usr/bin/env python3
"""Verify a public distribution ZIP without executing its contents."""
import argparse
import hashlib
import json
import pathlib
import plistlib
import zipfile


def verify(path):
    with zipfile.ZipFile(path) as archive:
        if archive.testzip() is not None:
            raise ValueError('ZIP CRC failure')
        names = archive.namelist()
        tops = {n.split('/')[0] for n in names}
        if len(tops) != 1:
            raise ValueError('ZIP must have one top-level directory')
        prefix = next(iter(tops)) + '/'
        relative = [n.removeprefix(prefix) for n in names]
        for name in relative:
            parts = pathlib.PurePosixPath(name).parts
            if '..' in parts or name.startswith('/'):
                raise ValueError(f'unsafe archive path: {name}')
            if any(x in parts for x in ['streamdeck-plugin', 'node_modules', 'incoming', '.venv']):
                raise ValueError(f'forbidden distribution file: {name}')
            if name.startswith('source/backend/vendor/') or 'CrDebugString.' in name:
                raise ValueError(f'SDK source in public archive: {name}')
            if name.endswith(('.dylib', '/fx6d')) and not name.startswith('FX6OperationApp.app/'):
                raise ValueError(f'standalone runtime in public archive: {name}')
            if 'QtVirtualKeyboard' in name or 'qtvirtualkeyboardplugin' in name:
                raise ValueError('unexpected Qt Virtual Keyboard component')
        info = json.loads(archive.read(prefix + 'build_info.json'))
        for stamp in ['source/build_info.json', 'FX6OperationApp.app/Contents/Resources/build_info.json',
                      'FX6OperationApp.app/Contents/Resources/backend/build/build_info.json']:
            if json.loads(archive.read(prefix + stamp)) != info:
                raise ValueError(f'build stamp mismatch: {stamp}')
        plist = plistlib.loads(archive.read(prefix + 'FX6OperationApp.app/Contents/Info.plist'))
        if plist['CFBundleVersion'] != info['build_id'] or plist['CFBundleShortVersionString'] != info['version']:
            raise ValueError('Info.plist stamp mismatch')
        inventory = json.loads(archive.read(prefix + 'FILE_SHA256.json'))
        for name, expected in inventory.items():
            if hashlib.sha256(archive.read(prefix + name)).hexdigest() != expected:
                raise ValueError(f'file hash mismatch: {name}')
    return {'status': 'PASS', 'build_id': info['build_id'], 'version': info['version'],
            'zip': pathlib.Path(path).name, 'sha256': hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest(),
            'bytes': pathlib.Path(path).stat().st_size, 'entries': len(names), 'hashed_files': len(inventory),
            'crc': 'PASS', 'build_stamps': 'PASS', 'public_sdk_scope': 'PASS', 'plugin_absent': 'PASS'}


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('zip', type=pathlib.Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.zip), indent=2))
