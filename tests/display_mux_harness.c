#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    EVENT_GPIO,
    EVENT_DELAY_TICKS,
    EVENT_DELAY_US
} event_type_t;

typedef struct {
    event_type_t type;
    int pin;
    int level;
    int value;
} event_t;

static event_t events[128];
static int event_count;
static int digit_hold_count;
static jmp_buf stop_after_one_scan;

static void record_event(event_t event)
{
    if(event_count >= (int)(sizeof(events) / sizeof(events[0]))) {
        fprintf(stderr, "too many events\n");
        exit(1);
    }

    events[event_count++] = event;
}

void gpio_set_level(int gpio_num, int level)
{
    record_event((event_t){
        .type = EVENT_GPIO,
        .pin = gpio_num,
        .level = level,
    });
}

int gpio_get_level(int gpio_num)
{
    (void)gpio_num;
    return 1;
}

void gpio_reset_pin(int gpio_num)
{
    (void)gpio_num;
}

void gpio_set_direction(int gpio_num, int mode)
{
    (void)gpio_num;
    (void)mode;
}

void gpio_set_pull_mode(int gpio_num, int mode)
{
    (void)gpio_num;
    (void)mode;
}

void xTaskCreatePinnedToCore(
    void (*task_func)(void *),
    const char *name,
    unsigned int stack_depth,
    void *params,
    unsigned int priority,
    void **task_handle,
    int core_id
)
{
    (void)task_func;
    (void)name;
    (void)stack_depth;
    (void)params;
    (void)priority;
    (void)task_handle;
    (void)core_id;
}

void esp_rom_delay_us(unsigned int us)
{
    record_event((event_t){
        .type = EVENT_DELAY_US,
        .value = (int)us,
    });
}

void vTaskDelay(int ticks)
{
    record_event((event_t){
        .type = EVENT_DELAY_TICKS,
        .value = ticks,
    });

    digit_hold_count++;
    if(digit_hold_count == 2) {
        longjmp(stop_after_one_scan, 1);
    }
}

#include "../main/blink_example_main.c"

static void assert_true(int condition, const char *message)
{
    if(!condition) {
        fprintf(stderr, "%s\n", message);
        exit(1);
    }
}

static int find_digit_enable(int pin, int start)
{
    for(int i = start; i < event_count; i++) {
        if(events[i].type == EVENT_GPIO &&
           events[i].pin == pin &&
           events[i].level == 0) {
            return i;
        }
    }

    return -1;
}

static int has_blank_before(int enable_index)
{
    int segment_start = 0;
    int saw_d1_off = 0;
    int saw_d2_off = 0;

    for(int i = enable_index - 1; i >= 0; i--) {
        if(events[i].type == EVENT_DELAY_TICKS) {
            segment_start = i + 1;
            break;
        }
    }

    for(int i = segment_start; i < enable_index; i++) {
        if(events[i].type == EVENT_GPIO && events[i].pin == 26 && events[i].level == 1) {
            saw_d1_off = 1;
        }

        if(events[i].type == EVENT_GPIO && events[i].pin == 27 && events[i].level == 1) {
            saw_d2_off = 1;
        }

        if(events[i].type == EVENT_DELAY_US && saw_d1_off && saw_d2_off) {
            return events[i].value >= 50;
        }
    }

    return 0;
}

int main(void)
{
    count = 42;

    if(setjmp(stop_after_one_scan) == 0) {
        display_task(NULL);
    }

    assert_true(digit_hold_count == 2, "expected one complete two-digit scan");

    int first_digit = find_digit_enable(26, 0);
    int second_digit = find_digit_enable(27, first_digit + 1);

    assert_true(first_digit >= 0, "expected D1 to be enabled");
    assert_true(second_digit >= 0, "expected D2 to be enabled after D1");
    assert_true(has_blank_before(first_digit), "expected blanking delay before D1 enable");
    assert_true(has_blank_before(second_digit), "expected blanking delay before D2 enable");

    for(int i = 0; i < event_count; i++) {
        if(events[i].type == EVENT_DELAY_TICKS) {
            assert_true(events[i].value > 0, "expected digit hold delay to be at least one FreeRTOS tick");
        }
    }

    return 0;
}
