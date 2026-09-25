#ifndef TEST_FAKE_TASK_H
#define TEST_FAKE_TASK_H

#include "freertos/FreeRTOS.h"

typedef void *TaskHandle_t;

void vTaskDelay(TickType_t ticks);
void xTaskCreatePinnedToCore(
    void (*task_func)(void *),
    const char *name,
    unsigned int stack_depth,
    void *params,
    unsigned int priority,
    TaskHandle_t *task_handle,
    int core_id
);

#endif
