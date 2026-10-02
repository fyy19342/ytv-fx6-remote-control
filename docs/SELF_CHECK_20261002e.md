# SELF CHECK 20261002e

Version: 0.4.7 / 2026-10-02T15:19:01.810196+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261002e/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261002e/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261002e/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261002e/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261002e/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261002e/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261002e/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261002e/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261002e/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261002e/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261002e/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261002e/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261002e/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261002e/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261002e/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261002e/python-unit-and-qt-shortcuts.log |
| PASS | native-cocoa-shortcuts-and-layout | dist/checks/20261002e/native-cocoa-shortcuts-and-layout.log |
| PASS | ctest | dist/checks/20261002e/ctest.log |
| PASS | dylib-report | dist/checks/20261002e/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261002e/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261002e/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261002e/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261002e, PID=76695, SDK initialized, cameras=1; verification/results.json |
| PASS | bundle-runtime | build=20261002e, PID=76701, SDK initialized, cameras=1; verification/results.json |
| PASS | app-cocoa | build=20261002e, PID=76721, SDK initialized, cameras=1; verification/results.json |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | Native app visual inspection | CUA inspected the final 20261002e Cocoa app: the connected operation page remained readable; disconnect returned to the 900x600 compact login with no notice/link block or consent checkbox, no clipping, and an enabled Connect button. The default connected page expanded to 1040x800. |
| PASS | FX6 connection without consent checkbox | After user credential entry, the final app authenticated an ILME-FX6V over Wi-Fi without a consent checkbox. Native UI showed Connected and live exposure readouts; /api/state confirmed connected=true. /api/health build 20261002e, embedded executable path and listening PID 76266 matched. |
| PASS | FX6 disconnect and return to compact login | CUA clicked Disconnect, observed the compact login and disconnected message; /api/state confirmed connected=false. |
| NOT RUN | Physical exposure and AWB command regression | This build changes the login UI. No scripted exposure or AWB command sequence was run against the camera in this session. Existing shortcuts, ND, shutter and AWB interval behavior are covered by C++/Qt regression tests; previous-build hardware results are not reused. |
| NOT RUN | AWB white-target measurement accuracy | No white reference target measurement or optical color verification was performed for this build. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
実機認証・キー操作・SDK 読み戻し・映像の光学的変化は個別の項目として判定する。記載のない試験を実施済みとは扱わない。
