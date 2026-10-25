#include "TimeManager.hpp"

#include <sys/time.h>

#include <algorithm>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "nvs_flash.h"

namespace Clock {

static const char* TAG = "TimeManager";

static constexpr uint32_t BACKOFF_BASE_MS = 1000;
static constexpr uint32_t BACKOFF_MAX_MS = 30000;
static constexpr uint32_t NTP_RETRY_SEC = 30;

// pdMS_TO_TICKS คูณด้วย TickType_t 32 บิต จะล้นถ้ารอเป็นชั่วโมง จึงแปลงจากวินาทีตรง ๆ
static constexpr TickType_t secondsToTicks(uint32_t s) { return static_cast<TickType_t>(s) * configTICK_RATE_HZ; }

static void formatDateTime(const DateTime& dt, char* buf, size_t len) {
    snprintf(buf, len, "%04u-%02u-%02u %02u:%02u:%02u", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
}

TimeManager::TimeManager(DS1307& rtc) : rtc_(rtc), events_(xEventGroupCreateStatic(&eventStorage_)) {
    // ตั้ง TZ ตั้งแต่สร้างออบเจกต์ ทุกการแปลง local time จึงถูกต้องแม้ยังไม่ได้เรียก loadFromRtc
    setenv("TZ", CONFIG_DC_TZ, 1);
    tzset();
}

bool TimeManager::loadFromRtc() {
    DateTime utc;
    if (!rtc_.getTime(utc)) {
        ESP_LOGE(TAG, "RTC unreadable, system clock not set");
        return false;
    }
    const timeval tv = {.tv_sec = static_cast<time_t>(toEpoch(utc)), .tv_usec = 0};
    if (settimeofday(&tv, nullptr) != 0) {
        ESP_LOGE(TAG, "settimeofday failed");
        return false;
    }
    char buf[24];
    formatDateTime(utc, buf, sizeof(buf));
    ESP_LOGI(TAG, "System clock loaded from DS1307: %s UTC (TZ=%s)", buf, CONFIG_DC_TZ);
    return true;
}

bool TimeManager::initWifi() {
    if (xEventGroupGetBits(events_) & WIFI_STARTED_BIT) return true;

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs erase (%s)", esp_err_to_name(err));
        if (nvs_flash_erase() != ESP_OK) return false;
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init: %s", esp_err_to_name(err));
        return false;
    }
    if ((err = esp_netif_init()) != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init: %s", esp_err_to_name(err));
        return false;
    }
    // INVALID_STATE = มีโมดูลอื่นสร้าง default loop ไปแล้ว ใช้ร่วมกันได้
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default: %s", esp_err_to_name(err));
        return false;
    }
    if (staNetif_ == nullptr) staNetif_ = esp_netif_create_default_wifi_sta();
    if (reconnectTimer_ == nullptr) {
        const esp_timer_create_args_t args = {
            .callback = &TimeManager::onReconnectTimer,
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "wifi_reconnect",
            .skip_unhandled_events = true,
        };
        if ((err = esp_timer_create(&args, &reconnectTimer_)) != ESP_OK) {
            ESP_LOGE(TAG, "esp_timer_create: %s", esp_err_to_name(err));
            return false;
        }
    }

    const wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if ((err = esp_wifi_init(&cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(err));
        return false;
    }
    if ((err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &TimeManager::onEvent, this,
                                                   nullptr)) != ESP_OK ||
        (err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &TimeManager::onEvent, this,
                                                   nullptr)) != ESP_OK) {
        ESP_LOGE(TAG, "event handler register: %s", esp_err_to_name(err));
        return false;
    }

    wifi_config_t wifi = {};
    strncpy(reinterpret_cast<char*>(wifi.sta.ssid), CONFIG_DC_WIFI_SSID, sizeof(wifi.sta.ssid));
    strncpy(reinterpret_cast<char*>(wifi.sta.password), CONFIG_DC_WIFI_PASSWORD, sizeof(wifi.sta.password));
    // AP เปิด (Wokwi-GUEST) ต้องยอมรับ OPEN ไม่งั้น driver จะไม่ยอมต่อ
    wifi.sta.threshold.authmode = CONFIG_DC_WIFI_PASSWORD[0] == '\0' ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    if ((err = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK ||
        (err = esp_wifi_set_config(WIFI_IF_STA, &wifi)) != ESP_OK || (err = esp_wifi_start()) != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi start: %s", esp_err_to_name(err));
        return false;
    }
    xEventGroupSetBits(events_, WIFI_STARTED_BIT);
    ESP_LOGI(TAG, "Wi-Fi STA started, connecting to \"%s\" (%s)", CONFIG_DC_WIFI_SSID,
             CONFIG_DC_WIFI_PASSWORD[0] == '\0' ? "open" : "WPA2");
    return true;
}

