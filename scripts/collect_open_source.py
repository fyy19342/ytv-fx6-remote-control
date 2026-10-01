#!/usr/bin/env python3
"""Download checksum-pinned LGPL sources and make a separate release asset."""
import hashlib
import json
import pathlib
import tarfile
import urllib.request

root = pathlib.Path(__file__).resolve().parents[1]
info = json.loads((root / 'build_info.json').read_text())
manifest = root / 'scripts/open_source_dependencies.json'
cache = root / 'dist/open-source/cache'
cache.mkdir(parents=True, exist_ok=True)
entries = json.loads(manifest.read_text())
for entry in entries:
    path = cache / entry['file']
    if not path.exists():
        partial = path.with_suffix(path.suffix + '.partial')
        with urllib.request.urlopen(entry['url'], timeout=90) as response, partial.open('wb') as out:
            while block := response.read(1024 * 1024):
                out.write(block)
        partial.replace(path)
    if hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
        raise SystemExit(f'Source checksum mismatch: {entry["file"]}')
archive = root / 'dist' / f'FX6OperationApp-{info["build_id"]}-open-source-dependencies.tar'
with tarfile.open(archive, 'w') as out:
    for entry in entries:
        out.add(cache / entry['file'], arcname=entry['file'], recursive=False)
    out.add(manifest, arcname='SOURCES.json')
    out.add(root / 'THIRD_PARTY_NOTICES.md', arcname='THIRD_PARTY_NOTICES.md')
checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
archive.with_suffix('.tar.sha256').write_text(checksum + '  ' + archive.name + '\n')
print(archive)
print(checksum)
