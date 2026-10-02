# Distribution manifest 20261002d

- Version: 0.4.6
- Build ID: 20261002d
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-02T15:05:21.734964+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: 2471621b23cbf3733f3a9c3e063c4c869d0204a2f480cd6f25f3562c06f8fdf3
- Verified artifacts SHA256: d644e829decf420db99c11049ca49a2f425c482efb29ad938f8f109a6c90dab0
- GUI executable SHA256: a18a7864472f943992ebf7ca4da18b52fcd738e0ab4cb46bcdbe4121bcb6e5f0
- Embedded backend SHA256: c1edadd4b804e187c24db51af87546f5d8bdc66fefd18be3c5ae5bc0943808b2

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
