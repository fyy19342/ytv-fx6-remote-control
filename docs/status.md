# Status — 20261002f / 0.4.8

キーボード操作、Gain (ISO) 表記、ND ON の最小濃度化、ND OFF 中の step 拒否、ND ON 失敗時の OFF 復帰要求、backend build ID 照合、plugin を含まない release を実装しています。

0.4.8 は操作画面を800×480に整理し、4枚の露出カードを横1列に表示します。カメラ・backend 情報を省スペース化し、詳しいキー説明は開閉できます。White Balance の結果と操作フィードバックは常に表示します。ログイン画面は900×600で、同意チェックはありません。AWB の3秒間隔、B の ND トグル、S のシャッタースピード操作、IPv4 指定と指紋照合を維持します。カメラの結果通知と結果未確認を区別して表示します。

実行結果の正本は [SELF_CHECK_20261002f.md](SELF_CHECK_20261002f.md) と配布 zip の verification/results.json です。build 成功だけを runtime 確認とは扱いません。GUI QTest は実 QShortcut を使いますが、カメラ API は test double です。

実機の認証、SDK からの読み戻し、映像の光学的な変化はそれぞれ独立して記録します。実機が見つからないときは列挙 API の応答が正常でも検出は WARN です。未実施の項目は NOT RUN とし、Qt の API test double の結果で実機の PASS を代用しません。

GitHub 向け CI / Release の手順は [DISTRIBUTION](DISTRIBUTION.md)。SDK は公開ソースから除外します。
