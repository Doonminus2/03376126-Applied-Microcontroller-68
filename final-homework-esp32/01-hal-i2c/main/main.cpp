// Spiral 1: HAL + I2C - สแกนบัส, DS1307, SSD1306
// ใช้ component: i2c_bus, ds1307, ssd1306, clock_logic
#include <cstdio>
#include <cstdlib>

#include "DS1307.hpp"
#include "I2CBus.hpp"
#include "SSD1306.hpp"
#include "clock_logic/datetime.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "hal_demo";

static Clock::I2CBus i2cBus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
static Clock::DS1307 rtc(i2cBus);
static Clock::SSD1306 display(i2cBus);

static void scanBus() {
  int found = 0;
  for (uint8_t addr = 0x08; addr <= 0x77; ++addr) {
    if (i2cBus.probe(addr)) {
      ESP_LOGI(TAG, "I2C device found at 0x%02X", addr);
      ++found;
    }
  }
  ESP_LOGI(TAG, "Scan done: %d device(s)", found);
}

extern "C" void app_main(void) {
  if (!i2cBus.init()) {
    ESP_LOGE(TAG, "I2C bus init failed");
    return;
  }
  scanBus();

  ESP_LOGI(TAG, "DS1307 init: %s", rtc.init() ? "OK" : "FAIL");
  ESP_LOGI(TAG, "SSD1306 init: %s", display.init() ? "OK" : "FAIL");

  // เขียนเวลา 2026-10-28 15:30:00 แล้วอ่านกลับมาเทียบ
  Clock::DateTime written;
  written.year = 2026;
  written.month = 10;
  written.day = 28;
  written.weekday =
      Clock::weekdayFromDate(written.year, written.month, written.day);
  written.hour = 15;
  written.minute = 25;
  written.second = 0;
  Clock::DateTime readBack;
  if (rtc.setTime(written) && rtc.getTime(readBack)) {
    const long diff = labs(
        static_cast<long>(Clock::toEpoch(readBack) - Clock::toEpoch(written)));
    ESP_LOGI(TAG, "Read-back check: %s (diff=%lds)",
             diff <= 2 ? "PASS" : "FAIL", diff);
  } else {
    ESP_LOGE(TAG, "Read-back check: FAIL (I2C error)");
  }

  // วนทุก 1 วินาที อ่านเวลาจาก DS1307 แสดงบนจอและ Serial
  while (true) {
    Clock::DateTime now;
    if (rtc.getTime(now)) {
      char date[16], time[16];
      snprintf(date, sizeof(date), "%04u-%02u-%02u", now.year, now.month,
               now.day);
      snprintf(time, sizeof(time), "%02u:%02u:%02u", now.hour, now.minute,
               now.second);
      display.clear();
      display.drawString(0, 0, "SPIRAL 1: HAL I2C");
      display.drawString(0, 16, date, 2);
      display.drawString(0, 40, time, 2);
      display.flush();
      ESP_LOGI(TAG, "DS1307 %s %s", date, time);
    } else {
      ESP_LOGW(TAG, "DS1307 read failed");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
