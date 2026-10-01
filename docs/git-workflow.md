# Git workflow

remote は `https://github.com/fyy19342/ytv-fx6-remote-control.git`、既定ブランチは `main`。変更を分ける場合は `codex/` 接頭辞のブランチを使用します。

SDK を取り込んだ開発ツリーをそのまま `git add -f` しないでください。`.gitignore` で proprietary SDK・生成物・旧 plugin を除外し、push 前に `python3 scripts/audit_public_source.py` を実行します。CI でも tracked files を同じ基準で確認します。

ビルド済み app はソースに commit せず [配布手順](DISTRIBUTION.md) に従って GitHub Release に置きます。Sony SDK が必要なビルドはローカル Mac、SDK 不要のテストは GitHub-hosted runner で実行します。
