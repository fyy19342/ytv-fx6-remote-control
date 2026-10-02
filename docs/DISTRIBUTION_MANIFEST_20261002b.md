# Distribution manifest 20261002b

- Version: 0.4.4
- Build ID: 20261002b
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-02T14:19:43.172736+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: 815a918118c21275fdcfa9f5be37d00ce1a79d991a87ae654511b81ea7b9fd3e
- Verified artifacts SHA256: 3eef820333d577e158b1989422b8b5885a6a6e376651d1ac86af22c6828e8629
- GUI executable SHA256: bd11376bfeed80eed36e6ee0a85d709a16e23d9f8d1080b3abfe41fbcbcbca75
- Embedded backend SHA256: d60d62b79fa89422469fb384bf2fcb9196528316d9965e85266332cea780042c

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
