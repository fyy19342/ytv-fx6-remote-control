# Status — 20261001c / 0.4.2

キーボード操作、Gain (ISO) 表記、ND ON の最小濃度化、ND OFF 中の step 拒否、ND ON 失敗時の OFF 復帰要求、backend build ID 照合、plugin を含まない release を実装しています。

0.4.2 では macOS のログインフォームで入力欄が縮む不具合、日本語ボタンの文字切れ、ラベルの不要な枠を修正しました。800×600 以上に対応し、小さい画面では縦スクロールで全項目にアクセスできます。長い backend / log パスの全文は各表示のツールチップで確認できます。

実行結果の正本は [SELF_CHECK_20261001c.md](SELF_CHECK_20261001c.md) と配布 zip の verification/results.json です。build 成功だけを runtime 確認とは扱いません。GUI QTest は実 QShortcut を使いますが、カメラ API は test double です。

FX6 実機の認証、実レンズ・ISO・ND の変化、物理キーボードでのカメラ操作は NOT RUN。実機が見つからないときは列挙 API の応答が正常でも検出は WARN と記録します。

GitHub 向け CI / Release の手順は [DISTRIBUTION](DISTRIBUTION.md)。SDK は公開ソースから除外します。
