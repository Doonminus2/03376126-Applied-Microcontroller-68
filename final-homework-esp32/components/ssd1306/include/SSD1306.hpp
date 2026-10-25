#pragma once
#include <cstddef>
#include <cstdint>

#include "I2CBus.hpp"

namespace Clock {

// จอ OLED 128x64 ควบคุมด้วย SSD1306 ผ่าน I2C วาดลง framebuffer 1024 ไบต์ก่อน
// แล้วค่อย flush() ส่งทั้งเฟรมทีเดียว (การวาดเองไม่แตะบัส จึงเรียกจาก task ไหนก็ได้
// ตราบใดที่ task เดียวเป็นเจ้าของ framebuffer)
class SSD1306 {
public:
    static constexpr int WIDTH = 128;
    static constexpr int HEIGHT = 64;
    static constexpr size_t PAGES = HEIGHT / 8;
    static constexpr size_t BUFFER_SIZE = WIDTH * PAGES;
    static constexpr uint8_t DEFAULT_ADDR = 0x3C;

    explicit SSD1306(I2CBus& bus, uint8_t addr = DEFAULT_ADDR) : bus_(bus), addr_(addr) {}

    bool init();
    void clear();
    void setPixel(int x, int y, bool on = true);
    // วาดตัวอักษร ASCII ขนาด 5x7 ขยาย scale เท่า (ตัวอักษรกว้าง 6*scale รวมช่องไฟ) คืนตำแหน่ง x ถัดไป
    int drawChar(int x, int y, char c, int scale = 1);
    int drawString(int x, int y, const char* str, int scale = 1);
    void invertRect(int x, int y, int w, int h);  // ใช้ไฮไลต์ฟิลด์ที่กำลังแก้
    bool flush();

    const uint8_t* buffer() const { return buffer_; }

private:
    bool command(uint8_t cmd);
    bool commands(const uint8_t* cmds, size_t len);

    I2CBus& bus_;
    uint8_t addr_;
    uint8_t buffer_[BUFFER_SIZE] = {};
};

}  // namespace Clock
