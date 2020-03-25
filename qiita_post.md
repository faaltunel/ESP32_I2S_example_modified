# はじめに
　ESP-IDFで外部I2Sを使って音声信号を出力しようとしたところ、公式のサンプルプログラムがうまく動きませんでした。調べてみると、「サンプルがうまく動かない」という先例はいくつか見つかりましたが、上手く動くプログラムが見つからなかったので、本投稿をすることにしました。


　今回は、ESP32開発ボードとMAX98357A(I2S DAC)モジュールとスピーカ１つを使って、単純な正弦波を出力します。Githubに動作するプログラムと配線図を置いていますので、本稿の通り進めれば動作します。また、ESP-IDFでのI2S出力の基本的なお作法についても、私なりの理解を簡単に整理しました。間違いがあればご指摘頂けると助かります。



# 準備するもの
- PC(筆者はMac)
- ESP32開発ボード(NodeMCU, ESP-WROOM-32等)
- MAX98357A I2S DACモジュール(Adafruit製や類似品)
- スピーカユニット（使用するDACに適したもの。0.5～3W程度の小さいもの）



# 環境構築
## ESP-IDFのセットアップ
　[ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)の通りに進めます。
　本稿では2020年3月時点で最新のver4を使用します。



## Githubからプログラムを入手（git clonoe）
　[リポジトリ](https://github.com/moppii-hub/ESP32_I2S_example_modified)から以下のコマンドでプログラムを入手します。

```
$ cd ~/esp
$ git clone https://github.com/moppii-hub/ESP32_I2S_example_modified.git
```



## 回路の製作
　以下の通りに配線します。
![ESP32_I2S_example_modified_wiring.png](https://qiita-image-store.s3.ap-northeast-1.amazonaws.com/0/603551/20ed0e84-7233-4fa5-71ab-d0998ecd68af.png)



# とりあえず動作させる
　まず、ESP32ボードとPCをUSBケーブルで接続し、シリアルポートを確認します。


```
$ ls /dev/tty.*
```  


　私の環境(macOSX High Sierra)では、以下のように出力されます。  


```
/dev/tty.SLAB_USBtoUART
```


　各OS毎のポートの調べ方は、[公式](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html#get-started-connect)に説明がありますが、macは`/dev/cu.*`になると書かれていますが、私の環境では`/dev/tty.*`でした。


　次に、以下のコマンドでプログラムのコンパイル・書き込みをします。


```
$ cd ~/esp/ESP32_I2S_example_modified
$ idf.py build
$ idf.py -p /dev/tty.SLAB_USBtoUART flash
```


　これで、スピーカから正弦波が出るはずです。



# プログラムの説明
※プログラム本体は[main/i2s_example_modified_main.c](https://github.com/moppii-hub/ESP32_I2S_example_modified/blob/master/main/i2s_example_modified_main.c)です。

## 全体の流れ
1. 初期設定をする(`i2s_driver_install()`、`i2s_set_pin()`、`i2s_set_clk()`)
2. 再生する信号(unsigned int型)を生成しておく（`setup_waves()`）
3. `i2s_write()`で再生する信号をDACへ出力



## 各関数に関する要点
### i2s_driver_install()
　i2s_configを使って以下の設定をします。（理解必要な項目のみ抜粋）

| 変数 | 意味 |
|:---|:---|
| .sample_rate | 信号のサンプリング周波数（Fs） |
| .bits_per_sample | 1サンプル当たりの量子化ビット数（振幅分解能） |
| .channel_format | モノラルかステレオか |
| .dma_buf_count | DMA(Direct Memory Access)のバッファ数 |
| .dma_buf_len | DMAの1バッファの長さ |
| .use_apll | 高精度クロックを使うか否か |
| .intr_alloc_flags | 割り込み設定（よくわからない） |


　また、i2s_driver_install()関数の1つ目の引数`I2S_NUM`はESP32内部のI2S出力用ポートの番号で、外部のI2S-DACを使うなら変更の必要はありません。3つ目と4つ目の引数はI2Sイベントのキューへのポインタとキューのサイズだそうで、使わなければポインタはNULL指定しサイズも0にすれば良いようです。このキューを使う状況というのは、まだよく分かりません。



### i2s_set_pin()とi2s_set_clk()
　名前の通りなので説明割愛します。公式ドキュメントとプログラム内のコメント参照。



### setup_waves()
　この関数では、信号を生成しています。この信号は、16bitのunsigned int型、つまり振幅上下限が0～65,535までの信号になります。sin()関数で信号の値を計算し、その結果をグローバル変数samples_data[]に代入しています。
　信号の長さは、正弦波1周期分としています。これは、`i2s_write()`へ受け渡す信号があまり長いと時間が掛かるため良くないかと思ったのと、元の公式サンプルも1周期分としていたことから、このように決めました。変えてみて挙動を見ると、`i2s_write()`周りの仕様が分かってくるかもしれません。



### i2s_write()
　この関数は、ESP32からDACへデータ送信するための関数です。引数にI2S出力のポートと、信号波形へのポインタ、信号波形のサイズ、データ送信が何らかの理由で中断(タイムアウト)した場合の中断位置を保管する(返す)変数アドレス、タイムアウトまでの待ち時間、を入れます。


　公式ドキュメントの説明が少ないため推測になりますが、おそらくDMAへデータを書き込む関数かと思います。DMAへ書かれた値は、おそらく`i2s_driver_install()`と`i2s_set_pin()`と`i2s_set_clk()`で設定されたポートにCPU処理を経ずに出力される仕様になっているものと思われます。



## 公式サンプルとの違い
　[公式サンプル](https://github.com/espressif/esp-idf/blob/master/examples/peripherals/i2s/main/i2s_example_main.c)との違いは、以下のとおりです。


- サンプルの量子化bit数を再生中に切り替えない仕様にした
- 信号生成とi2s_writeを分割
- モノラル化（ついでに、三角波の計算をやめた）
- ピン配置（GPIOの番号）


　量子化bit数を切り替える仕様をやめたのは、簡潔化のためです。公式サンプルではbit数を切り替えるために信号長が可変になっており、各引数の意味がぱっと見で分からず、間違いが見つけにくい状態でした。まずi2s_writeの基本的な使い方を知りたかったので、16bitに固定し、各変数をなるべく定数(マクロ)化しました。


　信号生成とi2s_writeの分割は、問題切り分けのためです。音声を途切れること無く再生するためには、i2s_write()は高速に連続実行する必要があります。（1回で渡すサンプル数が非常に少ないので。）また、オーディオ制御で出力段と信号生成を完全同期させることは普通無いだろうと考え、分けました。


　モノラル化とピン配置は、私の都合です。MAX98357Aモジュールが手元に1個しかなかったのと、将来的にESP-lyraTという開発ボードを使いたいと考えており、これに配線を合わせました。もしステレオ化したい場合は、公式のように、信号配列にL, R, L, R, ...の順番で信号を詰める必要があります。



# さいごに
　半年ほど前にESP8266とESP32の存在を知り、購入して遊んでいたのですが、ESP-IDFの使い方について、基礎的な情報がうまく見つからなかったので、それならばと(初)投稿してみました。誰かの役に立つなら幸いです。まだまだ勉強中・試行錯誤している状況ですので、間違いの指摘は大歓迎です。



## 先人の例
https://qiita.com/h_nari/items/b52c525f0c5b35aaf692
http://www.narimatsu.net/blog/?p=11293
※いずれもv3以下を使っており、やや古い情報です。
（というか、最近v4になったばかりなので仕方がない）



## 材料の購入元例
- [ESP-WROOM-32](http://akizukidenshi.com/catalog/g/gM-11819/)
- [Adafruit I2S 3W Class D Amplifier Breakout](https://www.marutsu.co.jp/pc/i/829728/)  
※私は事情で現在中国在住のため、中国で類似品を購入し使用しています。


