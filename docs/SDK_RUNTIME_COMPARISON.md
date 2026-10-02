# SDK runtime layout — 20261002d

Sony RemoteCli / SimpleCli の配置に合わせています。

```text
fx6d
build_info.json
libCr_Core.dylib
libmonitor_protocol.dylib
libmonitor_protocol_pf.dylib
Contents/Frameworks/CrAdapter/
  libCr_PTP_IP.dylib
  libCr_PTP_USB.dylib
  libssh2.dylib
  libusb-1.0.0.dylib
```

開発 build と app の Contents/Resources/backend/build に同じ配置を使用します。配布版に単体 runtime は含めません。app には CMake 中間物を含めません。依存の静的解決と、開発 cwd に依存しない /private/tmp からの実起動を自己確認に含めます。
