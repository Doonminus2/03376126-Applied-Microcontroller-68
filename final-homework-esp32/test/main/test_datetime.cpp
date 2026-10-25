#include <cstring>

#include "clock_logic/datetime.hpp"
#include "tests.hpp"

using namespace Clock;

static void expectDateTime(const DateTime& dt, unsigned year, unsigned month, unsigned day, unsigned weekday,
                           unsigned hour, unsigned minute, unsigned second) {
    TEST_ASSERT_EQUAL_UINT_MESSAGE(year, dt.year, "year");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(month, dt.month, "month");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(day, dt.day, "day");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(weekday, dt.weekday, "weekday");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(hour, dt.hour, "hour");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(minute, dt.minute, "minute");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(second, dt.second, "second");
}

static DateTime makeDateTime(unsigned year, unsigned month, unsigned day, unsigned weekday, unsigned hour,
                             unsigned minute, unsigned second) {
    DateTime dt;
    dt.year = static_cast<uint16_t>(year);
    dt.month = static_cast<uint8_t>(month);
    dt.day = static_cast<uint8_t>(day);
    dt.weekday = static_cast<uint8_t>(weekday);
    dt.hour = static_cast<uint8_t>(hour);
    dt.minute = static_cast<uint8_t>(minute);
    dt.second = static_cast<uint8_t>(second);
    return dt;
}

// ค่าที่ decode ล้มเหลวต้องไม่ไปแตะ out เลย จึงตั้งค่าเฝ้าไว้ก่อน
static const DateTime SENTINEL = makeDateTime(2042, 7, 7, 7, 7, 7, 7);

static bool decodeHour(uint8_t hourReg, DateTime& out) {
    const uint8_t raw[7] = {0x00, 0x00, hourReg, 0x01, 0x01, 0x01, 0x00};
    return ds1307Decode(raw, out);
}

void test_ds1307_decode_known_registers() {
    const uint8_t raw[7] = {0x45, 0x30, 0x08, 0x01, 0x25, 0x10, 0x26};
    DateTime dt = SENTINEL;
    TEST_ASSERT_TRUE(ds1307Decode(raw, dt));
    expectDateTime(dt, 2026, 10, 25, 1, 8, 30, 45);

    const uint8_t raw2[7] = {0x59, 0x59, 0x23, 0x07, 0x31, 0x12, 0x99};
    TEST_ASSERT_TRUE(ds1307Decode(raw2, dt));
    expectDateTime(dt, 2099, 12, 31, 7, 23, 59, 59);
}

void test_ds1307_encode_known_datetime() {
    uint8_t raw[7];
    memset(raw, 0xFF, sizeof raw);
    ds1307Encode(makeDateTime(2026, 10, 25, 1, 8, 30, 45), raw);
    const uint8_t expected[7] = {0x45, 0x30, 0x08, 0x01, 0x25, 0x10, 0x26};
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, raw, 7);
}

