#include "I2CBus.hpp"

#include <cstring>

#include "esp_log.h"

namespace Clock {

static const char* TAG = "I2CBus";
static constexpr int XFER_TIMEOUT_MS = 100;
static constexpr TickType_t LOCK_TIMEOUT = pdMS_TO_TICKS(500);

namespace {
class Lock {
public:
    explicit Lock(SemaphoreHandle_t m) : m_(m), ok_(xSemaphoreTake(m, LOCK_TIMEOUT) == pdTRUE) {}
    ~Lock() {
        if (ok_) xSemaphoreGive(m_);
    }
    bool ok() const { return ok_; }

private:
    SemaphoreHandle_t m_;
    bool ok_;
};
}  // namespace

I2CBus::I2CBus(i2c_port_num_t port, gpio_num_t sda, gpio_num_t scl, uint32_t speedHz)
    : port_(port), sda_(sda), scl_(scl), speedHz_(speedHz) {}

I2CBus::~I2CBus() {
    for (auto& s : slots_) {
        if (s.handle) i2c_master_bus_rm_device(s.handle);
    }
    if (bus_) i2c_del_master_bus(bus_);
    if (mutex_) vSemaphoreDelete(mutex_);
}

bool I2CBus::init() {
    if (bus_) return true;
    mutex_ = xSemaphoreCreateMutex();
    if (!mutex_) return false;

    i2c_master_bus_config_t cfg = {};
    cfg.i2c_port = port_;
    cfg.sda_io_num = sda_;
    cfg.scl_io_num = scl_;
    cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = true;

    esp_err_t err = i2c_new_master_bus(&cfg, &bus_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(err));
        bus_ = nullptr;
        return false;
    }
    return true;
}

i2c_master_dev_handle_t I2CBus::handleFor(uint8_t addr) {
    for (auto& s : slots_) {
        if (s.handle && s.addr == addr) return s.handle;
    }
    for (auto& s : slots_) {
        if (s.handle) continue;
        i2c_device_config_t dev = {};
        dev.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev.device_address = addr;
        dev.scl_speed_hz = speedHz_;
        if (i2c_master_bus_add_device(bus_, &dev, &s.handle) != ESP_OK) {
            s.handle = nullptr;
            return nullptr;
        }
        s.addr = addr;
        return s.handle;
    }
    ESP_LOGE(TAG, "no free device slot for 0x%02X", addr);
    return nullptr;
}

bool I2CBus::probe(uint8_t addr) {
    if (!bus_) return false;
    Lock lock(mutex_);
    return lock.ok() && i2c_master_probe(bus_, addr, XFER_TIMEOUT_MS) == ESP_OK;
}

bool I2CBus::write(uint8_t addr, const uint8_t* data, size_t len) {
    if (!bus_) return false;
    Lock lock(mutex_);
    if (!lock.ok()) return false;
    i2c_master_dev_handle_t dev = handleFor(addr);
    return dev && i2c_master_transmit(dev, data, len, XFER_TIMEOUT_MS) == ESP_OK;
}

bool I2CBus::writeReg(uint8_t addr, uint8_t reg, uint8_t value) {
    const uint8_t buf[2] = {reg, value};
    return write(addr, buf, sizeof(buf));
}

bool I2CBus::writeRegs(uint8_t addr, uint8_t reg, const uint8_t* data, size_t len) {
    if (len + 1 > MAX_WRITE) return false;
    uint8_t buf[MAX_WRITE];
    buf[0] = reg;
    memcpy(buf + 1, data, len);
    return write(addr, buf, len + 1);
}

bool I2CBus::readRegs(uint8_t addr, uint8_t reg, uint8_t* buffer, size_t len) {
    if (!bus_) return false;
    Lock lock(mutex_);
    if (!lock.ok()) return false;
    i2c_master_dev_handle_t dev = handleFor(addr);
    return dev && i2c_master_transmit_receive(dev, &reg, 1, buffer, len, XFER_TIMEOUT_MS) == ESP_OK;
}

}  // namespace Clock
