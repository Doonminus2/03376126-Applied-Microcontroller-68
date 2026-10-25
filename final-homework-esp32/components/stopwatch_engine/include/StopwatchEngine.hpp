#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "clock_logic/stopwatch_core.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Clock {

struct StopwatchSnapshot {
    bool running = false;
    uint64_t elapsedMs = 0;
    size_t lapCount = 0;
    LapRecord laps[STOPWATCH_MAX_LAPS];
};

// มิลลิวินาที -> "MM:SS.cc" (นาทีเกิน 99 จะยาวขึ้นแต่ไม่ตัดทิ้ง)
inline void formatStopwatch(uint64_t ms, char* out, size_t len) {
    snprintf(out, len, "%02llu:%02llu.%02llu", static_cast<unsigned long long>(ms / 60000),
             static_cast<unsigned long long>(ms / 1000 % 60), static_cast<unsigned long long>(ms / 10 % 100));
}

// StopwatchCore แบบ thread-safe ที่อ่านเวลาจริงจาก esp_timer_get_time() (ความละเอียด 1 us)
class StopwatchEngine {
public:
    StopwatchEngine();
    StopwatchEngine(const StopwatchEngine&) = delete;
    StopwatchEngine& operator=(const StopwatchEngine&) = delete;

    void toggle();
    void lapOrReset();  // กำลังเดิน = บันทึก lap, หยุดอยู่ = รีเซ็ต
    void reset();
    StopwatchSnapshot snapshot() const;

private:
    SemaphoreHandle_t mutex_;
    StopwatchCore core_;
};

}  // namespace Clock
