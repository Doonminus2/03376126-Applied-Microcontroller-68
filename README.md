<!-- generated-by: gsd-doc-writer -->
# 03376126-Applied-Microcontroller-68

โปรเจกต์รายวิชา Applied Microcontroller สำหรับทดลอง ESP32 ด้วย ESP-IDF และจำลองวงจรผ่าน Wokwi โดยไม่ต้องใช้บอร์ดจริง

โปรแกรมปัจจุบันควบคุม 7-segment แบบ 2 หลักชนิด common cathode ด้วยวิธี multiplex และนับเลข `00` ถึง `99` โดยเปลี่ยนค่าทุก 1 วินาที

## Technology

- ESP32 / ESP-WROOM-32
- ESP-IDF v6.0.2
- FreeRTOS (`vTaskDelay()` และ `xTaskGetTickCount()`)
- Visual Studio Code
- Wokwi Simulator
- ภาษา C

โปรเจกต์นี้ใช้ ESP-IDF โดยตรง ไม่ได้ใช้ Arduino framework

## GPIO mapping

| ESP32 GPIO | 7-segment pin | หน้าที่ |
| --- | --- | --- |
| GPIO17 | A | Segment ด้านบน |
| GPIO18 | B | Segment ขวาบน |
| GPIO19 | C | Segment ขวาล่าง |
| GPIO21 | D | Segment ด้านล่าง |
| GPIO22 | E | Segment ซ้ายล่าง |
| GPIO23 | F | Segment ซ้ายบน |
| GPIO25 | G | Segment ตรงกลาง |
| GPIO26 | DIG1 | เลือกหลักซ้าย |
| GPIO27 | DIG2 | เลือกหลักขวา |

7-segment ใน `diagram.json` ตั้งค่าเป็น `common: "cathode"` และ `digits: "2"`

## Project structure

```text
blink/
├── CMakeLists.txt
├── diagram.json                 # วงจร ESP32 และ 7-segment สำหรับ Wokwi
├── wokwi.toml                   # ตำแหน่ง firmware และ ELF ที่ Wokwi โหลด
├── sdkconfig                    # ESP-IDF project configuration
├── main/
│   ├── CMakeLists.txt
│   └── blink_example_main.c     # GPIO, multiplex และ counter 00-99
└── build/                       # ไฟล์ที่สร้างจาก idf.py build
```

## Prerequisites

ติดตั้งโปรแกรมต่อไปนี้บน macOS ก่อนเริ่มใช้งาน:

