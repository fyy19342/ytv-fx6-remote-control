"""Shared integrity stamps for build reuse and release verification."""
import hashlib
import pathlib


def sha256(path):
    digest = hashlib.sha256()
    with pathlib.Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def tree_digest(root, paths):
    digest = hashlib.sha256()
    files = set()
    for path in paths:
        files.update(path.rglob('*') if path.is_dir() else [path])
    for path in sorted(files):
        if '__pycache__' in path.parts or path.suffix == '.pyc' or path.name == '.DS_Store':
            continue
        if path.is_symlink():
            value = 'symlink:' + str(path.readlink())
        elif path.is_file():
            value = sha256(path)
        else:
            continue
        digest.update((str(path.relative_to(root)) + '\0' + value + '\n').encode())
    return digest.hexdigest()


def fingerprints(root):
    app = root / 'frontend-pyside/dist/FX6OperationApp.app'
    backend = root / 'backend/build'
    inputs = [root / p for p in ['build_info.json', 'backend/CMakeLists.txt', 'backend/src', 'backend/tests',
              'backend/vendor/sony', 'backend/third_party', 'frontend-pyside/src', 'frontend-pyside/tests',
              'frontend-pyside/FX6OperationApp.spec', 'frontend-pyside/requirements.txt', 'scripts',
              'licenses', 'THIRD_PARTY_NOTICES.md', 'docs/TERMS.md']]
    artifacts = [app, backend / 'fx6d', backend / 'build_info.json', backend / 'Contents', *backend.glob('*.dylib')]
    return {'inputs_sha256': tree_digest(root, inputs), 'artifacts_sha256': tree_digest(root, artifacts)}
