/*
 * Lab 2: Hardware I2C Master Driver & LCD (PCF8574 @ 0x27)
 * อ้างอิง: docs/lab-manual-th.html (หัวข้อ 2)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

void app_main(void)
{
    /* TODO: เขียนโค้ดตามขั้นตอนในคู่มือ
     * 1. ตั้งค่า i2c_master_bus_config_t (SDA=GPIO21, SCL=GPIO22, pull-up ภายใน)
     * 2. i2c_new_master_bus()
     * 3. ลงทะเบียนอุปกรณ์ที่อยู่ 0x27 ความเร็ว 100 kHz (i2c_master_bus_add_device)
     * 4. ส่งข้อมูลทดสอบด้วย i2c_master_transmit() แล้วตรวจ ACK
     */
}
