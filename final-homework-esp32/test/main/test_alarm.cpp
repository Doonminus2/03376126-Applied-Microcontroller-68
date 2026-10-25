#include "clock_logic/alarm_logic.hpp"
#include "tests.hpp"

using namespace Clock;

static AlarmSetting alarmAt(uint8_t hour, uint8_t minute, bool enabled = true) {
    AlarmSetting a;
    a.hour = hour;
    a.minute = minute;
    a.enabled = enabled;
    return a;
}

void test_alarm_matches_enabled_exact_minute() {
    const AlarmSetting a = alarmAt(7, 30);
    TEST_ASSERT_TRUE(alarmMatches(a, 7, 30));
    TEST_ASSERT_FALSE(alarmMatches(a, 7, 31));
    TEST_ASSERT_FALSE(alarmMatches(a, 7, 29));
    TEST_ASSERT_FALSE(alarmMatches(a, 8, 30));
    TEST_ASSERT_FALSE(alarmMatches(a, 19, 30));
}

void test_alarm_matches_disabled_never() {
    const AlarmSetting a = alarmAt(7, 30, false);
    TEST_ASSERT_FALSE(alarmMatches(a, 7, 30));
    TEST_ASSERT_FALSE(alarmMatches(a, 7, 31));
}

void test_alarm_state_starts_once_in_matching_minute() {
    const AlarmSetting a = alarmAt(7, 30);
    AlarmState s(30000);
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 29, 0));
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 7, 30, 1000));
    TEST_ASSERT_TRUE(s.ringing());
    int starts = 0;
    for (uint32_t t = 2000; t < 31000; t += 1000) {
        if (s.update(a, 7, 30, t) == AlarmEvent::Start) ++starts;
    }
    TEST_ASSERT_EQUAL_INT(0, starts);
    TEST_ASSERT_TRUE(s.ringing());
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.update(a, 7, 30, 31000));
    for (uint32_t t = 32000; t < 60000; t += 1000) {
        ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 30, t));
    }
    TEST_ASSERT_FALSE(s.ringing());
}

void test_alarm_state_silence_no_retrigger_same_minute() {
    const AlarmSetting a = alarmAt(7, 30);
    AlarmState s(30000);
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 7, 30, 1000));
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.silence());
    TEST_ASSERT_FALSE(s.ringing());
    ASSERT_ENUM_EQ(AlarmEvent::None, s.silence());
    for (uint32_t t = 2000; t < 60000; t += 1000) {
        ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 30, t));
    }
    TEST_ASSERT_FALSE(s.ringing());
}

void test_alarm_state_stops_after_ring_timeout() {
    const AlarmSetting a = alarmAt(6, 0);
    AlarmState s(5000);
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 6, 0, 10000));
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 6, 0, 14999));
    TEST_ASSERT_TRUE(s.ringing());
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.update(a, 6, 0, 15000));
    TEST_ASSERT_FALSE(s.ringing());
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 6, 0, 16000));
}

void test_alarm_state_stops_when_disabled_while_ringing() {
    AlarmSetting a = alarmAt(7, 30);
    AlarmState s(30000);
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 7, 30, 1000));
    a.enabled = false;
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.update(a, 7, 30, 2000));
    TEST_ASSERT_FALSE(s.ringing());
    a.enabled = true;
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 30, 3000));
    TEST_ASSERT_FALSE(s.ringing());
}

void test_alarm_state_disabled_never_starts() {
    const AlarmSetting a = alarmAt(7, 30, false);
    AlarmState s(30000);
    for (uint32_t t = 0; t < 60000; t += 1000) {
        ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 30, t));
    }
    TEST_ASSERT_FALSE(s.ringing());
}

void test_alarm_state_rearms_after_minute_passes() {
    const AlarmSetting a = alarmAt(7, 30);
    AlarmState s(30000);
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 7, 30, 0));
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.silence());
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 30, 59000));
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 31, 60000));
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 7, 30, 86400000));
    TEST_ASSERT_TRUE(s.ringing());
}

void test_alarm_state_midnight_boundary() {
    const AlarmSetting midnight = alarmAt(0, 0);
    AlarmState s(30000);
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(midnight, 23, 59, 0));
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(midnight, 0, 0, 1000));
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.silence());
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(midnight, 0, 0, 2000));
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(midnight, 0, 1, 61000));
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(midnight, 23, 59, 86340000));
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(midnight, 0, 0, 86401000));

    const AlarmSetting lastMinute = alarmAt(23, 59);
    AlarmState s2(30000);
    ASSERT_ENUM_EQ(AlarmEvent::Start, s2.update(lastMinute, 23, 59, 0));
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s2.update(lastMinute, 23, 59, 30000));
    ASSERT_ENUM_EQ(AlarmEvent::None, s2.update(lastMinute, 0, 0, 60000));
    ASSERT_ENUM_EQ(AlarmEvent::None, s2.update(lastMinute, 0, 0, 61000));
}

void test_alarm_state_timeout_survives_ms_wraparound() {
    const AlarmSetting a = alarmAt(7, 30);
    AlarmState s(30000);
    const uint32_t start = 0xFFFFF000u;
    ASSERT_ENUM_EQ(AlarmEvent::Start, s.update(a, 7, 30, start));
    ASSERT_ENUM_EQ(AlarmEvent::None, s.update(a, 7, 30, start + 29999u));
    TEST_ASSERT_TRUE(s.ringing());
    ASSERT_ENUM_EQ(AlarmEvent::Stop, s.update(a, 7, 30, start + 30000u));
}
