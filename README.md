# HarvestAndMQTT

この装置にあるネットワーク接続のためのモデムを使って、\
ネットワークレディでないデバイスにネットワーク機能を付加しようとする試み。

本装置へのネットワークトラフィックを適宜解釈し、その内容をUART上でデバイスと通信することで擬似的なネットワーク接続を提供する。

## 背景

ネットワークに簡単に接続できる世の中なので、できれば単機能な計測回路をサクッとネットに繋いで遠隔から監視・操作したいというデマンドは潜在的に大きいと思います。
しかしながら、簡単な計測装置（たとえば「雨が降っているかどうか」を確認するような）であればあるほど、ネットに繋ぐ設計の方が負担が大きくなってしまいます。
そこで、比較的開発が容易なUARTによる通信をネットワーク（MQTT）でリレーしてネットワークと接続できるようにしよう、というのがこのデバイスの開発動機です。

### 用意するモノ

- 通信するためのデバイス
```
- Soracom air SIM
```
デバイスからネットワークに接続すためのメディアとして、携帯電話回線を使います。\
SORACOMのサービスを使うことを前提にしているので、SORACOMのSIMが必要となります。

```
- Seeed WioLTE
```
ハードウエアとしてSeeed社の[WioLTE](https://wiki.seeedstudio.com/Wio_LTE_Cat.1/)という、LTE/3GモデムとSTM32マイコン(ARM)が一体となっているマイコンボードを利用します。\
モデムはQuectelのEC21-Jが使われています。

### このデバイスがやること
- ネットワーク上（MQTT - subscribe）の通信を受け取って、適宜解釈を施し、UARTに送ります。
- UARTからの通信を必要に応じて加工し、ネットワーク上（MQTT - publish)に送り出します。

MQTTのサーバ（broker)はAWS-IoTを想定しています。\
さらに回線はSORACOMのサービスを用い、AWSとは認証と暗号化で保護された通信を確保します。

## これから実装するべき項目

- コマンド・結果をやりとりするためのプロトコル
- プロトコルを実現するためのデバイスのパラメタやフラグ（「シャドウ」に反映する必要も含めて）


## 開発環境
ソフトウエア開発については
* [Arduino](https://www.arduino.cc/) - STmicroによるSTM32のためのエクステンションを利用

IDEとして
* [Microsoft VisualStudio code](https://azure.microsoft.com/ja-jp/products/visual-studio-code/) - Arduino拡張機能をインストール

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. 
<!-- For the versions available, see the [tags on this repository](https://github.com/your/project/tags).  -->

## Authors

* **MasakazuMiyamoto** - *Initial work* - 

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc
