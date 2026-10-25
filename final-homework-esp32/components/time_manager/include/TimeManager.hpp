#pragma once
#include <atomic>
#include <cstdint>

#include "DS1307.hpp"
#include "clock_logic/datetime.hpp"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace Clock {

// เวลาระบบ (time()) เก็บเป็น UTC เสมอ DS1307 ก็เก็บ UTC ส่วนการแสดงผลแปลงผ่าน TZ (CONFIG_DC_TZ)
// เปิดเครื่องอ่าน RTC ก่อนเพื่อให้นาฬิกาเดินได้แม้ไม่มีเน็ต แล้วค่อยให้ NTP แก้เวลาทับเมื่อต่อได้
class TimeManager {
public:
    explicit TimeManager(DS1307& rtc);
    TimeManager(const TimeManager&) = delete;
    TimeManager& operator=(const TimeManager&) = delete;

    bool loadFromRtc();
    bool initWifi();
    bool syncNow(uint32_t timeoutMs);
    bool isWifiConnected() const;
    bool isNtpSynced() const;
    bool getLocalTime(DateTime& out) const;
    bool setLocalTime(const DateTime& local);
    [[noreturn]] void taskNtpLoop();

private:
    static constexpr EventBits_t WIFI_STARTED_BIT = BIT0;
    static constexpr EventBits_t WIFI_CONNECTED_BIT = BIT1;
    static constexpr EventBits_t WIFI_GAVE_UP_BIT = BIT2;
    static constexpr EventBits_t NTP_SYNCED_BIT = BIT3;

    static void onEvent(void* arg, esp_event_base_t base, int32_t id, void* data);
    static void onReconnectTimer(void* arg);
    void handleDisconnect();
    void restartGivenUpWifi();

    DS1307& rtc_;
    StaticEventGroup_t eventStorage_;
    EventGroupHandle_t events_;
    std::atomic<int> retry_{0};
    esp_netif_t* staNetif_ = nullptr;
    esp_timer_handle_t reconnectTimer_ = nullptr;
};

}  // namespace Clock
