# SDK notes — 20261002f

照合元はローカルの Sony Camera Remote SDK 2.01.00 HTML reference、機種別 Function List、および実機が返す property の有効値・書き込み可否です。incoming と SDK headers/runtime は GitHub のソースへ含めません。

- FNumber: UInt16Array、実 F 値の100倍。0xfffd=CLOSE、0xfffe/0xffff は表示なし。
- IsoSensitivity: UInt32Array。下位24bit が ISO 数値、上位は mode/extension。0xffffff=AUTO。GUI は Gain (ISO) と明記し、数値部分でソート、設定時は元の raw を保持。
- FX6 の ND は **NDFilterValue (UInt64Array)** を使用。上位32bit が分子、下位32bit が分母で、透過率を表す。分数の大小で明るい順に並べ、同じ比率の重複と Clear／不明値を除外する。SDK が返した raw を変換せずに設定する。
- NDFilterOpticalDensityValue は機種別表で FX6 の対応対象ではない。取得できる場合があっても制御には使わない。旧実装では光学濃度と実機の段数が合わず、通知後に別の値へ変わるケースがあった。API の旧フィールドは診断用として残すが、GUI は ndValue の透過率を表示する。
- NDFilter OFF/ON、NDFilterModeSetting Manual を使用。NDFilterSwitchingSetting は有効値から Variable を優先し、対応する場合だけ Step を使う。FX6 の実機が返した候補は Preset / Variable。Preset では濃度と ON/OFF が書き込み不可になるため、切り替えてから候補と可否を読み直す。「1段操作」は候補値の隣へ移動する意味であり、SDK の Step モードを必須としない。

ND ON は設定通知を待って読み戻す複数の処理です。失敗後は OFF を要求して確認します。ND OFF 中の u/d は property 書き込み前に拒否します。ND が自動制御中なら濃度調整前に Manual へ切り替えます。Variable / Manual の設定は操作後も維持します。

IP 指定は CreateCameraObjectInfoEthernetConnection（FX6 / SSH ON）と GetFingerprint を使用します。IPv4 の第1オクテットを下位8bitに格納します。6byte 識別子は reference の許容どおりオブジェクト固有の値を使い、実 MAC としては表示しません。指紋取得では認証や設定変更を行わず、接続直前の指紋と一致することを確認します。

macOS runtime は3個の sibling dylib + Contents/Frameworks/CrAdapter の4個の dylib。SDK 起動、実機認証、SDK 読み戻し、映像の光学変化は別々に検証します。

ShutterSpeedValue は FX6 対応の UInt64Array で、上位32bit が秒数の分子、下位32bit が分母です。ShutterSpeed（UInt32）とは区別します。ShutterModeStatus の Speed を有効値・書き込み可否と照合して設定し、その後に速度の候補を読み直します。分数を交差乗算で比較して隣へ移動し、同じ速度の別表現は重複として扱います。範囲端は現在値を維持します。

ND トグルは SDK mutex 内で現在の NDFilter を読み、ON/OFF 操作を続けます。GUI の1秒周期キャッシュから ON/OFF の行き先を決めません。読み取り不可・未知値では書き込みを行いません。

FX6 の AWB は CrDeviceProperty_AWB (UInt16Array) の Down → Up を使用します。一般 ILC の WhiteBalance=AWB や AWBLButton は FX6 の機種別対応表では対象外です。WhiteBalanceModeSetting の Manual を確認して、1回の測定後に ATW へ戻さないようにします。WHT BAL のメモリー A/B 選択は本体側で行います。FX6 非対応の WhiteBalanceSwitch に書き込みません。

Down/Up は値の固定設定ではなくボタンの押下・解放として扱い、100ms の間隔を置きます。Down 前の descriptor を保持して Up を必ず試み、途中のプロパティ取得不可で解放を失わないようにしています。Down のエラー後も Up を送り、成功通知がなくても色温度変化だけでは完了にしません。

OnWarningExt の CrWarningExt_OperationResults / OperationInvalid を api=SetDeviceProperty、code=AWB で照合します。OK のほか暗すぎる、明るすぎる、色温度範囲外、白領域不足等を表示します。無通知15秒・切断は結果未確認です。Colortemp は読み取り表示のみで、この機能から Kelvin 数値や Tint を直接変更しません。

SDK 2.01.00 の機種別表では GetCRSDKOperationResultsSupported は FX6 非対応です。この API で通知能力を推定しません。今回の個体では AWB Down/Up の送信成功後も結果通知が返らない状態を観測したため、通知未確認を明示する表示が必要です。

連打防止は steady_clock による3秒の期限で、結果待ち15秒から独立しています。state の awb.retryAfterMs を GUI の monotonic clock に反映し、GUI のポーリングを待たず3秒後に再入力できます。backend でも期限を確認します。SDK の AWB 通知には要求 ID がないため、未確定の測定に重ねて再実行した後は結果を未確認として扱い、接続をやり直すまで遅れた通知を新しい測定の成功へ誤適用しません。
