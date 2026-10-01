# 0.4.1 / 20261001b

- キーボード操作版を GitHub 配布用に整備。SDK headers・サンプル由来診断コード・単体 SDK runtime を公開ソースから除外。
- 診断文字列をプロジェクト独自の実装に置換。
- SDK 不要の C++ / Qt CI、公開ソース監査、Release 用 package / verify / publish スクリプトを追加。
- 利用条件の表示と接続前の同意欄、第三者ライセンスと LGPL source asset を追加。
- 配布に不要な Qt plugins / modules を除外。依存バージョンを固定。
- build 20261001a のキー仕様と ND 失敗時 OFF 復帰は維持。
- 同梱ライブラリの最低 OS を調査し、macOS 15 以降と明示。backend の deployment target も固定。
- C++ テストの assert が Release でも実行されるよう修正。
