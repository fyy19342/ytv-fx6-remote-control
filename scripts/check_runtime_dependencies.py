#!/usr/bin/env python3
"""Resolve every non-system load command in the Sony backend runtime."""
import pathlib
import re
import subprocess
import sys
import platform

exe = pathlib.Path(sys.argv[1]).resolve()
root = exe.parent
files = [exe, *sorted(root.rglob('*.dylib'))]
rpaths = [root, root / 'Contents/Frameworks/CrAdapter']
failures = []
for binary in files:
    commands = subprocess.check_output(['otool', '-arch', platform.machine(), '-l', str(binary)], text=True)
    for value in re.findall(r'cmd LC_RPATH\n.*?path (.*?) \(offset', commands, re.S):
        if value.startswith('/Users/') or value.startswith('/opt/'):
            failures.append(f'nonportable rpath: {binary.name}: {value}')
    # Parse LC_LOAD commands, not LC_ID_DYLIB (vendor install IDs can be absolute).
    loads = re.findall(r'cmd LC_(?:LOAD_DYLIB|LOAD_WEAK_DYLIB|REEXPORT_DYLIB)\n\s*cmdsize \d+\n\s*name (.*?) \(offset', commands)
    for dep in loads:
        if dep.startswith(('/System/Library/', '/usr/lib/')):
            continue
        # dylib LC_ID_DYLIB may be its own bare name.
        if dep in (binary.name, str(binary)):
            continue
        choices = []
        if dep.startswith('@rpath/'):
            choices = [p / dep.removeprefix('@rpath/') for p in rpaths]
        elif dep.startswith('@loader_path/'):
            choices = [binary.parent / dep.removeprefix('@loader_path/')]
        elif dep.startswith('@executable_path/'):
            choices = [root / dep.removeprefix('@executable_path/')]
        if not any(p.exists() for p in choices):
            failures.append(f'unresolved/nonportable dependency: {binary.name}: {dep}')
    print(f'CHECK {binary.relative_to(root)}')
if failures:
    raise SystemExit('\n'.join(failures))
print(f'PASS: {len(files)} backend runtime Mach-O files resolve without development paths')