void TimeManager::onEvent(void* arg, esp_event_base_t base, int32_t id, void* data) {
    auto* self = static_cast<TimeManager*>(arg);
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        self->handleDisconnect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        const auto* got = static_cast<const ip_event_got_ip_t*>(data);
        ESP_LOGI(TAG, "Wi-Fi connected, IP " IPSTR, IP2STR(&got->ip_info.ip));
        self->retry_ = 0;
        xEventGroupClearBits(self->events_, WIFI_GAVE_UP_BIT);
        xEventGroupSetBits(self->events_, WIFI_CONNECTED_BIT);
    }
}

// handler รันใน event loop task ห้าม delay ตรงนี้ จึงหน่วงการต่อใหม่ด้วย esp_timer แบบ back-off
void TimeManager::handleDisconnect() {
    xEventGroupClearBits(events_, WIFI_CONNECTED_BIT);
    const int attempt = retry_.load();
    if (attempt >= CONFIG_DC_WIFI_MAX_RETRY) {
        ESP_LOGW(TAG, "Wi-Fi gave up after %d retries, next try on NTP cycle", attempt);
        xEventGroupSetBits(events_, WIFI_GAVE_UP_BIT);
        return;
    }
    retry_ = attempt + 1;
    const uint32_t delayMs = std::min(BACKOFF_BASE_MS << std::min(attempt, 5), BACKOFF_MAX_MS);
    ESP_LOGW(TAG, "Wi-Fi disconnected, retry %d/%d in %" PRIu32 " ms", attempt + 1, CONFIG_DC_WIFI_MAX_RETRY,
             delayMs);
    if (esp_timer_start_once(reconnectTimer_, static_cast<uint64_t>(delayMs) * 1000) != ESP_OK) esp_wifi_connect();
}

void TimeManager::onReconnectTimer(void*) { esp_wifi_connect(); }

void TimeManager::restartGivenUpWifi() {
    if (!(xEventGroupClearBits(events_, WIFI_GAVE_UP_BIT) & WIFI_GAVE_UP_BIT)) return;
    ESP_LOGI(TAG, "Wi-Fi retry budget reset, connecting again");
    retry_ = 0;
    esp_wifi_connect();
}

bool TimeManager::syncNow(uint32_t timeoutMs) {
    if (!initWifi()) return false;
    restartGivenUpWifi();

    const EventBits_t bits =
        xEventGroupWaitBits(events_, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(timeoutMs));
    if (!(bits & WIFI_CONNECTED_BIT)) {
        ESP_LOGW(TAG, "No Wi-Fi after %" PRIu32 " ms", timeoutMs);
        return false;
    }

    // init ใหม่ทุกรอบเพื่อให้ได้ semaphore ใหม่ ไม่งั้น sync_wait อาจคืน OK จากการ sync เก่าที่ค้างอยู่
    esp_netif_sntp_deinit();
    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_DC_NTP_SERVER);
    esp_err_t err = esp_netif_sntp_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_sntp_init: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "SNTP query %s (timeout %" PRIu32 " ms)", CONFIG_DC_NTP_SERVER, timeoutMs);
    err = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeoutMs));
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "SNTP sync failed: %s", esp_err_to_name(err));
        return false;
    }

    DateTime utc;
    if (!fromEpoch(static_cast<int64_t>(time(nullptr)), utc)) {
        ESP_LOGE(TAG, "SNTP returned a time outside 2000..2099");
        return false;
    }
    xEventGroupSetBits(events_, NTP_SYNCED_BIT);
    const bool rtcOk = rtc_.setTime(utc);
    DateTime local;
    getLocalTime(local);
    char utcBuf[24];
    char localBuf[24];
    formatDateTime(utc, utcBuf, sizeof(utcBuf));
    formatDateTime(local, localBuf, sizeof(localBuf));
    ESP_LOGI(TAG, "ntp_demo: UTC %s -> DS1307 %s | local (%s) %s", utcBuf, rtcOk ? "written" : "WRITE FAILED",
             CONFIG_DC_TZ, localBuf);
    return true;
}

