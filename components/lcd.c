/*
 * Mission 1: LCD1 shows "System Ready!" + bargraph.
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
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "lcd_hd44780.h"

static const char *TAG = "main_app";

typedef struct {
    uint8_t line;
    uint8_t column;
    char text[17];
} lcd_msg_t;

static QueueHandle_t lcd_queue = NULL;

void lcd_server_task(void *pvParameters) {
    lcd_handle_t lcd = (lcd_handle_t)pvParameters;
    lcd_msg_t msg;

    while (1) {
        if (xQueueReceive(lcd_queue, &msg, portMAX_DELAY) == pdTRUE) {
            lcd_set_cursor(lcd, msg.line, msg.column);
            lcd_write_string(lcd, msg.text);
        }
    }
}

void telemetry_task(void *pvParameters) {
    uint8_t percent = 0;
    lcd_handle_t lcd = (lcd_handle_t)pvParameters;

    while (1) {
        lcd_draw_bargraph(lcd, 1, 0, 16, percent);
        percent = (percent + 5) % 105;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting Component-Based LCD System...");

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

    lcd_handle_t lcd1 = NULL;
    ESP_ERROR_CHECK(lcd_new_device(&lcd1_config, &lcd1));
    ESP_ERROR_CHECK(lcd_init_bargraph_glyphs(lcd1));

    lcd_set_cursor(lcd1, 0, 0);
    lcd_write_string(lcd1, "System Ready!");

    lcd_queue = xQueueCreate(10, sizeof(lcd_msg_t));
    if (lcd_queue != NULL) {
        xTaskCreate(lcd_server_task, "lcd_server", 3072, (void*)lcd1, 5, NULL);
        xTaskCreate(telemetry_task, "telemetry", 3072, (void*)lcd1, 4, NULL);
    }
}
