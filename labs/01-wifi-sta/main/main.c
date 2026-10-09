/*
 * Lab 1: Wi-Fi Connectivity & Event Loop (STA) via Wokwi-GUEST
 * อ้างอิง: docs/lab-manual-th.html (หัวข้อ 1)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

void app_main(void)
{
    /* TODO: เขียนโค้ดตามขั้นตอนในคู่มือ
     * 1. เริ่มต้น NVS Flash (nvs_flash_init / nvs_flash_erase)
     * 2. esp_netif_init() + esp_event_loop_create_default()
     * 3. esp_netif_create_default_wifi_sta()
     * 4. ลงทะเบียน handler ของ WIFI_EVENT และ IP_EVENT
     * 5. ตั้งค่า wifi_config_t (SSID "Wokwi-GUEST") แล้ว esp_wifi_start()
     * 6. รอ IP ด้วย xEventGroupWaitBits()
     */
}
