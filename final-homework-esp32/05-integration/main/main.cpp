// Spiral 5: ระบบเต็ม แบ่ง FreeRTOS task ตามตารางในสเปก
// ใช้ component: i2c_bus, ds1307, ssd1306, button_manager, time_manager,
//                alarm_engine, stopwatch_engine, clock_logic
//
// Task                Prio  Stack  Core  จังหวะ / หน้าที่
// vTaskClockDisplay    3    4096   1     100 ms      วาดจอ (เป็นเจ้าของจอตัวเดียว)
// vTaskButtonHandler   5    3072   0     ISR queue   debounce, แยก short/long
// vTaskUiEvents        4    4096   0     event ปุ่ม   สลับโหมด / สั่งงาน
// vTaskAlarmCheck      4    3072   1     1000 ms     เทียบเวลาปลุก
// vTaskNTPSync         1    6144   0     3600 s      Wi-Fi + SNTP + เขียน DS1307
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
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char* TAG = "main";

static Clock::I2CBus i2cBus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
static Clock::DS1307 rtc(i2cBus);
static Clock::SSD1306 display(i2cBus);
static Clock::TimeManager timeMgr(rtc);
static Clock::ButtonManager buttons;
static Clock::AlarmEngine alarmEngine(GPIO_NUM_25);
static Clock::StopwatchEngine stopwatch;

// ข้อมูลที่ UI task กับ task จอใช้ร่วมกัน อยู่หลัง mutex ตัวเดียว
struct SharedUi {
    Clock::UiState ui;
    Clock::DateTime timeDraft;       // เวลาที่กำลังแก้ในโหมด SetTime
    Clock::AlarmSetting alarmDraft;  // เวลาปลุกที่กำลังแก้ในโหมด Alarm
    bool timeDirty = false;
};
static SharedUi shared;
static SemaphoreHandle_t sharedMutex;

static uint32_t nowMs() { return static_cast<uint32_t>(esp_timer_get_time() / 1000); }

// เข้าโหมด SetTime/Alarm: คัดลอกค่าปัจจุบันมาเป็นร่างให้แก้
static void beginEdit(Clock::Mode mode) {
    if (mode == Clock::Mode::SetTime) {
        timeMgr.getLocalTime(shared.timeDraft);
        shared.timeDirty = false;
    } else if (mode == Clock::Mode::Alarm) {
        shared.alarmDraft = alarmEngine.setting();
    }
}

// ออกจากโหมด SetTime/Alarm: บันทึกค่าที่แก้
static void commitEdit(Clock::Mode mode) {
    if (mode == Clock::Mode::SetTime && shared.timeDirty) {
        if (timeMgr.setLocalTime(shared.timeDraft)) ESP_LOGI(TAG, "[time] manual set saved to RTC");
    } else if (mode == Clock::Mode::Alarm) {
        alarmEngine.setSetting(shared.alarmDraft);
        ESP_LOGI(TAG, "[alarm] set to %02u:%02u", shared.alarmDraft.hour, shared.alarmDraft.minute);
    }
}

static void incrementField() {
    if (shared.ui.mode == Clock::Mode::SetTime) {
        uint8_t* f[3] = {&shared.timeDraft.hour, &shared.timeDraft.minute, &shared.timeDraft.second};
        *f[shared.ui.field] = Clock::wrapIncrement(*f[shared.ui.field], shared.ui.field == 0 ? 24 : 60);
        shared.timeDirty = true;
    } else if (shared.ui.mode == Clock::Mode::Alarm) {
        if (shared.ui.field == 0) {
            shared.alarmDraft.hour = Clock::wrapIncrement(shared.alarmDraft.hour, 24);
        } else {
            shared.alarmDraft.minute = Clock::wrapIncrement(shared.alarmDraft.minute, 60);
        }
    }
}

// ---------- tasks ----------

static void vTaskButtonHandler(void*) { buttons.taskLoop(); }

static void vTaskNTPSync(void*) { timeMgr.taskNtpLoop(); }

static void vTaskUiEvents(void*) {
    while (true) {
        Clock::ButtonEvent ev;
        if (!buttons.receive(ev, portMAX_DELAY)) continue;
        if (alarmEngine.silence()) continue;  // กดปุ่มใดก็หยุดเสียงปลุก

        xSemaphoreTake(sharedMutex, portMAX_DELAY);
        const Clock::UiResult r = Clock::handlePress(shared.ui, ev.button, ev.press);
        if (r.modeChanged) {
            commitEdit(r.previousMode);
            beginEdit(shared.ui.mode);
            ESP_LOGI(TAG, "[mode] %s -> %s", Clock::modeName(r.previousMode), Clock::modeName(shared.ui.mode));
        }
        switch (r.command) {
            case Clock::Command::IncrementField: incrementField(); break;
            case Clock::Command::ToggleAlarm: alarmEngine.toggleEnabled(); break;
            case Clock::Command::StopwatchToggle: stopwatch.toggle(); break;
            case Clock::Command::StopwatchLapOrReset: stopwatch.lapOrReset(); break;
            case Clock::Command::StopwatchReset: stopwatch.reset(); break;
            default: break;
        }
        xSemaphoreGive(sharedMutex);
    }
}

