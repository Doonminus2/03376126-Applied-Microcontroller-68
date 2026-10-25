#pragma once
#include <cstdint>

#include "clock_logic/alarm_logic.hpp"
#include "clock_logic/datetime.hpp"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Clock {

// นาฬิกาปลุก + บัซเซอร์ (LEDC PWM) ที่ thread-safe
//   check()    เรียกทุกวินาทีจาก task ปลุก ตัดสินด้วย AlarmState แล้วเปิด/ปิดเสียง
//   silence()  / toggleEnabled() / setSetting() เรียกจาก task UI ได้
// เสียงปลุกดัง 200 ms / เงียบ 200 ms ขับด้วย esp_timer แบบ periodic ไม่กินเวลา task ที่เรียก
class AlarmEngine {
public:
    explicit AlarmEngine(gpio_num_t buzzerPin = GPIO_NUM_25, uint32_t ringMs = 30000);
    AlarmEngine(const AlarmEngine&) = delete;
    AlarmEngine& operator=(const AlarmEngine&) = delete;

    bool init();

    AlarmSetting setting() const;
    void setSetting(const AlarmSetting& s);
    bool toggleEnabled();  // คืนสถานะใหม่ ถ้าปิดขณะกำลังดังจะหยุดเสียงทันที

    AlarmEvent check(const DateTime& localNow, uint32_t nowMs);
    bool silence();  // คืน true ถ้าหยุดเสียงที่กำลังดังอยู่จริง
    bool ringing() const;

    void beep(uint32_t freqHz, uint32_t durationMs);

private:
    enum class StopReason : uint8_t { Timeout, Silenced, Disabled };

    static void onRingTick(void* arg);
    static void onBeepEnd(void* arg);

    // เรียกตอนถือ mutex_ แล้วเท่านั้น
    void startRingLocked();
    void stopRingLocked();
    void outputLocked(bool on);
    bool applySettingLocked(const AlarmSetting& s);  // คืน true ถ้าการปิดปลุกทำให้เสียงที่ดังอยู่หยุด

    static void logStop(StopReason reason);

    gpio_num_t pin_;
    SemaphoreHandle_t mutex_;
    esp_timer_handle_t ringTimer_ = nullptr;
    esp_timer_handle_t beepTimer_ = nullptr;
    bool ready_ = false;
    bool toneOn_ = false;
    AlarmSetting setting_;
    AlarmState state_;
};

}  // namespace Clock
