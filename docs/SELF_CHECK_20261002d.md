# SELF CHECK 20261002d

Version: 0.4.6 / 2026-10-02T15:04:51.560920+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261002d/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261002d/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261002d/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261002d/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261002d/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261002d/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261002d/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261002d/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261002d/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261002d/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261002d/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261002d/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261002d/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261002d/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261002d/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261002d/python-unit-and-qt-shortcuts.log |
| PASS | native-cocoa-shortcuts-and-layout | dist/checks/20261002d/native-cocoa-shortcuts-and-layout.log |
| PASS | ctest | dist/checks/20261002d/ctest.log |
| PASS | dylib-report | dist/checks/20261002d/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261002d/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261002d/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261002d/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261002d, PID=74294, SDK initialized, cameras=1; verification/results.json |
| PASS | bundle-runtime | build=20261002d, PID=74300, SDK initialized, cameras=1; verification/results.json |
| PASS | app-cocoa | build=20261002d, PID=74308, SDK initialized, cameras=1; verification/results.json |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | Native app visual inspection | CUA verified the final app showed the 3-second retry guide, the duplicate-key wait message, and the amber unconfirmed message after retry. Japanese text and six cards remained readable at the default window size. |
| PASS | FX6 native AWB duplicate-key rejection | Authenticated ILME-FX6V over Wi-Fi in the final app. CUA selected S and pressed A twice immediately. The second key showed the 3-second wait message without an extra AWB command. Shutter Speed operation mode remained selected. |
| PASS | FX6 AWB three-second backend retry | An immediate direct API retry returned HTTP 400. The next request returned HTTP 200 after 3 seconds. SDK log recorded exactly two Down/Up pairs, with 3.108 s between Down sends; neither required a completion notification. |
| PASS | FX6 AWB release readback | Both Up writes were accepted. A sample 700 ms after the second API response still showed Pressed; a later read confirmed Released and Manual. No additional AWB was sent during confirmation. |
| WARN | FX6 AWB result attribution | No completion callback was observed. Retrying an unresolved request displays unconfirmed and asks the user to check the camera; delayed callbacks are not presented as a confirmed result for the newer request. |
| NOT RUN | AWB white-target measurement accuracy | A white reference target was unavailable. Command interval and SDK state were checked; optical color accuracy and the camera body success indication were not verified. |
| NOT RUN | Other physical exposure and ATW transition regression | This hardware session focused on the AWB interval with WB already Manual. Other controls and transition/failure cases are covered by C++ and Qt regression tests; earlier build hardware evidence is not reused. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
実機認証・キー操作・SDK 読み戻し・映像の光学的変化は個別の項目として判定する。記載のない試験を実施済みとは扱わない。
