# SELF CHECK 20261002b

Version: 0.4.4 / 2026-10-02T14:19:01.504308+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261002b/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261002b/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261002b/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261002b/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261002b/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261002b/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261002b/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261002b/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261002b/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261002b/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261002b/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261002b/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261002b/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261002b/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261002b/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261002b/python-unit-and-qt-shortcuts.log |
| PASS | native-cocoa-shortcuts-and-layout | dist/checks/20261002b/native-cocoa-shortcuts-and-layout.log |
| PASS | ctest | dist/checks/20261002b/ctest.log |
| PASS | dylib-report | dist/checks/20261002b/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261002b/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261002b/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261002b/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261002b, PID=67058, SDK initialized, cameras=1; verification/results.json |
| PASS | bundle-runtime | build=20261002b, PID=67082, SDK initialized, cameras=1; verification/results.json |
| PASS | app-cocoa | build=20261002b, PID=67087, SDK initialized, cameras=1; verification/results.json |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | Native app visual inspection | CUA inspection of build 20261002b showed the connected operation page with six aligned cards, readable Japanese instructions, Shutter Speed 1/50 s and B ON/OFF feedback. No clipped controls observed at the default size. |
| PASS | FX6 authentication and native keyboard ND toggle | Wi-Fi ILME-FX6V authentication in the new Cocoa app. CUA B changed ON 1/6 to OFF, n/u/d left OFF, B returned ON at minimum 1/4. M set OFF. B/M retained Shutter Speed mode. Native n/d/d restored 1/6. Delayed SDK readbacks recorded. |
| PASS | FX6 native shutter shortcuts | CUA S/D changed 1/50 s to 1/60 s; U returned to 1/50 s. GUI values and delayed SDK readbacks agreed. S selects the operation mode without an immediate camera setting change. |
| PASS | FX6 shutter full range and limits | All 11 reported speed values (1/50 to 1/8000 s) traversed faster and slower via backend. 22 checks passed with immediate and delayed SDK readbacks, both limit clamps, and no changes to Iris/Gain/ND. Restored F5, ISO 12800, Speed 1/50 s, ND ON 1/6. |
| NOT RUN | FX6 shutter transition from OFF/Auto/Angle/ECS | The physical camera started in manual Speed. Mode preparation and failure paths were tested with an injected property API; these other starting modes were not exercised on hardware. |
| NOT RUN | Optical image measurement | Exposure values were verified using real SDK readback and GUI display. No independent optical measurement of recorded images was performed. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
実機認証・キー操作・SDK 読み戻し・映像の光学的変化は個別の項目として判定する。記載のない試験を実施済みとは扱わない。
