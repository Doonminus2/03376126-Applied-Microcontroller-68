#include <stdio.h>

#include "driver/gpio.h"
#include "esp_rom_sys.h"

// ================================
// GPIO A-G
// ================================

static const gpio_num_t seg_pins[7] = {
    GPIO_NUM_16,   // A
    GPIO_NUM_17,   // B
    GPIO_NUM_18,   // C
    GPIO_NUM_19,   // D
    GPIO_NUM_21,   // E
    GPIO_NUM_22,   // F
    GPIO_NUM_23    // G
};

#define D1 GPIO_NUM_26
#define D2 GPIO_NUM_27

#define SW GPIO_NUM_25


// ================================
// เลข 0-9
// Common Cathode
// ================================

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


// ================================
// แสดงเลข 1 หลักลง A-G
// ================================

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


// ================================
// MUX จอ 1 รอบ
// ================================

void display_mux(int value)
{
    int tens = value / 10;
    int ones = value % 10;


    // ----------------
    // หลักสิบ
    // ----------------

    // ปิดก่อนทั้งสองหลัก
    gpio_set_level(D1, 1);
    gpio_set_level(D2, 1);

    display_num(tens);

    // เปิด D1
    gpio_set_level(D1, 0);

    esp_rom_delay_us(5000);   // 5 ms


    // ----------------
    // หลักหน่วย
    // ----------------

    // ปิดก่อนทั้งสองหลัก
    gpio_set_level(D1, 1);
    gpio_set_level(D2, 1);

    display_num(ones);

    // เปิด D2
    gpio_set_level(D2, 0);

    esp_rom_delay_us(5000);   // 5 ms
}


// ================================
// app_main
// ================================

void app_main(void)
{
    int count = 0;


    // ================================
    // ตั้ง A-G เป็น OUTPUT
    // ================================

    for(int i = 0; i < 7; i++)
    {
        gpio_reset_pin(seg_pins[i]);

        gpio_set_direction(
            seg_pins[i],
            GPIO_MODE_OUTPUT
        );
    }


    // ================================
    // ตั้ง D1 D2 เป็น OUTPUT
    // ================================

    gpio_reset_pin(D1);
    gpio_set_direction(D1, GPIO_MODE_OUTPUT);

    gpio_reset_pin(D2);
    gpio_set_direction(D2, GPIO_MODE_OUTPUT);


    // ปิดทั้งสองหลักไว้ก่อน
    gpio_set_level(D1, 1);
    gpio_set_level(D2, 1);


    // ================================
    // Switch เป็น INPUT + Pull-up
    // ================================

    gpio_reset_pin(SW);

    gpio_set_direction(
        SW,
        GPIO_MODE_INPUT
    );

    gpio_set_pull_mode(
        SW,
        GPIO_PULLUP_ONLY
    );


    // ================================
    // Loop หลัก
    // ================================

    while(1)
    {
        // MUX 7-segment ตลอดเวลา
        display_mux(count);


        // ถ้ากด Switch
        // Pull-up: กด = 0
        if(gpio_get_level(SW) == 0)
        {
            // debounce ประมาณ 30ms
            for(int i = 0; i < 3; i++)
            {
                display_mux(count);
            }


            // เช็คอีกครั้งว่ายังกดจริง
            if(gpio_get_level(SW) == 0)
            {
                count++;

                if(count > 99)
                {
                    count = 0;
                }

                printf("Count = %d\n", count);


                // รอจนกว่าจะปล่อยปุ่ม
                // แต่ยัง MUX จอต่อไป
                while(gpio_get_level(SW) == 0)
                {
                    display_mux(count);
                }
            }
        }
    }
}