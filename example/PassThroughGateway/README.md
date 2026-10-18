# CH1 ↔ CH2 全通過Gateway

[PassThroughGateway.ino](PassThroughGateway.ino) は、CH1の受信フレームをCH2へ、CH2の受信フレームをCH1へ転送するArduinoスケッチです。IDフィルタは設定せず、標準／拡張ID、通常CAN（データ／RTR）、CAN FD（BRSあり／なし）を転送対象とします。ID、フレーム種別、長さ、データを保持して再送信します。CRC、ACK、ESI、エラーフレーム、タイムスタンプの透過転送は対象外です。

## ビルド確認

2026-10-16、macOS arm64でコンパイル・リンク成功を確認しました。2026-10-18に発振子・GPIO設定を訂正し、再ビルドも成功しています。`--warnings all` 指定で警告出力はありません。同日、書き込み・起動・通常CAN／CAN FDの双方向転送を下記条件で確認しました。

| 項目 | 確認値 |
|---|---|
| Arduino CLI | 1.5.2-rc.1（commit fef6e48df） |
| ESP32コア | esp32:esp32 3.3.12 |
| ライブラリ | ACAN2517FD 2.1.16 |
| FQBN | esp32:esp32:esp32（ESP32 Dev Module、既定オプション） |
| プログラム領域 | 286,408 / 1,310,720 bytes（21%） |
| 静的RAM | 23,092 / 327,680 bytes（7%） |

