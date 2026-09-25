#ifndef TEST_FAKE_FREERTOS_H
#define TEST_FAKE_FREERTOS_H

#define configTICK_RATE_HZ 100
#define pdMS_TO_TICKS(ms) (((ms) * configTICK_RATE_HZ) / 1000)

typedef int TickType_t;

#endif
