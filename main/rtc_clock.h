#pragma once
#include "driver/i2c_master.h"
#include "rtc_time.h"

esp_err_t pp_rtc_clock_init(i2c_master_bus_handle_t bus);
esp_err_t pp_rtc_clock_read(int64_t *epoch);
esp_err_t pp_rtc_clock_write(int64_t epoch);
