#pragma once

#include "esp_err.h"
#include <time.h>
#include <stdbool.h>

esp_err_t rtc_manager_init(void);
bool rtc_manager_read(struct tm *out_time);
bool rtc_manager_write(const struct tm *time);
bool rtc_manager_is_valid(void);
