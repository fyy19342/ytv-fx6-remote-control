# 運用手順 — 20261002e

1. 旧 app と backend を終了し、`lsof -nP -iTCP:39061 -sTCP:LISTEN` で確認します。
2. zip を新しいフォルダに展開し `FX6OperationApp.app` を起動します。
3. タイトル・Runtime build が `20261002e` で、backend executable が今回の bundle を指すことを確認します。
4. カメラ一覧を更新して FX6 を選びます。自動検出が0台なら FX6 IP → IP を確認で指定します。Fingerprint を本体と照合し、User / Password を入力して「接続」を押します。指紋取得だけでは認証は完了していません。
5. 操作画面をクリックし、i / g / n / s でモードを選び、u / d で1段操作します。b は ND ON/OFF 切り替え（ON 時は最小濃度）、m は ND OFF。s の u は遅く、d は速くします。a は WB メモリー A/B と白い被写体を準備してから押します。
6. 終了ボタン／ウィンドウを閉じて終了します。自動起動した backend は停止し、再利用した backend は残ります。

開発ツリーでは `bash scripts/run_backend_local.sh`、`bash scripts/run_app_from_bundle.sh` を使用できます。配布版は app 内の backend を使用します。単体 SDK runtime は配布しません。

```bash
curl http://127.0.0.1:39061/api/health
curl http://127.0.0.1:39061/api/cameras
curl http://127.0.0.1:39061/api/state
```

health の buildId / pid / executablePath、lsof の PID を照合してください。health の ok=true は実機接続成功を意味しません。

実機確認チェック: i/u で F が小さくなる、g/u で Gain (ISO) が大きくなる、b で最小濃度、n/d で分母増加、n/u で分母減少、m 後の u/d で OFF 維持、b 再押下で OFF、次の b で ON + 最小濃度、s/u で遅く、s/d で速く、上下限、別アプリへフォーカスを移した間は無反応、切断・終了。実際に確認した結果だけを PASS と記録します。

AWB 確認: 本体をメモリー A/B、同じ照明の白い被写体と適切な露出で準備し、A を1回押します。3秒未満の連打拒否と3秒後の再実行、結果通知、色温度、Manual の維持を確認します。SDK が成功を通知しても、映像の色再現や測定精度の独立検証とは区別します。
