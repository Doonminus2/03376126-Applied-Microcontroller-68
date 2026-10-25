#include "SSD1306.hpp"

#include <cstring>

#include "esp_log.h"
#include "font5x7.hpp"

namespace Clock {

static const char* TAG = "SSD1306";

namespace {
constexpr uint8_t CTRL_CMD = 0x00;
constexpr uint8_t CTRL_DATA = 0x40;
}  // namespace

bool SSD1306::command(uint8_t cmd) {
    const uint8_t buf[2] = {CTRL_CMD, cmd};
    return bus_.write(addr_, buf, sizeof(buf));
}

bool SSD1306::commands(const uint8_t* cmds, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (!command(cmds[i])) return false;
    }
    return true;
}

bool SSD1306::init() {
    // ลำดับเริ่มต้นมาตรฐานของ SSD1306 128x64 ใช้ charge pump ภายใน
    static const uint8_t seq[] = {
        0xAE,        // display off
        0xD5, 0x80,  // clock divide / oscillator frequency
        0xA8, 0x3F,  // multiplex ratio = 64
        0xD3, 0x00,  // display offset = 0
        0x40,        // start line = 0
        0x8D, 0x14,  // charge pump enable
        0x20, 0x00,  // horizontal addressing mode
        0xA1,        // segment remap (กลับซ้าย-ขวา)
        0xC8,        // COM scan direction ย้อนกลับ (กลับบน-ล่าง)
        0xDA, 0x12,  // COM pins configuration
        0x81, 0xCF,  // contrast
        0xD9, 0xF1,  // pre-charge period
        0xDB, 0x40,  // VCOMH deselect level
        0xA4,        // แสดงตามเนื้อหา RAM
        0xA6,        // normal (ไม่กลับสี)
        0x2E,        // ปิด scroll
        0xAF,        // display on
    };
    if (!commands(seq, sizeof(seq))) {
        ESP_LOGE(TAG, "init failed: no ACK from 0x%02X", addr_);
        return false;
    }
    clear();
    return flush();
}

void SSD1306::clear() { memset(buffer_, 0, sizeof(buffer_)); }

void SSD1306::setPixel(int x, int y, bool on) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    const uint8_t mask = static_cast<uint8_t>(1u << (y & 7));
    uint8_t& cell = buffer_[x + (y >> 3) * WIDTH];
    if (on) {
        cell |= mask;
    } else {
        cell &= static_cast<uint8_t>(~mask);
    }
}

int SSD1306::drawChar(int x, int y, char c, int scale) {
    if (scale < 1) scale = 1;
    const unsigned char uc = static_cast<unsigned char>(c);
    const uint8_t* glyph = FONT5X7[(uc < FONT_FIRST || uc > FONT_LAST) ? '?' - FONT_FIRST : uc - FONT_FIRST];
    for (int col = 0; col < FONT_WIDTH; ++col) {
        for (int row = 0; row < FONT_HEIGHT; ++row) {
            if (!((glyph[col] >> row) & 1)) continue;
            for (int dx = 0; dx < scale; ++dx) {
                for (int dy = 0; dy < scale; ++dy) {
                    setPixel(x + col * scale + dx, y + row * scale + dy, true);
                }
            }
        }
    }
    return x + (FONT_WIDTH + 1) * scale;
}

int SSD1306::drawString(int x, int y, const char* str, int scale) {
    for (; *str; ++str) x = drawChar(x, y, *str, scale);
    return x;
}

void SSD1306::invertRect(int x, int y, int w, int h) {
    for (int yy = y; yy < y + h; ++yy) {
        for (int xx = x; xx < x + w; ++xx) {
            if (xx < 0 || xx >= WIDTH || yy < 0 || yy >= HEIGHT) continue;
            buffer_[xx + (yy >> 3) * WIDTH] ^= static_cast<uint8_t>(1u << (yy & 7));
        }
    }
}

bool SSD1306::flush() {
    // ตั้งช่วงคอลัมน์ 0..127 และหน้า 0..7 แล้วส่งข้อมูลทีละหน้า (control byte 0x40 + 128 ไบต์)
    static const uint8_t addrWindow[] = {0x21, 0x00, 0x7F, 0x22, 0x00, 0x07};
    if (!commands(addrWindow, sizeof(addrWindow))) return false;
    for (size_t page = 0; page < PAGES; ++page) {
        if (!bus_.writeRegs(addr_, CTRL_DATA, buffer_ + page * WIDTH, WIDTH)) return false;
    }
    return true;
}

}  // namespace Clock
