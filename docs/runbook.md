# 運用手順 — 20261001b

1. 旧 app と backend を終了し、`lsof -nP -iTCP:39061 -sTCP:LISTEN` で確認します。
2. zip を新しいフォルダに展開し `FX6OperationApp.app` を起動します。
3. タイトル・Runtime build が `20261001b` で、backend executable が今回の bundle を指すことを確認します。
4. 利用条件の同意欄を確認後、カメラ一覧を更新し、FX6 を選んで認証情報を入力・接続します。0 台の場合は未接続です。
5. 操作画面をクリックし、i / g / n でモードを選び、u / d で1段操作します。b は ND ON + 最小濃度、m は ND OFF。
6. 終了ボタン／ウィンドウを閉じて終了します。自動起動した backend は停止し、再利用した backend は残ります。

開発ツリーでは `bash scripts/run_backend_local.sh`、`bash scripts/run_app_from_bundle.sh` を使用できます。配布版は app 内の backend を使用します。単体 SDK runtime は配布しません。

```bash
curl http://127.0.0.1:39061/api/health
curl http://127.0.0.1:39061/api/cameras
curl http://127.0.0.1:39061/api/state
```

health の buildId / pid / executablePath、lsof の PID を照合してください。health の ok=true は実機接続成功を意味しません。

実機確認チェック: i/u で F が小さくなる、g/u で Gain (ISO) が大きくなる、b で最小濃度、n/d で分母増加、n/u で分母減少、m 後の u/d で OFF 維持、b 再押下で最小濃度、上下限、別アプリへフォーカスを移した間は無反応、切断・終了。実際に確認した結果だけを PASS と記録します。
