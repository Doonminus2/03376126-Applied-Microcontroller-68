#pragma once
#include "I2CBus.hpp"
#include "clock_logic/datetime.hpp"

namespace Clock {

class DS1307 {
public:
    static constexpr uint8_t I2C_ADDR = 0x68;

    explicit DS1307(I2CBus& bus) : bus_(bus) {}

    // เคลียร์ CH ถ้านาฬิกาหยุด และบังคับโหมด 24 ชั่วโมง คืน false ถ้าอ่านไม่ได้
    bool init();
    bool getTime(DateTime& dt);
    bool setTime(const DateTime& dt);

private:
    I2CBus& bus_;
};

}  // namespace Clock
