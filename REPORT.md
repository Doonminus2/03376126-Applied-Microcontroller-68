# รายงานผลการทดลอง: การจำลอง ESP32 ด้วย ESP-IDF และ FreeRTOS บน Wokwi

> งานทดแทนการเข้าเรียนวันที่ 9 ต.ค. 2569 (5 คะแนน)
> คู่มือการทดลอง: [docs/lab-manual-th.html](docs/lab-manual-th.html) (ฉบับภาษาอังกฤษ: [docs/lab-manual-en.html](docs/lab-manual-en.html))

| หัวข้อ | รายละเอียด |
| --- | --- |
| ชื่อ-นามสกุล | ชิษณุพัศก์ วายุสุวรณวิทย์ |
| รหัสนักศึกษา | 68030063 |
| รายวิชา | 03376126 Applied Microcontroller |
| ผู้สอน | Piya Jitthammapirom |
| วันที่ทำการทดลอง | 9/10/26    |
| Branch | `Wi-Fi-Connectivity-9-oct-26` |

---

## 0. สภาพแวดล้อมการทดลอง

| รายการ | เวอร์ชัน / รายละเอียด |
| --- | --- |
| บอร์ด (จำลอง) | ESP32 DevKitC V4 (`board-esp32-devkit-c-v4`) |
| ESP-IDF | _(เช่น v6.0.2 — ดูได้จาก `idf.py --version`)_ |
| VS Code Extension | Espressif IDF, Wokwi Simulator |
| ระบบปฏิบัติการ | _(เช่น macOS 15)_ |

**วิธี build และรันแต่ละแลป** (ทำใน ESP-IDF Terminal):

```bash
cd labs-9-oct-26/01-wifi-sta        # เปลี่ยนเป็นโฟลเดอร์ของแลปที่ต้องการ
idf.py set-target esp32
idf.py build
```

จากนั้นใน VS Code: `F1` → `Wokwi: Select Config File` → เลือก `labs-9-oct-26/<แลป>/wokwi.toml` → `Wokwi: Start Simulator`

---

## 1. โมดูลที่ 1: การเชื่อมต่อ Wi-Fi ผ่าน Wokwi-GUEST (STA)

**โค้ด:** [labs-9-oct-26/01-wifi-sta/main/main.c](labs-9-oct-26/01-wifi-sta/main/main.c) · **วงจร:** [labs-9-oct-26/01-wifi-sta/diagram.json](labs-9-oct-26/01-wifi-sta/diagram.json)

### 1.1 วัตถุประสงค์
เข้าใจลำดับการเริ่มต้น Wi-Fi ของ ESP32 ในโหมด Station: NVS, `esp_netif`, default event loop, event callback และ FreeRTOS Event Group

### 1.2 หลักการทำงาน / สิ่งที่เรียนรู้
_(อธิบายด้วยคำพูดของตัวเอง เช่น ลำดับการทำงานตั้งแต่ `nvs_flash_init()` จนได้ IP และหน้าที่ของ Event Group)_

### 1.3 ขั้นตอนการทดลอง
1. _(สรุปสิ่งที่ทำจริง)_
2.
3.

### 1.4 ผลการทดลอง

![ผลการจำลอง Lab 1](docs/images/lab01/image.png)

