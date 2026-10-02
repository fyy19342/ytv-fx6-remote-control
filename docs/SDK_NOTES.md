# SDK notes — 20261002a

照合元はローカルの Sony Camera Remote SDK 2.01.00 HTML reference、機種別 Function List、および実機が返す property の有効値・書き込み可否です。incoming と SDK headers/runtime は GitHub のソースへ含めません。

- FNumber: UInt16Array、実 F 値の100倍。0xfffd=CLOSE、0xfffe/0xffff は表示なし。
- IsoSensitivity: UInt32Array。下位24bit が ISO 数値、上位は mode/extension。0xffffff=AUTO。GUI は Gain (ISO) と明記し、数値部分でソート、設定時は元の raw を保持。
- FX6 の ND は **NDFilterValue (UInt64Array)** を使用。上位32bit が分子、下位32bit が分母で、透過率を表す。分数の大小で明るい順に並べ、同じ比率の重複と Clear／不明値を除外する。SDK が返した raw を変換せずに設定する。
- NDFilterOpticalDensityValue は機種別表で FX6 の対応対象ではない。取得できる場合があっても制御には使わない。旧実装では光学濃度と実機の段数が合わず、通知後に別の値へ変わるケースがあった。API の旧フィールドは診断用として残すが、GUI は ndValue の透過率を表示する。
- NDFilter OFF/ON、NDFilterModeSetting Manual を使用。NDFilterSwitchingSetting は有効値から Variable を優先し、対応する場合だけ Step を使う。FX6 の実機が返した候補は Preset / Variable。Preset では濃度と ON/OFF が書き込み不可になるため、切り替えてから候補と可否を読み直す。「1段操作」は候補値の隣へ移動する意味であり、SDK の Step モードを必須としない。

ND ON は設定通知を待って読み戻す複数の処理です。失敗後は OFF を要求して確認します。ND OFF 中の u/d は property 書き込み前に拒否します。ND が自動制御中なら濃度調整前に Manual へ切り替えます。Variable / Manual の設定は操作後も維持します。

IP 指定は CreateCameraObjectInfoEthernetConnection（FX6 / SSH ON）と GetFingerprint を使用します。IPv4 の第1オクテットを下位8bitに格納します。6byte 識別子は reference の許容どおりオブジェクト固有の値を使い、実 MAC としては表示しません。指紋取得では認証や設定変更を行わず、接続直前の指紋と一致することを確認します。

macOS runtime は3個の sibling dylib + Contents/Frameworks/CrAdapter の4個の dylib。SDK 起動、実機認証、SDK 読み戻し、映像の光学変化は別々に検証します。
