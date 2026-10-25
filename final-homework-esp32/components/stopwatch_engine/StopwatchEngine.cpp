#include "StopwatchEngine.hpp"

#include "esp_log.h"
#include "esp_timer.h"

namespace Clock {

static const char* TAG = "sw";

namespace {
class Lock {
public:
    explicit Lock(SemaphoreHandle_t m) : m_(m) { xSemaphoreTake(m_, portMAX_DELAY); }
    ~Lock() { xSemaphoreGive(m_); }

private:
    SemaphoreHandle_t m_;
};

uint64_t nowUs() { return static_cast<uint64_t>(esp_timer_get_time()); }
}  // namespace

StopwatchEngine::StopwatchEngine() : mutex_(xSemaphoreCreateMutex()) {}

void StopwatchEngine::toggle() {
    bool running;
    uint64_t elapsed;
    {
        Lock lock(mutex_);
        const uint64_t now = nowUs();
        if (core_.running()) {
            core_.stop(now);
        } else {
            core_.start(now);
        }
        running = core_.running();
        elapsed = core_.elapsedMs(now);
    }
    char text[16];
    formatStopwatch(elapsed, text, sizeof(text));
    ESP_LOGI(TAG, "[sw] %s at %s", running ? "start" : "stop", text);
}

void StopwatchEngine::lapOrReset() {
    bool wasRunning;
    bool recorded = false;
    size_t index = 0;
    LapRecord lap;
    {
        Lock lock(mutex_);
        wasRunning = core_.running();
        if (wasRunning) {
            recorded = core_.lap(nowUs());
            index = core_.lapCount();
            if (recorded) lap = core_.lapAt(index - 1);
        } else {
            core_.reset();
        }
    }
    if (!wasRunning) {
        ESP_LOGI(TAG, "[sw] reset");
        return;
    }
    if (!recorded) {
        ESP_LOGW(TAG, "[sw] lap ignored (max %u laps)", static_cast<unsigned>(STOPWATCH_MAX_LAPS));
        return;
    }
    char total[16];
    char split[16];
    formatStopwatch(lap.totalMs, total, sizeof(total));
    formatStopwatch(lap.splitMs, split, sizeof(split));
    ESP_LOGI(TAG, "[sw] lap %u total=%s split=%s", static_cast<unsigned>(index), total, split);
}

void StopwatchEngine::reset() {
    {
        Lock lock(mutex_);
        core_.reset();
    }
    ESP_LOGI(TAG, "[sw] reset");
}

StopwatchSnapshot StopwatchEngine::snapshot() const {
    StopwatchSnapshot s;
    Lock lock(mutex_);
    s.running = core_.running();
    s.elapsedMs = core_.elapsedMs(nowUs());
    s.lapCount = core_.lapCount();
    for (size_t i = 0; i < s.lapCount; ++i) s.laps[i] = core_.lapAt(i);
    return s;
}

}  // namespace Clock
