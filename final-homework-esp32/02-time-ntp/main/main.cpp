// Spiral 2: Wi-Fi STA + SNTP + ซิงก์เวลาลง DS1307
// ใช้ component: i2c_bus, ds1307, ssd1306, clock_logic, time_manager
#include <cstdio>

#include "DS1307.hpp"
#include "I2CBus.hpp"
#include "SSD1306.hpp"
#include "TimeManager.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "time_demo";

static Clock::I2CBus i2cBus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
static Clock::DS1307 rtc(i2cBus);
static Clock::SSD1306 display(i2cBus);
static Clock::TimeManager timeMgr(rtc);

static void ntpTask(void*) { timeMgr.taskNtpLoop(); }

extern "C" void app_main(void) {
    if (!i2cBus.init() || !rtc.init() || !display.init()) {
        ESP_LOGE(TAG, "hardware init failed");
        return;
    }

    // อ่าน DS1307 ตั้งเวลาระบบก่อนต่อเน็ต นาฬิกาจึงถูกตั้งแต่วินาทีแรก
    if (timeMgr.loadFromRtc()) ESP_LOGI(TAG, "Time from RTC loaded");

    // NVS -> esp_netif -> Wi-Fi STA -> SNTP -> เขียนลง DS1307 ทำงานใน task แยก
    xTaskCreatePinnedToCore(ntpTask, "ntp_sync", 6144, nullptr, 1, nullptr, 0);

    const int64_t bootUs = esp_timer_get_time();
    bool failureLogged = false;

    while (true) {
        Clock::DateTime now;
        if (timeMgr.getLocalTime(now)) {
            const bool synced = timeMgr.isNtpSynced();
            const char* status = synced ? "NTP SYNCED" : (timeMgr.isWifiConnected() ? "SYNCING" : "RTC ONLY");

            char date[16], time[16];
            snprintf(date, sizeof(date), "%04u-%02u-%02u", now.year, now.month, now.day);
            snprintf(time, sizeof(time), "%02u:%02u:%02u", now.hour, now.minute, now.second);
            display.clear();
            display.drawString(0, 0, status);
            display.drawString(0, 16, time, 2);
            display.drawString(0, 40, date, 2);
            display.flush();
            ESP_LOGI(TAG, "[time] %s %s source=%s", date, time, synced ? "NTP" : "RTC");
        }

        // ผ่านไป 60 วินาทีแล้วยังซิงก์ไม่ได้ = ใช้เวลาจาก RTC ต่อไป
        if (!failureLogged && !timeMgr.isNtpSynced() && esp_timer_get_time() - bootUs > 60'000'000) {
            ESP_LOGW(TAG, "NTP failed, using RTC time");
            failureLogged = true;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
