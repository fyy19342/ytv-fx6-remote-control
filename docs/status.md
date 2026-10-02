# Status — 20261002a / 0.4.3

キーボード操作、Gain (ISO) 表記、ND ON の最小濃度化、ND OFF 中の step 拒否、ND ON 失敗時の OFF 復帰要求、backend build ID 照合、plugin を含まない release を実装しています。

0.4.3 は IPv4 での直接接続と指紋照合に対応します。実機検証で判明した ND の非対応 Step 指定を修正し、カメラの有効値から Variable モードを選択します。フォームの文字切れ修正は 0.4.2 から引き継ぎ、800×600 以上と縦スクロールに対応します。

実行結果の正本は [SELF_CHECK_20261002a.md](SELF_CHECK_20261002a.md) と配布 zip の verification/results.json です。build 成功だけを runtime 確認とは扱いません。GUI QTest は実 QShortcut を使いますが、カメラ API は test double です。

実機の認証、SDK からの読み戻し、映像の光学的な変化はそれぞれ独立して記録します。実機が見つからないときは列挙 API の応答が正常でも検出は WARN です。未実施の項目は NOT RUN とし、Qt の API test double の結果で実機の PASS を代用しません。

GitHub 向け CI / Release の手順は [DISTRIBUTION](DISTRIBUTION.md)。SDK は公開ソースから除外します。
