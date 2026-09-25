#ifndef TEST_FAKE_GPIO_H
#define TEST_FAKE_GPIO_H

typedef int gpio_num_t;

#define GPIO_NUM_16 16
#define GPIO_NUM_17 17
#define GPIO_NUM_18 18
#define GPIO_NUM_19 19
#define GPIO_NUM_21 21
#define GPIO_NUM_22 22
#define GPIO_NUM_23 23
#define GPIO_NUM_25 25
#define GPIO_NUM_26 26
#define GPIO_NUM_27 27

#define GPIO_MODE_INPUT 0
#define GPIO_MODE_OUTPUT 1
#define GPIO_PULLUP_ONLY 1

void gpio_set_level(gpio_num_t gpio_num, int level);
int gpio_get_level(gpio_num_t gpio_num);
void gpio_reset_pin(gpio_num_t gpio_num);
void gpio_set_direction(gpio_num_t gpio_num, int mode);
void gpio_set_pull_mode(gpio_num_t gpio_num, int mode);

#endif
