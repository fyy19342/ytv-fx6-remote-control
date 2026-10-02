# Distribution manifest 20261002a

- Version: 0.4.3
- Build ID: 20261002a
- Platform: macOS / arm64; host macOS 26.5.1
- Packaged: 2026-10-02T13:58:30.706529+09:00
- App signature: ad-hoc; Apple notarization NOT RUN
- Source input SHA256: 35a6a986087498200e2822825dfa2db4bd41a326939bcc44f24ce14446c36ee1
- Verified artifacts SHA256: bac38c966ee3bca2f4c90ef3b910e056d76d8aa063063a28c00ea34d6cd81bb4
- GUI executable SHA256: dd4b750c084624cc5636be6a2f221666877dbf9e076b7dc47a24f931672eae22
- Embedded backend SHA256: e20a3b42d465300d1ec418592ef5b1b0376418d7011e1a86ba5139361a73b8eb

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
