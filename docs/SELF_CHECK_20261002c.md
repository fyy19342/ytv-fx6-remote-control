# SELF CHECK 20261002c

Version: 0.4.5 / 2026-10-02T14:47:53.260443+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261002c/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261002c/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261002c/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261002c/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261002c/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261002c/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261002c/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261002c/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261002c/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261002c/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261002c/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261002c/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261002c/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261002c/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261002c/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261002c/python-unit-and-qt-shortcuts.log |
| PASS | native-cocoa-shortcuts-and-layout | dist/checks/20261002c/native-cocoa-shortcuts-and-layout.log |
| PASS | ctest | dist/checks/20261002c/ctest.log |
| PASS | dylib-report | dist/checks/20261002c/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261002c/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261002c/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261002c/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261002c, PID=71698, SDK initialized, cameras=1; verification/results.json |
| PASS | bundle-runtime | build=20261002c, PID=71703, SDK initialized, cameras=1; verification/results.json |
| PASS | app-cocoa | build=20261002c, PID=71708, SDK initialized, cameras=1; verification/results.json |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | Native app visual inspection | CUA inspected the final 20261002c app before and after A. Six aligned cards, readable Japanese instructions, White Balance status, amber result-unconfirmed message and static request-sent feedback were visible without clipped controls at the default size. |
| PASS | FX6 authentication and native A shortcut | Authenticated Wi-Fi ILME-FX6V in the final Cocoa app. CUA selected S and pressed A once. The app retained Shutter Speed mode, displayed the request status and sent one AWB Down/Up pair 105 ms apart. |
| PASS | FX6 AWB SDK readback | SDK accepted Down and Up. Readback observed Pressed then Released, Manual remained selected, and Colortemp changed from 4847 K to 3049 K. This verifies command handling and readback, not measurement accuracy. |
| WARN | FX6 AWB completion notification | No AWB result callback was received. The app changed running to unconfirmed after the 15-second deadline and showed an amber message; it did not report measurement success. |
| NOT RUN | FX6 AWB from ATW and injected failure paths | Physical camera was already in Manual. ATW-to-Manual sequencing, rejected Down/Up, busy state, callback outcomes, timeout and disconnect are covered by SDK-independent C++ tests, not hardware fault injection. |
| NOT RUN | AWB white-target measurement accuracy | A white reference target was unavailable. No independent image/color measurement or confirmation of the camera body success indication was performed. |
| NOT RUN | Full physical exposure regression on this build | This build was physically tested for AWB. Iris/Gain/ND/Shutter mappings and controllers are covered by regression tests; prior build hardware results are not reused as PASS for these artifacts. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
実機認証・キー操作・SDK 読み戻し・映像の光学的変化は個別の項目として判定する。記載のない試験を実施済みとは扱わない。
