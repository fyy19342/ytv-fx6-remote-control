# SELF CHECK 20261001b

Version: 0.4.1 / 2026-10-01T16:35:02.977654+09:00

Environment: macOS-26.5.1-arm64-arm-64bit

PASS = 実行し期待結果を確認。WARN = 実行済みだが制約あり。FAIL = 期待結果と不一致。NOT RUN = 未実施。

Qt キー試験は実 QShortcut/QTest と API test double を使用。Sony 実機の操作試験とは区別する。

| Status | Check | Evidence / detail |
| --- | --- | --- |
| PASS | python-syntax | dist/checks/20261001b/python-syntax.log |
| PASS | shell-build_app_macos.sh | dist/checks/20261001b/shell-build_app_macos.sh.log |
| PASS | shell-build_backend_macos.sh | dist/checks/20261001b/shell-build_backend_macos.sh.log |
| PASS | shell-collect_dylib_report.sh | dist/checks/20261001b/shell-collect_dylib_report.sh.log |
| PASS | shell-import_sdk.sh | dist/checks/20261001b/shell-import_sdk.sh.log |
| PASS | shell-init_git_repo.sh | dist/checks/20261001b/shell-init_git_repo.sh.log |
| PASS | shell-macos_backend_doctor.sh | dist/checks/20261001b/shell-macos_backend_doctor.sh.log |
| PASS | shell-package_distribution.sh | dist/checks/20261001b/shell-package_distribution.sh.log |
| PASS | shell-package_mac_app.sh | dist/checks/20261001b/shell-package_mac_app.sh.log |
| PASS | shell-package_release.sh | dist/checks/20261001b/shell-package_release.sh.log |
| PASS | shell-run_app_from_bundle.sh | dist/checks/20261001b/shell-run_app_from_bundle.sh.log |
| PASS | shell-run_backend_local.sh | dist/checks/20261001b/shell-run_backend_local.sh.log |
| PASS | shell-self_check.sh | dist/checks/20261001b/shell-self_check.sh.log |
| PASS | shell-verify_app_bundle.sh | dist/checks/20261001b/shell-verify_app_bundle.sh.log |
| PASS | shell-build_app.sh | dist/checks/20261001b/shell-build_app.sh.log |
| PASS | python-unit-and-qt-shortcuts | dist/checks/20261001b/python-unit-and-qt-shortcuts.log |
| PASS | ctest | dist/checks/20261001b/ctest.log |
| PASS | dylib-report | dist/checks/20261001b/dylib-report.log |
| PASS | standalone-dependency-resolution | dist/checks/20261001b/standalone-dependency-resolution.log |
| PASS | bundle-files-stamps-dependencies-signature | dist/checks/20261001b/bundle-files-stamps-dependencies-signature.log |
| PASS | backend-http-and-cocoa-app-launch | dist/checks/20261001b/backend-http-and-cocoa-app-launch.log |
| PASS | standalone | build=20261001b, PID=46133, SDK initialized, cameras=0; verification/results.json |
| PASS | bundle-runtime | build=20261001b, PID=46145, SDK initialized, cameras=0; verification/results.json |
| PASS | app-cocoa | build=20261001b, PID=46176, SDK initialized, cameras=0; verification/results.json |
| NOT RUN | FX6 physical camera login/exposure/hardware keyboard operation | No physical camera credentials/operation were used. Qt tests use an injected API. |
| WARN | camera discovery | HTTP and SDK enumeration ran; zero cameras discovered. |
| WARN | Developer ID / Apple notarization | Local ad-hoc signature only. Not notarized. |
| PASS | native-login-display-and-consent | CUA: packaged Cocoa app shows build 20261001b; consent unchecked and Connect disabled; notice/links visible; app quit normally. Camera connection NOT RUN. |

詳細ログはローカル dist/checks に保存。公開配布の verification/ はパスを置換した結果 JSON のみ。過去 build の結果は流用していない。
FX6 の認証・実レンズ・ND の光学的変化・物理キーボードによる実機操作は NOT RUN。
