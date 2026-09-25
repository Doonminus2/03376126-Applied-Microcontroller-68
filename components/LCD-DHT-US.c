/*
 * LCD 8-bit shows DHT11 + HC-SR04 readings.
 *   Line 1: T:28.0°C H: 65%
 *   Line 2: Dist:  123.4 cm
 *
 * LCD (8-bit) wiring:
 *   LCD pin 1  VSS -> GND
 *   LCD pin 2  VDD -> 5V
 *   LCD pin 3  V0  -> GND (contrast, no potentiometer)
 *   LCD pin 4  RS  -> GPIO21
 *   LCD pin 5  R/W -> GND
 *   LCD pin 6  E   -> GPIO22
 *   LCD pin 7  D0  -> GPIO13
 *   LCD pin 8  D1  -> GPIO14
 *   LCD pin 9  D2  -> GPIO23
 *   LCD pin 10 D3  -> GPIO25
 *   LCD pin 11 D4  -> GPIO26
 *   LCD pin 12 D5  -> GPIO27
 *   LCD pin 13 D6  -> GPIO32
 *   LCD pin 14 D7  -> GPIO33
 *   LCD pin 15 A   -> 5V (via 220 ohm)
 *   LCD pin 16 K   -> GND
 *
 * DHT11 wiring:
 *   VCC  -> 3.3V
 *   DATA -> GPIO4 (10k pull-up to 3.3V if the module has none)
 *   GND  -> GND
 *
 * HC-SR04 wiring:
 *   VCC  -> 5V
 *   TRIG -> GPIO18
 *   ECHO -> GPIO19 (ECHO is 5V: use divider 1k in series + 2k to GND)
 *   GND  -> GND
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "lcd_hd44780.h"
#include "dht11.h"

static const char *TAG = "lcd_dht_us";

#define DHT_GPIO        GPIO_NUM_4
#define US_TRIG_GPIO    GPIO_NUM_18
#define US_ECHO_GPIO    GPIO_NUM_19

#define ECHO_TIMEOUT_US 30000 // ~5 m round trip ceiling
#define US_MIN_CM       2.0f  // HC-SR04 usable range
#define US_MAX_CM       400.0f
#define US_SAMPLES      5     // median of 5 rejects single bad echoes
#define DHT_PERIOD_MS   2000  // DHT11 needs >= 1 s between reads
#define US_PERIOD_MS    250

static void ultrasonic_setup(void) {
    gpio_reset_pin(US_TRIG_GPIO);
    gpio_set_direction(US_TRIG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(US_TRIG_GPIO, 0);

    gpio_reset_pin(US_ECHO_GPIO);
    gpio_set_direction(US_ECHO_GPIO, GPIO_MODE_INPUT);
}

/* Echo pulse width in microseconds (same logic as components/ultrasonic). */
static esp_err_t ultrasonic_echo_us(int64_t *echo_us) {
    // 10 us trigger pulse.
    gpio_set_level(US_TRIG_GPIO, 1);
    esp_rom_delay_us(10);
    gpio_set_level(US_TRIG_GPIO, 0);

    int64_t start = esp_timer_get_time();
    while (gpio_get_level(US_ECHO_GPIO) == 0) {
        if (esp_timer_get_time() - start > ECHO_TIMEOUT_US) {
            return ESP_ERR_TIMEOUT;
        }
    }

    int64_t echo_start = esp_timer_get_time();
    while (gpio_get_level(US_ECHO_GPIO) == 1) {
        if (esp_timer_get_time() - echo_start > ECHO_TIMEOUT_US) {
            return ESP_ERR_TIMEOUT;
        }
    }

    *echo_us = esp_timer_get_time() - echo_start;
    return ESP_OK;
}

/* Distance in cm, median of US_SAMPLES echoes.
 * Speed of sound depends on air temperature: v = 331.3 + 0.606 * T (m/s).
 * Echo time is the round trip, so distance = time * v / 2. */
