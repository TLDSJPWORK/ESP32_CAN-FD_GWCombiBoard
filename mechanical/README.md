# 基板・筐体の3Dデータ

| データ | 用途 |
|---|---|
| [基板STEP](ESP32_CAN-FD_GWCombiBoard.step) | 基板形状の参照・筐体設計 |
| [上筐体STL](ESP32_CAN-FD_GWCombiBoard-上筐体.stl) | 上筐体の3Dプリント |
| [下筐体STL](ESP32_CAN-FD_GWCombiBoard-下筐体.stl) | 下筐体の3Dプリント |

筐体は販売品に含みません。

## MakerWorldでの配布

[ESP32 CAN-FD GateWay CombiBoard用 上下筐体](https://makerworld.com/ja/models/3444573-upper-and-lower-enclosure-for-esp32-can-fd-gateway)をCC BY 4.0で公開しています。上下筐体のSTLと、印刷方向・設定・組立方法を掲載しています。

- 下筐体：`enclosure-bottom.stl`を1個。
- 上筐体：`enclosure-top-print-oriented.stl`を1個。天面を下にした印刷方向に回転済みです。

MakerWorld配布版の下筐体は重複面を除去しています。このリポジトリの元STLはそのまま残しています。写真のUSB用開口は追加加工によるもので、配布STLには含まれません。

## 組立例

![筐体に収めた状態](../docs/images/enclosure-photo.jpg)

ESP32とCANモジュール2枚を載せた基板を上下筐体に収めた例です。DB9を外へ出し、上面の窓に表示器を配置しています。USB給電のために筐体の一部を切断し、ESP32のUSB端子へケーブルを挿せるようにしています。写真のUSB側の開口は追加加工によるものです。

![下筐体と実装基板](../docs/images/assembly-photo.jpg)
