# 03376126-Applied-Microcontroller-68 — Branch `Wi-Fi-Connectivity-9-oct-26`

งานทดแทนการเข้าเรียนวันที่ 9 ต.ค. 2569: ทดลอง ESP32 ด้วย ESP-IDF + FreeRTOS บน Wokwi Simulator จำนวน 4 โมดูล

**📄 รายงานผลการทดลอง: [REPORT.md](REPORT.md)**
**📘 คู่มือการทดลอง (ภาษาไทย): [docs/lab-manual-th.html](docs/lab-manual-th.html)**

> README ของโปรเจกต์หลัก (7-segment, วิธีติดตั้ง ESP-IDF และ Wokwi) อยู่ใน branch `main`

## โครงสร้างโปรเจกต์

```text
blink/
├── REPORT.md                    # รายงานผลการทดลอง
├── docs/
│   ├── lab-manual-th.html       # คู่มือแลป (แปลไทย)
│   ├── lab-manual-en.html       # คู่มือแลป (ต้นฉบับ)
│   └── images/lab01..lab04/     # ภาพหน้าจอผลการทดลอง
└── labs-9-oct-26/               # แต่ละแลปเป็นโปรเจกต์ ESP-IDF แยกกัน
    ├── 01-wifi-sta/             # Wi-Fi STA + Event Loop (Wokwi-GUEST)
    ├── 02-i2c-lcd/              # I2C Master + LCD1602 (PCF8574 @ 0x27)
    ├── 03-isr-queue/            # GPIO ISR + FreeRTOS Queue
    └── 04-spiffs/               # SPIFFS + VFS (มี partitions.csv)
```

แต่ละโฟลเดอร์ใน `labs-9-oct-26/` มีไฟล์:

| ไฟล์ | หน้าที่ |
| --- | --- |
| `CMakeLists.txt` | ไฟล์โปรเจกต์ ESP-IDF |
| `main/main.c` | ซอร์สโค้ดของแลป |
| `main/CMakeLists.txt` | ลงทะเบียนคอมโพเนนต์ `main` และ component ที่ต้องใช้ |
| `diagram.json` | วงจรสำหรับ Wokwi |
| `wokwi.toml` | บอก Wokwi ว่าจะโหลด firmware จากไหน |
| `sdkconfig.defaults` | ค่าตั้งต้นของ sdkconfig (เฉพาะแลปที่ต้องใช้) |

## วิธี build และรัน

เปิด `ESP-IDF: Open ESP-IDF Terminal` ใน VS Code แล้วรัน:

```bash
cd labs-9-oct-26/01-wifi-sta
source ~/.espressif/tools/activate_idf_v6.0.2.sh
idf.py set-target esp32
idf.py build
```

จากนั้น `F1` → `Wokwi: Select Config File` → เลือก `labs-9-oct-26/01-wifi-sta/wokwi.toml` → `Wokwi: Start Simulator`

ทำแบบเดียวกันกับ `02-i2c-lcd`, `03-isr-queue` และ `04-spiffs`
