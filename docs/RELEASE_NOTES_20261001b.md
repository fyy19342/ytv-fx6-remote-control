## FX6 Operation App 0.4.1 — build 20261001b

macOS 15 以降 / Apple Silicon 用の評価版です。アプリにフォーカスがある操作画面で `i / g / n / u / d / b / m` を使用します。Gain は ISO sensitivity として扱い `Gain (ISO)` と表示します。ND ON は最小濃度、ND OFF 中の step は OFF を維持します。

- `FX6OperationApp-20261001b-macos-arm64.zip`: アプリ、SDK-free source/docs/scripts、利用条件、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: Qt/PySide/gettext/libusb のソースと検証用 checksum。通常のアプリ起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) に同意した場合にダウンロード・使用してください。Sony 公式製品ではありません。本アプリで使用・制御する機器のメーカー保証には Sony SDK 規約の条件が適用されます。[インストール手順](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

確認結果は同梱 SELF_CHECK に記載しています。SDK 初期化・backend HTTP・実 Cocoa app 起動と、実機の認証・レンズ/ISO/ND 動作は別項目です。実機は NOT RUN、検出0台は WARN です。Apple 公証は未実施のため、macOS の初回起動確認が必要な場合があります。

GitHub の「Source code」zip にはアプリや Sony SDK が入っていません。
