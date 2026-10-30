#include "battery.h"
pp_battery_reading pp_battery_decode(bool success,uint16_t soc,uint16_t status,
                                    uint16_t operation,int16_t current) {
  if(!success || soc>100 || !(status&(1u<<3)) || !(operation&(1u<<5)))
    return (pp_battery_reading){.percent=-1};
  return (pp_battery_reading){.valid=true,.percent=soc,
    .charging=current>0 && !(status&1u) && !(status&(1u<<9))};
}
