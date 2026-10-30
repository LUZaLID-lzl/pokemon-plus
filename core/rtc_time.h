#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PP_CLOCK_MIN_EPOCH INT64_C(1735689600) /* 2025-01-01 UTC */
#define PP_CLOCK_MAX_EPOCH INT64_C(4102444800) /* 2100-01-01 UTC, exclusive */

/* PCF85063A calendar is stored in UTC, with a 2000-based two-digit year.
 * Display timezone is applied only by the system clock, never by the RTC. */
bool pp_rtc_decode(uint8_t control, const uint8_t calendar[7], int64_t *epoch);
bool pp_rtc_encode(int64_t epoch, uint8_t calendar[7]);
