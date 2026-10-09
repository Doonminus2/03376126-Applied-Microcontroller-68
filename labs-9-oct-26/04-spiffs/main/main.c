/*
 * Lab 4: Flash File Systems (SPIFFS / VFS)
 * อ้างอิง: docs/lab-manual-th.html (หัวข้อ 4)
 */
#include <stdio.h>
#include <string.h>
#include "esp_spiffs.h"
#include "esp_log.h"

void app_main(void)
{
    /* TODO: เขียนโค้ดตามขั้นตอนในคู่มือ
     * 1. ตั้งค่า esp_vfs_spiffs_conf_t (base_path "/spiffs", format_if_mount_failed)
     * 2. esp_vfs_spiffs_register() และตรวจ error
     * 3. เขียนไฟล์ /spiffs/log.txt ด้วย fopen/fprintf/fclose
     * 4. อ่านกลับด้วย fopen/fgets/fclose แล้วเทียบข้อมูล
     * 5. esp_vfs_spiffs_unregister()
     */
}
