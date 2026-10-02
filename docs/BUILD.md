# Build — 20261002a

`build_info.json` が build ID / version / port の単一ソースです。アプリ配布は macOS / arm64、開発には Xcode Command Line Tools、CMake、Python 3.11 が必要です。Python 依存は requirements.txt で PySide6 6.11.0 / PyInstaller 6.19.0 に固定しています。

## SDK を使わない検証

README の手順または GitHub Actions `CI` を使用します。`-DFX6_BUILD_SDK_BACKEND=OFF` は SDK を使わない ND 状態遷移・失敗時 OFF 復帰・露出方向・IP アドレス変換の4テストを構築します。CI は Debug build を使います。テスト target では Release build でも assert を明示的に有効にします。Qt の実 QShortcut/QTest は offscreen で実行し、カメラ API は test double を使用します。CI は同じ版の PySide6-Essentials のみを導入し、テストで使わない Addons は取得しません。

## アプリを構築

Sony SDK を Sony のサイトから取得し、利用規約に従ってローカルに取り込みます。SDK headers/runtime はこのリポジトリには含みません。

```bash
bash scripts/import_sdk.sh /path/to/RemoteCli-or-SimpleCli-or-CrSDK-root
bash scripts/build_app_macos.sh
bash scripts/self_check.sh --skip-build
bash scripts/package_distribution.sh --reuse-verified-build
```

build_app は backend CMake Release と CTest、PyInstaller、runtime とライセンス文書の同梱、stamp・依存・署名検証を実行します。Qt Core/Gui/Widgets/Network/DBus と必要な platform/style plugins に絞って bundle を構成します。

self_check は Python/shell 文法、Python/Qt tests、CTest、runtime 依存解決、署名、単体 backend・同梱 backend・実 Cocoa app の起動、health / cameras、build ID / PID / path を検証します。既存 port を占有するプロセスを勝手に終了しません。camera 接続は実施しません。

Qt のレイアウト試験は 800×600・1040×800・1280×900 で入力欄の幅、ボタンと選択欄の文字領域、同意欄へのスクロール、カードの枠と長いパスによる横はみ出しを検証します。macOS では Cocoa でも実行し、`dist/checks/BUILD_ID/gui-layout/` に画像を保存します。配布 app の目視結果は build と artifact digest を照合して自己確認へ取り込みます。

## 成果物

`package_distribution.sh` は通常 build → self_check → LGPL source archive → SDK-free source/app staging → zip 内容検証 → SHA256 を実行します。`--reuse-verified-build` は検証済み input/artifact の digest が一致する場合だけ許可します。

- `dist/FX6OperationApp-20261002a-macos-arm64.zip` と `.zip.sha256`
- `dist/FX6OperationApp-20261002a-open-source-dependencies.tar` と `.tar.sha256`
- `docs/SELF_CHECK_20261002a.md` / `docs/DISTRIBUTION_MANIFEST_20261002a.md`
- 詳細ログはローカル `dist/checks/20261002a/`。公開 zip はパスを置換した JSON 要約のみ。

従来の `package_release.sh` は SDK を含むローカル保管用の丸ごと置換版を生成します。**この成果物を GitHub や不特定の第三者へアップロードしないでください。** GitHub 配布は `package_distribution.sh` の出力を使います。npm / Stream Deck は使用しません。
