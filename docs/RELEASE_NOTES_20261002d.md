## FX6 Operation App 0.4.6 — build 20261002d

A キーの AWB 再実行待ちを3秒に変更しました。前回の実行から3秒未満は連打を拒否し、3秒後は結果通知を待たずに再実行できます。完了／失敗通知が早く届いても3秒間の制限は維持します。長押しの自動反復は引き続き無効です。

GUI と backend の両方で待ち時間を確認します。GUI は状態の更新周期を待たず3秒後に再入力可能になります。前回の結果が未確認のまま再実行した場合は、遅れて届いた通知を今回の測定の成功と取り違えないよう、結果はカメラ本体で確認する表示になります。

このビルドの ILME-FX6V / Wi-Fi 実機検証:

- **PASS**: 実際の A 連打を拒否。直後の直接 API 要求も HTTP 400 で拒否。
- **PASS**: 約3秒後の再実行は HTTP 200。SDK ログの押下間隔は **3.108秒**。押下・解放は各2回で、余分な連打送信はありません。
- **PASS**: 解放の読み戻しと Manual 維持。2回目の応答から700msの読み戻しでは Pressed が残りましたが、後の取得で Released を確認。確認中の追加 AWB は行っていません。
- **WARN**: AWB 完了通知は返りませんでした。未確認の測定に重ねて再実行した場合は「結果はカメラ本体で確認してください」と表示します。
- **NOT RUN**: 白い基準被写体での測定精度、映像の色再現、カメラ本体の成功表示。今回の実機試験は再実行間隔を対象としています。

詳細な実行結果は同梱 SELF_CHECK_20261002d.md と verification/results.json を参照してください。C++ と実 QShortcut テストは 2999ms / 3000ms の境界、無通知、早い完了／失敗通知、待ち時間の再設定も対象にしています。

macOS 15 以降 / Apple Silicon 向けの評価版です。Apple 公証は未実施です。旧版を終了してから app を丸ごと置き換えてください。

- `FX6OperationApp-20261002d-macos-arm64.zip`: アプリ、SDK を除いた source/docs/scripts、ライセンス、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: LGPL コンポーネントのソースと checksum。通常の起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) に同意した場合にダウンロード・使用してください。Sony 公式製品ではありません。[インストール](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [キー操作](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/KEYBOARD_CONTROLS.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

パスワードは保存しません。AWB はカメラの WB メモリー A/B と白い被写体を準備して実行してください。測定後も WB は Manual を維持します。自動検出が0台のときは IP 指定と指紋照合を使用できます。