```text
(วาง Serial Monitor log ที่นี่ 

st:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0040,len:6272
load:0x40078000,len:15820
load:0x40080400,len:3920
entry 0x40080644
I (12) boot: ESP-IDF v6.0.2 2nd stage bootloader
I (13) boot: compile time Oct  9 2026 20:22:39
I (13) boot: Multicore bootloader
I (14) boot: chip revision: v3.0
I (16) boot.esp32: SPI Speed      : 40MHz
I (19) boot.esp32: SPI Mode       : DIO
I (23) boot.esp32: SPI Flash Size : 2MB
I (27) boot: Enabling RNG early entropy source...
I (31) boot: Partition Table:
I (34) boot: ## Label            Usage          Type ST Offset   Length
I (40) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (46) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (53) boot:  2 factory          factory app      00 00 00010000 00177000
I (60) boot: End of partition table
I (63) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=19178h (102776) map
I (205) esp_image: segment 1: paddr=000291a0 vaddr=3ffb0000 size=04528h ( 17704) load
I (233) esp_image: segment 2: paddr=0002d6d0 vaddr=40080000 size=02948h ( 10568) load
I (251) esp_image: segment 3: paddr=00030020 vaddr=400d0020 size=85c8ch (547980) map
I (968) esp_image: segment 4: paddr=000b5cb4 vaddr=40082948 size=12cc0h ( 76992) load
I (1088) esp_image: segment 5: paddr=000c897c vaddr=50000000 size=00028h (    40) load
I (1124) boot: Loaded app from partition at offset 0x10000
I (1125) boot: Disabling RNG early entropy source...
I (1138) cpu_start: Multicore app
I (570) cpu_start: GPIO 3 and 1 are used as console UART I/O pins
I (571) cpu_start: Pro cpu start user code
I (571) cpu_start: cpu freq: 160000000 Hz
I (573) app_init: Application information:
I (576) app_init: Project name:     lab01_wifi_sta
I (581) app_init: App version:      8b64195-dirty
I (585) app_init: Compile time:     Oct  9 2026 20:22:36
I (590) app_init: ELF file SHA256:  4ac7a2a15...
I (595) app_init: ESP-IDF:          v6.0.2
I (598) efuse_init: Min chip rev:     v0.0
I (602) efuse_init: Max chip rev:     v3.99 
I (606) efuse_init: Chip rev:         v3.0
I (611) heap_init: Initializing. RAM available for dynamic allocation:
I (616) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (622) heap_init: At 3FFB8A40 len 000275C0 (157 KiB): DRAM
I (627) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (632) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (638) heap_init: At 40095608 len 0000A9F8 (42 KiB): IRAM
I (644) spi_flash: detected chip: winbond
I (647) spi_flash: flash io: dio
I (652) main_task: Started on CPU0
I (652) main_task: Calling app_main()
I (702) wifi:wifi driver task: 3ffc0264, prio:23, stack:6656, core=0
I (712) wifi:wifi firmware version: 00ad238
I (722) wifi:wifi certification version: v7.0
I (722) wifi:config NVS flash: enabled
I (722) wifi:config nano formatting: disabled
I (722) wifi:Init data frame dynamic rx buffer num: 32
I (722) wifi:Init static rx mgmt buffer num: 5
I (732) wifi:Init management short buffer num: 32
I (732) wifi:Init dynamic tx buffer num: 32
I (742) wifi:Init static rx buffer size: 1600
I (742) wifi:Init static rx buffer num: 10
I (742) wifi:Init dynamic rx buffer num: 32
I (752) wifi_init: rx ba win: 6
I (752) wifi_init: accept mbox: 6
I (752) wifi_init: tcpip mbox: 32
I (762) wifi_init: udp mbox: 6
I (762) wifi_init: tcp mbox: 6
I (762) wifi_init: tcp tx win: 5760
I (772) wifi_init: tcp rx win: 5760
I (772) wifi_init: tcp mss: 1440
I (772) wifi_init: WiFi IRAM OP enabled
I (782) wifi_init: WiFi RX IRAM OP enabled
I (882) phy_init: phy_version 4863,a3a4459,Oct 28 2025,14:30:06
W (882) phy_init: failed to load RF calibration data (0x1102), falling back to full calibration
I (1022) phy_init: Saving new calibration data due to checksum failure or outdated calibration data, mode(2)
I (1062) wifi:mode : sta (24:0a:c4:00:01:10)
I (1062) wifi:enable tsf
I (1062) WIFI_PROJECT: Wi-Fi initialization finished. Waiting for connection...
I (1682) wifi:new:<6,0>, old:<1,0>, ap:<255,255>, sta:<6,0>, prof:1, snd_ch_cfg:0x0
I (1682) wifi:state: init -> auth (0xb0)
I (1682) wifi:state: auth -> assoc (0x0)
I (1692) wifi:state: assoc -> run (0x10)
I (1692) wifi:connected with Wokwi-GUEST, aid = 1, channel 6, BW20, bssid = 42:13:37:55:aa:01
I (1692) wifi:security: Open Auth, phy: bg, rssi: -80, cipher(pairwise:0x0, group:0x0), pmf:0
I (1722) wifi:pm start, type: 1

I (1732) wifi:dp: 1, bi: 102400, li: 3, scale listen interval from 307200 us to 307200 us
I (1732) wifi:dp: 2, bi: 102400, li: 4, scale listen interval from 307200 us to 409600 us
I (1732) wifi:AP's beacon interval = 102400 us, DTIM period = 2
I (2802) esp_netif_handlers: sta ip: 10.13.37.2, mask: 255.255.255.0, gw: 10.13.37.1
I (2802) WIFI_PROJECT: Successfully acquired IP address: 10.13.37.2
I (2802) WIFI_PROJECT: Network pipeline ready for HTTP/MQTT traffic!
I (2802) main_task: Returned from app_main(

)
```

