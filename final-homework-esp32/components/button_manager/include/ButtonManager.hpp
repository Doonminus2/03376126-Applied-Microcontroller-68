#pragma once
#include <cstddef>
#include <cstdint>

#include "clock_logic/button_fsm.hpp"
#include "clock_logic/ui_mode.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace Clock {

struct ButtonEvent {
    Button button;
    PressType press;
};

// ปุ่ม 4 ตัว: ISR ส่งแค่ขอบสัญญาณดิบเข้า queue แล้ว taskLoop() ป้อนให้ ButtonFsm ของแต่ละปุ่ม
// ทำ debounce + แยก short/long แล้วส่ง ButtonEvent ออกทาง eventQueue()
// ต่อแบบ pull-up ภายใน กด = LOW
class ButtonManager {
public:
    static constexpr gpio_num_t PIN_MODE = GPIO_NUM_18, PIN_SET = GPIO_NUM_19, PIN_ALARM = GPIO_NUM_4,
                                PIN_LAP = GPIO_NUM_5;
    static constexpr size_t BUTTON_COUNT = 4;
    static constexpr size_t EDGE_QUEUE_DEPTH = 32;
    static constexpr size_t EVENT_QUEUE_DEPTH = 10;

    ButtonManager();
    ButtonManager(const ButtonManager&) = delete;
    ButtonManager& operator=(const ButtonManager&) = delete;

    bool init();
    [[noreturn]] void taskLoop();
    bool receive(ButtonEvent& ev, TickType_t timeout);
    QueueHandle_t eventQueue() const { return eventQueue_; }

    static constexpr const char* buttonName(Button b) {
        switch (b) {
            case Button::Mode: return "MODE";
            case Button::Set: return "SET";
            case Button::Alarm: return "ALARM";
            case Button::Lap: return "LAP";
        }
        return "?";
    }

private:
    struct EdgeEvent {
        uint8_t index;
        bool pressed;
        uint32_t ms;
    };

    struct Slot {
        Button id;
        gpio_num_t pin;
        ButtonFsm fsm;
        ButtonManager* owner;  // ISR ได้ arg ตัวเดียว ใช้ชี้กลับไปหา edgeQueue_
        uint16_t rawEdges;     // ขอบดิบที่เข้ามาตั้งแต่ปุ่มว่างครั้งก่อน (นับไว้โชว์ bounce)
        uint16_t acceptedEdges;
    };

    static void onEdgeIsr(void* arg);
    bool allIdle() const;
    void emit(const Slot& slot, PressType press);

    Slot slots_[BUTTON_COUNT];  // index = static_cast<uint8_t>(Button)
    QueueHandle_t edgeQueue_ = nullptr;
    QueueHandle_t eventQueue_ = nullptr;
};

}  // namespace Clock
