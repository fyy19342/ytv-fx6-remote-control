## FX6 Operation App 0.4.7 — build 20261002e

ログイン画面の長い説明、リンク群、同意チェックを削除しました。カメラを選び、User / Password を入力して接続できます。

タイトルと build 表示を1行にまとめ、余白と間隔を調整しました。ログイン画面は900×600で開きます。標準サイズで接続すると操作画面は1040×800に広がり、切断すると900×600へ戻ります。手動で変更したサイズは維持します。利用条件と第三者ライセンスの文書は配布物に同梱します。

このビルドの確認結果:

- **PASS**: ILME-FX6V / Wi-Fi 実機で、同意チェックなしのログインと接続状態の取得を確認。build ID・起動パス・待受 PID を照合しました。
- **PASS**: 実際の `.app` で操作画面の表示と、切断後のコンパクトなログイン画面を確認しました。
- **PASS**: C++ 6件、Python 27件、macOS Cocoa のキー・レイアウト19件。backend 単体・同梱 runtime・`.app` の起動、SDK 初期化、health / cameras と build ID の照合も完了しました。
- **NOT RUN**: この版での実機の露出・AWB キー操作の一連の再試験、白い基準被写体を使った AWB 測定精度。過去の build の実機試験をこの版の結果には流用していません。

A の AWB 再実行間隔は3秒、B は ND ON/OFF 切り替え、S → U/D はシャッタースピード操作です。既存の機能は C++ と実 QShortcut/QTest の回帰試験で確認しました。全検証結果は同梱 `SELF_CHECK_20261002e.md` と `verification/results.json` を参照してください。

macOS 15 以降 / Apple Silicon 向けの評価版です。Apple 公証は未実施です。旧版を終了してから app を丸ごと置き換えてください。

- `FX6OperationApp-20261002e-macos-arm64.zip`: アプリ、SDK を除いた source/docs/scripts、ライセンス、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: LGPL コンポーネントのソースと checksum。通常の起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) / [インストール](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [キー操作](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/KEYBOARD_CONTROLS.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

パスワードは保存しません。AWB はカメラの WB メモリー A/B と白い被写体を準備して実行してください。測定後も WB は Manual を維持します。自動検出が0台のときは IP 指定と指紋照合を使用できます。