### 1.5 ตรวจสอบตนเอง

| หัวข้อการประเมิน | ผลลัพธ์ที่ต้องตรวจสอบ | ผล |
| --- | --- | --- |
| การเริ่มต้น NVS | `nvs_flash_init()` คืนค่า `ESP_OK` |  ผ่าน ☑ ไม่ผ่าน |
| การเชื่อมต่อ SSID | เชื่อมต่อ `Wokwi-GUEST` สำเร็จ | ☑ ผ่าน ☐ ไม่ผ่าน |
| การได้รับ IP จาก DHCP | ได้ IPv4 ผ่าน `IP_EVENT_STA_GOT_IP` | ☑ ผ่าน ☐ ไม่ผ่าน |
| การซิงโครไนซ์อีเวนต์ | Event Group ปลดบล็อก `app_main` | ☑ ผ่าน ☐ ไม่ผ่าน |

### 1.6 วิเคราะห์ผล
_(IP ที่ได้คืออะไร ใช้เวลาเชื่อมต่อนานเท่าไร มีการ retry หรือไม่ เพราะอะไร)_

---

## 2. โมดูลที่ 2: ไดรเวอร์ I2C Master และจอ LCD

**โค้ด:** [labs-9-oct-26/02-i2c-lcd/main/main.c](labs-9-oct-26/02-i2c-lcd/main/main.c) · **วงจร:** [labs-9-oct-26/02-i2c-lcd/diagram.json](labs-9-oct-26/02-i2c-lcd/diagram.json)

### 2.1 วัตถุประสงค์
ใช้ API `driver/i2c_master.h` ของ ESP-IDF สร้างบัส I2C และสื่อสารกับ LCD1602 ผ่านโมดูล PCF8574 ที่ที่อยู่ `0x27`

### 2.2 การต่อวงจร

| ขา ESP32 | ขา LCD | หน้าที่ |
| --- | --- | --- |
| GPIO 21 | SDA | สายข้อมูล I2C |
| GPIO 22 | SCL | สายสัญญาณนาฬิกา I2C (100 kHz) |
| 5V | VCC | ไฟเลี้ยง +5V |
| GND | GND | กราวด์ร่วม |

### 2.3 หลักการทำงาน / สิ่งที่เรียนรู้
_(bus handle กับ device handle ต่างกันอย่างไร ทำไมบัส I2C ต้องมี pull-up)_

### 2.4 ขั้นตอนการทดลอง
1.
2.
3.

### 2.5 ผลการทดลอง

![ผลการจำลอง Lab 2](docs/images/lab02/result.png)

```text
(วาง Serial Monitor log ที่นี่)
```

### 2.6 ตรวจสอบตนเอง

| หัวข้อการประเมิน | ผลลัพธ์ที่ต้องตรวจสอบ | ผล |
| --- | --- | --- |
| การเริ่มต้น I2C Master | `i2c_new_master_bus()` คืนค่า `ESP_OK` | ☐ ผ่าน ☐ ไม่ผ่าน |
| การลงทะเบียนอุปกรณ์ Slave | ผูกกับที่อยู่ `0x27` ที่ 100 kHz | ☐ ผ่าน ☐ ไม่ผ่าน |
| การตอบกลับ ACK | `i2c_master_transmit()` ไม่ timeout | ☐ ผ่าน ☐ ไม่ผ่าน |

