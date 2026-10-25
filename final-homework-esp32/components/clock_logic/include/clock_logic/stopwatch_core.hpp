#pragma once
#include <cstddef>
#include <cstdint>

namespace Clock {

constexpr size_t STOPWATCH_MAX_LAPS = 10;

struct LapRecord {
    uint64_t totalMs = 0;  // เวลารวม ณ ตอนกด lap
    uint64_t splitMs = 0;  // เวลาของรอบนี้ (ห่างจาก lap ก่อนหน้า)
};

// สโตปวอทช์แบบไม่ผูกฮาร์ดแวร์ รับเวลาปัจจุบัน (ไมโครวินาที) จากภายนอก
class StopwatchCore {
public:
    void start(uint64_t nowUs) {
        if (running_) return;
        startedAtUs_ = nowUs;
        running_ = true;
    }

    void stop(uint64_t nowUs) {
        if (!running_) return;
        accumulatedUs_ += nowUs - startedAtUs_;
        running_ = false;
    }

    void reset() {
        running_ = false;
        accumulatedUs_ = 0;
        startedAtUs_ = 0;
        lapCount_ = 0;
        lastLapTotalMs_ = 0;
    }

    // บันทึก lap คืน false ถ้าไม่ได้เดินอยู่หรือ lap เต็ม
    bool lap(uint64_t nowUs) {
        if (!running_ || lapCount_ >= STOPWATCH_MAX_LAPS) return false;
        const uint64_t total = elapsedMs(nowUs);
        laps_[lapCount_].totalMs = total;
        laps_[lapCount_].splitMs = total - lastLapTotalMs_;
        lastLapTotalMs_ = total;
        ++lapCount_;
        return true;
    }

    uint64_t elapsedMs(uint64_t nowUs) const {
        const uint64_t us = accumulatedUs_ + (running_ ? nowUs - startedAtUs_ : 0);
        return us / 1000;
    }

    bool running() const { return running_; }
    size_t lapCount() const { return lapCount_; }
    const LapRecord& lapAt(size_t i) const { return laps_[i]; }

private:
    bool running_ = false;
    uint64_t startedAtUs_ = 0;
    uint64_t accumulatedUs_ = 0;
    uint64_t lastLapTotalMs_ = 0;
    size_t lapCount_ = 0;
    LapRecord laps_[STOPWATCH_MAX_LAPS];
};

}  // namespace Clock
