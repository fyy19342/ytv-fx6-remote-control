# Architecture — 20261002f

`FX6OperationApp.app` の PySide6 GUI がローカル HTTP backend `fx6d` に要求し、backend が Sony Camera Remote SDK を操作します。キーボード入力は GUI 内の QShortcut に限ります。グローバルホットキーや外部操作 plugin は使用しません。

起動時は既存 health の build ID を照合し、一致する backend のみ再利用します。存在しなければ環境 override / bundle / source runtime を探索して起動し、起動後も ID とプロセス生存を確認します。接続後は状態を毎秒更新し、選択モードと値を表示します。

カメラ選択は SDK 自動列挙と IPv4 指定に対応します。IP 指定は SSH の指紋を取得して表示した対象だけを接続候補にし、ログイン直前の指紋と一致することを確認します。認証情報は GUI から localhost の接続要求へ渡し、ログや設定ファイルには保存しません。

ND の値選択は `nd_state_machine`、設定順序と OFF への失敗処理は SDK 呼び出しを注入する `nd_controller` に分離しています。production は同じ controller に Sony property access を渡します。CTest では失敗が物理書込み後に発生する場合や rollback の失敗を注入して検証します。

SDK 設定は mutex 内で直列化し、変更通知待ちと読み戻しを行います。ND の複数 property 更新は原子的ではありません。GUI のテスト用 API 差し替えは tests に限定し、配布アプリは実 SDK backend に接続します。

runtime は `fx6d` の隣に3個の dylib、`Contents/Frameworks/CrAdapter/` に4個の dylibを配置します。開発パスへの依存を排除し、bundle 単体で解決します。

シャッターの速度候補選択と Speed モードへの切り替えは `shutter_controller` に分離しています。SDK 不要の CTest で分数の順序、同値、上下限、無効値、失敗と読み戻し不一致を検証します。

AWB は `awb_controller` に Manual 確認・Down/Up・失敗時の解放を分離し、`AwbProgress` で処理中・完了・失敗・結果未確認を管理します。コールバックは専用 mutex を使い SDK mutex へ入らず、GUI は毎秒の状態更新で結果を表示します。

AWB の再実行制限は3秒の monotonic deadline、結果確認は別の15秒 deadline を使います。GUI は retryAfterMs からローカル期限を持ち、1秒周期の状態更新がなくても3秒後に入力を再開できます。結果通知だけで待ち時間を解除しません。
