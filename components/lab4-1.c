#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"


// ========================================
// GPIO A - G
// ========================================

static const gpio_num_t seg_pins[7] = {
    GPIO_NUM_16,   // A
    GPIO_NUM_17,   // B
    GPIO_NUM_18,   // C
    GPIO_NUM_19,   // D
    GPIO_NUM_21,   // E
    GPIO_NUM_22,   // F
    GPIO_NUM_23    // G
};


// Digit 1 และ Digit 2
#define D1 GPIO_NUM_26
#define D2 GPIO_NUM_27


// ========================================
// รูปแบบเลข 0 - 9
// Common Cathode
// ========================================

static const int number[10][7] = {

    // A B C D E F G

    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1}  // 9
};


// เลขปัจจุบัน
volatile int count = 0;


// ========================================
// ฟังก์ชันแสดงเลข 1 หลัก
// ========================================

void display_num(int num)
{
    for(int i = 0; i < 7; i++)
    {
        gpio_set_level(
            seg_pins[i],
            number[num][i]
        );
    }
}


// ========================================
// Task 1 : นับเลข
// Core 0
// Priority 1
// ========================================

void counter_task(void *pvParameter)
{
    while(1)
    {
        // รอ 1 วินาที
        vTaskDelay(pdMS_TO_TICKS(1000));

        // เพิ่มเลข
        count++;

        // ถ้าเกิน 99 กลับไป 00
        if(count > 99)
        {
            count = 0;
        }

        printf("Count = %d\n", count);
    }
}


// ========================================
// Task 2 : แสดงผล 7 Segment
// Core 1
// Priority 2
// ========================================

void display_task(void *pvParameter)
{
    while(1)
    {
        // เอาค่า count มาแยกหลัก
        int value = count;

        int tens = value / 10;
        int ones = value % 10;


        // ==========================
        // แสดงหลักสิบ
        // ==========================

        // ปิดทั้งสองหลักก่อน
        gpio_set_level(D1, 1);
        gpio_set_level(D2, 1);

        // ใส่เลขหลักสิบลง A-G
        display_num(tens);

        // เปิด D1
        gpio_set_level(D1, 0);

        // ค้างไว้ 5 ms
        vTaskDelay(pdMS_TO_TICKS(5));


        // ==========================
        // แสดงหลักหน่วย
        // ==========================

        // ปิดทั้งสองหลักก่อน
        gpio_set_level(D1, 1);
        gpio_set_level(D2, 1);

        // ใส่เลขหลักหน่วยลง A-G
        display_num(ones);

        // เปิด D2
        gpio_set_level(D2, 0);

        // ค้างไว้ 5 ms
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}


// ========================================
// app_main
// ========================================

void app_main(void)
{
    // ==========================
    // ตั้ง A-G เป็น OUTPUT
    // ==========================

    for(int i = 0; i < 7; i++)
    {
        gpio_reset_pin(seg_pins[i]);

        gpio_set_direction(
            seg_pins[i],
            GPIO_MODE_OUTPUT
        );
    }


    // ==========================
    // ตั้ง D1
    // ==========================

    gpio_reset_pin(D1);
    gpio_set_direction(D1, GPIO_MODE_OUTPUT);


    // ==========================
    // ตั้ง D2
    // ==========================

    gpio_reset_pin(D2);
    gpio_set_direction(D2, GPIO_MODE_OUTPUT);


    // ปิดทั้งสองหลักก่อน
    gpio_set_level(D1, 1);
    gpio_set_level(D2, 1);


    // ==========================
    // สร้าง Task นับเลข
    // ==========================

    xTaskCreatePinnedToCore(
        counter_task,       // Function
        "Counter Task",     // ชื่อ Task
        2048,               // Stack
        NULL,               // Parameter
        1,                  // Priority
        NULL,               // Task Handle
        0                   // Core 0
    );


    // ==========================
    // สร้าง Task แสดงผล
    // ==========================

    xTaskCreatePinnedToCore(
        display_task,
        "Display Task",
        2048,
        NULL,
        2,                  // Priority สูงกว่า Counter
        NULL,
        1                   // Core 1
    );
}