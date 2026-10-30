#include "rtc_clock.h"
#include "sdkconfig.h"

/* U5 PCF85063ATL, 0x51, on the existing BSP SCL10/SDA11 bus.
 * Do not call the vendor init/reset: boot must preserve the running clock.
 * Use UTC/2000-based years instead of the vendor helper's 1970 convention. */
static i2c_master_dev_handle_t rtc;
/* Battery-backed RAM byte tags our UTC/2000 calendar. Factory/demo firmware
 * may have stored local time or a 1970-based year: do not silently reuse it. */
#define RTC_UTC_FORMAT 0xa7
static esp_err_t read_registers(uint8_t reg, void *out, size_t length) {
  return i2c_master_transmit_receive(rtc,&reg,1,out,length,20);
}
static esp_err_t write_control(uint8_t control) {
  uint8_t bytes[]={0,control};
  return i2c_master_transmit(rtc,bytes,sizeof(bytes),20);
}
esp_err_t pp_rtc_clock_init(i2c_master_bus_handle_t bus) {
  if (!bus) return ESP_ERR_INVALID_ARG;
  if (rtc) return ESP_OK;
  const i2c_device_config_t cfg={.dev_addr_length=I2C_ADDR_BIT_LEN_7,
    .device_address=0x51,.scl_speed_hz=CONFIG_BSP_I2C_CLK_SPEED_HZ};
  return i2c_master_bus_add_device(bus,&cfg,&rtc);
}
esp_err_t pp_rtc_clock_read(int64_t *epoch) {
  if (!epoch) return ESP_ERR_INVALID_ARG;
  if (!rtc) return ESP_ERR_INVALID_STATE;
  uint8_t registers[11];
  esp_err_t err=read_registers(0,registers,sizeof(registers));
  if (err!=ESP_OK) return err;
  return registers[3]==RTC_UTC_FORMAT && pp_rtc_decode(registers[0],registers+4,epoch)
    ?ESP_OK:ESP_ERR_INVALID_STATE;
}
esp_err_t pp_rtc_clock_write(int64_t epoch) {
  uint8_t bytes[8]={4},control;
  if (!pp_rtc_encode(epoch,bytes+1)) return ESP_ERR_INVALID_ARG;
  if (!rtc) return ESP_ERR_INVALID_STATE;
  esp_err_t err=read_registers(0,&control,1);
  if (err!=ESP_OK) return err;
  /* STOP freezes the divider for one coherent seven-register calendar write.
   * Keep the clock stopped if that write fails: never accept a torn date.
   * 12.5pF CAP_SEL matches the official board driver; preserve CIE/other bits. */
  control=(control & ~0xb2u)|1u;
  err=write_control(control|0x20);
  if (err!=ESP_OK) return err;
  err=i2c_master_transmit(rtc,bytes,sizeof(bytes),20);
  if (err!=ESP_OK) return err;
  err=write_control(control);
  if (err!=ESP_OK) return err;
  uint8_t format[]={3,RTC_UTC_FORMAT};
  err=i2c_master_transmit(rtc,format,sizeof(format),20);
  if (err!=ESP_OK) return err;
  int64_t actual;
  err=pp_rtc_clock_read(&actual);
  if (err!=ESP_OK) return err;
  return actual>=epoch && actual<=epoch+1?ESP_OK:ESP_ERR_INVALID_RESPONSE;
}
