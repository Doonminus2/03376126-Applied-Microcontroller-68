#pragma once
#include <cstdint>

namespace Clock {

constexpr uint8_t bcdToDec(uint8_t bcd) { return static_cast<uint8_t>((bcd >> 4) * 10 + (bcd & 0x0F)); }

constexpr uint8_t decToBcd(uint8_t dec) { return static_cast<uint8_t>(((dec / 10) << 4) | (dec % 10)); }

constexpr bool isValidBcd(uint8_t bcd) { return (bcd >> 4) <= 9 && (bcd & 0x0F) <= 9; }

}  // namespace Clock