void test_ds1307_encode_clears_ch_and_selects_24h() {
    uint8_t raw[7];
    memset(raw, 0xFF, sizeof raw);
    ds1307Encode(makeDateTime(2025, 12, 31, 4, 23, 59, 59), raw);
    TEST_ASSERT_EQUAL_HEX8(0x59, raw[0]);
    TEST_ASSERT_EQUAL_HEX8(0x00, raw[0] & DS1307_CH_BIT);
    TEST_ASSERT_EQUAL_HEX8(0x23, raw[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, raw[2] & DS1307_12H_BIT);
    ds1307Encode(makeDateTime(2025, 1, 1, 4, 13, 0, 0), raw);
    TEST_ASSERT_EQUAL_HEX8(0x13, raw[2]);
}

void test_ds1307_decode_ignores_clock_halt_bit() {
    const uint8_t raw[7] = {static_cast<uint8_t>(DS1307_CH_BIT | 0x37), 0x15, 0x09, 0x05, 0x09, 0x10, 0x25};
    DateTime dt = SENTINEL;
    TEST_ASSERT_TRUE(ds1307Decode(raw, dt));
    expectDateTime(dt, 2025, 10, 9, 5, 9, 15, 37);
}

void test_ds1307_decode_12h_mode() {
    struct Case {
        uint8_t reg;
        unsigned hour24;
    };
    const Case cases[] = {
        {DS1307_12H_BIT | 0x12, 0},                  // 12 AM
        {DS1307_12H_BIT | 0x01, 1},                  // 1 AM
        {DS1307_12H_BIT | 0x11, 11},                 // 11 AM
        {DS1307_12H_BIT | DS1307_PM_BIT | 0x12, 12}, // 12 PM
        {DS1307_12H_BIT | DS1307_PM_BIT | 0x01, 13}, // 1 PM
        {DS1307_12H_BIT | DS1307_PM_BIT | 0x11, 23}, // 11 PM
    };
    for (const Case& c : cases) {
        DateTime dt = SENTINEL;
        TEST_ASSERT_TRUE(decodeHour(c.reg, dt));
        TEST_ASSERT_EQUAL_UINT(c.hour24, dt.hour);
    }
}

void test_ds1307_decode_rejects_invalid_12h_hour() {
    DateTime dt = SENTINEL;
    TEST_ASSERT_FALSE(decodeHour(DS1307_12H_BIT | 0x00, dt));
    TEST_ASSERT_FALSE(decodeHour(DS1307_12H_BIT | DS1307_PM_BIT | 0x13, dt));
    TEST_ASSERT_FALSE(decodeHour(DS1307_12H_BIT | 0x1A, dt));
    TEST_ASSERT_TRUE(SENTINEL == dt);
}

void test_ds1307_decode_rejects_invalid_values() {
    struct Case {
        uint8_t raw[7];
        const char* what;
    };
    const Case cases[] = {
        {{0x5A, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00}, "seconds not BCD"},
        {{0x00, 0x60, 0x00, 0x01, 0x01, 0x01, 0x00}, "minute 60"},
        {{0x00, 0x00, 0x24, 0x01, 0x01, 0x01, 0x00}, "hour 24"},
        {{0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00}, "weekday 0"},
        {{0x00, 0x00, 0x00, 0x08, 0x01, 0x01, 0x00}, "weekday 8"},
        {{0x00, 0x00, 0x00, 0x01, 0x32, 0x01, 0x00}, "day 32"},
        {{0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00}, "day 0"},
        {{0x00, 0x00, 0x00, 0x01, 0x30, 0x02, 0x24}, "Feb 30"},
        {{0x00, 0x00, 0x00, 0x01, 0x29, 0x02, 0x25}, "Feb 29 in a common year"},
        {{0x00, 0x00, 0x00, 0x01, 0x01, 0x13, 0x00}, "month 13"},
        {{0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00}, "month 0"},
        {{0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0xA0}, "year not BCD"},
    };
    for (const Case& c : cases) {
        DateTime dt = SENTINEL;
        TEST_ASSERT_FALSE_MESSAGE(ds1307Decode(c.raw, dt), c.what);
        TEST_ASSERT_TRUE_MESSAGE(SENTINEL == dt, c.what);
    }
}

void test_ds1307_encode_decode_round_trip() {
    for (unsigned hour = 0; hour < 24; ++hour) {
        for (unsigned minute = 0; minute < 60; ++minute) {
            const DateTime in = makeDateTime(2000 + (hour * 60 + minute) % 100, 1 + minute % 12, 1 + hour % 28,
                                             1 + minute % 7, hour, minute, (hour + minute) % 60);
            uint8_t raw[7];
            ds1307Encode(in, raw);
            DateTime out = SENTINEL;
            TEST_ASSERT_TRUE(ds1307Decode(raw, out));
            TEST_ASSERT_TRUE(in == out);
        }
    }
}

void test_epoch_unix_origin() {
    TEST_ASSERT_EQUAL_INT64(0, toEpoch(makeDateTime(1970, 1, 1, 5, 0, 0, 0)));
    TEST_ASSERT_EQUAL_INT64(86399, toEpoch(makeDateTime(1970, 1, 1, 5, 23, 59, 59)));
}

void test_epoch_y2k() {
    TEST_ASSERT_EQUAL_INT64(946684800, toEpoch(makeDateTime(2000, 1, 1, 7, 0, 0, 0)));
    DateTime dt = SENTINEL;
    TEST_ASSERT_TRUE(fromEpoch(946684800, dt));
    expectDateTime(dt, 2000, 1, 1, 7, 0, 0, 0);
}

void test_epoch_known_datetime() {
    // ค่าอ้างอิงคำนวณแยกด้วย python: calendar.timegm((2026, 10, 25, 8, 30, 0))
    TEST_ASSERT_EQUAL_INT64(1792917000, toEpoch(makeDateTime(2026, 10, 25, 1, 8, 30, 0)));
    DateTime dt = SENTINEL;
    TEST_ASSERT_TRUE(fromEpoch(1792917000, dt));
    expectDateTime(dt, 2026, 10, 25, 1, 8, 30, 0);
}

void test_epoch_leap_day_2028() {
    TEST_ASSERT_EQUAL_INT64(1835395200, toEpoch(makeDateTime(2028, 2, 29, 3, 0, 0, 0)));
    DateTime dt = SENTINEL;
    TEST_ASSERT_TRUE(fromEpoch(1835395200, dt));
    expectDateTime(dt, 2028, 2, 29, 3, 0, 0, 0);
    TEST_ASSERT_TRUE(fromEpoch(1835395200 - 1, dt));
    expectDateTime(dt, 2028, 2, 28, 2, 23, 59, 59);
    TEST_ASSERT_TRUE(fromEpoch(1835395200 + 86400, dt));
    expectDateTime(dt, 2028, 3, 1, 4, 0, 0, 0);
}

void test_epoch_leap_year_rules() {
    TEST_ASSERT_TRUE(isLeapYear(2000));
    TEST_ASSERT_TRUE(isLeapYear(2028));
    TEST_ASSERT_FALSE(isLeapYear(2026));
    TEST_ASSERT_FALSE(isLeapYear(2100));
    TEST_ASSERT_EQUAL_INT(29, daysInMonth(2028, 2));
    TEST_ASSERT_EQUAL_INT(28, daysInMonth(2026, 2));
    TEST_ASSERT_EQUAL_INT(31, daysInMonth(2026, 12));
    TEST_ASSERT_EQUAL_INT(30, daysInMonth(2026, 4));
}

void test_epoch_weekday_known_dates() {
    TEST_ASSERT_EQUAL_UINT8(5, weekdayFromDate(2025, 10, 9));   // พฤหัส
    TEST_ASSERT_EQUAL_UINT8(5, weekdayFromDate(1970, 1, 1));    // พฤหัส
    TEST_ASSERT_EQUAL_UINT8(7, weekdayFromDate(2000, 1, 1));    // เสาร์
    TEST_ASSERT_EQUAL_UINT8(1, weekdayFromDate(2026, 10, 25));  // อาทิตย์
    TEST_ASSERT_EQUAL_UINT8(3, weekdayFromDate(2028, 2, 29));   // อังคาร
}

void test_epoch_from_out_of_range_rejected() {
    DateTime dt = SENTINEL;
    TEST_ASSERT_FALSE(fromEpoch(0, dt));
    TEST_ASSERT_FALSE(fromEpoch(-1, dt));
    TEST_ASSERT_FALSE(fromEpoch(946684799, dt));   // 1999-12-31 23:59:59
    TEST_ASSERT_FALSE(fromEpoch(4102444800, dt));  // 2100-01-01 00:00:00
    TEST_ASSERT_TRUE(SENTINEL == dt);
    TEST_ASSERT_TRUE(fromEpoch(4102444799, dt));
    expectDateTime(dt, 2099, 12, 31, 5, 23, 59, 59);
}

void test_epoch_round_trip() {
    for (int64_t e = 946684800; e < 4102444800; e += 1234567) {
        DateTime dt;
        TEST_ASSERT_TRUE(fromEpoch(e, dt));
        TEST_ASSERT_TRUE(isValid(dt));
        TEST_ASSERT_EQUAL_UINT8(weekdayFromDate(dt.year, dt.month, dt.day), dt.weekday);
        TEST_ASSERT_EQUAL_INT64(e, toEpoch(dt));
    }
}
