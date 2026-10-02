# Current pipeline — 20261002a

1. build_info.json から build ID / version / port を取得。
2. CMake で backend を構築し、ND と露出操作の CTest を実行。
3. PyInstaller でキーボード操作 GUI を .app にする。
4. backend runtime を .app に同梱し、stamp・依存解決・署名を検証。
5. self_check で Python/Qt と backend HTTP・実 Cocoa app の起動を検証。build ID / PID / executable path を記録。
6. 実行結果を PASS / WARN / FAIL / NOT RUN に分類。FAIL があれば package を停止。
7. SDK を除いた source・docs・scripts・app・検証結果の要約を配布 zip にまとめる。runtime は app 内だけに含める。
8. LGPL ソースを別の Release asset として用意。
9. archive の CRC、構成、plugin 非混入、build stamp、SHA256 を確認。

過去版の self check、docs/archive、incoming、plugin source、npm 依存物を取り込まない構成です。
