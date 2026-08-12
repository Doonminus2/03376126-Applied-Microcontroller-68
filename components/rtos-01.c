#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>


void task_1(void *pvParameters)
{
    while (1) {
        printf("Task 1 is running\n");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void task_2(void *pvParameterers)
{
    while (1) {
        printf("Task 2 is running\n");
        vTaskDelay(pdMS_TO_TICKS(3000)); // Delay for 3 seconds
    }
}

void app_main(void)
{
    // กำหนดพิน GPIO สำหรับ 7-segment display
    xTaskCreate(&task_1, "Task1" , 2048 , NULL , 1 , NULL);
    xTaskCreate(&task_2, "Task2" , 2048 , NULL , 1 , NULL);
}