static void vTaskAlarmCheck(void*) {
    while (true) {
        Clock::DateTime now;
        if (timeMgr.getLocalTime(now)) alarmEngine.check(now, nowMs());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void drawClock(const Clock::DateTime& now, bool ntp, const Clock::AlarmSetting& a) {
    char line[32];
    snprintf(line, sizeof(line), "%02u:%02u:%02u", now.hour, now.minute, now.second);
    display.drawString(0, 14, line, 2);
    snprintf(line, sizeof(line), "%04u-%02u-%02u", now.year, now.month, now.day);
    display.drawString(0, 36, line);
    display.drawString(104, 0, ntp ? "NTP" : "RTC");
    snprintf(line, sizeof(line), "ALARM %02u:%02u %s", a.hour, a.minute, a.enabled ? "ON" : "OFF");
    display.drawString(0, 52, line);
}

static void drawEditor(const SharedUi& s, bool isTime) {
    char line[16];
    if (isTime) {
        snprintf(line, sizeof(line), "%02u:%02u:%02u", s.timeDraft.hour, s.timeDraft.minute, s.timeDraft.second);
    } else {
        snprintf(line, sizeof(line), "%02u:%02u", s.alarmDraft.hour, s.alarmDraft.minute);
    }
    display.drawString(0, 24, line, 2);
    display.invertRect(s.ui.field * 36, 22, 24, 18);  // ไฮไลต์ฟิลด์ที่กำลังแก้ (2 หลัก x 12 พิกเซล + ":" 12 พิกเซล)
}

static void drawStopwatch() {
    char line[32];
    const Clock::StopwatchSnapshot sw = stopwatch.snapshot();
    Clock::formatStopwatch(sw.elapsedMs, line, sizeof(line));
    display.drawString(0, 14, line, 2);
    const size_t first = sw.lapCount > 3 ? sw.lapCount - 3 : 0;
    for (size_t i = first; i < sw.lapCount; ++i) {
        char split[16];
        Clock::formatStopwatch(sw.laps[i].splitMs, split, sizeof(split));
        snprintf(line, sizeof(line), "L%u %s", static_cast<unsigned>(i + 1), split);
        display.drawString(0, 36 + static_cast<int>(i - first) * 10, line);
    }
}

static void vTaskClockDisplay(void*) {
    int64_t lastLogUs = 0;
    while (true) {
        // คัดลอกสถานะออกมาก่อนวาด จะได้ไม่ถือ mutex ระหว่างส่ง I2C
        xSemaphoreTake(sharedMutex, portMAX_DELAY);
        const SharedUi snap = shared;
        xSemaphoreGive(sharedMutex);

        Clock::DateTime now;
        const bool haveTime = timeMgr.getLocalTime(now);
        const bool ntp = timeMgr.isNtpSynced();

        display.clear();
        display.drawString(0, 0, Clock::modeName(snap.ui.mode));
        switch (snap.ui.mode) {
            case Clock::Mode::Clock:
                if (haveTime) drawClock(now, ntp, alarmEngine.setting());
                break;
            case Clock::Mode::SetTime: drawEditor(snap, true); break;
            case Clock::Mode::Alarm: drawEditor(snap, false); break;
            case Clock::Mode::Stopwatch: drawStopwatch(); break;
        }
        display.flush();

        if (haveTime && esp_timer_get_time() - lastLogUs >= 1'000'000) {
            lastLogUs = esp_timer_get_time();
            ESP_LOGI(TAG, "[clock] %02u:%02u:%02u src=%s", now.hour, now.minute, now.second, ntp ? "NTP" : "RTC");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

extern "C" void app_main(void) {
    sharedMutex = xSemaphoreCreateMutex();

    if (!i2cBus.init() || !rtc.init() || !display.init() || !alarmEngine.init() || !buttons.init()) {
        ESP_LOGE(TAG, "hardware init failed");
        return;
    }
    timeMgr.loadFromRtc();

    xTaskCreatePinnedToCore(vTaskButtonHandler, "vTaskButtonHandler", 3072, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(vTaskUiEvents, "vTaskUiEvents", 4096, nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(vTaskClockDisplay, "vTaskClockDisplay", 4096, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(vTaskAlarmCheck, "vTaskAlarmCheck", 3072, nullptr, 4, nullptr, 1);
    xTaskCreatePinnedToCore(vTaskNTPSync, "vTaskNTPSync", 6144, nullptr, 1, nullptr, 0);
    ESP_LOGI(TAG, "All tasks started");
}
