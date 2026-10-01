# Distribution manifest 20261001b

- Version: 0.4.1
- Build ID: 20261001b
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-01T16:39:43.355950+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: 8887942e9e7477a6f7e1fa2014c648426e23c30b93f32af9e02f171f52c8706a
- Verified artifacts SHA256: 7bee6738728353ead663889c671d888f835eb3a8a0297cb5487e79433538310d
- GUI executable SHA256: 7c843b5efa2cf4d6e13cc9fe38f39fd060b6626c0889390303ec83956ed62bbb
- Embedded backend SHA256: 4f3763d3a016e3f35a1c8ea379f356bf4a485832193647a7fc26ae9b931f3783

The ZIP includes FX6OperationApp.app, SDK-free source/docs/scripts, licenses, build_info.json,
install instructions, terms, SELF_CHECK and a sanitized verification/results.json.
Sony runtime is embedded inside the app only. SDK headers/sample source, standalone SDK libraries,
Stream Deck plugins, incoming files, credentials and raw diagnostic logs are excluded.

FILE_SHA256.json records every regular file (symlinks are retained by ditto).
The archive checksum is in the external .zip.sha256 sidecar. The separate open-source-dependencies.tar
release asset supplies LGPL source archives; it contains no Sony proprietary SDK.

PASS/WARN/FAIL/NOT RUN are separate in SELF_CHECK. Camera enumeration returned zero (WARN).
FX6 authentication and optical/lens/ISO/ND behavior are NOT RUN. CI covers SDK-independent tests;
only the local runtime check exercises SDK initialization and Cocoa app launch.
