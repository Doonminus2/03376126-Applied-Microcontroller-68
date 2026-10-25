#include <cstdio>

#include "clock_logic/bcd.hpp"
#include "tests.hpp"

using namespace Clock;

void test_bcd_to_dec_known_values() {
    TEST_ASSERT_EQUAL_UINT8(0, bcdToDec(0x00));
    TEST_ASSERT_EQUAL_UINT8(9, bcdToDec(0x09));
    TEST_ASSERT_EQUAL_UINT8(10, bcdToDec(0x10));
    TEST_ASSERT_EQUAL_UINT8(59, bcdToDec(0x59));
    TEST_ASSERT_EQUAL_UINT8(99, bcdToDec(0x99));
}

void test_bcd_from_dec_known_values() {
    TEST_ASSERT_EQUAL_HEX8(0x00, decToBcd(0));
    TEST_ASSERT_EQUAL_HEX8(0x09, decToBcd(9));
    TEST_ASSERT_EQUAL_HEX8(0x10, decToBcd(10));
    TEST_ASSERT_EQUAL_HEX8(0x59, decToBcd(59));
    TEST_ASSERT_EQUAL_HEX8(0x99, decToBcd(99));
}

void test_bcd_round_trip_0_to_99() {
    for (unsigned d = 0; d <= 99; ++d) {
        const uint8_t bcd = decToBcd(static_cast<uint8_t>(d));
        // BCD ที่ถูกต้องเขียนเป็นเลขฐาน 16 แล้วต้องอ่านได้เหมือนเลขฐาน 10 เดิม
        char asHex[3];
        char asDec[3];
        snprintf(asHex, sizeof asHex, "%02X", bcd);
        snprintf(asDec, sizeof asDec, "%02u", d);
        TEST_ASSERT_EQUAL_STRING(asDec, asHex);
        TEST_ASSERT_EQUAL_UINT8(d, bcdToDec(bcd));
    }
}

void test_bcd_valid_accepts_all_decimal_digits() {
    for (unsigned tens = 0; tens <= 9; ++tens) {
        for (unsigned ones = 0; ones <= 9; ++ones) {
            TEST_ASSERT_TRUE(isValidBcd(static_cast<uint8_t>((tens << 4) | ones)));
        }
    }
}

void test_bcd_valid_rejects_non_decimal_nibbles() {
    TEST_ASSERT_FALSE(isValidBcd(0x1A));
    TEST_ASSERT_FALSE(isValidBcd(0xA0));
    TEST_ASSERT_FALSE(isValidBcd(0xFF));
    TEST_ASSERT_FALSE(isValidBcd(0x0F));
    TEST_ASSERT_FALSE(isValidBcd(0xF0));
}
