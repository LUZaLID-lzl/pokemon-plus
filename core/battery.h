#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { bool valid, charging; int percent; } pp_battery_reading;
/* BQ27220 status and operation masks from the bundled official driver. */
pp_battery_reading pp_battery_decode(bool success, uint16_t soc,
                                    uint16_t status, uint16_t operation,
                                    int16_t current);
