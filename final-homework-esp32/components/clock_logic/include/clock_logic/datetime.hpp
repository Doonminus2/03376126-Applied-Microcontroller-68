#pragma once
#include <cstdint>

#include "clock_logic/bcd.hpp"

namespace Clock {

struct DateTime {
    uint16_t year = 2000;  // 2000..2099 (DS1307 เก็บปี 2 หลัก)
    uint8_t month = 1;     // 1..12
    uint8_t day = 1;       // 1..31
    uint8_t weekday = 1;   // 1..7 (1 = อาทิตย์ ตามรีจิสเตอร์ DS1307)
    uint8_t hour = 0;      // 0..23
    uint8_t minute = 0;    // 0..59
    uint8_t second = 0;    // 0..59

    bool operator==(const DateTime& o) const {
        return year == o.year && month == o.month && day == o.day && weekday == o.weekday && hour == o.hour &&
               minute == o.minute && second == o.second;
    }
};

constexpr bool isLeapYear(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

constexpr int daysInMonth(int year, int month) {
    constexpr int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return (month == 2 && isLeapYear(year)) ? 29 : days[month - 1];
}

constexpr bool isValid(const DateTime& dt) {
    return dt.year >= 2000 && dt.year <= 2099 && dt.month >= 1 && dt.month <= 12 && dt.day >= 1 &&
           dt.day <= daysInMonth(dt.year, dt.month) && dt.weekday >= 1 && dt.weekday <= 7 && dt.hour <= 23 &&
           dt.minute <= 59 && dt.second <= 59;
}

// จำนวนวันนับจาก 1970-01-01 (อัลกอริทึม days_from_civil ของ Howard Hinnant)
constexpr int64_t daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const int64_t era = (y >= 0 ? y : y - 399) / 400;
    const int64_t yoe = y - era * 400;
    const int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

// 1 = อาทิตย์ ... 7 = เสาร์
constexpr uint8_t weekdayFromDate(int y, int m, int d) {
    const int64_t days = daysFromCivil(y, m, d);  // 1970-01-01 เป็นวันพฤหัส
    const int64_t w = ((days % 7) + 7 + 4) % 7;   // 0 = อาทิตย์
    return static_cast<uint8_t>(w + 1);
}

// DateTime (UTC) -> epoch วินาที
constexpr int64_t toEpoch(const DateTime& dt) {
    return daysFromCivil(dt.year, dt.month, dt.day) * 86400 + dt.hour * 3600 + dt.minute * 60 + dt.second;
}

// epoch วินาที -> DateTime (UTC) คืน false ถ้าอยู่นอกช่วงปี 2000..2099
inline bool fromEpoch(int64_t epoch, DateTime& out) {
    int64_t days = epoch / 86400;
    int64_t rem = epoch % 86400;
    if (rem < 0) {
        rem += 86400;
        days -= 1;
    }
    const int64_t z = days + 719468;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const int64_t doe = z - era * 146097;
    const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const int64_t mp = (5 * doy + 2) / 153;
    const int d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    const int m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    const int y = static_cast<int>(yoe + era * 400 + (m <= 2));

    DateTime dt;
    dt.year = static_cast<uint16_t>(y);
    dt.month = static_cast<uint8_t>(m);
    dt.day = static_cast<uint8_t>(d);
    dt.weekday = weekdayFromDate(y, m, d);
    dt.hour = static_cast<uint8_t>(rem / 3600);
    dt.minute = static_cast<uint8_t>((rem % 3600) / 60);
    dt.second = static_cast<uint8_t>(rem % 60);
    if (!isValid(dt)) return false;
    out = dt;
    return true;
}

// ---- รีจิสเตอร์ DS1307 (0x00..0x06) ----
//  0: bit7 = CH (clock halt), bit6..4 = สิบวินาที, bit3..0 = หน่วยวินาที
//  2: bit6 = 12/24h (1 = 12h), bit5 = AM/PM หรือสิบชั่วโมงบิต 1, bit4 = สิบชั่วโมง
constexpr uint8_t DS1307_CH_BIT = 0x80;
constexpr uint8_t DS1307_12H_BIT = 0x40;
constexpr uint8_t DS1307_PM_BIT = 0x20;

// raw[7] -> DateTime รองรับทั้งโหมด 12 และ 24 ชั่วโมง คืน false ถ้าค่าเสีย (เช่น BCD ผิด)
inline bool ds1307Decode(const uint8_t raw[7], DateTime& out) {
    const uint8_t secReg = raw[0] & static_cast<uint8_t>(~DS1307_CH_BIT);
    if (!isValidBcd(secReg) || !isValidBcd(raw[1]) || !isValidBcd(raw[3]) || !isValidBcd(raw[4]) ||
        !isValidBcd(raw[5]) || !isValidBcd(raw[6])) {
        return false;
    }
    uint8_t hour;
    if (raw[2] & DS1307_12H_BIT) {
        const uint8_t h12Reg = raw[2] & 0x1F;
        if (!isValidBcd(h12Reg)) return false;
        const uint8_t h12 = bcdToDec(h12Reg);
        if (h12 < 1 || h12 > 12) return false;
        hour = static_cast<uint8_t>(h12 % 12 + ((raw[2] & DS1307_PM_BIT) ? 12 : 0));
    } else {
        const uint8_t h24Reg = raw[2] & 0x3F;
        if (!isValidBcd(h24Reg)) return false;
        hour = bcdToDec(h24Reg);
    }
    DateTime dt;
    dt.second = bcdToDec(secReg);
    dt.minute = bcdToDec(raw[1]);
    dt.hour = hour;
    dt.weekday = bcdToDec(raw[3]);
    dt.day = bcdToDec(raw[4]);
    dt.month = bcdToDec(raw[5]);
    dt.year = static_cast<uint16_t>(2000 + bcdToDec(raw[6]));
    if (!isValid(dt)) return false;
    out = dt;
    return true;
}

// DateTime -> raw[7] เขียนเป็นโหมด 24 ชั่วโมงและเคลียร์ CH เสมอ (ให้นาฬิกาเดิน)
inline void ds1307Encode(const DateTime& dt, uint8_t raw[7]) {
    raw[0] = decToBcd(dt.second);  // bit7 = 0 -> CH ถูกเคลียร์
    raw[1] = decToBcd(dt.minute);
    raw[2] = decToBcd(dt.hour);  // bit6 = 0 -> โหมด 24 ชั่วโมง
    raw[3] = decToBcd(dt.weekday);
    raw[4] = decToBcd(dt.day);
    raw[5] = decToBcd(dt.month);
    raw[6] = decToBcd(static_cast<uint8_t>(dt.year - 2000));
}

}  // namespace Clock
