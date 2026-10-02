# SELF CHECK 20261002a

Version: 0.4.3 / 2026-10-02T13:57:47.008869+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261002a/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261002a/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261002a/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261002a/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261002a/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261002a/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261002a/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261002a/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261002a/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261002a/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261002a/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261002a/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261002a/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261002a/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261002a/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261002a/python-unit-and-qt-shortcuts.log |
| PASS | native-cocoa-shortcuts-and-layout | dist/checks/20261002a/native-cocoa-shortcuts-and-layout.log |
| PASS | ctest | dist/checks/20261002a/ctest.log |
| PASS | dylib-report | dist/checks/20261002a/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261002a/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261002a/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261002a/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261002a, PID=63895, SDK initialized, cameras=1; verification/results.json |
| PASS | bundle-runtime | build=20261002a, PID=63899, SDK initialized, cameras=1; verification/results.json |
| PASS | app-cocoa | build=20261002a, PID=63904, SDK initialized, cameras=1; verification/results.json |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | native-app-login-and-operation-display | CUA inspected the packaged Cocoa login and connected operation pages: readable Japanese buttons and fields, fingerprint/IP row, Gain (ISO), ND transmittance, mode and feedback. Build/PID/executable path matched the new app. Authentication and real keyboard operations are recorded separately. |
| PASS | FX6 Wi-Fi login: automatic discovery and direct IPv4 | Real ILME-FX6V. User entered credentials in the packaged app. SDK authenticated via automatic discovery, then disconnected and reconnected using a fingerprint-pinned IP target. Credentials were not read or stored by the test. |
| PASS | FX6 native GUI keyboard controls | CUA sent i/u/d, g/u/d, n/u/d, m, b to the running Cocoa app. F4 -> F3.5 -> F4; ISO 12800 -> 16000 -> 12800; ND 1/4 -> 1/5 -> 1/4. OFF remained OFF after u/d; b returned ON at 1/4 and retained the selected mode. These are real SDK readbacks, not an injected API. |
| PASS | FX6 full ND range and limits | Backend commands traversed all 21 reported transmittance values from 1/4 to 1/128 and back, with delayed SDK readback after every command. 49 assertions passed, including both limits, b reset while ON, OFF-step rejection and restoration to 1/4. |
| PASS | FX6 direct IP operation after reconnect | Native n/d/u after direct IPv4 authentication changed 1/4 -> 1/5 -> 1/4. Final values: F4, ISO 12800, ND ON 1/4. ND Variable/Manual is retained for remote operation. |
| NOT RUN | Optical image / photometric measurement | Agent verified SDK property readbacks and app display. Live images and light transmission were not independently measured. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
実機認証・キー操作・SDK 読み戻し・映像の光学的変化は個別の項目として判定する。記載のない試験を実施済みとは扱わない。
