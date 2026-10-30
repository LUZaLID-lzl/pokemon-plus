#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct {
  uint64_t last_ms, settled_ms, candidate_ms;
  float gravity, filtered;
  unsigned run;
  bool initialized, armed;
} pp_pedometer;
/* Orientation independent m/s² acceleration, sampled by existing BSP at ~20Hz.
 * Returns only confirmed walking cadence steps, never elapsed-time estimates. */
unsigned pp_pedometer_sample(pp_pedometer *,uint64_t ms,float x,float y,float z);
