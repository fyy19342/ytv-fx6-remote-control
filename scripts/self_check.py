#!/usr/bin/env python3
import argparse
import datetime
import json
import os
import pathlib
import platform
import subprocess
import sys

from release_support import fingerprints

parser = argparse.ArgumentParser()
parser.add_argument('--skip-build', action='store_true')
args = parser.parse_args()
root = pathlib.Path(__file__).resolve().parents[1]
info = json.loads((root / 'build_info.json').read_text())
output = root / 'dist/checks' / info['build_id']
output.mkdir(parents=True, exist_ok=True)
python = root / 'frontend-pyside/.venv/bin/python'
results = []

def run(label, command, env=None):
    log = output / (label + '.log')
    with log.open('w') as stream:
        completed = subprocess.run(list(map(str, command)), cwd=root, env=env, stdout=stream, stderr=subprocess.STDOUT)
    status = 'PASS' if completed.returncode == 0 else 'FAIL'
    results.append({'check': label, 'status': status, 'command': list(map(str, command)), 'log': str(log.relative_to(root)), 'exit': completed.returncode})
    print(status, label, flush=True)
    if completed.returncode:
        print(log.read_text()[-5000:], flush=True)
    return completed.returncode == 0

run('python-syntax', [python, '-m', 'compileall', '-q', root / 'frontend-pyside/src', root / 'frontend-pyside/tests', root / 'scripts'])
for script in sorted((root / 'scripts').glob('*.sh')) + [root / 'frontend-pyside/build_app.sh']:
    run('shell-' + script.name, ['bash', '-n', script])
env = os.environ.copy()
env['QT_QPA_PLATFORM'] = 'offscreen'
env['PYTHONPATH'] = str(root / 'frontend-pyside/src')
run('python-unit-and-qt-shortcuts', [python, '-X', 'faulthandler', '-m', 'unittest', 'discover', '-s', root / 'frontend-pyside/tests', '-v'], env)
if not args.skip_build:
    run('cpp-build-and-ctest', ['bash', root / 'scripts/build_backend_macos.sh'])
else:
    run('ctest', ['ctest', '--test-dir', root / 'backend/build', '--output-on-failure'])
run('dylib-report', ['bash', root / 'scripts/collect_dylib_report.sh'])
run('standalone-dependency-resolution', [sys.executable, root / 'scripts/check_runtime_dependencies.py', root / 'backend/build/fx6d'])
app = root / 'frontend-pyside/dist/FX6OperationApp.app'
if app.exists():
    run('bundle-files-stamps-dependencies-signature', ['bash', root / 'scripts/verify_app_bundle.sh', app])
    run('backend-http-and-cocoa-app-launch', [sys.executable, root / 'scripts/runtime_check.py'])
    runtime = output / 'runtime/results.json'
    if runtime.exists():
        results.extend(json.loads(runtime.read_text())['results'])
else:
    results.append({'check': 'bundle and runtime launch', 'status': 'NOT RUN', 'detail': 'Build the app first.'})
results.append({'check': 'Developer ID / Apple notarization', 'status': 'WARN', 'detail': 'Local ad-hoc signature only. Not notarized.'})
native_observation = output / 'native-observation.json'
if native_observation.exists():
    observed = json.loads(native_observation.read_text())
    if (observed.get('build_id') == info['build_id'] and
            observed.get('artifacts_sha256') == fingerprints(root)['artifacts_sha256']):
        results.append(observed['result'])
report = {'build_id': info['build_id'], 'version': info['version'],
          'time': datetime.datetime.now().astimezone().isoformat(), 'environment': platform.platform(), 'results': results,
          'fingerprints': fingerprints(root)}
(output / 'results.json').write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n')
lines = [f'# SELF CHECK {info["build_id"]}', '', f'Version: {info["version"]} / {report["time"]}', '',
         f'Environment: {report["environment"]}', '',
         'PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。', '',
         'Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。', '',
         '| Status | Check | Evidence / detail |', '| --- | --- | --- |']
for result in results:
    detail = result.get('detail', result.get('log', ''))
    if 'health' in result:
        h = result['health']
        detail = f'build={h["buildId"]}, PID={h["pid"]}, SDK initialized, cameras={result["camera_count"]}; verification/results.json'
    lines.append(f'| {result["status"]} | {result["check"]} | {detail} |')
lines += ['', '詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。',
          'FX6 の認証・実レンズ・ND の光学的変化・物理キーボードによる実機操作は NOT RUN。', '']
(root / 'docs' / f'SELF_CHECK_{info["build_id"]}.md').write_text('\n'.join(lines))
raise SystemExit(1 if any(r['status'] == 'FAIL' for r in results) else 0)
