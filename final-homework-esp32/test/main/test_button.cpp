#include <cstddef>

#include "clock_logic/button_fsm.hpp"
#include "tests.hpp"

using namespace Clock;

struct Edge {
    uint32_t atMs;
    bool pressed;
};

struct Event {
    uint32_t atMs;
    PressType type;
};

struct Timeline {
    static constexpr size_t MAX_EVENTS = 8;
    Event events[MAX_EVENTS];
    size_t count = 0;
};

// จำลองการใช้งานจริง: ป้อน edge ตามเวลา แล้ว poll ทุก 1 ms จนถึง endMs เก็บทุก event ที่ไม่ใช่ None
static Timeline simulate(const Edge* edges, size_t edgeCount, uint32_t endMs) {
    ButtonFsm fsm(30, 1000);
    Timeline tl;
    size_t next = 0;
    for (uint32_t t = 0; t <= endMs; ++t) {
        while (next < edgeCount && edges[next].atMs == t) {
            fsm.edge(edges[next].pressed, t);
            ++next;
        }
        const PressType p = fsm.poll(t);
        if (p != PressType::None) {
            TEST_ASSERT_LESS_THAN_size_t(Timeline::MAX_EVENTS, tl.count);
            tl.events[tl.count++] = {t, p};
        }
    }
    return tl;
}

static void expectEvent(const Timeline& tl, size_t i, uint32_t atMs, PressType type) {
    TEST_ASSERT_GREATER_THAN_size_t(i, tl.count);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(atMs, tl.events[i].atMs, "event time");
    ASSERT_ENUM_EQ_MSG(type, tl.events[i].type, "event type");
}

void test_button_clean_short_press() {
    const Edge edges[] = {{100, true}, {300, false}};
    const Timeline tl = simulate(edges, 2, 2000);
    TEST_ASSERT_EQUAL_size_t(1, tl.count);
    expectEvent(tl, 0, 330, PressType::Short);
}

void test_button_bounce_shorter_than_debounce_no_event() {
    const Edge edges[] = {{100, true}, {110, false}, {115, true}, {120, false}, {500, true}, {529, false}};
    const Timeline tl = simulate(edges, 6, 3000);
    TEST_ASSERT_EQUAL_size_t(0, tl.count);
}

void test_button_bounce_then_stable_press() {
    const Edge edges[] = {{100, true}, {105, false}, {110, true}, {400, false}, {405, true}, {410, false}};
    const Timeline tl = simulate(edges, 6, 2000);
    TEST_ASSERT_EQUAL_size_t(1, tl.count);
    expectEvent(tl, 0, 440, PressType::Short);

    ButtonFsm fsm(30, 1000);
    fsm.edge(true, 100);
    fsm.edge(false, 105);
    fsm.edge(true, 110);
    ASSERT_ENUM_EQ(PressType::None, fsm.poll(139));
    TEST_ASSERT_FALSE(fsm.pressed());
    ASSERT_ENUM_EQ(PressType::None, fsm.poll(140));
    TEST_ASSERT_TRUE(fsm.pressed());
}

void test_button_long_press_fires_once() {
    const Edge edges[] = {{100, true}, {3000, false}};
    const Timeline tl = simulate(edges, 2, 5000);
    TEST_ASSERT_EQUAL_size_t(1, tl.count);
    expectEvent(tl, 0, 1100, PressType::Long);
}

void test_button_release_before_long_is_short() {
    const Edge edges[] = {{100, true}, {900, false}};
    const Timeline tl = simulate(edges, 2, 3000);
    TEST_ASSERT_EQUAL_size_t(1, tl.count);
    expectEvent(tl, 0, 930, PressType::Short);
}

void test_button_idle_semantics() {
    ButtonFsm fsm(30, 1000);
    TEST_ASSERT_TRUE(fsm.idle());
    fsm.edge(true, 100);
    TEST_ASSERT_FALSE(fsm.idle());
    ASSERT_ENUM_EQ(PressType::None, fsm.poll(130));
    TEST_ASSERT_TRUE(fsm.pressed());
    TEST_ASSERT_FALSE(fsm.idle());
    ASSERT_ENUM_EQ(PressType::Long, fsm.poll(1100));
    TEST_ASSERT_TRUE(fsm.idle());
    fsm.edge(false, 1500);
    TEST_ASSERT_FALSE(fsm.idle());
    ASSERT_ENUM_EQ(PressType::None, fsm.poll(1530));
    TEST_ASSERT_FALSE(fsm.pressed());
    TEST_ASSERT_TRUE(fsm.idle());
    fsm.edge(false, 1600);
    TEST_ASSERT_TRUE(fsm.idle());
}

void test_button_two_presses_in_a_row() {
    const Edge edges[] = {{100, true}, {200, false}, {400, true}, {500, false}};
    const Timeline tl = simulate(edges, 4, 2000);
    TEST_ASSERT_EQUAL_size_t(2, tl.count);
    expectEvent(tl, 0, 230, PressType::Short);
    expectEvent(tl, 1, 530, PressType::Short);
}
