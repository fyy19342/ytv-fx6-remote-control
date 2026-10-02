# 現行仕様 — 20261002b / 0.4.4

操作は [KEYBOARD_CONTROLS](KEYBOARD_CONTROLS.md) を正本とします。GUI はログイン、検出、接続・切断、Iris / Gain (ISO) / Shutter Speed / ND / Camera / Backend のカード、選択モード、操作結果、終了ボタンを提供します。

QShortcut は操作ページ配下の WidgetWithChildrenShortcut とし、接続状態・active window・modal window を確認します。ログイン直後はモード未選択です。状態を1秒ごとに取得し、通信失敗時は次の状態確認までカメラ操作を抑止します。

| API | 意味 |
| --- | --- |
| GET /api/health | buildId / version / pid / executablePath / cwd / logPath |
| GET /api/cameras | SDK カメラ列挙（空リストは実機確認成功ではない） |
| POST /api/cameras/ip | ipAddress で FX6 の指紋を取得。認証・設定変更なし |
| GET /api/state | 接続、SDK 初期化、プロパティ状態 |
| POST /api/connect | cameraId, userId, password, fingerprint で接続。IP 指定時は直前に確認した指紋が必須 |
| POST /api/disconnect | 切断 |
| POST /api/iris/step | delta=+1 で小さい F、-1 で大きい F |
| POST /api/iso/step | delta=+1 で大きい ISO、-1 で小さい ISO |
| POST /api/shutter/step | delta=+1 で遅く、-1 で速く。手動 Speed にして候補の隣へ移動 |
| POST /api/nd/toggle | 最新 ON/OFF を読み、OFF → ON + 最小濃度、ON → OFF。状態不明は拒否 |
| POST /api/nd/step | delta=+1 で暗く、-1 で明るく。OFF は拒否 |
| POST /api/nd/on | ON + 最小濃度、失敗時は OFF 要求 |
| POST /api/nd/off | OFF |
| POST /api/quit | 切断して backend 終了 |

API の POST は form-urlencoded。成功は `{ok:true,data:...}`、失敗は HTTP 400 と `{ok:false,error:...}`。GUI は本文の具体的なエラーを表示します。既存の `/api/iso/base/{toggle,high,low}` は保守用に残しますが、GUI のキー割当には含めません。ND の前回値復帰と ECS の周波数操作はありません。shutterMode / shutterSpeed を状態に含みます。
