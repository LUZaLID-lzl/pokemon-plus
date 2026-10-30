#include "battery_monitor.h"
#include "sdkconfig.h"
#include <string.h>
/* BQ27220 is U13 on the official 1.85B Rev1.1 schematic. Address/registers
 * match espressif/bq27220 0.1.1 bundled by Waveshare's fixed example.
 * User-authorized 500 mAh RAM capacity correction follows TI SLUUBD4A,
 * sections 1.1.10 and 6.1. Never write OTP, chemistry, or charger settings. */
static i2c_master_dev_handle_t gauge;
bool pp_battery_monitor_init(i2c_master_bus_handle_t bus) {
  if(gauge) return true;
  if(!bus) return false;
  const i2c_device_config_t cfg={.dev_addr_length=I2C_ADDR_BIT_LEN_7,
    .device_address=0x55,.scl_speed_hz=CONFIG_BSP_I2C_CLK_SPEED_HZ};
  return i2c_master_bus_add_device(bus,&cfg,&gauge)==ESP_OK;
}
static bool read_u16(uint8_t reg,uint16_t *value) {
  uint8_t bytes[2];
  if(i2c_master_transmit_receive(gauge,&reg,1,bytes,2,20)!=ESP_OK) return false;
  *value=bytes[0]|((uint16_t)bytes[1]<<8);return true;
}
pp_battery_reading pp_battery_monitor_read(void) {
  uint16_t soc,status,operation,current;
  if(!gauge || !read_u16(0x2c,&soc) || !read_u16(0x0a,&status) ||
     !read_u16(0x3a,&operation) || !read_u16(0x0c,&current))
    return pp_battery_decode(false,0,0,0,0);
  return pp_battery_decode(true,soc,status,operation,(int16_t)current);
}

bool pp_battery_monitor_diagnostics(pp_battery_diagnostics *out) {
  pp_battery_diagnostics d={0};uint16_t current,average;
  if(!gauge || !out || !read_u16(0x2c,&d.soc) ||
     !read_u16(0x0a,&d.status) || !read_u16(0x3a,&d.operation) ||
     !read_u16(0x0c,&current) || !read_u16(0x08,&d.voltage_mv) ||
     !read_u16(0x14,&average) || !read_u16(0x06,&d.temperature_dk) ||
     !read_u16(0x10,&d.remaining_mah) || !read_u16(0x12,&d.full_mah) ||
     !read_u16(0x3c,&d.design_mah))return false;
  d.current_ma=(int16_t)current;d.average_ma=(int16_t)average;
  *out=d;return true;
}
static bool write_bytes(uint8_t reg,const uint8_t *data,size_t count) {
  uint8_t tx[33];
  if(count>32)return false;
  tx[0]=reg;memcpy(tx+1,data,count);
  return i2c_master_transmit(gauge,tx,count+1,20)==ESP_OK;
}
static bool control(uint16_t command) {
  uint8_t bytes[]={(uint8_t)command,(uint8_t)(command>>8)};
  return write_bytes(0,bytes,2);
}
static bool poll_operation(uint16_t mask,uint16_t expected,
                           void (*delay_ms)(unsigned)) {
  for(unsigned i=0;i<20;i++) {
    uint16_t operation;
    if(!read_u16(0x3a,&operation))return false;
    if((operation&mask)==expected)return true;
    delay_ms(100);
  }
  return false;
}
/* Read 32 bytes from the exact capacity block, including address echo,
 * checksum and length. Data-memory words are big-endian; standard words LE. */
