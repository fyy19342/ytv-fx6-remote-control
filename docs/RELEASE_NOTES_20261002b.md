## FX6 Operation App 0.4.4 — build 20261002b

B を ND の ON/OFF 切り替えに変更しました。ON なら OFF、OFF なら最も明るい濃度で ON にします。M は OFF 専用として残します。S で Shutter Speed を選び、U で1段遅く、D で1段速くできるようになりました。S だけでは設定を変えず、U/D の調整時に手動 Speed へ切り替えます。

実機 ILME-FX6V の Wi-Fi 接続で確認:

- ネイティブアプリの B で ON → OFF → ON（最小濃度 1/4）、M で OFF。ND OFF 中の U/D は OFF を維持。
- S → D で 1/50 秒 → 1/60 秒、U で 1/50 秒へ復帰。画面と時間を置いた SDK 読み戻しが一致。
- backend でシャッター全11候補（1/50〜1/8000 秒）を往復。上下限、Iris/Gain/ND の不変を含む22確認が PASS。
- 検証終了時に F5、ISO 12800、1/50 秒、ND ON 1/6 へ復帰。

これは実機の SDK 読み戻しと GUI の確認結果です。映像の光学的な測定、および実機の OFF/Auto/Angle/ECS から Speed への切り替え試験は NOT RUN。後者の設定順序と失敗処理は SDK を注入した C++ テストで検証しています。詳細は SELF_CHECK と verification/results.json を参照してください。

macOS 15 以降 / Apple Silicon 向けの評価版です。Apple 公証は未実施です。シャッターを加えた6枚のカードを表示し、800×600 以上の画面と縦スクロールに対応します。旧版を終了してから app を丸ごと置き換えてください。

- `FX6OperationApp-20261002b-macos-arm64.zip`: アプリ、SDK を除いた source/docs/scripts、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: LGPL コンポーネントのソースと checksum。通常の起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) に同意した場合にダウンロード・使用してください。Sony 公式製品ではありません。メーカー保証には Sony SDK 規約上の条件があります。[インストール](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

パスワードは保存しません。ND は Variable / Manual、シャッターは手動 Speed の設定を操作後も維持します。GitHub の「Source code」zip にはアプリや Sony SDK が入っていません。
