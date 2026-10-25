#include <cstdio>

#include "clock_logic/ui_mode.hpp"
#include "tests.hpp"

using namespace Clock;

static constexpr Mode ALL_MODES[] = {Mode::Clock, Mode::SetTime, Mode::Alarm, Mode::Stopwatch};

void test_ui_next_mode_cycle_order() {
    ASSERT_ENUM_EQ(Mode::SetTime, nextMode(Mode::Clock));
    ASSERT_ENUM_EQ(Mode::Alarm, nextMode(Mode::SetTime));
    ASSERT_ENUM_EQ(Mode::Stopwatch, nextMode(Mode::Alarm));
    ASSERT_ENUM_EQ(Mode::Clock, nextMode(Mode::Stopwatch));
}

void test_ui_mode_names() {
    TEST_ASSERT_EQUAL_STRING("CLOCK", modeName(Mode::Clock));
    TEST_ASSERT_EQUAL_STRING("SET TIME", modeName(Mode::SetTime));
    TEST_ASSERT_EQUAL_STRING("ALARM", modeName(Mode::Alarm));
    TEST_ASSERT_EQUAL_STRING("STOPWATCH", modeName(Mode::Stopwatch));
}

void test_ui_field_counts() {
    TEST_ASSERT_EQUAL_UINT8(0, fieldCount(Mode::Clock));
    TEST_ASSERT_EQUAL_UINT8(3, fieldCount(Mode::SetTime));
    TEST_ASSERT_EQUAL_UINT8(2, fieldCount(Mode::Alarm));
    TEST_ASSERT_EQUAL_UINT8(0, fieldCount(Mode::Stopwatch));
}

// ตารางคาดหวังเขียนจากคอมเมนต์ "ตารางหน้าที่ปุ่ม" ใน ui_mode.hpp ครบทุกคู่ (โหมด, ปุ่ม, ชนิดการกด) = 48 แถว
// เริ่มทุกแถวจาก field = 0
void test_ui_handle_press_full_table() {
    using B = Button;
    using P = PressType;
    using M = Mode;
    using C = Command;
    struct Row {
        M mode;
        B button;
        P press;
        C command;
        M modeAfter;
        uint8_t fieldAfter;
    };
    const Row rows[] = {
        {M::Clock, B::Mode, P::None, C::None, M::Clock, 0},
        {M::Clock, B::Mode, P::Short, C::None, M::SetTime, 0},
        {M::Clock, B::Mode, P::Long, C::None, M::Clock, 0},
        {M::Clock, B::Set, P::None, C::None, M::Clock, 0},
        {M::Clock, B::Set, P::Short, C::None, M::Clock, 0},
        {M::Clock, B::Set, P::Long, C::None, M::Clock, 0},
        {M::Clock, B::Alarm, P::None, C::None, M::Clock, 0},
        {M::Clock, B::Alarm, P::Short, C::ToggleAlarm, M::Clock, 0},
        {M::Clock, B::Alarm, P::Long, C::None, M::Clock, 0},
        {M::Clock, B::Lap, P::None, C::None, M::Clock, 0},
        {M::Clock, B::Lap, P::Short, C::None, M::Clock, 0},
        {M::Clock, B::Lap, P::Long, C::None, M::Clock, 0},

        {M::SetTime, B::Mode, P::None, C::None, M::SetTime, 0},
        {M::SetTime, B::Mode, P::Short, C::None, M::Alarm, 0},
        {M::SetTime, B::Mode, P::Long, C::None, M::Clock, 0},
        {M::SetTime, B::Set, P::None, C::None, M::SetTime, 0},
        {M::SetTime, B::Set, P::Short, C::IncrementField, M::SetTime, 0},
        {M::SetTime, B::Set, P::Long, C::None, M::SetTime, 1},
        {M::SetTime, B::Alarm, P::None, C::None, M::SetTime, 0},
        {M::SetTime, B::Alarm, P::Short, C::ToggleAlarm, M::SetTime, 0},
        {M::SetTime, B::Alarm, P::Long, C::None, M::SetTime, 0},
        {M::SetTime, B::Lap, P::None, C::None, M::SetTime, 0},
        {M::SetTime, B::Lap, P::Short, C::None, M::SetTime, 0},
        {M::SetTime, B::Lap, P::Long, C::None, M::SetTime, 0},

        {M::Alarm, B::Mode, P::None, C::None, M::Alarm, 0},
        {M::Alarm, B::Mode, P::Short, C::None, M::Stopwatch, 0},
        {M::Alarm, B::Mode, P::Long, C::None, M::Clock, 0},
        {M::Alarm, B::Set, P::None, C::None, M::Alarm, 0},
        {M::Alarm, B::Set, P::Short, C::IncrementField, M::Alarm, 0},
        {M::Alarm, B::Set, P::Long, C::None, M::Alarm, 1},
        {M::Alarm, B::Alarm, P::None, C::None, M::Alarm, 0},
        {M::Alarm, B::Alarm, P::Short, C::ToggleAlarm, M::Alarm, 0},
        {M::Alarm, B::Alarm, P::Long, C::None, M::Alarm, 0},
        {M::Alarm, B::Lap, P::None, C::None, M::Alarm, 0},
        {M::Alarm, B::Lap, P::Short, C::None, M::Alarm, 0},
        {M::Alarm, B::Lap, P::Long, C::None, M::Alarm, 0},

        {M::Stopwatch, B::Mode, P::None, C::None, M::Stopwatch, 0},
        {M::Stopwatch, B::Mode, P::Short, C::None, M::Clock, 0},
        {M::Stopwatch, B::Mode, P::Long, C::None, M::Clock, 0},
        {M::Stopwatch, B::Set, P::None, C::None, M::Stopwatch, 0},
        {M::Stopwatch, B::Set, P::Short, C::StopwatchToggle, M::Stopwatch, 0},
        {M::Stopwatch, B::Set, P::Long, C::None, M::Stopwatch, 0},
        {M::Stopwatch, B::Alarm, P::None, C::None, M::Stopwatch, 0},
        {M::Stopwatch, B::Alarm, P::Short, C::ToggleAlarm, M::Stopwatch, 0},
        {M::Stopwatch, B::Alarm, P::Long, C::None, M::Stopwatch, 0},
        {M::Stopwatch, B::Lap, P::None, C::None, M::Stopwatch, 0},
        {M::Stopwatch, B::Lap, P::Short, C::StopwatchLapOrReset, M::Stopwatch, 0},
        {M::Stopwatch, B::Lap, P::Long, C::StopwatchReset, M::Stopwatch, 0},
    };
    TEST_ASSERT_EQUAL_size_t(48, sizeof rows / sizeof rows[0]);
    for (size_t i = 0; i < sizeof rows / sizeof rows[0]; ++i) {
        const Row& row = rows[i];
        char where[48];
        snprintf(where, sizeof where, "row %u (mode %s)", static_cast<unsigned>(i), modeName(row.mode));
        UiState ui;
        ui.mode = row.mode;
        const UiResult r = handlePress(ui, row.button, row.press);
        ASSERT_ENUM_EQ_MSG(row.command, r.command, where);
        ASSERT_ENUM_EQ_MSG(row.modeAfter, ui.mode, where);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(row.fieldAfter, ui.field, where);
        TEST_ASSERT_EQUAL_MESSAGE(row.modeAfter != row.mode, r.modeChanged, where);
        ASSERT_ENUM_EQ_MSG(row.mode, r.previousMode, where);
    }
}

