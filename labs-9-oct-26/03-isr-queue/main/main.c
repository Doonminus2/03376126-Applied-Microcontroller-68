/*
 * Lab 3: Real-Time Deferred Interrupt Processing & Queues
 * อ้างอิง: docs/lab-manual-th.html (หัวข้อ 3)
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

void app_main(void)
{
    /* TODO: เขียนโค้ดตามขั้นตอนในคู่มือ
     * 1. สร้าง ISR handler (IRAM_ATTR) ที่เรียก xQueueSendFromISR()
     * 2. สร้าง worker task ที่รอ xQueueReceive(..., portMAX_DELAY)
     * 3. xQueueCreate(10, sizeof(uint32_t)) + xTaskCreate()
     * 4. ตั้งค่า GPIO4 เป็น input, pull-up, GPIO_INTR_NEGEDGE
     * 5. gpio_install_isr_service(0) + gpio_isr_handler_add()
     */
}
