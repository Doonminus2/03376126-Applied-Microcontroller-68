#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// HC-SR04 style: separate trig/echo pins.
esp_err_t ultrasonic_init(gpio_num_t trig_pin, gpio_num_t echo_pin);

// Blocking. Returns ESP_ERR_TIMEOUT if no echo within range.
esp_err_t ultrasonic_read_cm(float *cm_out);

#ifdef __cplusplus
}
#endif