void test_ui_mode_short_resets_field() {
    for (Mode m : ALL_MODES) {
        UiState ui;
        ui.mode = m;
        ui.field = 2;
        const UiResult r = handlePress(ui, Button::Mode, PressType::Short);
        TEST_ASSERT_TRUE(r.modeChanged);
        ASSERT_ENUM_EQ(m, r.previousMode);
        ASSERT_ENUM_EQ(nextMode(m), ui.mode);
        TEST_ASSERT_EQUAL_UINT8(0, ui.field);
    }
}

void test_ui_mode_long_returns_to_clock() {
    UiState ui;
    ui.mode = Mode::SetTime;
    ui.field = 2;
    UiResult r = handlePress(ui, Button::Mode, PressType::Long);
    TEST_ASSERT_TRUE(r.modeChanged);
    ASSERT_ENUM_EQ(Mode::SetTime, r.previousMode);
    ASSERT_ENUM_EQ(Mode::Clock, ui.mode);
    TEST_ASSERT_EQUAL_UINT8(0, ui.field);

    r = handlePress(ui, Button::Mode, PressType::Long);
    TEST_ASSERT_FALSE(r.modeChanged);
    ASSERT_ENUM_EQ(Mode::Clock, ui.mode);
}

void test_ui_set_long_wraps_field() {
    UiState ui;
    ui.mode = Mode::SetTime;
    const uint8_t setTimeFields[] = {1, 2, 0, 1};
    for (uint8_t expected : setTimeFields) {
        const UiResult r = handlePress(ui, Button::Set, PressType::Long);
        ASSERT_ENUM_EQ(Command::None, r.command);
        TEST_ASSERT_EQUAL_UINT8(expected, ui.field);
    }
    const UiResult inc = handlePress(ui, Button::Set, PressType::Short);
    ASSERT_ENUM_EQ(Command::IncrementField, inc.command);
    TEST_ASSERT_EQUAL_UINT8(1, ui.field);

    ui.mode = Mode::Alarm;
    ui.field = 0;
    const uint8_t alarmFields[] = {1, 0, 1};
    for (uint8_t expected : alarmFields) {
        handlePress(ui, Button::Set, PressType::Long);
        TEST_ASSERT_EQUAL_UINT8(expected, ui.field);
    }
}

void test_ui_wrap_increment() {
    TEST_ASSERT_EQUAL_UINT8(0, wrapIncrement(59, 60));
    TEST_ASSERT_EQUAL_UINT8(0, wrapIncrement(23, 24));
    TEST_ASSERT_EQUAL_UINT8(59, wrapIncrement(58, 60));
    TEST_ASSERT_EQUAL_UINT8(1, wrapIncrement(0, 24));
    TEST_ASSERT_EQUAL_UINT8(23, wrapIncrement(22, 24));
}
