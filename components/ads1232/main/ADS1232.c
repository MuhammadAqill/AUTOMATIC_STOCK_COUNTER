#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Definisikan pin yang dipakai ESP32
#define PDWN_PIN  26
#define DOUT_PIN  25
#define SCLK_PIN  33

// Inisialisasi GPIO untuk ADS1232
void ads1232_gpio_init(void) {
    gpio_config_t io_conf;

    // PDWN sebagai output
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PDWN_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    // DOUT sebagai input
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << DOUT_PIN);
    gpio_config(&io_conf);

    // SCLK sebagai output
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << SCLK_PIN);
    gpio_config(&io_conf);

    // Set PDWN ke HIGH (aktif)
    gpio_set_level(PDWN_PIN, 1);
    // Set SCLK awal LOW
    gpio_set_level(SCLK_PIN, 0);
}

// Fungsi baca 24-bit data mentah dari ADS1232
int32_t ads1232_read_raw(void) {
    int32_t value = 0;

    // Tunggu DOUT = LOW (data siap)
    while(gpio_get_level(DOUT_PIN) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Baca 24 bit dari ADS1232, bit per bit dengan SCLK
    for (int i = 0; i < 24; i++) {
        gpio_set_level(SCLK_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(1));

        value <<= 1;
        if (gpio_get_level(DOUT_PIN)) {
            value |= 1;
        }

        gpio_set_level(SCLK_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // ADS1232 mengirim data 24-bit two's complement
    // Sign extend ke 32-bit
    if (value & 0x800000) {
        value |= 0xFF000000;
    }

    return value;
}
