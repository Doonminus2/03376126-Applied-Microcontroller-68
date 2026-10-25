#pragma once
#include "unity.h"

// ทะเบียนเทสต์ชุดเดียว ใช้ร่วมกันทั้งบน ESP32 และบนเครื่อง host
#define CLOCK_LOGIC_TESTS(X)                              \
    X(test_bcd_to_dec_known_values)                       \
    X(test_bcd_from_dec_known_values)                     \
    X(test_bcd_round_trip_0_to_99)                        \
    X(test_bcd_valid_accepts_all_decimal_digits)          \
    X(test_bcd_valid_rejects_non_decimal_nibbles)         \
    X(test_ds1307_decode_known_registers)                 \
    X(test_ds1307_encode_known_datetime)                  \
    X(test_ds1307_encode_clears_ch_and_selects_24h)       \
    X(test_ds1307_decode_ignores_clock_halt_bit)          \
    X(test_ds1307_decode_12h_mode)                        \
    X(test_ds1307_decode_rejects_invalid_12h_hour)        \
    X(test_ds1307_decode_rejects_invalid_values)          \
    X(test_ds1307_encode_decode_round_trip)               \
    X(test_epoch_unix_origin)                             \
    X(test_epoch_y2k)                                     \
    X(test_epoch_known_datetime)                          \
    X(test_epoch_leap_day_2028)                           \
    X(test_epoch_leap_year_rules)                         \
    X(test_epoch_weekday_known_dates)                     \
    X(test_epoch_from_out_of_range_rejected)              \
    X(test_epoch_round_trip)                              \
    X(test_alarm_matches_enabled_exact_minute)            \
    X(test_alarm_matches_disabled_never)                  \
    X(test_alarm_state_starts_once_in_matching_minute)    \
    X(test_alarm_state_silence_no_retrigger_same_minute)  \
    X(test_alarm_state_stops_after_ring_timeout)          \
    X(test_alarm_state_stops_when_disabled_while_ringing) \
    X(test_alarm_state_disabled_never_starts)             \
    X(test_alarm_state_rearms_after_minute_passes)        \
    X(test_alarm_state_midnight_boundary)                 \
    X(test_alarm_state_timeout_survives_ms_wraparound)    \
    X(test_stopwatch_accumulates_across_pauses)           \
    X(test_stopwatch_repeated_start_stop_ignored)         \
    X(test_stopwatch_lap_total_and_split)                 \
    X(test_stopwatch_lap_rejected_when_stopped)           \
    X(test_stopwatch_lap_capacity_enforced)               \
    X(test_stopwatch_reset_clears_everything)             \
    X(test_button_clean_short_press)                      \
    X(test_button_bounce_shorter_than_debounce_no_event)  \
    X(test_button_bounce_then_stable_press)               \
    X(test_button_long_press_fires_once)                  \
    X(test_button_release_before_long_is_short)           \
    X(test_button_idle_semantics)                         \
    X(test_button_two_presses_in_a_row)                   \
    X(test_ui_next_mode_cycle_order)                      \
    X(test_ui_mode_names)                                 \
    X(test_ui_field_counts)                               \
    X(test_ui_handle_press_full_table)                    \
    X(test_ui_mode_short_resets_field)                    \
    X(test_ui_mode_long_returns_to_clock)                 \
    X(test_ui_set_long_wraps_field)                       \
    X(test_ui_wrap_increment)

#define CLOCK_LOGIC_DECLARE_TEST(fn) void fn();
CLOCK_LOGIC_TESTS(CLOCK_LOGIC_DECLARE_TEST)
#undef CLOCK_LOGIC_DECLARE_TEST

#define ASSERT_ENUM_EQ(expected, actual) TEST_ASSERT_EQUAL_INT(static_cast<int>(expected), static_cast<int>(actual))
#define ASSERT_ENUM_EQ_MSG(expected, actual, msg) \
    TEST_ASSERT_EQUAL_INT_MESSAGE(static_cast<int>(expected), static_cast<int>(actual), msg)

// รันเทสต์ทั้งหมด พิมพ์บรรทัดสรุป แล้วคืนจำนวนเทสต์ที่ fail
int runAllTests();