### 2.7 วิเคราะห์ผล
_(ได้ ACK หรือไม่ ถ้าลองเปลี่ยนที่อยู่เป็นค่าอื่นจะเกิดอะไรขึ้น)_

---

## 3. โมดูลที่ 3: อินเทอร์รัปต์แบบเลื่อนเวลา (ISR + Queue)

**โค้ด:** [labs-9-oct-26/03-isr-queue/main/main.c](labs-9-oct-26/03-isr-queue/main/main.c) · **วงจร:** [labs-9-oct-26/03-isr-queue/diagram.json](labs-9-oct-26/03-isr-queue/diagram.json)

### 3.1 วัตถุประสงค์
ใช้อินเทอร์รัปต์ GPIO แบบขอบขาลง (falling edge) ร่วมกับ `IRAM_ATTR` และส่งเหตุการณ์ไปยัง worker task ผ่าน FreeRTOS Queue แทนการ polling

### 3.2 การต่อวงจร
ปุ่มกด ขาหนึ่งต่อ GPIO 4 (เปิด pull-up ภายใน) อีกขาต่อ GND → กดปุ่มแล้วสัญญาณเปลี่ยนจาก HIGH เป็น LOW

### 3.3 หลักการทำงาน / สิ่งที่เรียนรู้
_(อธิบายเส้นทางข้อมูล ปุ่ม → ISR → Queue → Task และเหตุผลที่ไม่ทำงานหนักใน ISR)_

### 3.4 ขั้นตอนการทดลอง
1.
2.
3.

### 3.5 ผลการทดลอง

![ผลการจำลอง Lab 3](docs/images/lab03/result.png)

```text
(วาง Serial Monitor log ที่นี่ เช่น "ISR Event Received! Button on GPIO 4 pressed N times.")
```

### 3.6 ตรวจสอบตนเอง

| หัวข้อการประเมิน | ผลลัพธ์ที่ต้องตรวจสอบ | ผล |
| --- | --- | --- |
| ตำแหน่งการทำงานของ ISR | handler ระบุ `IRAM_ATTR` | ☐ ผ่าน ☐ ไม่ผ่าน |
| การส่งต่องานผ่านคิว | `xQueueSendFromISR()` ทำงานโดยไม่แครช | ☐ ผ่าน ☐ ไม่ผ่าน |
| การปลดบล็อก Task | worker task ได้รับข้อความแบบอะซิงโครนัส | ☐ ผ่าน ☐ ไม่ผ่าน |

### 3.7 วิเคราะห์ผล
_(กดหนึ่งครั้งนับได้กี่ครั้ง มีปัญหาปุ่มเด้ง (bounce) หรือไม่)_

---

## 4. โมดูลที่ 4: ระบบไฟล์บนแฟลช (SPIFFS / VFS)

**โค้ด:** [labs-9-oct-26/04-spiffs/main/main.c](labs-9-oct-26/04-spiffs/main/main.c) · **ตารางพาร์ทิชัน:** [labs-9-oct-26/04-spiffs/partitions.csv](labs-9-oct-26/04-spiffs/partitions.csv)

### 4.1 วัตถุประสงค์
สร้างพาร์ทิชัน SPIFFS เมานต์ผ่าน VFS แล้วเขียน/อ่านไฟล์ด้วยคำสั่ง POSIX (`fopen`, `fprintf`, `fgets`, `fclose`)

### 4.2 ตารางพาร์ทิชัน

| Name | Type | SubType | Offset | Size |
| --- | --- | --- | --- | --- |
| nvs | data | nvs | 0x9000 | 0x6000 |
| phy_init | data | phy | 0xf000 | 0x1000 |
| factory | app | factory | 0x10000 | 1M |
| storage | data | spiffs | (อัตโนมัติ) | 0x100000 |

### 4.3 หลักการทำงาน / สิ่งที่เรียนรู้
_(VFS เชื่อม `fopen()` เข้ากับ SPIFFS อย่างไร และ wear-leveling ช่วยอะไร)_

### 4.4 ขั้นตอนการทดลอง
1.
2.
3.

