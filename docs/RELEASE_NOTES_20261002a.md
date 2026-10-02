## FX6 Operation App 0.4.3 — build 20261002a

FX6 の ND が操作できない不具合を修正しました。非対応の Step モードを固定指定せず、対応する Variable モードへ切り替えます。濃度も FX6 対応の透過率プロパティを使い、実機が返す値の隣へ1段ずつ移動します。自動検出で見つからない場合の IPv4 直接接続と、接続前の指紋照合も追加しています。

実機 ILME-FX6V の Wi-Fi 接続で確認:

- アプリのキーボード操作で Iris F4 → F3.5 → F4、Gain (ISO) 12800 → 16000 → 12800。
- ND の GUI キー n/u/d、b=ON＋最小値、m=OFF。OFF 中の u/d は OFF を維持。
- backend で ND 全21段（1/4〜1/128）を往復し、毎回時間を置いて読み戻し。上下限・再 ON・OFF 中拒否を含む49項目が PASS。
- 自動検出での認証、切断、IP 指定での再認証と ND 操作。

これらは実機の SDK 読み戻しと画面を確認した結果です。映像の光学的変化の測定は NOT RUN。詳細は同梱 SELF_CHECK と verification/results.json に記載します。

macOS 15 以降 / Apple Silicon 向けの評価版です。Apple 公証は未実施です。ログインフォームの文字切れ修正、800×600 以上でのスクロール対応は継続しています。旧版を終了してから app を丸ごと置き換えてください。

- `FX6OperationApp-20261002a-macos-arm64.zip`: アプリ、SDK を除いた source/docs/scripts、自己確認結果。
- `.zip.sha256`: ダウンロード検証用。
- `open-source-dependencies.tar` / `.tar.sha256`: LGPL コンポーネントのソースと検証用 checksum。通常の起動には展開不要です。

[利用条件](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/TERMS.md) に同意した場合にダウンロード・使用してください。Sony 公式製品ではありません。メーカー保証には Sony SDK 規約上の条件があります。[インストール](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/docs/INSTALL.md) / [第三者ライセンス](https://github.com/fyy19342/ytv-fx6-remote-control/blob/main/THIRD_PARTY_NOTICES.md)

パスワードは保存しません。ND 操作後は Variable / Manual を維持します。GitHub の「Source code」zip にはアプリや Sony SDK が入っていません。
