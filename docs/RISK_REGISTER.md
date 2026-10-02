# 検証範囲と残る制約 — 20261002f

| 項目 | 対応と残る制約 |
| --- | --- |
| ND ON の部分成功 | 途中失敗時は OFF 要求と読み戻し。rollback 失敗は明示。通信断時に実機の状態を保証できない |
| ND OFF 中の u/d | GUI と backend が拒否。controller の無書込みを CTest で確認 |
| AWB | 本体のメモリー A/B・白い対象・適切な露出が必要。Manual を維持する。解放は途中失敗時にも試行し、通信断や結果通知なしは未確認と表示 |
| シャッター | 調整時に手動 Speed に切り替え、設定を維持。モード変更後の失敗で元モードへ自動復帰はしない。読戻し失敗は明示 |
| Gain の解釈 | Gain (ISO) = SDK ISO sensitivity。AUTO を除外し、数値順で操作 |
| キーの誤作動 | 操作ページ・active window・接続・modal 状態を確認。長押し反復は無効 |
| 旧 backend | 起動前後の build ID とプロセスを照合。自己確認は port/PID/path を確認 |
| runtime 欠落・開発パス依存 | runtime 8ファイルの load command 解決と単体・同梱 backend 起動を確認 |
| バージョン混在 | root/source/app/runtime stamp と Info.plist と runtime health を照合 |
| FX6 実機 | 検証した個体・状態の範囲を SELF_CHECK に記載。他のファームウェアやレンズを含む全組合せの互換性は未検証 |
| macOS 配布 | arm64 / ad-hoc 署名。Apple 公証・Intel 動作・他の macOS バージョンは未検証 |

ND の手動・Variable 設定は失敗時に残る場合があります。ND ON を複数の SDK 呼出しで処理するため、切替途中の一時値を保証する機構はありません。
