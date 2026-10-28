// Spiral 3: ปุ่ม ISR + debounce + short/long press + UI state machine
// ใช้ component: i2c_bus, ssd1306, button_manager, clock_logic (ไม่ใช้ DS1307 ใน spiral นี้)
#include <cstdio>

#include "ButtonManager.hpp"
#include "I2CBus.hpp"
#include "SSD1306.hpp"
#include "clock_logic/ui_mode.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ui_demo";

static Clock::I2CBus i2cBus(I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
static Clock::SSD1306 display(i2cBus);
static Clock::ButtonManager buttons;

static void buttonTask(void*) { buttons.taskLoop(); }

// ค่าตัวอย่างที่ SET เพิ่มได้ในโหมด SetTime / Alarm (ยังไม่มี RTC ใน spiral นี้)
struct Draft {
    uint8_t field[3] = {12, 0, 0};  // ชม. / นาที / วินาที
};

static void draw(const Clock::UiState& ui, const Draft& draft) {
    char line[24];
    display.clear();
    display.drawString(0, 0, Clock::modeName(ui.mode), 2);
    if (Clock::fieldCount(ui.mode) > 0) {
        snprintf(line, sizeof(line), "%02u:%02u:%02u", draft.field[0], draft.field[1], draft.field[2]);
        display.drawString(0, 28, line, 2);
        // ไฮไลต์ฟิลด์ที่กำลังแก้ (ตัวอักษร 2 หลัก กว้าง 12 พิกเซล x 2 = 24 พิกเซล บวกเครื่องหมาย ":" อีก 12)
        display.invertRect(ui.field * 36, 26, 24, 18);
    } else {
        display.drawString(0, 28, "press buttons", 1);
    }
    display.flush();
}

extern "C" void app_main(void) {
    esp_log_level_set("ButtonManager", ESP_LOG_DEBUG);  // ดูจำนวนขอบดิบเทียบกับที่ผ่าน debounce

    if (!i2cBus.init() || !display.init() || !buttons.init()) {
        ESP_LOGE(TAG, "hardware init failed");
        return;
    }
    xTaskCreatePinnedToCore(buttonTask, "vTaskButton", 3072, nullptr, 5, nullptr, 0);

    Clock::UiState ui;
    Draft draft;
    draw(ui, draft);

    // task UI (main task): รอ event จาก queue ที่ ButtonManager ส่งมา
    while (true) {
        Clock::ButtonEvent ev;
        if (!buttons.receive(ev, portMAX_DELAY)) continue;

        ESP_LOGI(TAG, "[btn] %s %s", Clock::ButtonManager::buttonName(ev.button),
                 ev.press == Clock::PressType::Long ? "long" : "short");

        const Clock::UiResult r = Clock::handlePress(ui, ev.button, ev.press);
        if (r.modeChanged) {
            ESP_LOGI(TAG, "[mode] %s -> %s", Clock::modeName(r.previousMode), Clock::modeName(ui.mode));
        }
        if (r.command == Clock::Command::IncrementField) {
            const uint8_t modulus = (ui.field == 0) ? 24 : 60;
            draft.field[ui.field] = Clock::wrapIncrement(draft.field[ui.field], modulus);
            ESP_LOGI(TAG, "[field] %u -> %02u", ui.field, draft.field[ui.field]);
        } else if (r.command != Clock::Command::None) {
            ESP_LOGI(TAG, "[cmd] %d", static_cast<int>(r.command));
        }
        draw(ui, draft);
    }
}
