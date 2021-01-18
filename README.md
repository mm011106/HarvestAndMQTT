# HarvestAndMQTT

ネットワークレディでないデバイスとUARTで通信することで、ネットワーク機能を付加しようとする試み。

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
ハードウエアとしてSeeed社のWioLTEという、LTE/3GモデムとSTM32マイコン(ARM)が一体となっているマイコンボードを利用します。\
モデムはQuectelのEC21-Jが使われています。

### このデバイスがやること

A step by step series of examples that tell you how to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## 開発環境
ソフトウエア開発については
* [Arduino](https://www.arduino.cc/) - STmicroによるSTM32のためのエクステンションを利用

IDEとして
* [Microsoft VisualStudio code](https://azure.microsoft.com/ja-jp/products/visual-studio-code/) - Arduino拡張機能をインストール

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **MasakazuMiyamoto** - *Initial work* - 

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc
