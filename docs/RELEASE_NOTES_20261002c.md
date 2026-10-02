## FX6 Operation App 0.4.5 — build 20261002c

A キーに、1回測定して固定する Auto White Balance（AWB）を追加しました。選択中の露出操作モードを保ち、必要な場合はホワイトバランスを Manual にしてから押下・解放します。測定後も Manual を維持します。B の ND ON/OFF 切り替え、M の OFF、S → U/D のシャッタースピード操作も使用できます。

カメラ本体の WHT BAL をメモリー A または B にし、被写体と同じ照明下の白い紙などを画面内に大きく映してから A を押してください。White Balance 行に色温度、Manual / ATW と結果を表示します。結果確認中の連打と長押し反復は受け付けません。Down が失敗した場合も Up を試み、解放失敗を表示します。

このビルドの ILME-FX6V / Wi-Fi 実機検証:

- **PASS**: 新しいアプリへの認証、実際の A キー入力、SDK の Down / Up 各1回（間隔105 ms）。読み戻しは Pressed → Released、Manual を維持。
- **PASS**: 色温度の読み戻しが 4847 K → 3049 K に変わり、GUI に反映。A の前に選んだ Shutter Speed 操作モードを維持。
- **WARN**: この個体から AWB 完了通知は返りませんでした。15秒の期限後は黄色の「結果未確認」を表示し、指示の送信成功を測定成功として表示しません。
- **NOT RUN**: 白い基準被写体を準備できなかったため、測定精度・映像の色再現・カメラ本体の成功表示は未確認。色温度の変化だけでは測定の成功と判定していません。
- **NOT RUN**: 実機は初めから Manual だったため、ATW からの切り替えと障害注入は実機未実施。これらの設定順序、Down/Up の失敗処理、結果通知・期限・切断は SDK を注入した C++ テストで確認しました。

自動検証は Python 26件、C++ 6件、macOS Cocoa の実キー・レイアウト18件が PASS。backend 単体、app 内 runtime、app 本体の起動、health の build ID / PID / 実行パス照合、依存解決も PASS です。3種類の起動試験でそれぞれカメラを1台検出しました。

詳細な実行結果は同梱の SELF_CHECK_20261002c.md と verification/results.json を参照してください。過去ビルドの実機結果をこのビルドの PASS として流用していません。操作画面には6枚のカードと White Balance 行を表示し、800×600 以上と縦スクロールに対応します。

macOS 15 以降 / Apple Silicon 向けの評価版です。Apple 公証は未実施です。旧版を終了してから app を丸ごと置き換えてください。

- `FX6OperationApp-20261002c-macos-arm64.zip`: アプリ、SDK を除いた source/docs/scripts、ライセンス、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: LGPL コンポーネントのソースと checksum。通常の起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) に同意した場合にダウンロード・使用してください。Sony 公式製品ではありません。メーカー保証には Sony SDK 規約上の条件があります。[インストール](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [キー操作](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/KEYBOARD_CONTROLS.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

パスワードは保存しません。ND は Variable / Manual、シャッターは手動 Speed、AWB 後の WB は Manual を維持します。GitHub の「Source code」zip にはアプリや Sony SDK が入っていません。自動検出が0台のときは、IP 指定と指紋照合で接続できます。
