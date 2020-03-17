# Summary 概要
This program is a sample for using external i2s for ESP32 by ESP-IDF.
Because original example program dosen't work well, so I modified it to work well. (And more simply for easy understanding)
Original is from https://github.com/espressif/esp-idf .

Modified:
- work well
- 1 channel(mono)
- Pin assign(for compatible with ESP-ADF dev-boards)

---
このプログラムはESP32の外部i2sをESP-IDFで使うサンプルプログラムです。
オリジナルのサンプルがうまく動かなかったので、動くよう修正しました。（かつ理解しやすく変更した）
オリジナルは https://github.com/espressif/esp-idf にあります。

変更点：
- 動作可能にした
- モノラル化
- ピンアサイン（ESP-ADF開発ボードに合わせた）

---
这程序是一个例子， 用ESP-IDF使用外部i2s功能。
原本样本程序不太成功， 我修改了一下。
原本是在 https://github.com/espressif/esp-idf 。

变更点：
- 能运行了
- 单声道化
- 端子分配 (跟ESP-ADF开发板一致了)


# Pin Assgin

| pin name (MAX98357A board print) | function | gpio_num |
|:---:|:---:|:---:|
| WS (LRC)  |word select (L/R Clock)| GPIO_NUM_25 |
| SCK (BCLK) |continuous serial clock (Block CLocK)| GPIO_NUM_5 |
| SD  |serial data (Data INput)| GPIO_NUM_26 |


# Origin

https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2s


# Fritzing footprint data
ESP32 : https://fritzing.org/projects/esp32_ds3231/
MAX9837A : https://github.com/adafruit/Fritzing-Library


# How to Use Example

## Hardware Required

* A development board with ESP32 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for power supply and programming

## Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.


