#!/usr/bin/env python3
"""Check all embedded Mach-O imports and the advertised minimum macOS version."""
import pathlib
import platform
import plistlib
import re
import subprocess
import sys

app = pathlib.Path(sys.argv[1]).resolve()
frameworks = app / 'Contents/Frameworks'
backend = app / 'Contents/Resources/backend/build'
info = plistlib.loads((app / 'Contents/Info.plist').read_bytes())
minimum = tuple(map(int, info['LSMinimumSystemVersion'].split('.')))
magic = {bytes.fromhex(h) for h in ['cffaedfe', 'feedfacf', 'cafebabe', 'bebafeca']}
count = 0
failures = []
for file in app.rglob('*'):
    if not file.is_file() or file.is_symlink():
        continue
    with file.open('rb') as stream:
        if stream.read(4) not in magic:
            continue
    count += 1
    commands = subprocess.check_output(['otool', '-arch', platform.machine(), '-l', file], text=True)
    for value in re.findall(r'\bminos ([0-9.]+)', commands):
        if tuple(map(int, value.split('.'))) > minimum:
            failures.append(f'{file.relative_to(app)} requires macOS {value}; advertised {info["LSMinimumSystemVersion"]}')
    executable = backend if file.is_relative_to(backend) else app / 'Contents/MacOS'
    def expand(value):
        return pathlib.Path(value.replace('@loader_path', str(file.parent)).replace('@executable_path', str(executable)))
    rpaths = [expand(p) for p in re.findall(r'cmd LC_RPATH\n.*?path (.*?) \(offset', commands, re.S)]
    rpaths += [frameworks, frameworks / 'PySide6/Qt/lib', backend, backend / 'Contents/Frameworks/CrAdapter']
    imports = re.findall(r'cmd LC_(?:LOAD_DYLIB|LOAD_WEAK_DYLIB|REEXPORT_DYLIB)\n\s*cmdsize \d+\n\s*name (.*?) \(offset', commands)
    for dep in imports:
        if dep.startswith(('/System/Library/', '/usr/lib/')):
            continue
        candidates = [p / dep.removeprefix('@rpath/') for p in rpaths] if dep.startswith('@rpath/') else [expand(dep)]
        if dep.startswith(('/Users/', '/opt/')) or not any(p.exists() for p in candidates):
            failures.append(f'{file.relative_to(app)}: unresolved or nonportable {dep}')
if failures:
    raise SystemExit('\n'.join(failures))
print(f'PASS: {count} embedded Mach-O files; dependency resolution and macOS {info["LSMinimumSystemVersion"]} minimum')
