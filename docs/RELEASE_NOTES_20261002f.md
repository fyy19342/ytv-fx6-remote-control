## FX6 Operation App 0.4.8 — build 20261002f

操作画面を **1040×800 から800×480** に縮めました。Iris・Gain (ISO)・Shutter・ND の4項目を横1列にまとめ、数値の読みやすさを保ちながら余白を減らしています。

- カメラ名はヘッダー、backend 情報はフッターへ移動しました。接続先 ID やパスはカーソルを重ねて確認できます。
- 簡単なキー案内は常時表示し、詳しい説明は「キー操作」ボタンで開閉できます。
- White Balance・AWB の状態と操作結果は常に表示します。AWB の3秒間隔、ND トグル、シャッタースピードなどのキー割り当ては同じです。
- 標準サイズで接続すると800×480へ縮まり、切断するとログイン画面900×600に戻ります。手動変更したサイズは各画面の最小サイズの範囲内で維持します。

このビルドの試験結果は同梱 `SELF_CHECK_20261002f.md` と `verification/results.json` を参照してください。実機の表示・接続と、Qt の API test double によるキー操作試験は区別して記録しています。

- **PASS**: ILME-FX6V / Wi-Fi の実機に接続した `.app` で、800×480 の表示、説明の開閉、i/g/n/s の各モード選択を確認。説明を閉じても選択モードを維持し、確認前後の露出・ND・WB 読み戻しは一致しました。
- **PASS**: 切断時に900×600のログイン画面へ戻ることを確認しました。
- **PASS**: C++ 6件、Python 27件、macOS Cocoa のキー・レイアウト19件。backend 単体・同梱 runtime・`.app` の実起動、SDK 初期化、health / cameras と build ID・PID・パスの照合も完了しました。
- **NOT RUN**: この版での実機の露出・AWB 調整の一連の再試験、白い基準被写体を使った測定精度。過去 build の実機結果は流用していません。

macOS 15 以降 / Apple Silicon 向けの評価版です。Apple 公証は未実施です。旧版を終了してから app を丸ごと置き換えてください。

- `FX6OperationApp-20261002f-macos-arm64.zip`: アプリ、SDK を除いた source/docs/scripts、ライセンス、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: LGPL コンポーネントのソースと checksum。通常の起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) / [インストール](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [キー操作](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/KEYBOARD_CONTROLS.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

パスワードは保存しません。AWB は本体の WB メモリー A/B と白い被写体を準備して実行してください。自動検出が0台のときは IP 指定と指紋照合を使用できます。
