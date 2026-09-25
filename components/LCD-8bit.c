/*
 * Mission 2: two LCDs, different bus modes, running at the same time.
 *   LCD1 (4-bit): "System Ready!" + bargraph (Mission 1 code)
 *   LCD2 (8-bit): system status (uptime, free heap, task count)
 *
 * LCD1 (4-bit) wiring:
 *   LCD pin 1  VSS -> GND
 *   LCD pin 2  VDD -> 5V
 *   LCD pin 3  V0  -> GND (contrast, no potentiometer)
 *   LCD pin 4  RS  -> GPIO4
 *   LCD pin 5  R/W -> GND
 *   LCD pin 6  E   -> GPIO5
 *   LCD pin 7-10  D0-D3 -> not connected
 *   LCD pin 11 D4  -> GPIO16
 *   LCD pin 12 D5  -> GPIO17
 *   LCD pin 13 D6  -> GPIO18
 *   LCD pin 14 D7  -> GPIO19
 *   LCD pin 15 A   -> 5V (via 220 ohm)
 *   LCD pin 16 K   -> GND
 *
 * LCD2 (8-bit) wiring:
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
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lcd_hd44780.h"

static const char *TAG = "main_app";

void telemetry_task(void *pvParameters) {
    uint8_t percent = 0;
    lcd_handle_t lcd = (lcd_handle_t)pvParameters;

    while (1) {
        lcd_draw_bargraph(lcd, 1, 0, 16, percent);
        percent = (percent + 5) % 105;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void status_task(void *pvParameters) {
    lcd_handle_t lcd = (lcd_handle_t)pvParameters;
    char line[32]; // extra room for snprintf, cut to 16 columns below

    while (1) {
        uint32_t uptime_s = xTaskGetTickCount() / configTICK_RATE_HZ;
        snprintf(line, sizeof(line), "Up:%5lus T:%-3u  ",
                 (unsigned long)uptime_s, (unsigned)uxTaskGetNumberOfTasks());
        line[16] = '\0';
        lcd_set_cursor(lcd, 0, 0);
        lcd_write_string(lcd, line);

        snprintf(line, sizeof(line), "Heap:%-7lu OK ", (unsigned long)esp_get_free_heap_size());
        line[16] = '\0';
        lcd_set_cursor(lcd, 1, 0);
        lcd_write_string(lcd, line);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting Dual LCD System...");

    lcd_config_t lcd1_config = {
        .bus_mode = LCD_BUS_MODE_4BIT,
        .rs_io = GPIO_NUM_4,  // LCD pin 4 (RS)
        .rw_io = GPIO_NUM_NC, // ต่อ RW ลง GND (LCD pin 5)
        .en_io = GPIO_NUM_5,  // LCD pin 6 (E)
        // 4-bit: data_ios[0..3] -> LCD D4..D7
        .data_ios = { GPIO_NUM_16,   // LCD pin 11 (D4)
                      GPIO_NUM_17,   // LCD pin 12 (D5)
                      GPIO_NUM_18,   // LCD pin 13 (D6)
                      GPIO_NUM_19 }  // LCD pin 14 (D7)
    };

    lcd_config_t lcd2_config = {
        .bus_mode = LCD_BUS_MODE_8BIT,
        .rs_io = GPIO_NUM_21, // LCD pin 4 (RS)
        .rw_io = GPIO_NUM_NC, // ต่อ RW ลง GND (LCD pin 5)
        .en_io = GPIO_NUM_22, // LCD pin 6 (E)
        // 8-bit: data_ios[0..7] -> LCD D0..D7
        .data_ios = { GPIO_NUM_13,   // LCD pin 7  (D0)
                      GPIO_NUM_14,   // LCD pin 8  (D1)
                      GPIO_NUM_23,   // LCD pin 9  (D2)
                      GPIO_NUM_25,   // LCD pin 10 (D3)
                      GPIO_NUM_26,   // LCD pin 11 (D4)
                      GPIO_NUM_27,   // LCD pin 12 (D5)
                      GPIO_NUM_32,   // LCD pin 13 (D6)
                      GPIO_NUM_33 }  // LCD pin 14 (D7)
    };

    lcd_handle_t lcd1 = NULL;
    lcd_handle_t lcd2 = NULL;
    ESP_ERROR_CHECK(lcd_new_device(&lcd1_config, &lcd1));
    ESP_ERROR_CHECK(lcd_new_device(&lcd2_config, &lcd2));
    ESP_ERROR_CHECK(lcd_init_bargraph_glyphs(lcd1));

    lcd_set_cursor(lcd1, 0, 0);
    lcd_write_string(lcd1, "System Ready!");

    xTaskCreate(telemetry_task, "telemetry", 3072, (void*)lcd1, 5, NULL);
    xTaskCreate(status_task, "status", 3072, (void*)lcd2, 4, NULL);
}
