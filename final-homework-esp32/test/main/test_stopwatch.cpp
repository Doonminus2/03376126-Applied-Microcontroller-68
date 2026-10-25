#include "clock_logic/stopwatch_core.hpp"
#include "tests.hpp"

using namespace Clock;

static void expectLap(const StopwatchCore& sw, size_t i, uint64_t totalMs, uint64_t splitMs) {
    TEST_ASSERT_EQUAL_UINT64_MESSAGE(totalMs, sw.lapAt(i).totalMs, "lap total");
    TEST_ASSERT_EQUAL_UINT64_MESSAGE(splitMs, sw.lapAt(i).splitMs, "lap split");
}

void test_stopwatch_accumulates_across_pauses() {
    StopwatchCore sw;
    TEST_ASSERT_EQUAL_UINT64(0, sw.elapsedMs(123456789));
    sw.start(1000000);
    TEST_ASSERT_TRUE(sw.running());
    TEST_ASSERT_EQUAL_UINT64(500, sw.elapsedMs(1500000));
    sw.stop(2250000);
    TEST_ASSERT_FALSE(sw.running());
    TEST_ASSERT_EQUAL_UINT64(1250, sw.elapsedMs(2250000));
    TEST_ASSERT_EQUAL_UINT64(1250, sw.elapsedMs(9999999));
    sw.start(5000000);
    TEST_ASSERT_EQUAL_UINT64(2000, sw.elapsedMs(5750500));
    sw.stop(6000999);
    TEST_ASSERT_EQUAL_UINT64(2250, sw.elapsedMs(7000000));
}

void test_stopwatch_repeated_start_stop_ignored() {
    StopwatchCore sw;
    sw.start(1000000);
    sw.start(2000000);
    TEST_ASSERT_EQUAL_UINT64(2000, sw.elapsedMs(3000000));
    sw.stop(4000000);
    sw.stop(9000000);
    TEST_ASSERT_EQUAL_UINT64(3000, sw.elapsedMs(9000000));
}

void test_stopwatch_lap_total_and_split() {
    StopwatchCore sw;
    sw.start(10000000);
    TEST_ASSERT_TRUE(sw.lap(11234000));
    TEST_ASSERT_TRUE(sw.lap(13000000));
    sw.stop(14000000);
    sw.start(20000000);
    TEST_ASSERT_TRUE(sw.lap(20500000));
    TEST_ASSERT_EQUAL_size_t(3, sw.lapCount());
    expectLap(sw, 0, 1234, 1234);
    expectLap(sw, 1, 3000, 1766);
    expectLap(sw, 2, 4500, 1500);
}

void test_stopwatch_lap_rejected_when_stopped() {
    StopwatchCore sw;
    TEST_ASSERT_FALSE(sw.lap(1000000));
    TEST_ASSERT_EQUAL_size_t(0, sw.lapCount());
    sw.start(1000000);
    sw.stop(3000000);
    TEST_ASSERT_FALSE(sw.lap(4000000));
    TEST_ASSERT_EQUAL_size_t(0, sw.lapCount());
    TEST_ASSERT_EQUAL_UINT64(2000, sw.elapsedMs(4000000));
}

void test_stopwatch_lap_capacity_enforced() {
    StopwatchCore sw;
    sw.start(0);
    for (size_t i = 1; i <= STOPWATCH_MAX_LAPS; ++i) {
        TEST_ASSERT_TRUE(sw.lap(i * 1000000));
    }
    TEST_ASSERT_FALSE(sw.lap(11000000));
    TEST_ASSERT_FALSE(sw.lap(12000000));
    TEST_ASSERT_EQUAL_size_t(10, STOPWATCH_MAX_LAPS);
    TEST_ASSERT_EQUAL_size_t(10, sw.lapCount());
    expectLap(sw, 9, 10000, 1000);
}

void test_stopwatch_reset_clears_everything() {
    StopwatchCore sw;
    sw.start(0);
    TEST_ASSERT_TRUE(sw.lap(2000000));
    TEST_ASSERT_TRUE(sw.lap(5000000));
    sw.reset();
    TEST_ASSERT_FALSE(sw.running());
    TEST_ASSERT_EQUAL_size_t(0, sw.lapCount());
    TEST_ASSERT_EQUAL_UINT64(0, sw.elapsedMs(7000000));
    sw.start(10000000);
    TEST_ASSERT_TRUE(sw.lap(10700000));
    TEST_ASSERT_EQUAL_size_t(1, sw.lapCount());
    expectLap(sw, 0, 700, 700);
}