static bool capacity_block_read(uint8_t out[32],void (*delay_ms)(unsigned)) {
  const uint8_t address[]={0x9d,0x92};uint8_t bytes[36],reg=0x3e;
  if(!write_bytes(0x3e,address,2))return false;
  delay_ms(10);
  if(i2c_master_transmit_receive(gauge,&reg,1,bytes,sizeof(bytes),20)!=ESP_OK ||
     bytes[0]!=address[0] || bytes[1]!=address[1] || bytes[35]!=36)return false;
  unsigned sum=0;for(unsigned i=0;i<34;i++)sum+=bytes[i];
  if(bytes[34]!=(uint8_t)(255-(sum&255)))return false;
  memcpy(out,bytes+2,32);return true;
}
static bool capacity_block_write(const uint8_t bytes[32],
                                 void (*delay_ms)(unsigned)) {
  /* The board's driver writes address+data in one short MAC transaction,
   * then allows 10ms processing before checksum/length. Only four capacity
   * bytes are changed; the remaining 28 bytes are still verified on readback. */
  uint8_t data[]={0x9d,0x92,bytes[0],bytes[1],bytes[2],bytes[3]};
  unsigned sum=0;for(unsigned i=0;i<sizeof(data);i++)sum+=data[i];
  uint8_t finish[]={(uint8_t)(255-(sum&255)),sizeof(data)+2};
  if(!write_bytes(0x3e,data,sizeof(data)))return false;
  delay_ms(10);
  if(!write_bytes(0x60,finish,2))return false;
  delay_ms(10);return true;
}
int pp_battery_monitor_configure_500mah(void (*delay_ms)(unsigned),
                                      pp_battery_profile_report *report) {
  pp_battery_profile_report local={0};uint16_t operation,status;
  bool was_sealed=false,access_changed=false,entered=false,may_have_written=false;
  int result=-1;
  if(!report)report=&local;
  memset(report,0,sizeof(*report));
  if(!gauge || !delay_ms || !read_u16(0x3a,&operation) ||
     !read_u16(0x0a,&status))return -1;
  if(!(status&8))return 0;
  if(operation&0x400) {
    /* Recover a previous interrupted host transaction before any new write. */
    if(!control(0x0092) || !poll_operation(0x400,0,delay_ms) ||
       !read_u16(0x3a,&operation))return -1;
  }
  if(!(operation&32))return -1;
  if(!read_u16(0x3c,&report->design_before) ||
     !read_u16(0x12,&report->full_before))return -1;
  report->stage=1;
  /* Preserve learned full-charge capacity on subsequent boots. */
  if(report->design_before==PP_BATTERY_CAPACITY_MAH)return 0;
  unsigned security=(operation>>1)&3;
  report->stage=2;
  was_sealed=security==3;
  if(was_sealed) {
    access_changed=true;
    if(!control(0x0414) || !control(0x3672))goto done;
    delay_ms(10);
    if(!read_u16(0x3a,&operation) || ((operation>>1)&3)==3)goto done;
    security=(operation>>1)&3;
  }
  /* TI §2.2.21 and the board's bundled driver permit ENTER_CFG_UPDATE in
   * UNSEALED mode. Do not require or attempt FULL ACCESS / OTP permissions. */
  if(security!=1 && security!=2)goto done;
  /* A failed transfer might still have reached the gauge. Always attempt exit. */
  entered=true;
  report->stage=3;
  if(!control(0x0090))goto done;
  delay_ms(2000);
  if(!poll_operation(0x400,0x400,delay_ms))goto done;
  report->stage=4;
  if(!capacity_block_read(report->before,delay_ms))goto done;
  report->block_saved=true;
  if(((unsigned)report->before[2]<<8|report->before[3])!=report->design_before)
    goto done;
  memcpy(report->after,report->before,32);
  report->after[0]=report->after[2]=PP_BATTERY_CAPACITY_MAH>>8;
  report->after[1]=report->after[3]=PP_BATTERY_CAPACITY_MAH&255;
  may_have_written=true;
  report->stage=5;
  if(!capacity_block_write(report->after,delay_ms))goto done;
  uint8_t actual[32];
  report->stage=6;
  if(!capacity_block_read(actual,delay_ms) || memcmp(actual,report->after,32))
    goto done;
  report->stage=7;
  if(!control(0x0091) || !poll_operation(0x400,0,delay_ms))goto done;
  entered=false;
  /* CFGUPDATE clears before the gauge publishes the refreshed FCC. The real
   * chip still reports its old FCC immediately after exit; wait for its next
   * gauging cycles instead of classifying the successful RAM write as failed. */
  uint16_t design,full;bool capacities_ready=false;
  report->stage=8;
  for(unsigned i=0;i<30;i++) {
    delay_ms(100);
    if(!read_u16(0x3c,&design) || !read_u16(0x12,&full))goto done;
    if(design==PP_BATTERY_CAPACITY_MAH && full==PP_BATTERY_CAPACITY_MAH) {
      capacities_ready=true;break;
    }
  }
  if(!capacities_ready)goto done;
  report->updated=true;report->stage=9;result=1;
done:
  if(result<0 && entered && may_have_written && report->block_saved) {
    uint8_t restored[32];uint16_t mode;
    /* EXIT_REINIT may have succeeded even if its subsequent read failed.
     * Never attempt a data-memory rollback outside confirmed CONFIG UPDATE. */
    if(read_u16(0x3a,&mode) && (mode&0x400))
      report->rollback_verified=capacity_block_write(report->before,delay_ms) &&
        capacity_block_read(restored,delay_ms) && !memcmp(restored,report->before,32);
  }
  if(entered && (!control(0x0092) || !poll_operation(0x400,0,delay_ms)))result=-1;
  /* Preserve sealed protection; an originally full-access gauge stays so. */
  if(was_sealed && access_changed &&
     (!control(0x0030) || !poll_operation(6,6,delay_ms)))result=-1;
  return result;
}
