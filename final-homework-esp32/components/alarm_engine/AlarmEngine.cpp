#include "AlarmEngine.hpp"

#include "driver/ledc.h"
#include "esp_log.h"

namespace Clock {

static const char* TAG = "alarm";

static constexpr ledc_mode_t LEDC_MODE = LEDC_LOW_SPEED_MODE;
static constexpr ledc_timer_t LEDC_TIMER = LEDC_TIMER_0;
static constexpr ledc_channel_t LEDC_CHANNEL = LEDC_CHANNEL_0;
static constexpr uint32_t DUTY_HALF = 512;  // 50 % ของ 10 บิต
static constexpr uint32_t RING_FREQ_HZ = 2000;
static constexpr uint64_t RING_HALF_PERIOD_US = 200 * 1000;

namespace {
class Lock {
public:
    explicit Lock(SemaphoreHandle_t m) : m_(m) { xSemaphoreTake(m_, portMAX_DELAY); }
    ~Lock() { xSemaphoreGive(m_); }

private:
    SemaphoreHandle_t m_;
};
}  // namespace

AlarmEngine::AlarmEngine(gpio_num_t buzzerPin, uint32_t ringMs)
    : pin_(buzzerPin), mutex_(xSemaphoreCreateMutex()), state_(ringMs) {}

bool AlarmEngine::init() {
    Lock lock(mutex_);
    if (ready_) return true;

    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_MODE;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num = LEDC_TIMER;
    timer.freq_hz = RING_FREQ_HZ;
    timer.clk_cfg = LEDC_AUTO_CLK;
    esp_err_t err = ledc_timer_config(&timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_timer_config failed: %s", esp_err_to_name(err));
        return false;
    }

    ledc_channel_config_t channel = {};
    channel.gpio_num = pin_;
    channel.speed_mode = LEDC_MODE;
    channel.channel = LEDC_CHANNEL;
    channel.timer_sel = LEDC_TIMER;
    channel.duty = 0;
    channel.hpoint = 0;
    err = ledc_channel_config(&channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ledc_channel_config failed: %s", esp_err_to_name(err));
        return false;
    }

    esp_timer_create_args_t ringArgs = {};
    ringArgs.callback = &AlarmEngine::onRingTick;
    ringArgs.arg = this;
    ringArgs.dispatch_method = ESP_TIMER_TASK;
    ringArgs.name = "alarm_ring";
    if (!ringTimer_ && esp_timer_create(&ringArgs, &ringTimer_) != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create(ring) failed");
        return false;
    }

    esp_timer_create_args_t beepArgs = ringArgs;
    beepArgs.callback = &AlarmEngine::onBeepEnd;
    beepArgs.name = "alarm_beep";
    if (!beepTimer_ && esp_timer_create(&beepArgs, &beepTimer_) != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create(beep) failed");
        return false;
    }

    ready_ = true;
    return true;
}

AlarmSetting AlarmEngine::setting() const {
    Lock lock(mutex_);
    return setting_;
}

void AlarmEngine::setSetting(const AlarmSetting& s) {
    bool stopped;
    {
        Lock lock(mutex_);
        stopped = applySettingLocked(s);
    }
    if (stopped) logStop(StopReason::Disabled);
}

bool AlarmEngine::toggleEnabled() {
    bool stopped;
    bool enabled;
    {
        Lock lock(mutex_);
        AlarmSetting s = setting_;
        s.enabled = !s.enabled;
        enabled = s.enabled;
        stopped = applySettingLocked(s);
    }
    if (stopped) logStop(StopReason::Disabled);
    return enabled;
}

AlarmEvent AlarmEngine::check(const DateTime& localNow, uint32_t nowMs) {
    AlarmEvent ev;
    bool enabled;
    {
        Lock lock(mutex_);
        ev = state_.update(setting_, localNow.hour, localNow.minute, nowMs);
        enabled = setting_.enabled;
        if (ev == AlarmEvent::Start) startRingLocked();
        if (ev == AlarmEvent::Stop) stopRingLocked();
    }
    if (ev == AlarmEvent::Start) ESP_LOGI(TAG, "[alarm] START %02u:%02u", localNow.hour, localNow.minute);
    if (ev == AlarmEvent::Stop) logStop(enabled ? StopReason::Timeout : StopReason::Disabled);
    return ev;
}

bool AlarmEngine::silence() {
    bool stopped;
    {
        Lock lock(mutex_);
        stopped = state_.silence() == AlarmEvent::Stop;
        if (stopped) stopRingLocked();
    }
    if (stopped) logStop(StopReason::Silenced);
    return stopped;
}

bool AlarmEngine::ringing() const {
    Lock lock(mutex_);
    return state_.ringing();
}

void AlarmEngine::beep(uint32_t freqHz, uint32_t durationMs) {
    Lock lock(mutex_);
    if (!ready_ || state_.ringing()) return;  // ขณะปลุก เสียงปลุกเป็นเจ้าของบัซเซอร์
    esp_timer_stop(beepTimer_);
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, freqHz);
    outputLocked(true);
    esp_timer_start_once(beepTimer_, static_cast<uint64_t>(durationMs) * 1000);
}

// callback ของ esp_timer อาจยังวิ่งอยู่ตอน stop ถูกเรียก จึงต้องเช็ก ringing ใต้ mutex
// ไม่งั้นจะเปิดเสียงค้างหลัง Stop ได้
void AlarmEngine::onRingTick(void* arg) {
    auto* self = static_cast<AlarmEngine*>(arg);
    Lock lock(self->mutex_);
    if (!self->state_.ringing()) return;
    self->outputLocked(!self->toneOn_);
}

void AlarmEngine::onBeepEnd(void* arg) {
    auto* self = static_cast<AlarmEngine*>(arg);
    Lock lock(self->mutex_);
    if (!self->state_.ringing()) self->outputLocked(false);
}

void AlarmEngine::startRingLocked() {
    if (!ready_) return;
    esp_timer_stop(beepTimer_);
    esp_timer_stop(ringTimer_);
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, RING_FREQ_HZ);
    outputLocked(true);
    esp_timer_start_periodic(ringTimer_, RING_HALF_PERIOD_US);
}

void AlarmEngine::stopRingLocked() {
    if (!ready_) return;
    esp_timer_stop(ringTimer_);
    outputLocked(false);
}

void AlarmEngine::outputLocked(bool on) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, on ? DUTY_HALF : 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    toneOn_ = on;
}

bool AlarmEngine::applySettingLocked(const AlarmSetting& s) {
    setting_ = s;
    if (s.enabled || state_.silence() != AlarmEvent::Stop) return false;
    stopRingLocked();
    return true;
}

void AlarmEngine::logStop(StopReason reason) {
    const char* text = reason == StopReason::Timeout ? "timeout" : reason == StopReason::Silenced ? "silenced" : "disabled";
    ESP_LOGI(TAG, "[alarm] STOP (%s)", text);
}

}  // namespace Clock
