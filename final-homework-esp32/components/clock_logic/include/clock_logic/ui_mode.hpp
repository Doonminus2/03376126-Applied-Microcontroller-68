#pragma once
#include <cstdint>

#include "clock_logic/button_fsm.hpp"

namespace Clock {

enum class Mode : uint8_t { Clock, SetTime, Alarm, Stopwatch };
constexpr uint8_t MODE_COUNT = 4;

enum class Button : uint8_t { Mode, Set, Alarm, Lap };

// ลำดับโหมด: Clock -> SetTime -> Alarm -> Stopwatch -> Clock
constexpr Mode nextMode(Mode m) { return static_cast<Mode>((static_cast<uint8_t>(m) + 1) % MODE_COUNT); }

constexpr const char* modeName(Mode m) {
    switch (m) {
        case Mode::Clock: return "CLOCK";
        case Mode::SetTime: return "SET TIME";
        case Mode::Alarm: return "ALARM";
        case Mode::Stopwatch: return "STOPWATCH";
    }
    return "?";
}

// จำนวนฟิลด์ที่แก้ไขได้ในแต่ละโหมด (SetTime: ชม./นาที/วินาที, Alarm: ชม./นาที)
constexpr uint8_t fieldCount(Mode m) {
    switch (m) {
        case Mode::SetTime: return 3;
        case Mode::Alarm: return 2;
        default: return 0;
    }
}

enum class Command : uint8_t {
    None,
    IncrementField,       // เพิ่มค่าฟิลด์ที่เลือกอยู่
    ToggleAlarm,          // เปิด/ปิดปลุก
    StopwatchToggle,      // เริ่ม/หยุดสโตปวอทช์
    StopwatchLapOrReset,  // กำลังเดิน = บันทึก lap, หยุดอยู่ = รีเซ็ต
    StopwatchReset,       // รีเซ็ตทันที
};

struct UiState {
    Mode mode = Mode::Clock;
    uint8_t field = 0;
};

struct UiResult {
    Command command = Command::None;
    bool modeChanged = false;
    Mode previousMode = Mode::Clock;  // ใช้ตัดสินว่าต้องบันทึกค่าที่แก้ (ออกจาก SetTime/Alarm)
};

// ตารางหน้าที่ปุ่ม
//   MODE  สั้น = โหมดถัดไป             ยาว = กลับหน้า Clock
//   SET   สั้น = เพิ่มค่า (SetTime/Alarm) หรือ เริ่ม/หยุด (Stopwatch)
//         ยาว = ไปฟิลด์ถัดไป (SetTime/Alarm)
//   ALARM สั้น = เปิด/ปิดปลุก (ทุกโหมด)
//   LAP   สั้น = lap/รีเซ็ต (Stopwatch)   ยาว = รีเซ็ต (Stopwatch)
inline UiResult handlePress(UiState& ui, Button button, PressType press) {
    UiResult r;
    r.previousMode = ui.mode;
    if (press == PressType::None) return r;

    switch (button) {
        case Button::Mode: {
            const Mode target = (press == PressType::Long) ? Mode::Clock : nextMode(ui.mode);
            if (target != ui.mode) {
                ui.mode = target;
                ui.field = 0;
                r.modeChanged = true;
            }
            break;
        }
        case Button::Set:
            if (ui.mode == Mode::SetTime || ui.mode == Mode::Alarm) {
                if (press == PressType::Short) {
                    r.command = Command::IncrementField;
                } else {
                    ui.field = static_cast<uint8_t>((ui.field + 1) % fieldCount(ui.mode));
                }
            } else if (ui.mode == Mode::Stopwatch && press == PressType::Short) {
                r.command = Command::StopwatchToggle;
            }
            break;
        case Button::Alarm:
            if (press == PressType::Short) r.command = Command::ToggleAlarm;
            break;
        case Button::Lap:
            if (ui.mode == Mode::Stopwatch) {
                r.command = (press == PressType::Short) ? Command::StopwatchLapOrReset : Command::StopwatchReset;
            }
            break;
    }
    return r;
}

// เพิ่มค่าแบบวนรอบ เช่น wrapIncrement(59, 60) = 0
constexpr uint8_t wrapIncrement(uint8_t value, uint8_t modulus) {
    return static_cast<uint8_t>((value + 1) % modulus);
}

}  // namespace Clock
