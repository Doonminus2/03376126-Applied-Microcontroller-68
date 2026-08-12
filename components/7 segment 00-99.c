#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "portmacro.h"
#include "soc/gpio_num.h"
#include <stdio.h>



const int seven_segment_pins[7] = {GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_1 , GPIO_NUM_3, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18};

void setup(void){

    for (int i = 0; i < 7; i++) {
        gpio_reset_pin(seven_segment_pins[i]);
        gpio_set_direction(seven_segment_pins[i], GPIO_MODE_OUTPUT);
    }

gpio_reset_pin(5);
gpio_set_direction(5, GPIO_MODE_OUTPUT);

gpio_reset_pin(17);
gpio_set_direction(17, GPIO_MODE_OUTPUT);



}

void display(int status , int delay)
{
    for(int i = 0; i < 7; i++) {
        gpio_set_level((gpio_num_t)  seven_segment_pins[i], status);
    }

    vTaskDelay(delay);
}

void display_num(int num , int delay)
{
    const int number[10][7] = {
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

    for(int i = 0; i < 7; i++) {
        gpio_set_level(seven_segment_pins[i], number[num][i]);
    }

    vTaskDelay(delay);
}

void digit(int digit , int status)
{
    gpio_set_level(digit, status);
}

void display_two_num(int data , int _digit , int status , int delay)
{
    digit(_digit, status);
    display_num(data % 10 , delay);
    digit(_digit, 0);
}

void _display_two_num(int data)
{
    digit(5, 0);
    digit(17, 1);
    display_num(data % 10 , 10 / portTICK_PERIOD_MS);
    digit(5 , 1 );
    digit(17, 0);
    display_num(data / 10 , 10 / portTICK_PERIOD_MS);
}

void app_main(void){
    setup();

    while (1) {
        for (int i = 0; i < 100; i++) 
        {
            for (int j = 0; j < 30; j++) 
            {
                _display_two_num(i);
                printf_float(i);
            }
        }
    }
}