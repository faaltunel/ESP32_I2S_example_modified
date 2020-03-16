/* I2S Example

    This example code will output 100Hz sine wave and triangle wave to 2-channel of I2S driver
    Every 5 seconds, it will change bits_per_sample [16, 24, 32] for i2s data

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.

    -----

    modified by moppii.

*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include <math.h>

#define SAMPLE_RATE     (44100)
#define I2S_NUM         (0)
#define WAVE_FREQ_HZ    (220)
#define MAXVOL_INT      (15000)
#define PI              (3.14159265)
#define I2S_BCK_IO      (GPIO_NUM_5) //(GPIO_NUM_26)
#define I2S_WS_IO       (GPIO_NUM_25) //(GPIO_NUM_25)
#define I2S_DO_IO       (GPIO_NUM_26) //(GPIO_NUM_22)
#define I2S_DI_IO       (-1)

#define SAMPLE_PER_CYCLE  (SAMPLE_RATE/WAVE_FREQ_HZ)
#define BUFFER_NUM        (6)
#define DELAY_MS          (5)

uint16_t samples_data[(SAMPLE_PER_CYCLE)];
size_t i2s_bytes_write = 0;

void setup_waves(void){
    double sin_float;
    for(int i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = sin(2 * i * PI / (double)SAMPLE_PER_CYCLE);
        samples_data[i] = (uint16_t)((sin_float + 1) * MAXVOL_INT);
//        printf("%d\n",samples_data[i]);     //to confirm signal datas
    }
}

/*
static void setup_triangle_sine_waves(int bits){}
*/

void app_main()
{
    //for 36Khz sample rates, we create 100Hz sine wave, every cycle need 36000/100 = 360 samples (4-bytes or 8-bytes each sample)
    //depend on bits_per_sample
    //using 6 buffers, we need 60-samples per buffer
    //if 2-channels, 16-bit each channel, total buffer is 360*4 = 1440 bytes
    //if 2-channels, 24/32-bit each channel, total buffer is 360*8 = 2880 bytes
    //if 1-channels, 16-bit at one channel, total buffer is 360*2 = 720 bytes

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,                            //mono
//        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                           //2-channels
        .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
        .dma_buf_count = BUFFER_NUM,
        .dma_buf_len = SAMPLE_PER_CYCLE,        //one cycle per one buffer
        .use_apll = false,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1                                //Interrupt level 1
    };
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_DO_IO,
        .data_in_num = I2S_DI_IO                                               //Not used
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM, &pin_config);
    i2s_set_clk(I2S_NUM, SAMPLE_RATE, 16, 1);       //16bits, 1 channels

    //generate sine wave data (unsigned int 16bit)
    setup_waves();

    while (1) {
        i2s_write(I2S_NUM, samples_data, sizeof(samples_data), &i2s_bytes_write, portMAX_DELAY);
        vTaskDelay(DELAY_MS/portTICK_RATE_MS);
    }

}
