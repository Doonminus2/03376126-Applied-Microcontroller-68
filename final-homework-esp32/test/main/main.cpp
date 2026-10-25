// Unity test app บน ESP32: รันเทสต์ทั้งหมดครั้งเดียวแล้วพักไว้ให้อ่านผลบน serial
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tests.hpp"

extern "C" void app_main(void) {
    runAllTests();
    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
}
