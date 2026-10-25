#pragma once
#include <cstdint>

namespace Clock {

struct AlarmSetting {
    uint8_t hour = 7;
    uint8_t minute = 0;
    bool enabled = false;
};

// เวลาปัจจุบัน (เวลาท้องถิ่น) ตรงกับเวลาปลุกหรือไม่ เทียบแค่ชั่วโมงและนาที
constexpr bool alarmMatches(const AlarmSetting& a, uint8_t hour, uint8_t minute) {
    return a.enabled && a.hour == hour && a.minute == minute;
}

enum class AlarmEvent : uint8_t { None, Start, Stop };

// ตัวตัดสินใจปลุก: เรียก update() ทุกวินาที
//  - Start  เมื่อเวลาตรงและยังไม่ได้ปลุกในนาทีนั้น (ปลุกครั้งเดียวต่อนาที)
//  - Stop   เมื่อเสียงดังครบ ringMs หรือผู้ใช้สั่งหยุด/ปิดปลุก
class AlarmState {
public:
    explicit AlarmState(uint32_t ringMs = 30000) : ringMs_(ringMs) {}

    AlarmEvent update(const AlarmSetting& a, uint8_t hour, uint8_t minute, uint32_t nowMs) {
        if (ringing_) {
            if (!a.enabled || nowMs - startedMs_ >= ringMs_) {
                ringing_ = false;
                return AlarmEvent::Stop;
            }
            return AlarmEvent::None;
        }
        if (!alarmMatches(a, hour, minute)) {
            armedForMatch_ = true;  // ออกจากนาทีที่ตรงแล้ว พร้อมปลุกรอบหน้า
            return AlarmEvent::None;
        }
        if (!armedForMatch_) return AlarmEvent::None;  // ปลุกในนาทีนี้ไปแล้ว
        armedForMatch_ = false;
        ringing_ = true;
        startedMs_ = nowMs;
        return AlarmEvent::Start;
    }

    // ผู้ใช้กดหยุดเสียง
    AlarmEvent silence() {
        if (!ringing_) return AlarmEvent::None;
        ringing_ = false;
        return AlarmEvent::Stop;
    }

    bool ringing() const { return ringing_; }

private:
    uint32_t ringMs_;
    uint32_t startedMs_ = 0;
    bool ringing_ = false;
    bool armedForMatch_ = true;
};

}  // namespace Clock