### 4.5 ผลการทดลอง

![ผลการจำลอง Lab 4](docs/images/lab04/result.png)

```text
(วาง Serial Monitor log ที่นี่ เช่น "Read back string from SPIFFS: ...")
```

### 4.6 ตรวจสอบตนเอง

| หัวข้อการประเมิน | ผลลัพธ์ที่ต้องตรวจสอบ | ผล |
| --- | --- | --- |
| การลงทะเบียนพาร์ทิชัน | `esp_vfs_spiffs_register()` คืนค่า `ESP_OK` | ☐ ผ่าน ☐ ไม่ผ่าน |
| การเขียนไฟล์ด้วย POSIX | `fopen()`/`fprintf()` บันทึกลงแฟลชได้ | ☐ ผ่าน ☐ ไม่ผ่าน |
| ความถูกต้องของข้อมูล | ข้อมูลที่อ่านกลับตรงกับที่เขียนทุกตัวอักษร | ☐ ผ่าน ☐ ไม่ผ่าน |

### 4.7 วิเคราะห์ผล
_(ข้อมูลที่อ่านกลับตรงกันหรือไม่ ครั้งแรกที่รันมีการ format หรือไม่)_

---

## 5. การประเมินตนเองแบบรวม (Rubric)

| ด้านการเรียนรู้ | สมรรถนะทางเทคนิคหลัก | คะแนน (เต็ม 5) | หลักฐานจากการทดลอง |
| --- | --- | :---: | --- |
| 1. สแต็กโปรโตคอลเครือข่าย | `esp_netif`, Wi-Fi แบบ event-driven, DHCP | _/5 | _(อ้างอิงผลข้อ 1.4)_ |
| 2. ไดรเวอร์บัสฮาร์ดแวร์ | `driver/i2c_master.h`, การสื่อสารกับ slave | _/5 | _(อ้างอิงผลข้อ 2.5)_ |
| 3. รูปแบบการใช้ RTOS | ISR ใน IRAM + FreeRTOS Queue | _/5 | _(อ้างอิงผลข้อ 3.5)_ |
| 4. ระบบจัดเก็บข้อมูลแบบฝังตัว | พาร์ทิชัน, VFS, POSIX file I/O | _/5 | _(อ้างอิงผลข้อ 4.5)_ |

## 6. คำถามท้ายการทดลอง

1. **ทำไมต้องเริ่มต้น NVS flash ก่อนเริ่มสแต็ก Wi-Fi?**
   _(ตอบด้วยคำพูดของตัวเอง)_
2. **สถาปัตยกรรมของ `driver/i2c.h` แบบเดิม ต่างจาก `driver/i2c_master.h` แบบใหม่อย่างไร?**
   _(ตอบ)_
3. **ทำไมจึงห้ามเรียกคำสั่งที่บล็อก (เช่น `vTaskDelay`, `printf`) ภายใน ISR ที่ระบุ `IRAM_ATTR`?**
   _(ตอบ)_
4. **VFS แปลงคำสั่ง POSIX (`fopen`) ไปยังหน่วยความจำแฟลชได้อย่างไร?**
   _(ตอบ)_

## 7. ปัญหาที่พบและวิธีแก้ไข

| ปัญหา | สาเหตุ | วิธีแก้ |
| --- | --- | --- |
| _(เช่น build error, Wokwi ไม่พบ firmware)_ | | |

## 8. สรุปผลการทดลอง
_(สรุปสั้นๆ ว่าได้เรียนรู้อะไรจากทั้ง 4 โมดูล)_

## 9. เอกสารอ้างอิง
- [Wokwi ESP-IDF Hello Wi-Fi](https://github.com/wokwi/esp32-idf-hello-wifi)
- [ESP-IDF Wi-Fi Station Example](https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/station/main/station_example_main.c)
- [ESP-IDF SPIFFS Example](https://github.com/espressif/esp-idf/blob/master/examples/storage/spiffs/main/spiffs_example_main.c)
- [esp_littlefs](https://github.com/joltwallet/esp_littlefs)
- [Wokwi Docs: ESP32](https://docs.wokwi.com/guides/esp32)
