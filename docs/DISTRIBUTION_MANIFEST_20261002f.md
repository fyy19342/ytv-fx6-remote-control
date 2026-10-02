# Distribution manifest 20261002f

- Version: 0.4.8
- Build ID: 20261002f
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-02T15:35:58.010412+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: ede8a23f0eb5371aeccea5ab29453cb0f28715eb83a94819693444d2f33c53a9
- Verified artifacts SHA256: 86f158cdb2348fcae268502323dbc604eede6694650ac3c5e3e133290cced9ff
- GUI executable SHA256: 67ca12f587f27f0792e1a105692257ee4eeab40c7f23fa13dfaa3e71f40ae3f0
- Embedded backend SHA256: 340419ec4ae6ab0a0c9fdca4abf761ecaf14f0089f7061a466569902d9eadb66

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
