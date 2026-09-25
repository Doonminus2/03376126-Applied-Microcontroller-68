#include "ultrasonic.h"

#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ECHO_TIMEOUT_US   30000   // ~5m round trip ceiling
#define SOUND_CM_PER_US   0.0343f // speed of sound / 2 (round trip)

static gpio_num_t s_trig = GPIO_NUM_NC;
static gpio_num_t s_echo = GPIO_NUM_NC;

esp_err_t ultrasonic_init(gpio_num_t trig_pin, gpio_num_t echo_pin)
{
    s_trig = trig_pin;
    s_echo = echo_pin;

    gpio_reset_pin(s_trig);
    gpio_set_direction(s_trig, GPIO_MODE_OUTPUT);
    gpio_set_level(s_trig, 0);

    gpio_reset_pin(s_echo);
    gpio_set_direction(s_echo, GPIO_MODE_INPUT);

    return ESP_OK;
}

esp_err_t ultrasonic_read_cm(float *cm_out)
{
    if (s_trig == GPIO_NUM_NC || s_echo == GPIO_NUM_NC || cm_out == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // 10us trigger pulse.
    gpio_set_level(s_trig, 1);
    esp_rom_delay_us(10);
    gpio_set_level(s_trig, 0);

    int64_t start = esp_timer_get_time();
    while (gpio_get_level(s_echo) == 0) {
        if (esp_timer_get_time() - start > ECHO_TIMEOUT_US) {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t echo_start = esp_timer_get_time();
    while (gpio_get_level(s_echo) == 1) {
        if (esp_timer_get_time() - echo_start > ECHO_TIMEOUT_US) {
            return ESP_ERR_TIMEOUT;
        }
    }
    int64_t echo_end = esp_timer_get_time();

    *cm_out = (float)(echo_end - echo_start) * SOUND_CM_PER_US;
    return ESP_OK;
}
