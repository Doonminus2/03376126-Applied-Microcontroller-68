# 03376126-Applied-Microcontroller-68 — Branch `final-homework-esp32`

Final Homework: นาฬิกาดิจิทัล ESP32 แบบ Modular C++ OOP ด้วย ESP-IDF v6.0.2 และ FreeRTOS บน Wokwi Simulator พัฒนาแบบ Spiral (bottom-up) จำนวน 5 รอบ

**📄 รายงานผลการทดลอง: [REPORT.md](REPORT.md)**
**📘 สเปกโจทย์ (AI สร้าง ข้อมูลขัดกันเองและมีบั๊ก ใช้เป็นสเปกเท่านั้น ไม่ได้ copy โค้ดมาใช้):**
- [docs/wokwi_esp32_digital_clock_suite.html](docs/wokwi_esp32_digital_clock_suite.html)
- [docs/wokwi_esp32_digital_clock_dashboard.html](docs/wokwi_esp32_digital_clock_dashboard.html)

> README ของโปรเจกต์หลักและแลปก่อนหน้า (7-segment, Wi-Fi/I2C/ISR/SPIFFS) อยู่ใน branch `main` และ `Wi-Fi-Connectivity-9-oct-26`

## อุปกรณ์และฟีเจอร์

- ESP32 DevKitC V4 + DS1307 RTC (I2C `0x68`) + SSD1306 OLED 128x64 (I2C `0x3C`) + ปุ่ม 4 ปุ่ม + piezo buzzer
- แสดงเวลาไทย (`TZ=ICT-7`) ซิงก์เวลาจาก Wi-Fi + SNTP ลงนาฬิกา DS1307 (ถ้าไม่มีอินเทอร์เน็ตใช้เวลาจาก RTC ต่อ)
- นาฬิกาปลุก (buzzer ผ่าน LEDC) และ stopwatch พร้อมบันทึก lap

## โครงสร้างโปรเจกต์

```text
blink/
├── README.md, REPORT.md         # คำอธิบายแลป และรายงานผลการทดลอง
├── wokwi.toml, diagram.json     # เปิดที่ root แล้วรัน Spiral 5 (ระบบเต็ม) ใน Wokwi ได้ทันที
├── docs/
│   ├── wokwi_esp32_digital_clock_suite.html      # สเปกโจทย์ (ไฟล์ที่ 1)
│   ├── wokwi_esp32_digital_clock_dashboard.html  # สเปกโจทย์ (ไฟล์ที่ 2)
│   └── images/spiral01..spiral05/                # ภาพหน้าจอผลการทดลอง
└── final-homework-esp32/        # แต่ละ spiral เป็นโปรเจกต์ ESP-IDF แยกกัน
    ├── components/              # คลาส C++ ที่ทุก spiral ใช้ร่วมกัน
    │   ├── clock_logic/         # logic ล้วน (BCD, เวลา, ปลุก, lap, ปุ่ม, โหมด) ไม่แตะฮาร์ดแวร์
    │   ├── i2c_bus/             # I2CBus
    │   ├── ds1307/              # DS1307
    │   ├── ssd1306/             # SSD1306 + ฟอนต์ 5x7
    │   ├── button_manager/      # ButtonManager
    │   ├── time_manager/        # TimeManager
    │   ├── alarm_engine/        # AlarmEngine
    │   └── stopwatch_engine/    # StopwatchEngine
    ├── 01-hal-i2c/              # Spiral 1: สแกน I2C, อ่าน/เขียน DS1307, แสดงข้อความบน SSD1306
    ├── 02-time-ntp/             # Spiral 2: Wi-Fi STA + SNTP + sync เวลาลง DS1307
    ├── 03-input-ui/             # Spiral 3: ปุ่มแบบ ISR + debounce + short/long press + state machine
    ├── 04-alarm-stopwatch/      # Spiral 4: alarm + buzzer (LEDC), stopwatch + lap
    ├── 05-integration/          # Spiral 5: ระบบเต็ม แบ่ง FreeRTOS task ตามตารางในสเปก
    └── test/                    # Unity test app สำหรับ logic ที่ไม่ผูกกับฮาร์ดแวร์
```

แต่ละโฟลเดอร์ spiral (และ `test/`) มีไฟล์:

