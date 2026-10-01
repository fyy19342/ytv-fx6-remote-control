# SDK notes — 20261001c

照合元は incoming の Sony Camera Remote SDK 2.01.00 HTML reference と SimpleCli / RemoteCli。展開 release には incoming を含みません。headers/runtime は開発者が backend/vendor/sony にローカルで取り込みます。GitHub と配布 source には含めません。

- FNumber: UInt16Array、実 F 値の100倍。0xfffd=CLOSE、0xfffe/0xffff は表示なし。
- IsoSensitivity: UInt32Array。下位24bit が ISO 数値、上位は mode/extension。0xffffff=AUTO。GUI は Gain (ISO) と明記し、数値部分でソート、設定時は元の raw を保持。
- NDFilterOpticalDensityValue: UInt16Array、raw/100 が光学濃度。0xffff は値なし。分母目安は 10^(raw/100)。正で有効な値の最小を ND ON に選ぶ。
- NDFilter OFF/ON、NDFilterModeSetting Manual、NDFilterSwitchingSetting Step を使用。

ND ON は設定通知を待って読み戻す複数の処理です。失敗後は OFF を要求して確認します。ND OFF 中の step は property 書込み前に拒否します。密度の取得リストは Sony サンプルと同じ GetValues/GetValueSize から読み出します。

macOS runtime は3個の sibling dylib + Contents/Frameworks/CrAdapter の4個の dylib。SDK 起動とカメラの認証・光学変化は別々に検証します。
