# 対向PiのIRQ設定例

この設定は2026-10-18のGateway試験に使った **Pi 5とテスト用4ch HATの実配線**専用です。Gatewayファームウェアの一部ではありません。

| SocketCAN | SPIデバイス | INTの実配線 |
|---|---|---|
| can0 | spi0.3 | GPIO22 |
| can1 | spi0.2 | GPIO27 |

初期のテスト用 `canfd4-reference-test` overlayは両方ともGPIO27へIRQを割り当てていました。can0の受信FIFOにRX割込みが残ってもGPIO27はHighのままで、can1の次の通信が共有IRQを起動するまでcan0のフレームが取得されませんでした。GPIO22を入力として観測するとRX保留中はLow、can0停止時はHighへ戻ることを確認しています。

[gw-can0-irq22-overlay.dts](gw-can0-irq22-overlay.dts)は、このテスト構成のspi0.3のIRQだけをGPIO22へ訂正します。CS、クロック、SPI速度、他コントローラは変更しません。GatewayのCH2 INTはGPIO27のままです。Pi側とGateway側のGPIO番号を混同しないでください。

同じテスト構成に適用する場合は既存の起動設定をバックアップしたうえで、次のようにコンパイルします。

```sh
dtc -@ -I dts -O dtb -o gw-can0-irq22.dtbo gw-can0-irq22-overlay.dts
sudo install -m 644 gw-can0-irq22.dtbo /boot/firmware/overlays/
```

`/boot/firmware/config.txt` の `[all]` セクションで、既存の `dtoverlay=canfd4-reference-test,...` の**後**に次の1行を置きます。

```text
dtoverlay=gw-can0-irq22
```

起動設定は再起動後に適用されます。`ip -details link`でSPIデバイスとインターフェースの対応を確認し、CANを有効にした後に `/proc/interrupts` でspi0.3がGPIO22、spi0.2がGPIO27へ登録されたことを確認してください。通常の別配線HATへはこの設定を適用しません。

試験機では、CANを停止してruntime overlayで対象SPIデバイスを再登録し、起動設定と同じIRQ割当で通信を確認しました。カーネルやCANドライバの置き換えは不要でした。起動設定もバックアップを取って保存していますが、今回の試験でPi全体の再起動は行っていません。
