# Troubleshooting — 20261001b

## 起動しない

`bash scripts/verify_app_bundle.sh` でファイル・build stamp・dylib 依存・署名を確認し、`bash scripts/run_app_from_bundle.sh` で標準エラーを確認します。配布版は `FX6OperationApp.app/Contents/MacOS/FX6OperationApp` を直接起動できます。今回の署名は ad-hoc、公証なしです。macOS が遮断した場合はシステム設定のプライバシーとセキュリティで信頼した app の起動を許可してください。

## 古い backend / ポート競合

`lsof -nP -iTCP:39061 -sTCP:LISTEN` と `/api/health` の `buildId`・`pid`・`executablePath` を照合します。20261001b 以外や buildId 欠落は再利用しません。正体を確認できた対象だけを終了してから再起動してください。無差別な killall は行わないでください。

## カメラ 0 台 / Adaptor_Create (0x8703)

health 成功はカメラ接続成功ではありません。`/api/cameras` と `/api/state`、logPath のログを確認してください。FX6 の接続・ネットワーク・リモート設定と Sony SDK runtime が必要です。`bash scripts/macos_backend_doctor.sh` と `scripts/check_runtime_dependencies.py` で runtime を確認します。今回の実行ログは verification/runtime にあります。

## キーが効かない

接続後の操作画面をクリックし、修飾キーなしの英字入力で i/g/n → u/d を試します。モード未選択、ログイン、別ウィンドウ、モーダル、切断中は無効です。ND が OFF のときは b で ON にします。操作結果欄と接続状態を確認してください。

## ND ON / 値設定のエラー

GUI には HTTP 400 本文の SDK エラーを表示します。`ND OFF confirmed` は失敗後の OFF 読み戻し成功、`ND OFF rollback FAILED` は OFF を確認できなかった意味です。後者は実機で確認してください。Manual/Step 設定は残る場合があります。Gain (ISO) や Iris が自動・取得不可ならカメラ側を手動にして再試行します。
