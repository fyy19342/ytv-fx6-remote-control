# GitHub 配布手順

配布先: [fyy19342/ytv-fx6-remote-control](https://github.com/fyy19342/ytv-fx6-remote-control)

Git 管理するのは SDK を含まないソース、現役 docs、scripts、依存ライセンス、CI 設定です。`backend/vendor/sony/`、`incoming/`、`dist/`、旧 plugin、認証情報は管理しません。初期履歴に SDK を含んだリポジトリを一般公開するときは、HEAD だけでなく過去履歴にも SDK が残らないことを確認してください。履歴の書き換えは管理者の承認とバックアップを伴う個別の作業です。

## 次のビルドを配布する

1. `build_info.json` の build ID / version を更新し、変更内容と Release notes を記載します。build ID の使い回しは避けます。
2. Sony SDK があるローカル Mac で `bash scripts/package_distribution.sh` を実行します。全 runtime 検証と source/artifact の digest が一致した成果物だけを配布します。
3. `python3 scripts/audit_public_source.py` を実行してからソースと検証文書を commit / push します。
4. GitHub Actions の CI が対象 commit で成功したことを確認します。CI は SDK を保持せず、純粋 C++ テストと GUI の offscreen 試験を実行します。
5. 完全な commit SHA を指定し、以下で評価版 Release を公開します。`--publish` を省くと draft です。

```bash
python3 scripts/publish_github_release.py --target FULL_40_CHARACTER_COMMIT_SHA --publish
```

このスクリプトは CI 成功、zip の stamp / SDK 配布範囲 / ファイル hash、zip と OSS source asset の SHA256 を確認し、`vVERSION-BUILDID` のタグと prerelease を作成します。既存 Release を上書きしません。認証は実行者の `gh auth login` を使用し、トークンをソースや成果物に埋め込みません。

Release から zip を再ダウンロードして `.sha256` を照合し、別フォルダへ展開して app 起動を確認します。Download 導線は README と INSTALL、問い合わせ先は Issues です。LGPL source archive も同じ Release に残してください。

## 署名と公証

現ビルドは ad-hoc 署名です。Developer ID 証明書・Apple 公証用資格情報がこの環境にないため、公証済みとは表示しません。一般配布用の公証を追加する場合は、Developer ID Application で全バイナリを適切な entitlements と hardened runtime を用いて署名し、`notarytool` で提出・成功確認・staple 後に runtime/self_check を再実行します。証明書や Apple ID のパスワードはリポジトリへ置きません。

実機試験をしていないリリースは prerelease とし、FX6 実機を PASS と記載しません。GitHub の可視性が Private の間、Release もアクセス権を持つユーザーだけが取得できます。
