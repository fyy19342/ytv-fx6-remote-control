# インストール — 20261002b

この配布物は Apple Silicon (arm64) 用です。同梱ライブラリの最低条件に合わせ、macOS 15 以降が必要です。Intel Mac は対象外です。実際に起動を検証した環境は macOS 26.5.1 / arm64 です。

1. [利用条件](TERMS.md) を確認し、同意する場合に [Release](https://github.com/fyy19342/ytv-fx6-remote-control/releases/tag/v0.4.4-20261002b) から `FX6OperationApp-20261002b-macos-arm64.zip` と同名の `.zip.sha256` を同じフォルダへダウンロードします。
2. ターミナルでダウンロード先へ移動し、`shasum -a 256 -c FX6OperationApp-20261002b-macos-arm64.zip.sha256` が `OK` になることを確認します。
3. 古い app と backend を終了します。必要なら `lsof -nP -iTCP:39061 -sTCP:LISTEN` で占有プロセスを確認します。
4. zip を展開し `FX6OperationApp.app` を Applications または任意の書込み可能なフォルダへコピーします。source はアプリ起動には不要です。
5. アプリを起動します。この評価版は ad-hoc 署名で Apple 公証は未実施です。macOS に拒否された場合は発行元と SHA256 を確認した上で「システム設定 → プライバシーとセキュリティ → このまま開く」を使用します。Gatekeeper 全体を無効にしないでください。
6. タイトルと Runtime build が `20261002b` であることを確認します。カメラを選び、表示された Fingerprint を FX6 本体の Network → Access Authentication に表示される指紋と照合します。User / Password を入力し、利用条件に同意する場合は本人がチェックして接続します。

ウィンドウの最小サイズは 800×600 です。下部が見えない場合は縦にスクロールできます。Runtime build 表示にカーソルを置くと、起動元の backend パスを確認できます。20261001b で発生したフォームの縮み・文字切れは 20261001c 以降で修正しています。

自動検出が0台でも Wi-Fi / LAN で IP が分かる場合は、「FX6 IP」に本体の IPv4 アドレスを入力し「IP を確認」を押します。指紋取得後に選択欄が「IP 指定」へ変わります。これは認証前の状態です。指紋を照合してからログインしてください。IP を確認しても接続できない場合は、macOS のローカルネットワーク許可と FX6 のリモート接続設定を確認します。ping 成功や health の `ok=true` は認証成功を意味しません。

## 更新と戻し方

アプリと backend を終了してから app を丸ごと置き換えます。app 内ファイルを部分的に上書きしないでください。以前の app を別フォルダに残しておけば、終了後に入れ替えて戻せます。異なる build の backend が動いていると新 app は起動を拒否します。

ログは通常 `~/Library/Logs/ytv-fx6-remote-control/` です。不具合報告は [Issues](https://github.com/fyy19342/ytv-fx6-remote-control/issues) へ、build ID と再現手順を添えてください。認証情報や個人情報は除去してください。
