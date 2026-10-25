#include "DS1307.hpp"

#include "esp_log.h"

namespace Clock {

static const char* TAG = "DS1307";

bool DS1307::init() {
    uint8_t raw[7];
    if (!bus_.readRegs(I2C_ADDR, 0x00, raw, sizeof(raw))) {
        ESP_LOGE(TAG, "no ACK from DS1307 at 0x%02X", I2C_ADDR);
        return false;
    }
    if (raw[0] & DS1307_CH_BIT) {
        ESP_LOGW(TAG, "clock halted (CH=1), starting oscillator");
        if (!bus_.writeReg(I2C_ADDR, 0x00, raw[0] & static_cast<uint8_t>(~DS1307_CH_BIT))) return false;
    }
    if (raw[2] & DS1307_12H_BIT) {
        DateTime dt;
        if (ds1307Decode(raw, dt)) {
            ESP_LOGW(TAG, "12-hour mode detected, converting to 24-hour");
            return setTime(dt);
        }
    }
    return true;
}

bool DS1307::getTime(DateTime& dt) {
    uint8_t raw[7];
    if (!bus_.readRegs(I2C_ADDR, 0x00, raw, sizeof(raw))) return false;
    return ds1307Decode(raw, dt);
}

bool DS1307::setTime(const DateTime& dt) {
    DateTime fixed = dt;
    fixed.weekday = weekdayFromDate(dt.year, dt.month, dt.day);
    if (!isValid(fixed)) return false;
    uint8_t raw[7];
    ds1307Encode(fixed, raw);
    return bus_.writeRegs(I2C_ADDR, 0x00, raw, sizeof(raw));
}

}  // namespace Clock
