# FX6 Operation App

[![CI](https://github.com/fyy19342/ytv-fx6-remote-control/actions/workflows/ci.yml/badge.svg)](https://github.com/fyy19342/ytv-fx6-remote-control/actions/workflows/ci.yml)

Sony FX6 を Sony Camera Remote SDK 経由で操作する macOS アプリです。**0.4.4 / build 20261002b**。アプリにフォーカスがある間、ログイン後の操作画面でキーボード操作が有効になります。Gain は SDK の ISO sensitivity として扱い、GUI では **Gain (ISO)** と表示します。

0.4.4 は B による ND の ON/OFF 切り替えと、S → U/D によるシャッタースピード操作に対応します。OFF から ON にするときは最も明るい ND 値に設定します。M は OFF 専用として残します。

## ダウンロード

[**GitHub Release — 0.4.4 / 20261002b**](https://github.com/fyy19342/ytv-fx6-remote-control/releases/tag/v0.4.4-20261002b) の `FX6OperationApp-20261002b-macos-arm64.zip` と `.zip.sha256` をダウンロードしてください。GitHub が自動生成する「Source code」にはアプリは入っていません。

[利用条件](docs/TERMS.md) に同意した場合にダウンロード・使用してください。Sony 公式アプリではありません。本アプリで使用・制御した機器のメーカー保証について Sony SDK 規約上の条件があります。

Apple Silicon 向けの評価版です。Apple 公証は未実施です。実機の検出・認証・設定値の読み戻し・映像の変化は別々に判定し、確認済みの内容と制約を [SELF_CHECK](docs/SELF_CHECK_20261002b.md) に記載しています。インストール方法・対応 OS は [INSTALL](docs/INSTALL.md) を参照してください。

このビルドで実施した試験は SELF_CHECK を参照してください。SDK 読み戻しと映像の光学的変化は分けて記録します。

## キー操作

| キー | 動作 |
| --- | --- |
| i | Iris モード |
| g | Gain (ISO) モード |
| n | ND モード |
| s | Shutter Speed モード |
| u | 選択モードを1段明るくする |
| d | 選択モードを1段暗くする |
| b | ND ON/OFF 切り替え。ON にするときは有効な最小濃度 |
| m | ND OFF |

Iris の u は F 値を小さく、Gain (ISO) の u は ISO を大きく、ND の u は分母を小さく、Shutter Speed の u はシャッターを遅くします。d は逆方向です。最初はモード未選択です。b/m はモードを変更しません。ND OFF 中の u/d は OFF を維持します。長押しの自動反復と修飾キー付きの操作は無効です。Stream Deck+ は使用しません。

S は操作対象を選ぶだけです。U/D で調整すると手動 Speed に切り替えます。候補はカメラが返す速度を使い、端で止まります。

ND ON は各設定を読み戻して確認し、途中で失敗すると OFF を要求します。OFF の確認にも失敗した場合は `rollback FAILED` と状態未確認を表示します。切替途中の一時的な濃度までは保証できません。[詳細仕様](docs/KEYBOARD_CONTROLS.md)

## 配布物とソース

配布 zip には `.app`、SDK を除いた source / docs / scripts、build stamp、自己確認結果、ファイル SHA256 が入ります。SDK runtime は `.app` 内だけに組み込みます。SDK ヘッダー、サンプル由来コード、単体 runtime、旧 Stream Deck plugin は GitHub に置きません。

LGPL コンポーネントのソースは同じ Release の `open-source-dependencies.tar` から取得できます。[第三者ライセンス](THIRD_PARTY_NOTICES.md) / [プロジェクトのライセンス状況](LICENSE.md)

## 開発

CMake、C++17、Python 3.11 を使用します。SDK を取得せずに実行できるテスト:

```bash
python3 -m venv frontend-pyside/.venv
frontend-pyside/.venv/bin/python -m pip install -r frontend-pyside/requirements.txt
QT_QPA_PLATFORM=offscreen PYTHONPATH=frontend-pyside/src frontend-pyside/.venv/bin/python -m unittest discover -s frontend-pyside/tests -v
cmake -S backend -B build-tests -DFX6_BUILD_SDK_BACKEND=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tests
ctest --test-dir build-tests --output-on-failure
```

SDK を使う macOS ビルドは、Sony から取得した SDK を `scripts/import_sdk.sh` でローカルに取り込んでから実行します。SDK のディレクトリは Git 管理対象外です。

```bash
bash scripts/build_app_macos.sh
bash scripts/self_check.sh --skip-build
bash scripts/package_distribution.sh --reuse-verified-build
```

[ビルド](docs/BUILD.md) / [配布・Release 手順](docs/DISTRIBUTION.md) / [運用](docs/runbook.md) / [トラブル対応](docs/TROUBLESHOOTING.md)

バックエンドは `127.0.0.1:39061` で待ち受けます。起動前後に build ID、PID、executable path を照合し、旧版や不明なプロセスを再利用しません。既存プロセスを強制終了する処理はありません。