| ไฟล์ | หน้าที่ |
| --- | --- |
| `CMakeLists.txt` | ไฟล์โปรเจกต์ ESP-IDF (ดึง `components/` ผ่าน `EXTRA_COMPONENT_DIRS`) |
| `main/main.cpp` | ซอร์สโค้ดของ spiral |
| `main/CMakeLists.txt` | ลงทะเบียนคอมโพเนนต์ `main` และ component ที่ต้องใช้ |
| `diagram.json` | วงจรสำหรับ Wokwi (ใส่เฉพาะอุปกรณ์ที่ spiral นั้นใช้) |
| `wokwi.toml` | บอก Wokwi ว่าจะโหลด firmware จากไหน |
| `sdkconfig.defaults` | ค่าตั้งต้นของ sdkconfig (เฉพาะ spiral ที่ใช้ Wi-Fi) |

คลาสใน `components/` (แต่ละตัวเป็น ESP-IDF component):

| Component | คลาส / ไฟล์หลัก | หน้าที่ |
| --- | --- | --- |
| `clock_logic` | `bcd.hpp`, `datetime.hpp`, `alarm_logic.hpp`, `stopwatch_core.hpp`, `button_fsm.hpp`, `ui_mode.hpp` | logic ล้วนที่ไม่เรียก ESP-IDF จึงเขียน unit test รันจริงได้ |
| `i2c_bus` | `I2CBus` | บัส I2C master (100 kHz) ป้องกันด้วย mutex ใช้ร่วมกันได้หลาย task |
| `ds1307` | `DS1307` | อ่าน/เขียนเวลา RTC (เก็บเป็น UTC โหมด 24 ชั่วโมง) |
| `ssd1306` | `SSD1306`, `font5x7.hpp` | ส่ง init sequence, วาดตัวอักษรลง framebuffer 1024 ไบต์แล้ว flush |
| `button_manager` | `ButtonManager` | ISR (ANYEDGE) → Queue → task debounce → short/long press |
| `time_manager` | `TimeManager` | Wi-Fi STA + SNTP + ตั้งเวลาลง RTC + แปลงเป็นเวลาไทย |
| `alarm_engine` | `AlarmEngine` | เทียบเวลาปลุกและเปิดเสียง buzzer ผ่าน LEDC |
| `stopwatch_engine` | `StopwatchEngine` | จับเวลาด้วย `esp_timer` พร้อม lap |

## ขาที่ใช้

| อุปกรณ์ | ขา ESP32 |
| --- | --- |
| I2C SDA / SCL (DS1307 + SSD1306) | GPIO 21 / GPIO 22 |
| ปุ่ม MODE / SET / ALARM / LAP-RST | GPIO 18 / 19 / 4 / 5 (pull-up ภายใน, กด = LOW) |
| Buzzer | GPIO 25 (LEDC PWM) |

## วิธี build และรัน

เปิด `ESP-IDF: Open ESP-IDF Terminal` ใน VS Code แล้วรัน:

```bash
cd final-homework-esp32/01-hal-i2c
source ~/.espressif/tools/activate_idf_v6.0.2.sh
idf.py set-target esp32
idf.py build
```

จากนั้น `F1` → `Wokwi: Select Config File` → เลือก `final-homework-esp32/01-hal-i2c/wokwi.toml` → `Wokwi: Start Simulator`

ทำแบบเดียวกันกับ `02-time-ntp`, `03-input-ui`, `04-alarm-stopwatch` และ `05-integration`

**ทางลัดสำหรับระบบเต็ม:** build `05-integration` แล้วเลือก `wokwi.toml` ที่ root ของ repo ได้เลย (ชี้ไป firmware ของ Spiral 5)

**Unity test:** `cd final-homework-esp32/test` แล้ว build เหมือนด้านบนและรันใน Wokwi ดูผลใน Serial Monitor หรือรันบนเครื่อง (ไม่ต้องใช้บอร์ด) ด้วย `bash final-homework-esp32/test/host/run.sh`

## ตั้งค่า Wi-Fi สำหรับบอร์ดจริง

ค่าเริ่มต้นใช้ `Wokwi-GUEST` (ไม่มีรหัส) เพราะ Wokwi ไม่มี AP ชื่อ `iotexpo` เมื่อใช้บอร์ดจริงให้รัน `idf.py menuconfig` → `Digital Clock` แล้วเปลี่ยน SSID เป็น `iotexpo` และรหัสผ่านเป็น `12345678` (ตั้งเขตเวลา NTP server ได้ที่เมนูเดียวกัน)
