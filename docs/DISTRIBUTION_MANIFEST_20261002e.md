# Distribution manifest 20261002e

- Version: 0.4.7
- Build ID: 20261002e
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-02T15:19:50.440117+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: b9ff7ead15b51c9938150c7e913d406757cbe4e4c59d3d7ca377dd39a499ef5c
- Verified artifacts SHA256: b4eb1a523cb445acf9a8672ced016af9aa165e1429a6d44985f8fbcda7eeba82
- GUI executable SHA256: f9f4febd3c003a768bf7e1a4c2dab1dfb73c4d91ea92b5fcd4e163bfb30a4d85
- Embedded backend SHA256: ef95ba1b6f142e6220c0ddf87dc453bc0c0d81ca3f7d2d8b96f930c84ff26272

The ZIP includes FX6OperationApp.app, SDK-free source/docs/scripts, licenses, build_info.json,
install instructions, terms, SELF_CHECK and a sanitized verification/results.json.
Sony runtime is embedded inside the app only. SDK headers/sample source, standalone SDK libraries,
Stream Deck plugins, incoming files, credentials and raw diagnostic logs are excluded.

FILE_SHA256.json records every regular file (symlinks are retained by ditto).
The archive checksum is in the external .zip.sha256 sidecar. The separate open-source-dependencies.tar
release asset supplies LGPL source archives; it contains no Sony proprietary SDK.

PASS/WARN/FAIL/NOT RUN are separate in SELF_CHECK and verification/results.json.
Camera discovery, authentication, SDK readback after exposure commands, and optical image changes
are separate checks. CI covers SDK-independent tests. Local runtime checks exercise SDK
initialization and Cocoa app launch; any live camera evidence is tied to the verified artifact digest.
