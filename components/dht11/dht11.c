#include "dht11.h"

#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static gpio_num_t s_pin = GPIO_NUM_NC;

static esp_err_t wait_for_level(int level, int timeout_us)
{
    int elapsed = 0;
    while (gpio_get_level(s_pin) != level) {
        if (elapsed >= timeout_us) {
            return ESP_ERR_TIMEOUT;
        }
        esp_rom_delay_us(1);
        elapsed++;
    }
    return ESP_OK;
}

esp_err_t dht11_init(gpio_num_t pin)
{
    s_pin = pin;
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);
    return ESP_OK;
}

esp_err_t dht11_read(dht11_data_t *out)
{
    if (s_pin == GPIO_NUM_NC || out == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t bits[5] = {0};

    // Host start signal: pull low >=18ms, then release.
    gpio_set_direction(s_pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(s_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(s_pin, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(s_pin, GPIO_MODE_INPUT);

    // Sensor response: low ~80us, then high ~80us.
    esp_err_t err = wait_for_level(0, 100);
    if (err != ESP_OK) return err;
    err = wait_for_level(1, 100);
    if (err != ESP_OK) return err;
    err = wait_for_level(0, 100);
    if (err != ESP_OK) return err;

    for (int i = 0; i < 40; i++) {
        err = wait_for_level(1, 60);
        if (err != ESP_OK) return err;

        // Each bit: 50us low, then high for ~26-28us (0) or ~70us (1).
        int high_us = 0;
        while (gpio_get_level(s_pin) == 1) {
            esp_rom_delay_us(1);
            high_us++;
            if (high_us > 100) return ESP_ERR_TIMEOUT;
        }

        int bit = (high_us > 40) ? 1 : 0;
        bits[i / 8] = (bits[i / 8] << 1) | bit;
    }

    uint8_t checksum = bits[0] + bits[1] + bits[2] + bits[3];
    if (checksum != bits[4]) {
        return ESP_ERR_INVALID_CRC;
    }

    out->humidity = (float)bits[0] + (float)bits[1] / 10.0f;
    out->temperature = (float)bits[2] + (float)bits[3] / 10.0f;
    return ESP_OK;
}
