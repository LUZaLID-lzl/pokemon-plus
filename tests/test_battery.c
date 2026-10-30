#include "battery_monitor.h"
#include <assert.h>
#include <stdio.h>
static unsigned calls,reads,fail_reg=256;
static uint16_t soc=72,status=8,operation=32,current;
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t dev,const void *tx,size_t n,int timeout){
 (void)dev;(void)tx;(void)n;(void)timeout;assert(!"ordinary sampling must never write gauge configuration");return -1;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,const i2c_device_config_t *cfg,i2c_master_dev_handle_t *out){
 assert(bus==(void *)1&&cfg->device_address==0x55&&cfg->scl_speed_hz==400000&&cfg->dev_addr_length==I2C_ADDR_BIT_LEN_7);
 calls++;*out=(void *)2;return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t dev,const void *tx,size_t txlen,void *rx,size_t rxlen,int timeout){
 assert(dev==(void *)2&&txlen==1&&rxlen==2&&timeout==20);reads++;
 uint8_t reg=*(const uint8_t *)tx;if(reg==fail_reg)return -1;
 uint16_t value=reg==0x2c?soc:reg==0x0a?status:reg==0x3a?operation:current;
 assert(reg==0x2c||reg==0x0a||reg==0x3a||reg==0x0c);
 ((uint8_t *)rx)[0]=value&255;((uint8_t *)rx)[1]=value>>8;return ESP_OK;
}
int main(void){
 assert(!pp_battery_monitor_init(NULL));assert(!pp_battery_monitor_read().valid);
 assert(pp_battery_monitor_init((void *)1)&&calls==1);assert(pp_battery_monitor_init((void *)1)&&calls==1);
 pp_battery_reading b=pp_battery_monitor_read();assert(b.valid&&b.percent==72&&!b.charging&&reads==4);
 current=100;b=pp_battery_monitor_read();assert(b.charging);
 current=(uint16_t)-100;b=pp_battery_monitor_read();assert(!b.charging);
 status=9;current=20;b=pp_battery_monitor_read();assert(!b.charging);status=8;
 for(unsigned i=0;i<=100;i++){soc=i;b=pp_battery_monitor_read();assert(b.valid&&b.percent==(int)i);}
 soc=101;assert(!pp_battery_monitor_read().valid);soc=65535;assert(!pp_battery_monitor_read().valid);soc=72;
 status=0;b=pp_battery_monitor_read();assert(!b.valid&&b.percent==-1);status=8;
 operation=0;assert(!pp_battery_monitor_read().valid);operation=32;
 unsigned regs[]={0x2c,0x0a,0x3a,0x0c};for(unsigned i=0;i<4;i++){fail_reg=regs[i];reads=0;b=pp_battery_monitor_read();assert(!b.valid&&b.percent==-1&&reads==i+1);}
 puts("PASS: read-only BQ27220 on BSP bus, little-endian SOC/status/current, 0..100%, absent/uninitialized/error fallback, one device handle and bounded I2C timeouts");
}
