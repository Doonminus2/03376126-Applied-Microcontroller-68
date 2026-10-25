#pragma once
#include <cstddef>
#include <cstdint>

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace Clock {

// ตัวห่อ I2C master ของ ESP-IDF v6 ที่ thread-safe (mutex คุมทุกการส่ง/รับ)
// อุปกรณ์หลายตัว (DS1307, SSD1306) ใช้บัสเดียวกันจากหลาย task ได้
class I2CBus {
public:
    static constexpr size_t MAX_DEVICES = 4;
    static constexpr size_t MAX_WRITE = 160;  // เฟรมจอ 1 หน้า = 1 + 128 ไบต์

    I2CBus(i2c_port_num_t port, gpio_num_t sda, gpio_num_t scl, uint32_t speedHz = 100000);
    ~I2CBus();
    I2CBus(const I2CBus&) = delete;
    I2CBus& operator=(const I2CBus&) = delete;

    bool init();
    bool probe(uint8_t addr);
    bool write(uint8_t addr, const uint8_t* data, size_t len);
    bool writeReg(uint8_t addr, uint8_t reg, uint8_t value);
    bool writeRegs(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len);
    bool readRegs(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len);

private:
    struct Slot {
        uint8_t addr = 0;
        i2c_master_dev_handle_t handle = nullptr;
    };

    i2c_master_dev_handle_t handleFor(uint8_t addr);  // เรียกตอนถือ mutex แล้วเท่านั้น

    i2c_port_num_t port_;
    gpio_num_t sda_;
    gpio_num_t scl_;
    uint32_t speedHz_;
    i2c_master_bus_handle_t bus_ = nullptr;
    SemaphoreHandle_t mutex_ = nullptr;
    Slot slots_[MAX_DEVICES];
};

}  // namespace Clock
