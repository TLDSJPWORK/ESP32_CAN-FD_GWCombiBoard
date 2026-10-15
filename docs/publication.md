# 収録データとライセンス

公開先：[TLDSJPWORK/ESP32_CAN-FD_GWCombiBoard](https://github.com/TLDSJPWORK/ESP32_CAN-FD_GWCombiBoard)

## 収録内容

KiCad設計データ、製造データ、回路図PDF、基板STEP、上下筐体STL、接続表、組立ガイド、実物写真を収録しています。オリジナル設計・資料は[CC BY 4.0](../LICENSE)です。第三者の素材・ライブラリには各配布元の条件が適用されます。

DB9の3Dモデルは同梱していません。PCBの参照先は`${KIPRJMOD}/optional-3d/`です。モデルがなくてもPCB編集・製造データの参照は可能ですが、KiCadの3D表示ではDB9モデルが省略されます。専用サンプルプログラムは[未収録](../example/README.md)です。

## 画像・回路図

| 素材 | ファイル・内容 |
|---|---|
| 販売状態・表面 | [board-front-photo.jpg](images/board-front-photo.jpg)：DB9のみ実装 |
| 販売状態・裏面 | [board-back-photo.jpg](images/board-back-photo.jpg) |
| 組立例 | [assembly-photo.jpg](images/assembly-photo.jpg)：ESP32とCANモジュール2枚を追加 |
| 筐体使用例 | [enclosure-photo.jpg](images/enclosure-photo.jpg)：USB側は追加切断加工 |
| 接続説明図 | [board-connections.png](images/board-connections.png)：設計の3D画像への注釈 |
| 回路図 | [schematic.pdf](schematic.pdf) |

実物写真は2026年10月14日撮影。組立例のESP32、CANモジュール、表示器、ケーブル、筐体は販売内容物に含みません。3D画像は実物写真とは区別しています。
