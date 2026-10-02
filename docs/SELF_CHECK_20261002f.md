# SELF CHECK 20261002f

Version: 0.4.8 / 2026-10-02T15:35:25.715715+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261002f/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261002f/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261002f/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261002f/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261002f/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261002f/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261002f/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261002f/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261002f/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261002f/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261002f/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261002f/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261002f/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261002f/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261002f/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261002f/python-unit-and-qt-shortcuts.log |
| PASS | native-cocoa-shortcuts-and-layout | dist/checks/20261002f/native-cocoa-shortcuts-and-layout.log |
| PASS | ctest | dist/checks/20261002f/ctest.log |
| PASS | dylib-report | dist/checks/20261002f/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261002f/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261002f/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261002f/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261002f, PID=79341, SDK initialized, cameras=1; verification/results.json |
| PASS | bundle-runtime | build=20261002f, PID=79355, SDK initialized, cameras=1; verification/results.json |
| PASS | app-cocoa | build=20261002f, PID=79372, SDK initialized, cameras=1; verification/results.json |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | Native compact operation screen | CUA inspected the final app connected to an ILME-FX6V: the 800x480 content area displayed all four exposure cards, WB status, feedback and buttons without clipping. Expanded and collapsed keyboard help fitted the same window. Disconnect returned to the 900x600 login. |
| PASS | FX6 authentication and compact screen | The user authenticated an ILME-FX6V over Wi-Fi in build 20261002f. UI Connected and /api/state readback were confirmed. /api/health, listening PID 78758 and the embedded backend path matched. |
| PASS | Native mode shortcuts while help is open | CUA opened keyboard help and pressed i, g, n and s in sequence; each corresponding mode appeared. Closing help preserved Shutter Speed mode. Before/after API readbacks of exposure, ND and WB matched; no exposure or AWB adjustment command was sent. |
| PASS | FX6 disconnect and login resize | CUA clicked Disconnect and observed the disconnected message and 900x600 login before quitting the app. |
| NOT RUN | Physical exposure and AWB adjustment regression | This build changes the operation UI. Hardware adjustment sequences and optical changes were not repeated. The C++ and Qt suites cover keyboard command mapping, focus, ND, shutter and AWB interval regression; prior-build hardware evidence is not reused. |
| NOT RUN | AWB white-target measurement accuracy | No white reference target measurement or optical color verification was performed for this build. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
実機認証・キー操作・SDK 読み戻し・映像の光学的変化は個別の項目として判定する。記載のない試験を実施済みとは扱わない。
