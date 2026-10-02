# Troubleshooting — 20261002b

## 起動しない

`bash scripts/verify_app_bundle.sh` でファイル・build stamp・dylib 依存・署名を確認し、`bash scripts/run_app_from_bundle.sh` で標準エラーを確認します。配布版は `FX6OperationApp.app/Contents/MacOS/FX6OperationApp` を直接起動できます。今回の署名は ad-hoc、公証なしです。macOS が遮断した場合はシステム設定のプライバシーとセキュリティで信頼した app の起動を許可してください。

## 古い backend / ポート競合

`lsof -nP -iTCP:39061 -sTCP:LISTEN` と `/api/health` の `buildId`・`pid`・`executablePath` を照合します。20261002b 以外や buildId 欠落は再利用しません。正体を確認できた対象だけを終了してから再起動してください。無差別な killall は行わないでください。

## カメラ 0 台 / Adaptor_Create (0x8703)

health 成功はカメラ接続成功ではありません。`/api/cameras` と `/api/state`、logPath のログを確認してください。FX6 の接続・ネットワーク・リモート設定と Sony SDK runtime が必要です。`bash scripts/macos_backend_doctor.sh` と `scripts/check_runtime_dependencies.py` で runtime を確認します。詳細ログはローカルの dist/checks にあり、公開配布には結果の要約だけが入ります。

Wi-Fi / LAN で ping が通っても SDK の自動検出が0台になることがあります。ログイン画面の FX6 IP に本体の IPv4 アドレスを入力し「IP を確認」を試してください。指紋取得は認証とは別です。User / Password は FX6 本体の Access Authentication 設定を使用します。Wi-Fi の Station Mode では Camera Remote Control を有効にします。

「fingerprint changed」「Check the IP and displayed fingerprint again」は接続対象の再確認が必要な状態です。カメラ本体の指紋と照合し、再度「IP を確認」してください。認証を無効にしたり、パスワードをログや不具合報告へ貼り付けたりしないでください。

## キーが効かない

接続後の操作画面をクリックし、修飾キーなしの英字入力で i/g/n/s → u/d を試します。モード未選択、ログイン、別ウィンドウ、モーダル、切断中は無効です。ND が OFF のときは b で ON にします。操作結果欄と接続状態を確認してください。

## ND ON / 値設定のエラー

GUI には HTTP 400 本文の SDK エラーを表示します。`ND OFF confirmed` は失敗後の OFF 読み戻し成功、`ND OFF rollback FAILED` は OFF を確認できなかった意味です。後者は実機で確認してください。Manual/Variable 設定は残る場合があります。Gain (ISO) や Iris が自動・取得不可ならカメラ側を手動にして再試行します。

20261001c 以前は非対応の Step モードを固定指定していたため、FX6 が Preset の場合に ND が操作できませんでした。20261002a 以降は有効な Variable モードに切り替えてから操作します。ND OFF 中の u/d ではこの切り替えも設定変更も行いません。

濃度の段飛び・読み戻し不一致も 20261002a で制御プロパティを見直しています。FX6 は NDFilterValue の透過率を使い、FX6 対応外の NDFilterOpticalDensityValue には書き込みません。

## シャッタースピード

S で選び、U で遅く、D で速くします。S だけではカメラ設定は変わりません。U/D 時の Speed 切り替えや速度設定に対応していない状態ではエラーが出ます。カメラ本体の手動シャッター Speed 設定と操作結果欄を確認してください。Angle / ECS の数値をシャッタースピードとして変更する操作はありません。