- [Visual Studio Code](https://code.visualstudio.com/)
- Git
- Python 3
- ESP-IDF v6.0.2 และ toolchain สำหรับ ESP32
- บัญชี Wokwi สำหรับเปิดใช้งาน Wokwi Simulator extension

ตรวจสอบเครื่องมือพื้นฐานได้ด้วย:

```bash
git --version
python3 --version
code --version
```

หากคำสั่ง `code` ยังใช้ไม่ได้ ให้เปิด Command Palette ใน VS Code แล้วเลือก `Shell Command: Install 'code' command in PATH`

## Install VS Code extensions

Extension ที่จำเป็นมี 2 ตัว:

| Extension | Extension ID | หน้าที่ |
| --- | --- | --- |
| Espressif IDF | `espressif.esp-idf-extension` | ติดตั้ง/เลือก ESP-IDF, build และจัดการโปรเจกต์ ESP32 |
| Wokwi Simulator | `wokwi.wokwi-vscode` | จำลอง ESP32 และวงจรจาก `diagram.json` |

ติดตั้งจาก Extensions view ใน VS Code หรือใช้ terminal:

```bash
code --install-extension espressif.esp-idf-extension
code --install-extension wokwi.wokwi-vscode
```

หลังติดตั้ง Wokwi ให้เปิด Command Palette (`Cmd+Shift+P`) แล้วเลือก:

```text
Wokwi: Request a New License
```

ทำตามขั้นตอนใน browser จน VS Code แสดงว่า license ถูกเปิดใช้งานแล้ว

## Install and configure ESP-IDF v6.0.2

1. เปิด Command Palette ใน VS Code
2. เลือก `ESP-IDF: Open ESP-IDF Installation Manager`
3. ติดตั้ง ESP-IDF v6.0.2 พร้อม tools และ Python environment
4. เลือก `ESP-IDF: Select Current ESP-IDF Version`
5. เลือก setup ของ ESP-IDF v6.0.2 สำหรับ workspace นี้
6. เลือก `ESP-IDF: Set Espressif Device Target` แล้วเลือก `esp32`
7. ตรวจสอบการติดตั้งด้วย `ESP-IDF: Doctor Command`

เมื่อต้องใช้คำสั่ง `idf.py` ให้เปิด terminal ผ่าน Command Palette:

```text
ESP-IDF: Open ESP-IDF Terminal
```

Terminal นี้จะตั้งค่า `IDF_PATH`, toolchain และ Python environment ให้อัตโนมัติ

## Install idf-wokwi with pip

แพ็กเกจ `idf-wokwi` เพิ่มคำสั่ง `idf.py wokwi` ให้ ESP-IDF v6.0 ขึ้นไป ควรติดตั้งภายใน ESP-IDF Terminal เพื่อให้ลงใน Python environment ที่ถูกต้อง:

```bash
python -m pip install --upgrade idf-wokwi
```

ตรวจสอบว่าติดตั้งสำเร็จ:

```bash
idf.py --help
```

ในรายการคำสั่งควรมี action ชื่อ `wokwi`

> การติดตั้ง `idf-wokwi` ใช้สำหรับรัน Wokwi จาก terminal/CI ส่วนการกด Start Simulator ภายใน VS Code ใช้ Wokwi Simulator extension

## Wokwi configuration

### `wokwi.toml`

ไฟล์นี้อยู่ที่ root ของโปรเจกต์และบอก Wokwi ว่าต้องโหลด firmware ใด:

```toml
[wokwi]
version = 1
firmware = 'build/flasher_args.json'
elf = 'build/blink.elf'
```

- `build/flasher_args.json` ถูกสร้างโดย `idf.py build` และระบุตำแหน่ง bootloader, partition table และ application binary
- `build/blink.elf` ใช้ข้อมูล symbol ของโปรแกรม
- ชื่อ `blink` มาจาก `project(blink)` ใน `CMakeLists.txt`

### `diagram.json`

วงจรใช้ ESP32 DevKitC V4 และ 7-segment แบบ 2 หลักหนึ่งตัว:

```json
{
  "parts": [
    { "type": "board-esp32-devkit-c-v4", "id": "esp" },
    {
      "type": "wokwi-7segment",
      "id": "display",
      "attrs": {
        "common": "cathode",
        "digits": "2",
        "color": "red"
      }
    }
  ]
}
```

ขา segment ใช้ชื่อ `A` ถึง `G` และขาเลือกหลักต้องใช้ `DIG1`/`DIG2` ไม่ใช่ `COM` หรือ `COM.1`

## Build the project

เปิด ESP-IDF Terminal ที่ root ของโปรเจกต์ แล้วรัน:

```bash
idf.py build
```

เมื่อต้องการล้าง build เดิมทั้งหมดก่อน build ใหม่:

```bash
idf.py fullclean
idf.py build
```

หลัง build สำเร็จ ตรวจสอบไฟล์ที่ Wokwi ต้องใช้:

```bash
ls -lh build/flasher_args.json build/blink.bin build/blink.elf
```

## Run with Wokwi Simulator in VS Code

1. เปิดโฟลเดอร์ `blink` เป็น workspace root ใน VS Code
2. รัน `idf.py build` ทุกครั้งหลังแก้ source code
3. เปิด Command Palette (`Cmd+Shift+P`)
4. หาก workspace มีหลาย config ให้เลือก `Wokwi: Select Config File` แล้วเลือก `wokwi.toml` ที่ root
5. เลือก `Wokwi: Start Simulator`

ผลลัพธ์ที่คาดหวัง:

- เห็น ESP32 ต่อกับ 7-segment แบบ 2 หลักหนึ่งตัว
- เห็นสายทั้งหมด 9 เส้น
- ตัวเลขเริ่มที่ `00`
- ตัวเลขเปลี่ยนเป็น `01`, `02`, ... ทุก 1 วินาที
- หลัง `99` จะกลับไป `00`

เมื่อแก้ source code ให้ build ใหม่ แล้วเลือก `Wokwi: Restart Simulator` เพื่อให้แน่ใจว่า simulator โหลด firmware ล่าสุด

## Run Wokwi from the terminal

วิธีนี้ต้องติดตั้ง `idf-wokwi` และมี Wokwi CI API token:

```bash
export WOKWI_CLI_TOKEN="your-wokwi-ci-token"
idf.py build
idf.py wokwi --diagram-file diagram.json
```

อย่าเขียน token จริงลงใน README, source code หรือ commit ของ Git ให้ตั้งผ่าน environment variable เท่านั้น

## Validate the configuration

ตรวจ JSON syntax:

```bash
python -m json.tool diagram.json >/dev/null
```

ตรวจ build artifacts:

```bash
test -s build/flasher_args.json
test -s build/blink.bin
test -s build/blink.elf
```

build โปรเจกต์แบบเต็มเพื่อยืนยัน source และ configuration:

```bash
idf.py build
```

## Troubleshooting

### `idf.py: command not found`

เปิด terminal ด้วย `ESP-IDF: Open ESP-IDF Terminal` หรือ activate ESP-IDF environment ก่อนรันคำสั่ง

### Wokwi แจ้งว่าไม่พบ firmware หรือ ELF

รัน `idf.py build` และตรวจว่า `build/flasher_args.json` กับ `build/blink.elf` มีอยู่จริง จากนั้นตรวจ path ใน `wokwi.toml`

### เห็น component แต่ไม่เห็นสาย

- ตรวจว่า VS Code เปิด `diagram.json` จาก workspace นี้
- เลือก `Wokwi: Select Config File` ให้เป็น `wokwi.toml` ที่ root
- ใช้ board type `board-esp32-devkit-c-v4`
- ใช้ขา `DIG1` และ `DIG2` สำหรับ display หลายหลัก
- หยุด simulator เดิมแล้วเลือก `Wokwi: Start Simulator` ใหม่

### ตัวเลขไม่เปลี่ยนหรือ simulator ยังใช้โปรแกรมเก่า

```bash
idf.py build
```

จากนั้นเลือก `Wokwi: Restart Simulator` หากยังเห็นค่าเดิม ให้ปิดแท็บ Simulator แล้ว Start ใหม่

### `idf.py` ไม่มีคำสั่ง `wokwi`

เปิด ESP-IDF Terminal แล้วติดตั้งแพ็กเกจอีกครั้ง:

```bash
python -m pip install --upgrade idf-wokwi
```

ตรวจด้วยว่า ESP-IDF ที่เลือกเป็น v6.0 หรือใหม่กว่า

## References

- [ESP-IDF extension for VS Code](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/)
- [Wokwi for VS Code](https://docs.wokwi.com/vscode/getting-started)
- [Wokwi project configuration](https://docs.wokwi.com/vscode/project-config)
- [idf-wokwi usage](https://docs.wokwi.com/wokwi-ci/idf-wokwi-usage)
- [Wokwi 7-segment reference](https://docs.wokwi.com/parts/wokwi-7segment)
