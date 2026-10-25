#include "ButtonManager.hpp"

#include "esp_attr.h"
#include "esp_log.h"
#include "esp_timer.h"

namespace Clock {

static const char* TAG = "ButtonManager";

// FreeRTOS tick 100 Hz ทำให้ pdMS_TO_TICKS(5) = 0 ซึ่งจะกลายเป็น busy loop จึงบังคับอย่างน้อย 1 tick
static constexpr TickType_t POLL_TICKS = pdMS_TO_TICKS(5) > 0 ? pdMS_TO_TICKS(5) : 1;

static IRAM_ATTR uint32_t nowMs() { return static_cast<uint32_t>(esp_timer_get_time() / 1000); }

ButtonManager::ButtonManager()
    : slots_{
          {Button::Mode, PIN_MODE, ButtonFsm(), this, 0, 0},
          {Button::Set, PIN_SET, ButtonFsm(), this, 0, 0},
          {Button::Alarm, PIN_ALARM, ButtonFsm(), this, 0, 0},
          {Button::Lap, PIN_LAP, ButtonFsm(), this, 0, 0},
      } {
    static_assert(static_cast<uint8_t>(Button::Mode) == 0 && static_cast<uint8_t>(Button::Set) == 1 &&
                      static_cast<uint8_t>(Button::Alarm) == 2 && static_cast<uint8_t>(Button::Lap) == 3,
                  "slots_ is indexed by Button");
}

void IRAM_ATTR ButtonManager::onEdgeIsr(void* arg) {
    const Slot* slot = static_cast<const Slot*>(arg);
    const EdgeEvent ev{static_cast<uint8_t>(slot->id), gpio_get_level(slot->pin) == 0, nowMs()};
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(slot->owner->edgeQueue_, &ev, &woken);
    if (woken == pdTRUE) portYIELD_FROM_ISR();
}

bool ButtonManager::init() {
    if (edgeQueue_ == nullptr) edgeQueue_ = xQueueCreate(EDGE_QUEUE_DEPTH, sizeof(EdgeEvent));
    if (eventQueue_ == nullptr) eventQueue_ = xQueueCreate(EVENT_QUEUE_DEPTH, sizeof(ButtonEvent));
    if (edgeQueue_ == nullptr || eventQueue_ == nullptr) {
        ESP_LOGE(TAG, "queue create failed");
        return false;
    }

    uint64_t mask = 0;
    for (const Slot& slot : slots_) mask |= 1ULL << slot.pin;
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = mask;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type = GPIO_INTR_ANYEDGE;
    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config failed: %s", esp_err_to_name(err));
        return false;
    }

    // INVALID_STATE = มีคนติดตั้ง ISR service ไว้แล้ว (เช่นเรียก init ซ้ำ) ใช้ต่อได้เลย
    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "gpio_install_isr_service failed: %s", esp_err_to_name(err));
        return false;
    }

    for (Slot& slot : slots_) {
        err = gpio_isr_handler_add(slot.pin, onEdgeIsr, &slot);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "gpio_isr_handler_add(%s, GPIO%d) failed: %s", buttonName(slot.id), slot.pin,
                     esp_err_to_name(err));
            return false;
        }
    }

    ESP_LOGI(TAG, "ready: MODE=GPIO%d SET=GPIO%d ALARM=GPIO%d LAP=GPIO%d", PIN_MODE, PIN_SET, PIN_ALARM, PIN_LAP);
    return true;
}

bool ButtonManager::allIdle() const {
    for (const Slot& slot : slots_) {
        if (!slot.fsm.idle()) return false;
    }
    return true;
}

void ButtonManager::emit(const Slot& slot, PressType press) {
    const ButtonEvent ev{slot.id, press};
    if (xQueueSend(eventQueue_, &ev, 0) != pdTRUE) {
        ESP_LOGW(TAG, "event queue full, dropped %s %s", buttonName(slot.id),
                 press == PressType::Long ? "long" : "short");
    }
}

void ButtonManager::taskLoop() {
    configASSERT(edgeQueue_ != nullptr && eventQueue_ != nullptr);
    for (;;) {
        // ถ้ามีปุ่มรอ debounce หรือรอครบ long press ต้องตื่นมา poll เองแม้ไม่มีขอบใหม่
        const TickType_t wait = allIdle() ? portMAX_DELAY : POLL_TICKS;
        EdgeEvent edge;
        if (xQueueReceive(edgeQueue_, &edge, wait) == pdTRUE) {
            do {
                Slot& slot = slots_[edge.index];
                slot.fsm.edge(edge.pressed, edge.ms);
                ++slot.rawEdges;
            } while (xQueueReceive(edgeQueue_, &edge, 0) == pdTRUE);
        }

        const uint32_t now = nowMs();
        for (Slot& slot : slots_) {
            const bool wasPressed = slot.fsm.pressed();
            const PressType press = slot.fsm.poll(now);
            if (slot.fsm.pressed() != wasPressed) ++slot.acceptedEdges;
            if (press != PressType::None) emit(slot, press);

            if (slot.fsm.idle() && slot.rawEdges > 0) {
                ESP_LOGD(TAG, "%s: %u raw edges, %u accepted, %u dropped by debounce", buttonName(slot.id),
                         slot.rawEdges, slot.acceptedEdges, slot.rawEdges - slot.acceptedEdges);
                slot.rawEdges = 0;
                slot.acceptedEdges = 0;
            }
        }
    }
}

bool ButtonManager::receive(ButtonEvent& ev, TickType_t timeout) {
    if (eventQueue_ == nullptr) return false;
    return xQueueReceive(eventQueue_, &ev, timeout) == pdTRUE;
}

}  // namespace Clock
