#include "driver/dac_oneshot.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lcd_hd44780.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "main_app";

typedef struct {
  uint8_t line;
  uint8_t column;
  char text[17];
} lcd_msg_t;

static QueueHandle_t lcd_queue = NULL;

/* No potentiometer: DAC drives V0 (contrast), 2 buttons adjust it.
 * LCD V0 (pin 3) -> GPIO25 (DAC_CHAN_0)
 * Button UP   -> GPIO32 (active low, internal pull-up) : lower V0 = darker
 * Button DOWN -> GPIO33 (active low, internal pull-up) : higher V0 = lighter */
#define BTN_UP GPIO_NUM_32
#define BTN_DOWN GPIO_NUM_33
#define CONTRAST_STEP 5

static dac_oneshot_handle_t contrast_dac = NULL;
static uint8_t contrast_level = 30; // 30/255 * 3.3 V ~= 0.39 V on V0

void contrast_task(void *pvParameters) {
  while (1) {
    int up = gpio_get_level(BTN_UP) == 0;
    int down = gpio_get_level(BTN_DOWN) == 0;

    if (up && contrast_level >= CONTRAST_STEP) {
      contrast_level -= CONTRAST_STEP;
    } else if (down && contrast_level <= 255 - CONTRAST_STEP) {
      contrast_level += CONTRAST_STEP;
    }

    if (up || down) {
      dac_oneshot_output_voltage(contrast_dac, contrast_level);
      ESP_LOGI(TAG, "Contrast DAC = %u (V0 ~= %.2f V)", contrast_level,
               contrast_level * 3.3f / 255.0f);
    }
    vTaskDelay(pdMS_TO_TICKS(150)); // debounce + repeat rate while held
  }
}

static void contrast_init(void) {
  dac_oneshot_config_t dac_cfg = {.chan_id = DAC_CHAN_0}; // GPIO25
  ESP_ERROR_CHECK(dac_oneshot_new_channel(&dac_cfg, &contrast_dac));
  ESP_ERROR_CHECK(dac_oneshot_output_voltage(contrast_dac, contrast_level));

  gpio_config_t btn_conf = {.pin_bit_mask =
                                (1ULL << BTN_UP) | (1ULL << BTN_DOWN),
                            .mode = GPIO_MODE_INPUT,
                            .pull_up_en = GPIO_PULLUP_ENABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_DISABLE};
  ESP_ERROR_CHECK(gpio_config(&btn_conf));
}

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

  contrast_init();

  lcd_config_t lcd1_config = {
      .bus_mode = LCD_BUS_MODE_4BIT,
      .rs_io = GPIO_NUM_4,
      .rw_io = GPIO_NUM_NC, // ต่อ RW ลง GND
      .en_io = GPIO_NUM_5,
      .data_ios = {GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_19}};

  lcd_handle_t lcd1 = NULL;
  ESP_ERROR_CHECK(lcd_new_device(&lcd1_config, &lcd1));
  ESP_ERROR_CHECK(lcd_init_bargraph_glyphs(lcd1));

  lcd_set_cursor(lcd1, 0, 0);
  lcd_write_string(lcd1, "System Ready!");

  lcd_queue = xQueueCreate(10, sizeof(lcd_msg_t));
  if (lcd_queue != NULL) {
    xTaskCreate(lcd_server_task, "lcd_server", 3072, (void *)lcd1, 5, NULL);
    xTaskCreate(telemetry_task, "telemetry", 3072, (void *)lcd1, 4, NULL);
    xTaskCreate(contrast_task, "contrast", 3072, NULL, 3, NULL);
  }
}
