#include "rtc_manager.h"
#include "pcf85063a.h"
#include "board.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "RTC";
static pcf85063a_dev_t s_rtc_dev;
static bool s_initialized = false;

esp_err_t rtc_manager_init(void) {
    if (s_initialized) return ESP_OK;

    // Получаем I2C-шину из платы
    i2c_master_bus_handle_t bus = Board::GetInstance().GetI2cBus();
    if (!bus) {
        ESP_LOGE(TAG, "I2C bus not available");
        return ESP_FAIL;
    }

    esp_err_t ret = pcf85063a_init(&s_rtc_dev, bus, PCF85063A_ADDRESS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCF85063A init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "RTC initialized");
    return ESP_OK;
}

bool rtc_manager_read(struct tm *out_time) {
    if (!out_time || !s_initialized) return false;

    pcf85063a_datetime_t dt;
    if (pcf85063a_get_time_date(&s_rtc_dev, &dt) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read RTC");
        return false;
    }

    memset(out_time, 0, sizeof(struct tm));
    out_time->tm_year = dt.year - 1900;  // struct tm хранит год с 1900
    out_time->tm_mon  = dt.month - 1;    // struct tm хранит месяц 0-11
    out_time->tm_mday = dt.day;
    out_time->tm_hour = dt.hour;
    out_time->tm_min  = dt.min;
    out_time->tm_sec  = dt.sec;
    out_time->tm_wday = dt.dotw;

    return true;
}

bool rtc_manager_write(const struct tm *time) {
    if (!time || !s_initialized) return false;

    pcf85063a_datetime_t dt;
    dt.year  = time->tm_year + 1900;
    dt.month = time->tm_mon + 1;
    dt.day   = time->tm_mday;
    dt.dotw  = time->tm_wday;
    dt.hour  = time->tm_hour;
    dt.min   = time->tm_min;
    dt.sec   = time->tm_sec;

    if (pcf85063a_set_time_date(&s_rtc_dev, dt) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write RTC");
        return false;
    }

    ESP_LOGI(TAG, "RTC time written: %04d-%02d-%02d %02d:%02d:%02d",
             dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
    return true;
}

bool rtc_manager_is_valid(void) {
    if (!s_initialized) return false;

    // Читаем время — если год >= 2025, значит время валидно
    struct tm t;
    if (!rtc_manager_read(&t)) return false;
    return (t.tm_year + 1900) >= 2025;
}
