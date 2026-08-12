#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"


// ========================================
// GPIO ของ 7 Segment
// A B C D E F G
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


// ขาเลือกหลัก
#define D1 GPIO_NUM_26
#define D2 GPIO_NUM_27

// Switch
#define SW GPIO_NUM_25


// ========================================
// Pattern เลข 0 - 9
// Common Cathode
// 1 = ติด
// 0 = ดับ
// ========================================

static const int number[10][7] = {

    // A B C D E F G

    {1,1,1,1,1,1,0},  // 0
    {0,1,1,0,0,0,0},  // 1
    {1,1,0,1,1,0,1},  // 2
    {1,1,1,1,0,0,1},  // 3
    {0,1,1,0,0,1,1},  // 4
    {1,0,1,1,0,1,1},  // 5
    {1,0,1,1,1,1,1},  // 6
    {1,1,1,0,0,0,0},  // 7
    {1,1,1,1,1,1,1},  // 8
    {1,1,1,1,0,1,1}   // 9

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
// Task 1
// อ่าน Switch และนับเลข
// ========================================

void counter_task(void *pvParameter)
{
    while(1)
    {
        // Pull-up
        // ไม่กด = 1
        // กด = 0

        if(gpio_get_level(SW) == 0)
        {
            // debounce
            vTaskDelay(pdMS_TO_TICKS(30));

            // เช็คอีกทีว่ายังกดอยู่
            if(gpio_get_level(SW) == 0)
            {
                count++;

                if(count > 99)
                {
                    count = 0;
                }

                printf("Count = %d\n", count);


                // รอจนกว่าจะปล่อยปุ่ม
                while(gpio_get_level(SW) == 0)
                {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


// ========================================
// Task 2
// MUX 7 Segment
// ========================================

void display_task(void *pvParameter)
{
    while(1)
    {
        int value = count;

        // แยกหลัก
        int tens = value / 10;
        int ones = value % 10;


        // ==================================
        // หลักสิบ D1
        // ==================================

        // ปิดทั้งสองหลักก่อน
        gpio_set_level(D1, 1);
        gpio_set_level(D2, 1);

        // เตรียมเลขหลักสิบ
        display_num(tens);

        // เปิด D1
        gpio_set_level(D1, 0);

        // แสดงประมาณ 5 ms
        vTaskDelay(pdMS_TO_TICKS(5));


        // ==================================
        // หลักหน่วย D2
        // ==================================

        // ปิดทั้งสองหลักก่อน
        gpio_set_level(D1, 1);
        gpio_set_level(D2, 1);

        // เตรียมเลขหลักหน่วย
        display_num(ones);

        // เปิด D2
        gpio_set_level(D2, 0);

        // แสดงประมาณ 5 ms
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}


// ========================================
// app_main
// ========================================

void app_main(void)
{

    // ==================================
    // ตั้ง GPIO A-G เป็น OUTPUT
    // ==================================

    for(int i = 0; i < 7; i++)
    {
        gpio_reset_pin(seg_pins[i]);

        gpio_set_direction(
            seg_pins[i],
            GPIO_MODE_OUTPUT
        );
    }


    // ==================================
    // ตั้ง D1 เป็น OUTPUT
    // ==================================

    gpio_reset_pin(D1);
    gpio_set_direction(D1, GPIO_MODE_OUTPUT);


    // ==================================
    // ตั้ง D2 เป็น OUTPUT
    // ==================================

    gpio_reset_pin(D2);
    gpio_set_direction(D2, GPIO_MODE_OUTPUT);


    // ปิดทั้ง 2 digit ตอนเริ่ม
    gpio_set_level(D1, 1);
    gpio_set_level(D2, 1);


    // ==================================
    // ตั้ง Switch เป็น INPUT
    // ==================================

    gpio_reset_pin(SW);

    gpio_set_direction(
        SW,
        GPIO_MODE_INPUT
    );

    gpio_set_pull_mode(
        SW,
        GPIO_PULLUP_ONLY
    );


    // ==================================
    // สร้าง Counter Task
    // Priority 1
    // Core 0
    // ==================================

    xTaskCreatePinnedToCore(
        counter_task,
        "Counter Task",
        2048,
        NULL,
        1,
        NULL,
        0
    );


    // ==================================
    // สร้าง Display Task
    // Priority 2
    // Core 1
    // ==================================

    xTaskCreatePinnedToCore(
        display_task,
        "Display Task",
        2048,
        NULL,
        2,
        NULL,
        1
    );
}