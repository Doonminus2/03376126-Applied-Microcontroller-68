#include <cstdio>

#include "tests.hpp"

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int runAllTests() {
    UNITY_BEGIN();
#define CLOCK_LOGIC_RUN_TEST(fn) RUN_TEST(fn);
    CLOCK_LOGIC_TESTS(CLOCK_LOGIC_RUN_TEST)
#undef CLOCK_LOGIC_RUN_TEST
    const int failures = UNITY_END();
    printf("\n==== final_test: %s: %u tests, %d failures ====\n", failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED",
           static_cast<unsigned>(Unity.NumberOfTests), failures);
    fflush(stdout);
    return failures;
}
