// Spiral 4: นาฬิกาปลุก + buzzer (LEDC) และ stopwatch + lap
// ใช้ component: i2c_bus, ds1307, ssd1306, clock_logic, time_manager, button_manager,
//                alarm_engine, stopwatch_engine
#include <cstdio>

#include "AlarmEngine.hpp"
#include "ButtonManager.hpp"
#include "DS1307.hpp"
#include "I2CBus.hpp"
#include "SSD1306.hpp"
#include "StopwatchEngine.hpp"
#include "TimeManager.hpp"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "alarm_demo";

static Clock::I2CBus i2cBus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
static Clock::DS1307 rtc(i2cBus);
static Clock::SSD1306 display(i2cBus);
static Clock::TimeManager timeMgr(rtc);
static Clock::ButtonManager buttons;
static Clock::AlarmEngine alarmEngine(GPIO_NUM_25);
static Clock::StopwatchEngine stopwatch;

static void buttonTask(void*) { buttons.taskLoop(); }

// task ตรวจปลุก: เทียบเวลาท้องถิ่นกับเวลาปลุกทุก 1 วินาที
static void alarmTask(void*) {
    while (true) {
        Clock::DateTime now;
        if (timeMgr.getLocalTime(now)) {
            alarmEngine.check(now, static_cast<uint32_t>(esp_timer_get_time() / 1000));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void draw(const Clock::UiState& ui) {
    char line[32];
    display.clear();
    display.drawString(0, 0, Clock::modeName(ui.mode));
    if (ui.mode == Clock::Mode::Stopwatch) {
        const Clock::StopwatchSnapshot s = stopwatch.snapshot();
        Clock::formatStopwatch(s.elapsedMs, line, sizeof(line));
        display.drawString(0, 14, line, 2);
        // lap ล่าสุด 3 รอบ
        const size_t first = s.lapCount > 3 ? s.lapCount - 3 : 0;
        for (size_t i = first; i < s.lapCount; ++i) {
            char split[16];
            Clock::formatStopwatch(s.laps[i].splitMs, split, sizeof(split));
            snprintf(line, sizeof(line), "L%u %s", static_cast<unsigned>(i + 1), split);
            display.drawString(0, 34 + static_cast<int>(i - first) * 10, line);
        }
    } else {
        Clock::DateTime now;
        if (timeMgr.getLocalTime(now)) {
            snprintf(line, sizeof(line), "%02u:%02u:%02u", now.hour, now.minute, now.second);
            display.drawString(0, 14, line, 2);
        }
        const Clock::AlarmSetting a = alarmEngine.setting();
        snprintf(line, sizeof(line), "ALARM %02u:%02u %s%s", a.hour, a.minute, a.enabled ? "ON" : "OFF",
                 alarmEngine.ringing() ? " RING" : "");
        display.drawString(0, 40, line);
    }
    display.flush();
}

extern "C" void app_main(void) {
    if (!i2cBus.init() || !rtc.init() || !display.init() || !buttons.init() || !alarmEngine.init()) {
        ESP_LOGE(TAG, "hardware init failed");
        return;
    }
    timeMgr.loadFromRtc();

    // ตั้งปลุกตัวอย่างเป็นเวลาปัจจุบัน + 1 นาที จะได้ทดสอบเสียงได้ทันที
    Clock::DateTime now;
    if (timeMgr.getLocalTime(now)) {
        Clock::AlarmSetting a;
        a.hour = (now.minute == 59) ? (now.hour + 1) % 24 : now.hour;
        a.minute = (now.minute + 1) % 60;
        a.enabled = true;
        alarmEngine.setSetting(a);
        ESP_LOGI(TAG, "[alarm] demo alarm set to %02u:%02u", a.hour, a.minute);
    }

    xTaskCreatePinnedToCore(buttonTask, "vTaskButton", 3072, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(alarmTask, "vTaskAlarm", 3072, nullptr, 4, nullptr, 1);

    Clock::UiState ui;
    while (true) {
        Clock::ButtonEvent ev;
        // รอ event ได้นานสุด 100 ms แล้ววาดจอใหม่ (10 FPS)
        if (buttons.receive(ev, pdMS_TO_TICKS(100))) {
            // กดปุ่มใดก็หยุดเสียงปลุกที่กำลังดัง
            if (alarmEngine.silence()) {
                draw(ui);
                continue;
            }
            const Clock::UiResult r = Clock::handlePress(ui, ev.button, ev.press);
            switch (r.command) {
                case Clock::Command::ToggleAlarm:
                    ESP_LOGI(TAG, "[alarm] %s", alarmEngine.toggleEnabled() ? "enabled" : "disabled");
                    break;
                case Clock::Command::StopwatchToggle: stopwatch.toggle(); break;
                case Clock::Command::StopwatchLapOrReset: stopwatch.lapOrReset(); break;
                case Clock::Command::StopwatchReset: stopwatch.reset(); break;
                default: break;
            }
        }
        draw(ui);
    }
}