新しく構築する場合はArduino CLIをインストールし、次のコマンドを実行します。

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install 'ACAN2517FD@2.1.16'
# このREADMEのあるディレクトリで実行
arduino-cli compile --fqbn esp32:esp32:esp32 --warnings all .
```

## 書き込み・起動確認

2026-10-18、GPIOを下記の正しい割当に訂正し、ESP32-D0WD-V3への書き込みと書き込みデータのハッシュ照合に成功しました。CH2のINTは公開回路図どおりGPIO27です。

```text
CH1 begin=0x00000000, CH2 begin=0x00000000
Gateway ready: CH1 <-> CH2, all IDs, CAN / CAN FD.
```

リセット後12秒間のログで、両CHの初期化成功、周期ログの継続、両CHのCANエラーカウンタ0を確認しました。受信・送信キュー受付はいずれも0で、CANフレームの送受信・双方向転送を確認したものではありません。

## 通信確認（2026-10-18）

対向はRaspberry Piの4ch CAN HATの2ポート（SocketCAN `can0`／`can1`）。別々のCANバスで各ポートとGatewayの各CHを接続し、配線・両端の終端は利用者が確認しました。Gatewayのログから、`can0`送信はCH1受信→CH2送信、`can1`送信はCH2受信→CH1送信と対応しました。

| 試験 | 公称／データ速度 | 結果 |
|---|---|---|
| 通常CAN | 500 kbit/s | 片方向各100件、同時双方向各100件を照合して一致 |
| CAN FD・BRSなし | 500 kbit/s | 同上 |
| CAN FD・BRSあり | 500 kbit/s／2 Mbit/s | 同上 |

標準／拡張ID、通常CANの0～8バイトおよび拡張RTR、CAN FDの0・1・8・12・16・20・24・32・48・64バイトを含むデータを使用しました。ID・長さ・データ・RTR/BRSの一致を確認し、対象データの欠落・重複は0でした。送信間隔は5 ms（同時双方向では各ポート5 ms）。高負荷・長時間・故障復帰・遅延の上限は未評価です。

**確認条件と制限：**

- CAN FDの合格条件では、Piの公称・データ両サンプル点を80%に設定し、公称SJW=8、データSJW=2としました。初回のPi既定値（公称87.5%、データ70%）ではCANエラーと送信バッファ不足が発生し、その条件は不合格です。
- 初回試験で観測した末尾1件の滞留は、対向PiのCAN0（spi0.3）のINT実配線がGPIO22なのに、テスト用overlayでGPIO27へ割り当てていたことが原因でした。CAN1（spi0.2）はGPIO27です。Pi側IRQを実配線に合わせた追試では、確認用フレームなしで単発・有限列の末尾まで受信できました。GatewayのGPIO設定や転送処理の変更は不要でした。
- 合格試験中のGateway CANエラーカウンタは両CHとも0でした。試験後はPi側CANを停止し、Gatewayサンプルは書き込み済みのままです。

Pi側の合格時のビットタイミング設定例（IRQは別途、実配線に合わせます）：

```sh
sudo ip link set can0 down
sudo ip link set can1 down
sudo ip link set can0 up type can bitrate 500000 sample-point 0.8 sjw 8 dbitrate 2000000 dsample-point 0.8 dsjw 2 fd on
sudo ip link set can1 up type can bitrate 500000 sample-point 0.8 sjw 8 dbitrate 2000000 dsample-point 0.8 dsjw 2 fd on
```

### 末尾フレームの回帰試験

同日のIRQ修正後、[test_socketcan_gateway.py](test_socketcan_gateway.py)で確認用フレームを一切送らず、通常CAN・FD BRSなし・FD BRSありについて、単発、各方向100件の有限列、同時双方向を計18条件・1,212フレーム検証しました。各試験は最後の送信後に1秒間送信を止め、その末尾までID・長さ・種別・データ・順序を照合しました。欠落・重複・内容相違・順序違いは0でした。

単発は通常CAN 8バイト、FD 64バイトを使用しました。単発受信までのアプリ上の観測時間は約1.1～4.2 msで、遅延上限を保証する測定ではありません。別途、通常CANの単発8バイトを各方向に送り、後続通信なしで2秒間待つ試験も両方向成功しました。

上記のCAN FD設定を済ませたLinux対向機で実行します。Python標準ライブラリだけで動作し、専用の試験バスに生成データを送信します。

```sh
python3 test_socketcan_gateway.py can0 can1
```

使用したPi 5＋テストHAT専用のIRQ設定例は[test-support](test-support/README.md)にあります。別の基板へそのまま適用せず、各コントローラのCSとINTの実配線に合わせてください。

## 実機で使う前の設定

- Arduino IDEの場合は上記ESP32コアとライブラリをインストールし、ESP32 Dev Moduleを選択します。従来型ESP32向けであり、S3/C3向けではありません。
- `CH1_OSC` / `CH2_OSC` はCAN-FD_Board公開回路図のY1（20 MHz）に合わせて `OSC_20MHz` に設定しています。実装品が異なる場合はその発振子と一致させてください。
- 初期設定は両チャンネルとも公称500 kbit/s、データ倍率x4（BRS時2 Mbit/s）。`CH1_BIT_RATE` / `CH2_BIT_RATE` と `CH1_DATA_FACTOR` / `CH2_DATA_FACTOR` を各バスに合わせてください。
- GPIOは[仕様書](../../docs/hardware.md)に対応します。CH1はVSPI（SCK26、MISO18、MOSI19、CS25、INT32）、CH2はHSPI（SCK14、MISO12、MOSI13、CS15、INT27）です。モジュールは3.3V設定とし、GPIO12・15の起動への影響も確認してください。
- CH1とCH2は別々のバスに接続し、各バスの両端を終端します。各バスにACK可能な相手機器が必要です。同一バスへの接続や別経路で戻る構成はフレームの循環を起こします。重複除去はありません。

シリアル115200 bpsで両チャンネルの `begin=0x00000000` と `Gateway ready` を確認します。片方でも初期化エラーなら転送を停止します。

送信キュー満杯時はその方向のフレームを保持し、**新しい受信がなくても毎loopで再試行**します。最後の1件の送信に次のフレームは必要ありません。反対方向の処理も継続します。FIFOは有限であり、過負荷や送信先切断時の無損失転送は保証しません。毎秒表示する `rx` はスケッチが受け取った件数、`queued` は送信キュー受付件数、`pending` は保留の有無です。到達・欠落なしの確認には外部機器が必要です。

まず低負荷の通常CANで両方向を確認し、次に拡張ID、RTR、CAN FDの各長さ（最大64バイト）、BRS、同時双方向、負荷増加、相手切断時について欠落・重複・順序・遅延を測定してください。

使用ライブラリ：[ACAN2517FD公式ソース](https://github.com/pierremolinaro/acan2517FD)。ライブラリ本体は同梱していません。
