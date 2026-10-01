# Distribution manifest 20261001c

- Version: 0.4.2
- Build ID: 20261001c
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-01T17:23:06.348427+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: fc82a016cf69aed4277e6f55d309059bd700a85bddcf90eb130446e8b5470a90
- Verified artifacts SHA256: 6b4e3ce419197c46949a32b4944e0a846f22679fa8e4332734db010ba66661f8
- GUI executable SHA256: 206451f4a479efab6903b4ec5e1c61b979447b0ad3ae2a86bf02e3455c548d7e
- Embedded backend SHA256: d6c1386ba6f6d886cc2441f3234c65c506993fc5e728a81fe1ced16c865478dd

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
