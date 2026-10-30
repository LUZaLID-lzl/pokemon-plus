#define _POSIX_C_SOURCE 200809L
#include "rtc_clock.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint8_t regs[18];
static unsigned adds,reads,writes,fail_read,fail_write;
static bool add_fail,corrupt;
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,const i2c_device_config_t *cfg,i2c_master_dev_handle_t *out) {
  assert(bus==(void *)1 && cfg->device_address==0x51 && cfg->scl_speed_hz==400000 && cfg->dev_addr_length==I2C_ADDR_BIT_LEN_7);
  adds++;if (add_fail) return ESP_FAIL;*out=(void *)2;return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t dev,const void *tx,size_t txlen,void *rx,size_t rxlen,int timeout) {
  assert(dev==(void *)2 && txlen==1 && timeout==20);
  uint8_t reg=*(const uint8_t *)tx;
  assert(reg==0 && (rxlen==1 || rxlen==11));
  if (++reads==fail_read) return ESP_FAIL;
  memcpy(rx,regs+reg,rxlen);
  if (corrupt && rxlen==11) ((uint8_t *)rx)[4]=0x55;
  return ESP_OK;
}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t dev,const void *data,size_t len,int timeout) {
  assert(dev==(void *)2 && timeout==20);
  const uint8_t *tx=data;
  assert((tx[0]==0 && len==2 && !(tx[1]&0x92)) || (tx[0]==4 && len==8) ||
         (tx[0]==3 && len==2 && tx[1]==0xa7));
  if (++writes==fail_write) return ESP_FAIL;
  if (tx[0]==4) assert(regs[0]&0x20); /* Atomic update must stop divider first. */
  memcpy(regs+tx[0],tx+1,len-1);return ESP_OK;
}
static void fixture(int64_t epoch) {
  memset(regs,0,sizeof(regs));regs[3]=0xa7;assert(pp_rtc_encode(epoch,regs+4));
  reads=writes=fail_read=fail_write=0;corrupt=false;
}
int main(void) {
  uint8_t raw[7],invalid[7];int64_t epoch=42;
  assert(!pp_rtc_encode(PP_CLOCK_MIN_EPOCH-1,raw));
  assert(!pp_rtc_encode(PP_CLOCK_MAX_EPOCH,raw));
  assert(!pp_rtc_encode(PP_CLOCK_MIN_EPOCH,NULL));
  /* Every day through 2099 against the host Gregorian UTC calendar. Covers
   * leap days, month/year rollover, 2038, two-digit years and weekday mapping. */
  unsigned samples=0;
  for (int64_t day=PP_CLOCK_MIN_EPOCH;day<PP_CLOCK_MAX_EPOCH;day+=86400) {
    const unsigned offsets[]={0,1,43210,86399};
    for (unsigned i=0;i<4;i++) {
      time_t t=(time_t)(day+offsets[i]);struct tm utc;assert(gmtime_r(&t,&utc));
      assert(pp_rtc_encode(t,raw));
      assert(raw[6]==((utc.tm_year-100)/10)*16+(utc.tm_year-100)%10);
      assert(raw[4]==utc.tm_wday);
      assert(pp_rtc_decode(0x01,raw,&epoch) && epoch==t);samples++;
    }
  }
  /* Decode must be timezone-independent; UI alone applies UTC+8. */
  assert(setenv("TZ","CST-8",1)==0);tzset();
  assert(pp_rtc_encode(INT64_C(1792987200),raw)); /* 2026-10-26 04:00 UTC. */
  assert(pp_rtc_decode(1,raw,&epoch) && epoch==INT64_C(1792987200));
  time_t t=epoch;struct tm local;assert(localtime_r(&t,&local));assert(local.tm_hour==12);
  for (unsigned control=0;control<256;control++)
    assert(pp_rtc_decode(control,raw,&epoch)==!(control&0xa2));
  assert(!pp_rtc_decode(0,NULL,&epoch));assert(!pp_rtc_decode(0,raw,NULL));
  const uint8_t bad[]={0x80,0x6a,0x24,0,7,0x13,0xfa};
  for (unsigned i=0;i<7;i++) {
    memcpy(invalid,raw,7);invalid[i]=bad[i];epoch=42;
    assert(!pp_rtc_decode(0,invalid,&epoch) && epoch==42);
  }
  memcpy(invalid,raw,7);invalid[3]=0x29;invalid[5]=0x02;invalid[6]=0x26;
  assert(!pp_rtc_decode(0,invalid,&epoch));invalid[6]=0x28;
  assert(pp_rtc_decode(0,invalid,&epoch));
  assert(pp_rtc_clock_read(&epoch)==ESP_ERR_INVALID_STATE);
  assert(pp_rtc_clock_read(NULL)==ESP_ERR_INVALID_ARG);
  assert(pp_rtc_clock_write(PP_CLOCK_MIN_EPOCH)==ESP_ERR_INVALID_STATE);
  assert(pp_rtc_clock_init(NULL)==ESP_ERR_INVALID_ARG);
  add_fail=true;assert(pp_rtc_clock_init((void *)1)==ESP_FAIL);add_fail=false;
  assert(pp_rtc_clock_init((void *)1)==ESP_OK && adds==2);
  assert(pp_rtc_clock_init((void *)1)==ESP_OK && adds==2 && !writes);
  const int64_t target=INT64_C(1792987200);
  fixture(target);assert(pp_rtc_clock_read(&epoch)==ESP_OK && epoch==target && !writes);
  fixture(target);fail_read=1;epoch=42;
  assert(pp_rtc_clock_read(&epoch)==ESP_FAIL && epoch==42 && !writes);
  fixture(target);regs[4]|=0x80;assert(pp_rtc_clock_read(&epoch)==ESP_ERR_INVALID_STATE && !writes);
  fixture(target);regs[3]=0;assert(pp_rtc_clock_read(&epoch)==ESP_ERR_INVALID_STATE && !writes);
  assert(pp_rtc_clock_write(target)==ESP_OK && regs[3]==0xa7);
  fixture(target);regs[3]=0;fail_write=4;
  assert(pp_rtc_clock_write(target)==ESP_FAIL && regs[3]==0);
  assert(pp_rtc_clock_read(&epoch)==ESP_ERR_INVALID_STATE);
  fail_write=0;assert(pp_rtc_clock_write(target)==ESP_OK);
  for (unsigned i=1;i<=4;i++) {
    fixture(target);fail_write=i;
    assert(pp_rtc_clock_write(target+60)==ESP_FAIL && writes==i);
    if (i==2 || i==3) assert(regs[0]&0x20); /* Torn calendar stays invalid. */
    fail_write=0;
    assert(pp_rtc_clock_write(target+60)==ESP_OK);
    assert(pp_rtc_clock_read(&epoch)==ESP_OK && epoch==target+60);
  }
  fixture(target);regs[0]=0xa7;regs[4]|=0x80;
  assert(pp_rtc_clock_write(target+60)==ESP_OK && regs[0]==0x05 && writes==4);
  assert(pp_rtc_clock_read(&epoch)==ESP_OK && epoch==target+60);
  fixture(target);corrupt=true;assert(pp_rtc_clock_write(target)==ESP_ERR_INVALID_RESPONSE);
  for (unsigned i=1;i<=2;i++) {
    fixture(target);fail_read=i;assert(pp_rtc_clock_write(target)==ESP_FAIL);
  }
  fixture(target);assert(pp_rtc_clock_write(0)==ESP_ERR_INVALID_ARG && !writes && !reads);
  printf("PASS: %u UTC calendar samples; leap/2038/timezone; OS/STOP/BCD validation; read-only boot; atomic writes/readback; I2C failure/retry\n",samples);
}
