# インストール — 20261001b

この配布物は Apple Silicon (arm64) 用です。同梱ライブラリの最低条件に合わせ、macOS 15 以降が必要です。Intel Mac は対象外です。実際に起動を検証した環境は macOS 26.5.1 / arm64 です。

1. [利用条件](TERMS.md) を確認し、同意する場合に [Release](https://github.com/fyy19342/ytv-fx6-remote-control/releases/tag/v0.4.1-20261001b) から `FX6OperationApp-20261001b-macos-arm64.zip` と同名の `.zip.sha256` を同じフォルダへダウンロードします。
2. ターミナルでダウンロード先へ移動し、`shasum -a 256 -c FX6OperationApp-20261001b-macos-arm64.zip.sha256` が `OK` になることを確認します。
3. 古い app と backend を終了します。必要なら `lsof -nP -iTCP:39061 -sTCP:LISTEN` で占有プロセスを確認します。
4. zip を展開し `FX6OperationApp.app` を Applications または任意の書込み可能なフォルダへコピーします。source はアプリ起動には不要です。
5. アプリを起動します。この評価版は ad-hoc 署名で Apple 公証は未実施です。macOS に拒否された場合は発行元と SHA256 を確認した上で「システム設定 → プライバシーとセキュリティ → このまま開く」を使用します。Gatekeeper 全体を無効にしないでください。
6. タイトルと Runtime build が `20261001b` であることを確認します。利用条件のチェック欄を本人が確認し、カメラを選び認証情報を入力して接続します。

カメラ一覧が0台の場合は未接続です。health が `ok=true` でも実機認証が成功した意味ではありません。macOS のローカルネットワーク許可、FX6 のリモート接続設定、同一ネットワークまたは対応 USB 接続を確認してください。

## 更新と戻し方

アプリと backend を終了してから app を丸ごと置き換えます。app 内ファイルを部分的に上書きしないでください。以前の app を別フォルダに残しておけば、終了後に入れ替えて戻せます。異なる build の backend が動いていると新 app は起動を拒否します。

ログは通常 `~/Library/Logs/ytv-fx6-remote-control/` です。不具合報告は [Issues](https://github.com/fyy19342/ytv-fx6-remote-control/issues) へ、build ID と再現手順を添えてください。認証情報や個人情報は除去してください。
