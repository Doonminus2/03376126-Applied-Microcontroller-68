#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define SEG_A GPIO_NUM_17
#define SEG_B GPIO_NUM_18
#define SEG_C GPIO_NUM_19
#define SEG_D GPIO_NUM_21
#define SEG_E GPIO_NUM_22
#define SEG_F GPIO_NUM_23
#define SEG_G GPIO_NUM_25

#define DIGIT1 GPIO_NUM_26
#define DIGIT2 GPIO_NUM_27

#define DIGIT_HOLD_MS 10
#define COUNT_PERIOD_MS 1000

static const gpio_num_t seg_pins[7] = {
    SEG_A,
    SEG_B,
    SEG_C,
    SEG_D,
    SEG_E,
    SEG_F,
    SEG_G
};

static const int number[10][7] = {
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

void set_segment(int num)
{
    for (int i = 0; i < 7; i++) {
        gpio_set_level(seg_pins[i], number[num][i]);
    }
}

void display_two_digit(int value)
{
    int left  = value / 10;
    int right = value % 10;

    // หลักซ้าย
    gpio_set_level(DIGIT1, 0);
    gpio_set_level(DIGIT2, 1);

    set_segment(left);

    vTaskDelay(pdMS_TO_TICKS(DIGIT_HOLD_MS));

    // ปิดก่อนเปลี่ยนเลข
    gpio_set_level(DIGIT1, 1);
    gpio_set_level(DIGIT2, 1);

    // หลักขวา
    gpio_set_level(DIGIT1, 1);
    gpio_set_level(DIGIT2, 0);

    set_segment(right);

    vTaskDelay(pdMS_TO_TICKS(DIGIT_HOLD_MS));

    // ปิดทั้งคู่
    gpio_set_level(DIGIT1, 1);
    gpio_set_level(DIGIT2, 1);
}

void app_main(void)
{
    for (int i = 0; i < 7; i++) {
        gpio_reset_pin(seg_pins[i]);
        gpio_set_direction(seg_pins[i], GPIO_MODE_OUTPUT);
    }

    gpio_reset_pin(DIGIT1);
    gpio_reset_pin(DIGIT2);

    gpio_set_direction(DIGIT1, GPIO_MODE_OUTPUT);
    gpio_set_direction(DIGIT2, GPIO_MODE_OUTPUT);

    int count = 0;
    TickType_t last_count_tick = xTaskGetTickCount();
    const TickType_t count_period_ticks = pdMS_TO_TICKS(COUNT_PERIOD_MS);

    while (1) {
        display_two_digit(count);

        TickType_t now = xTaskGetTickCount();
        if ((now - last_count_tick) >= count_period_ticks) {
            count = (count + 1) % 100;
            last_count_tick = now;
        }
    }
}
