# Distribution manifest 20261002c

- Version: 0.4.5
- Build ID: 20261002c
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-02T14:48:28.624688+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: 2a7dee15e0be44bf81860a44a84f18f870941ad764fbd0c30f6fc80b62b58dad
- Verified artifacts SHA256: 73fef064e2eaf7a8461e924032e65446bf8c2ba566df6d9935e9af29083132a7
- GUI executable SHA256: bebe0d3fe700f74bf8b72f843ec2f4d71237c634d57eb57f4c4972a053e7b67c
- Embedded backend SHA256: 9beb83255023f6489b1d374fb01a5a16a1159d23d4e5c8f93a73ab6c624c035e

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