static esp_err_t us_read_distance_cm(float temp_c, float *cm_out) {
    float samples[US_SAMPLES];
    int count = 0;
    float cm_per_us = (331.3f + 0.606f * temp_c) / 10000.0f; // m/s -> cm/us

    for (int i = 0; i < US_SAMPLES; i++) {
        int64_t echo_us;
        if (ultrasonic_echo_us(&echo_us) == ESP_OK) {
            float cm = (float)echo_us * cm_per_us / 2.0f;
            // insertion sort, keeps samples[] ordered for the median
            int j = count++;
            while (j > 0 && samples[j - 1] > cm) {
                samples[j] = samples[j - 1];
                j--;
            }
            samples[j] = cm;
        }
        vTaskDelay(pdMS_TO_TICKS(60)); // HC-SR04 needs ~60 ms between pings
    }

    if (count == 0) {
        return ESP_ERR_TIMEOUT;
    }
    *cm_out = samples[count / 2];
    return ESP_OK;
}

void sensor_display_task(void *pvParameters) {
    lcd_handle_t lcd = (lcd_handle_t)pvParameters;
    char line[32]; // extra room for snprintf, cut to 16 columns below
    float temp_c = 25.0f; // used for speed of sound until first DHT read
    TickType_t last_dht = xTaskGetTickCount() - pdMS_TO_TICKS(DHT_PERIOD_MS);

    vTaskDelay(pdMS_TO_TICKS(1000)); // DHT11 settle time after power-up

    while (1) {
        // Line 1: DHT11
        if (xTaskGetTickCount() - last_dht >= pdMS_TO_TICKS(DHT_PERIOD_MS)) {
            last_dht = xTaskGetTickCount();
            dht11_data_t dht;
            if (dht11_read(&dht) == ESP_OK) {
                temp_c = dht.temperature;
                // 0xDF = degree sign in HD44780 ROM
                snprintf(line, sizeof(line), "T:%4.1f\xDF" "C H:%3.0f%%  ",
                         dht.temperature, dht.humidity);
                ESP_LOGI(TAG, "Temp %.1f C, Humidity %.0f %%", dht.temperature, dht.humidity);
            } else {
                snprintf(line, sizeof(line), "DHT11 read error");
                ESP_LOGW(TAG, "DHT11 read failed");
            }
            line[16] = '\0';
            lcd_set_cursor(lcd, 0, 0);
            lcd_write_string(lcd, line);
        }

        // Line 2: HC-SR04
        float cm;
        if (us_read_distance_cm(temp_c, &cm) != ESP_OK || cm > US_MAX_CM) {
            snprintf(line, sizeof(line), "Dist: out range ");
        } else if (cm < US_MIN_CM) {
            snprintf(line, sizeof(line), "Dist: too close ");
        } else {
            snprintf(line, sizeof(line), "Dist: %6.1f cm  ", cm);
            ESP_LOGI(TAG, "Distance %.1f cm", cm);
        }
        line[16] = '\0';
        lcd_set_cursor(lcd, 1, 0);
        lcd_write_string(lcd, line);

        vTaskDelay(pdMS_TO_TICKS(US_PERIOD_MS));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting LCD + DHT11 + HC-SR04...");

    lcd_config_t lcd_config = {
        .bus_mode = LCD_BUS_MODE_8BIT,
        .rs_io = GPIO_NUM_21, // LCD pin 4 (RS)
        .rw_io = GPIO_NUM_NC, // ต่อ RW ลง GND (LCD pin 5)
        .en_io = GPIO_NUM_22, // LCD pin 6 (E)
        .data_ios = { GPIO_NUM_13,   // LCD pin 7  (D0)
                      GPIO_NUM_14,   // LCD pin 8  (D1)
                      GPIO_NUM_23,   // LCD pin 9  (D2)
                      GPIO_NUM_25,   // LCD pin 10 (D3)
                      GPIO_NUM_26,   // LCD pin 11 (D4)
                      GPIO_NUM_27,   // LCD pin 12 (D5)
                      GPIO_NUM_32,   // LCD pin 13 (D6)
                      GPIO_NUM_33 }  // LCD pin 14 (D7)
    };

    lcd_handle_t lcd = NULL;
    ESP_ERROR_CHECK(lcd_new_device(&lcd_config, &lcd));

    lcd_set_cursor(lcd, 0, 0);
    lcd_write_string(lcd, "T:--.-\xDF" "C H: --%");
    lcd_set_cursor(lcd, 1, 0);
    lcd_write_string(lcd, "Dist:   ---- cm");

    ESP_ERROR_CHECK(dht11_init(DHT_GPIO));
    ultrasonic_setup();

    xTaskCreate(sensor_display_task, "sensor_lcd", 4096, (void*)lcd, 5, NULL);
}
