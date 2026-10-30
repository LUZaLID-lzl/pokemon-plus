#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Fixed seeds belong to reproducible captures/tests, never normal new games. */
uint32_t pp_simulator_seed(bool reproducible);
