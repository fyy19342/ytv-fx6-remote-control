# Adaptor_Create 診断 — 20261001b

Sony reference の CrError_Adaptor_Create (0x8703) は adaptor creation failure を示します。原因をカメラ不在やネットワークだけに断定しないでください。runtime 欠落、配置、署名、依存解決をまず確認します。

現行版は Sony sample と同じ runtime 配置を採用し、app に backend と全7 dylib を同梱します。実行時の log と DYLD_PRINT_LIBRARIES は verification/runtime に収録します。今回の結果は SELF_CHECK_20261001b を参照してください。過去版の失敗原因の推測を現行の検証結果として扱いません。
