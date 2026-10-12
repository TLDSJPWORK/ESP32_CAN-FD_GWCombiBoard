# ESP32 CAN-FD GateWay CombiBoard｜公開用リポジトリ

ESP32開発ボードとCAN FD関連モジュールを接続するための実験用基板、ESP32_CAN-FD_GWCombiBoardの資料です。

> 公開準備中の草案です。対応部品・電源条件・動作確認結果は確定前です。

## 特徴

- ESP32開発ボード用の接続部。
- Ch1／Ch2の接続部。
- DB9コネクタ実装部とMonitor端子。
- KiCad 9.0の設計データと、基板STEP・上下筐体STL。

2系統を使う実験やゲートウェイ用途を想定しています。転送機能は使用するモジュールとプログラムに依存します。動作保証範囲は【要確認：実機検証結果】です。

## 内容物・別途必要な部品

販売内容物は基板1枚のみです。部品は別途購入し、購入者がはんだ付けして実装します。
ESP32、CAN FDモジュール、コネクタ、電源部品、表示器、筐体の型番と同梱有無は[組立ガイド](docs/assembly.md)に整理します。

## 資料

- [仕様・接続表](docs/hardware.md)
- [組立・動作確認ガイド](docs/assembly.md)
- [KiCad設計データ](hardware/kicad/)
- [保存済み製造データ](hardware/production/)
- [筐体・3Dデータ](mechanical/)
- [サンプルの準備状況](example/README.md)
- [検証記録](docs/validation.md)
- [公開・ライセンス整備状況](docs/publication.md)

[回路図PDF](docs/schematic.pdf)を用意しました。基板写真と接続説明図は準備中です。旧CombiBoard用資料を本基板の配線説明として使用しないでください。

## 販売・問い合わせ

スイッチサイエンスでの委託販売を予定しています。
販売ページ：【要確認：発売後のURL】
技術問い合わせ先：tldsjpwork@gmail.com

## 関連プロジェクト

## ライセンス

本プロジェクトのオリジナル設計・製造データ・筐体データ・資料は[CC BY 4.0](LICENSE)で公開します。出典表示：JP WORKS / TLDSJPWORK、ESP32 CAN-FD GateWay CombiBoard。変更した場合は変更内容を明示してください。第三者の素材・ライブラリはそれぞれの利用条件に従います。

[従来のESP32_CAN-FD_CombiBoard](https://github.com/TLDSJPWORK/ESP32_CAN-FD_CombiBoard)は別設計です。対応部品やサンプルの互換性は確認中です。
