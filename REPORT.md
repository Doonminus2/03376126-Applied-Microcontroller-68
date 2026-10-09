# รายงานผลการทดลอง: การจำลอง ESP32 ด้วย ESP-IDF และ FreeRTOS บน Wokwi

> งานทดแทนการเข้าเรียนวันที่ 9 ต.ค. 2569 (5 คะแนน)
> คู่มือการทดลอง: [docs/lab-manual-th.html](docs/lab-manual-th.html) (ฉบับภาษาอังกฤษ: [docs/lab-manual-en.html](docs/lab-manual-en.html))

| หัวข้อ | รายละเอียด |
| --- | --- |
| ชื่อ-นามสกุล | _(กรอก)_ |
| รหัสนักศึกษา | _(กรอก)_ |
| รายวิชา | 03376126 Applied Microcontroller |
| ผู้สอน | Piya Jitthammapirom |
| วันที่ทำการทดลอง | _(กรอก)_ |
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
cd labs/01-wifi-sta        # เปลี่ยนเป็นโฟลเดอร์ของแลปที่ต้องการ
idf.py set-target esp32
idf.py build
```

จากนั้นใน VS Code: `F1` → `Wokwi: Select Config File` → เลือก `labs/<แลป>/wokwi.toml` → `Wokwi: Start Simulator`

---

## 1. โมดูลที่ 1: การเชื่อมต่อ Wi-Fi ผ่าน Wokwi-GUEST (STA)

**โค้ด:** [labs/01-wifi-sta/main/main.c](labs/01-wifi-sta/main/main.c) · **วงจร:** [labs/01-wifi-sta/diagram.json](labs/01-wifi-sta/diagram.json)

### 1.1 วัตถุประสงค์
เข้าใจลำดับการเริ่มต้น Wi-Fi ของ ESP32 ในโหมด Station: NVS, `esp_netif`, default event loop, event callback และ FreeRTOS Event Group

### 1.2 หลักการทำงาน / สิ่งที่เรียนรู้
_(อธิบายด้วยคำพูดของตัวเอง เช่น ลำดับการทำงานตั้งแต่ `nvs_flash_init()` จนได้ IP และหน้าที่ของ Event Group)_

### 1.3 ขั้นตอนการทดลอง
1. _(สรุปสิ่งที่ทำจริง)_
2.
3.

### 1.4 ผลการทดลอง

![ผลการจำลอง Lab 1](docs/images/lab01/result.png)

```text
(วาง Serial Monitor log ที่นี่ เช่น บรรทัด "Successfully acquired IP address: ...")
```

### 1.5 ตรวจสอบตนเอง

| หัวข้อการประเมิน | ผลลัพธ์ที่ต้องตรวจสอบ | ผล |
| --- | --- | --- |
| การเริ่มต้น NVS | `nvs_flash_init()` คืนค่า `ESP_OK` | ☐ ผ่าน ☐ ไม่ผ่าน |
| การเชื่อมต่อ SSID | เชื่อมต่อ `Wokwi-GUEST` สำเร็จ | ☐ ผ่าน ☐ ไม่ผ่าน |
| การได้รับ IP จาก DHCP | ได้ IPv4 ผ่าน `IP_EVENT_STA_GOT_IP` | ☐ ผ่าน ☐ ไม่ผ่าน |
| การซิงโครไนซ์อีเวนต์ | Event Group ปลดบล็อก `app_main` | ☐ ผ่าน ☐ ไม่ผ่าน |

### 1.6 วิเคราะห์ผล
_(IP ที่ได้คืออะไร ใช้เวลาเชื่อมต่อนานเท่าไร มีการ retry หรือไม่ เพราะอะไร)_

---

## 2. โมดูลที่ 2: ไดรเวอร์ I2C Master และจอ LCD

**โค้ด:** [labs/02-i2c-lcd/main/main.c](labs/02-i2c-lcd/main/main.c) · **วงจร:** [labs/02-i2c-lcd/diagram.json](labs/02-i2c-lcd/diagram.json)

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

**โค้ด:** [labs/03-isr-queue/main/main.c](labs/03-isr-queue/main/main.c) · **วงจร:** [labs/03-isr-queue/diagram.json](labs/03-isr-queue/diagram.json)

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

**โค้ด:** [labs/04-spiffs/main/main.c](labs/04-spiffs/main/main.c) · **ตารางพาร์ทิชัน:** [labs/04-spiffs/partitions.csv](labs/04-spiffs/partitions.csv)

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
