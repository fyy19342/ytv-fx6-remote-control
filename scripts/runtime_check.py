#!/usr/bin/env python3
"""Launch genuine SDK runtimes/app; refuse an occupied port, verify PID and stamp."""
import argparse
import datetime
import json
import os
import pathlib
import socket
import subprocess
import time
import urllib.request

parser = argparse.ArgumentParser()
parser.add_argument('--root', type=pathlib.Path, default=pathlib.Path(__file__).resolve().parents[1])
parser.add_argument('--app', type=pathlib.Path)
parser.add_argument('--backend', type=pathlib.Path)
parser.add_argument('--out', type=pathlib.Path)
args = parser.parse_args()
root = args.root.resolve()
info = json.loads((root / 'build_info.json').read_text())
app = (args.app or root / 'frontend-pyside/dist/FX6OperationApp.app').resolve()
backend = (args.backend or root / 'backend/build/fx6d').resolve()
out = args.out or root / 'dist/checks' / info['build_id'] / 'runtime'
out.mkdir(parents=True, exist_ok=True)
port = info['backend_port']
url = f'http://127.0.0.1:{port}'
results = []

def request(path, post=False, timeout=60):
    with urllib.request.urlopen(urllib.request.Request(url + path, data=b'' if post else None), timeout=timeout) as response:
        payload = json.load(response)
    assert payload['ok'], payload
    return payload['data']

def check_free():
    try:
        with socket.create_connection(('127.0.0.1', port), timeout=0.4):
            pass
    except OSError:
        return
    listing = subprocess.run(['lsof', '-nP', f'-iTCP:{port}', '-sTCP:LISTEN'], capture_output=True, text=True)
    raise RuntimeError('Port is already occupied; no process was stopped.\n' + listing.stdout)

try:
    for label, binary, expected_backend in [
        ('standalone', backend, backend),
        ('bundle-runtime', app / 'Contents/Resources/backend/build/fx6d', app / 'Contents/Resources/backend/build/fx6d'),
        ('app-cocoa', app / 'Contents/MacOS/FX6OperationApp', app / 'Contents/Resources/backend/build/fx6d')]:
        check_free()
        env = os.environ.copy()
        for key in ['QT_QPA_PLATFORM', 'FX6_BACKEND_BINARY', 'PYTHONPATH', 'DYLD_LIBRARY_PATH', 'DYLD_FALLBACK_LIBRARY_PATH']:
            env.pop(key, None)
        env['FX6_LOG_DIR'] = str((out / label / 'logs').resolve())
        env['DYLD_PRINT_LIBRARIES'] = '1'
        health = None
        with (out / f'{label}.log').open('w') as log:
            process = subprocess.Popen([str(binary)], cwd='/private/tmp', env=env, stdout=log, stderr=log)
            try:
                deadline = time.monotonic() + 30
                while time.monotonic() < deadline:
                    if process.poll() is not None:
                        raise RuntimeError(f'{label} exited {process.returncode}; see {log.name}')
                    try:
                        health = request('/api/health', timeout=1)
                        break
                    except (OSError, ValueError):
                        time.sleep(0.2)
                assert health is not None, 'health timeout'
                assert health['buildId'] == info['build_id'] and health['version'] == info['version'], health
                assert pathlib.Path(health['executablePath']).resolve() == expected_backend.resolve(), health
                if label != 'app-cocoa':
                    assert health['pid'] == process.pid, health
                else:
                    ppid = subprocess.check_output(['ps', '-o', 'ppid=', '-p', str(health['pid'])], text=True).strip()
                    assert int(ppid) == process.pid, (ppid, process.pid)
                listener = subprocess.check_output(['lsof', '-nP', f'-iTCP:{port}', '-sTCP:LISTEN'], text=True)
                assert str(health['pid']) in listener, listener
                state = request('/api/state')
                assert state['sdkInitialized'], state
                cameras = request('/api/cameras')
                assert isinstance(cameras, list), cameras
                if label == 'app-cocoa':
                    # Allow the scheduled camera enumeration/login paint to finish.
                    time.sleep(2)
                    assert process.poll() is None, 'app quit unexpectedly'
                entry = {'check': label, 'status': 'PASS', 'health': health, 'listener': listener,
                         'camera_count': len(cameras), 'sdk_initialized': state['sdkInitialized']}
                results.append(entry)
                print(f'PASS {label}: build={health["buildId"]} pid={health["pid"]} cameras={len(cameras)}', flush=True)
            finally:
                if health and pathlib.Path(health.get('executablePath', '')).resolve() == expected_backend.resolve():
                    try:
                        current = request('/api/health', timeout=1)
                        if current.get('pid') == health['pid']:
                            # GUI's scheduled enumeration may still hold the SDK
                            # mutex. Wait for its bounded scan before shutdown.
                            request('/api/quit', post=True, timeout=60)
                    except (OSError, ValueError):
                        pass
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.terminate()
                    process.wait(timeout=5)
                time.sleep(0.3)
        deadline = time.monotonic() + 10
        while True:
            try:
                check_free()
                break
            except RuntimeError:
                if time.monotonic() >= deadline:
                    raise
                time.sleep(0.2)
    results.append({'check': 'FX6 physical camera login/exposure/hardware keyboard operation', 'status': 'NOT RUN',
                    'detail': 'No physical camera credentials/operation were used. Qt tests use an injected API.'})
    if all(r.get('camera_count', 0) == 0 for r in results):
        results.append({'check': 'camera discovery', 'status': 'WARN', 'detail': 'HTTP and SDK enumeration ran; zero cameras discovered.'})
except Exception as exc:
    results.append({'check': 'runtime validation', 'status': 'FAIL', 'detail': str(exc)})
    raise
finally:
    (out / 'results.json').write_text(json.dumps({'build_id': info['build_id'], 'time': datetime.datetime.now().astimezone().isoformat(), 'results': results}, indent=2) + '\n')
