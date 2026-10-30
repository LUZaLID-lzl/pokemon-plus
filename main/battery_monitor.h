#pragma once
#include "battery.h"
#include "driver/i2c_master.h"
bool pp_battery_monitor_init(i2c_master_bus_handle_t bus);
pp_battery_reading pp_battery_monitor_read(void);
/* This device's user-confirmed 3.7 V / 500 mAh / 4.2 V cell. Only the
 * volatile capacity fields are changed; charger and chemistry data stay intact. */
#define PP_BATTERY_CAPACITY_MAH 500u
typedef struct {
  uint16_t design_before, full_before;
  uint8_t before[32], after[32];
  unsigned stage;
  bool block_saved, updated, rollback_verified;
} pp_battery_profile_report;
/* 0 already configured/absent battery; 1 updated; -1 failed, show unknown SOC.
 * Run once before the UI, outside the display lock. */
int pp_battery_monitor_configure_500mah(void (*delay_ms)(unsigned),
                                      pp_battery_profile_report *report);
typedef struct {
  uint16_t soc, status, operation, voltage_mv, temperature_dk;
  uint16_t remaining_mah, full_mah, design_mah;
  int16_t current_ma, average_ma;
} pp_battery_diagnostics;
bool pp_battery_monitor_diagnostics(pp_battery_diagnostics *out);