bool TimeManager::isWifiConnected() const { return xEventGroupGetBits(events_) & WIFI_CONNECTED_BIT; }

bool TimeManager::isNtpSynced() const { return xEventGroupGetBits(events_) & NTP_SYNCED_BIT; }

bool TimeManager::getLocalTime(DateTime& out) const {
    const time_t now = time(nullptr);
    tm t;
    if (localtime_r(&now, &t) == nullptr) return false;
    DateTime dt;
    dt.year = static_cast<uint16_t>(t.tm_year + 1900);
    dt.month = static_cast<uint8_t>(t.tm_mon + 1);
    dt.day = static_cast<uint8_t>(t.tm_mday);
    dt.weekday = static_cast<uint8_t>(t.tm_wday + 1);
    dt.hour = static_cast<uint8_t>(t.tm_hour);
    dt.minute = static_cast<uint8_t>(t.tm_min);
    dt.second = static_cast<uint8_t>(t.tm_sec);
    if (!isValid(dt)) return false;
    out = dt;
    return true;
}

bool TimeManager::setLocalTime(const DateTime& local) {
    DateTime checked = local;
    checked.weekday = weekdayFromDate(local.year, local.month, local.day);
    if (!isValid(checked)) {
        ESP_LOGE(TAG, "setLocalTime: invalid date/time");
        return false;
    }
    tm t = {};
    t.tm_year = checked.year - 1900;
    t.tm_mon = checked.month - 1;
    t.tm_mday = checked.day;
    t.tm_hour = checked.hour;
    t.tm_min = checked.minute;
    t.tm_sec = checked.second;
    t.tm_isdst = -1;
    const time_t epoch = mktime(&t);
    if (epoch == static_cast<time_t>(-1)) return false;
    const timeval tv = {.tv_sec = epoch, .tv_usec = 0};
    if (settimeofday(&tv, nullptr) != 0) return false;
    DateTime utc;
    const bool rtcOk = fromEpoch(static_cast<int64_t>(epoch), utc) && rtc_.setTime(utc);
    char buf[24];
    formatDateTime(checked, buf, sizeof(buf));
    ESP_LOGI(TAG, "Manual set local %s, DS1307 %s", buf, rtcOk ? "updated" : "WRITE FAILED");
    return rtcOk;
}

void TimeManager::taskNtpLoop() {
    const uint32_t timeoutMs = CONFIG_DC_NTP_TIMEOUT_SEC * 1000u;
    for (;;) {
        if (!initWifi()) {
            ESP_LOGE(TAG, "Wi-Fi init failed, retry in %" PRIu32 " s", NTP_RETRY_SEC);
            vTaskDelay(secondsToTicks(NTP_RETRY_SEC));
            continue;
        }
        if (syncNow(timeoutMs)) {
            ESP_LOGI(TAG, "NTP sync OK, next resync in %d s", CONFIG_DC_NTP_RESYNC_SEC);
            vTaskDelay(secondsToTicks(CONFIG_DC_NTP_RESYNC_SEC));
        } else {
            ESP_LOGW(TAG, "%s (retry in %" PRIu32 " s)",
                     isNtpSynced() ? "NTP resync failed, keeping last NTP time" : "NTP failed, using RTC time",
                     NTP_RETRY_SEC);
            vTaskDelay(secondsToTicks(NTP_RETRY_SEC));
        }
    }
}

}  // namespace Clock
