#pragma once
#include <cstdint>

namespace Clock {

enum class PressType : uint8_t { None, Short, Long };

// State machine ของปุ่มหนึ่งปุ่ม: debounce + แยก short/long press ไม่ผูกฮาร์ดแวร์
//   edge()  รับระดับสัญญาณดิบพร้อมเวลา (เรียกจาก task ที่รับ event จาก ISR)
//   poll()  ตัดสินว่าสัญญาณนิ่งพอหรือยัง และคืน event ได้ครั้งละหนึ่งอัน
// ปุ่มต่อแบบ pull-up: pressed = true คือขาเป็น LOW
class ButtonFsm {
public:
    explicit ButtonFsm(uint32_t debounceMs = 30, uint32_t longPressMs = 1000)
        : debounceMs_(debounceMs), longPressMs_(longPressMs) {}

    void edge(bool pressed, uint32_t nowMs) {
        if (pressed == raw_) return;
        raw_ = pressed;
        rawChangedMs_ = nowMs;
    }

    PressType poll(uint32_t nowMs) {
        // ระดับดิบต้องนิ่งอย่างน้อย debounceMs_ จึงจะถือเป็นระดับจริง
        if (raw_ != stable_ && nowMs - rawChangedMs_ >= debounceMs_) {
            stable_ = raw_;
            if (stable_) {
                pressedAtMs_ = rawChangedMs_;
                longReported_ = false;
            } else if (!longReported_) {
                return PressType::Short;
            }
            return PressType::None;
        }
        if (stable_ && !longReported_ && nowMs - pressedAtMs_ >= longPressMs_) {
            longReported_ = true;
            return PressType::Long;
        }
        return PressType::None;
    }

    // true เมื่อไม่มีอะไรต้องรอแล้ว task บล็อกรอ event จาก ISR ได้เลย
    bool idle() const { return raw_ == stable_ && (!stable_ || longReported_); }

    bool pressed() const { return stable_; }

private:
    uint32_t debounceMs_;
    uint32_t longPressMs_;
    bool raw_ = false;
    bool stable_ = false;
    bool longReported_ = false;
    uint32_t rawChangedMs_ = 0;
    uint32_t pressedAtMs_ = 0;
};

}  // namespace Clock
