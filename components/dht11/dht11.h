#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature;   // degrees C
    float humidity;      // % RH
} dht11_data_t;

// Call once before dht11_read(). Configures the data pin.
esp_err_t dht11_init(gpio_num_t pin);

// Blocking read. DHT11 needs >=1s between reads.
// Returns ESP_ERR_TIMEOUT / ESP_ERR_INVALID_CRC on protocol failure.
esp_err_t dht11_read(dht11_data_t *out);

#ifdef __cplusplus
}
#endif
