#include "battery_monitor.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static uint8_t block[32],original[32],pending[32];
static uint16_t status=8,operation=0x24,design=3000,full=3000;
static unsigned writes,commits,enters,reinits,exits,waited,commands,fail_call;
static bool enter_timeout,exit_timeout,bad_checksum,bad_echo,bad_length,corrupt_once;
static uint16_t previous_key;
static unsigned settle_reads,settle_left;
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,const i2c_device_config_t *cfg,i2c_master_dev_handle_t *out){
 assert(bus==(void *)1&&cfg->device_address==0x55);*out=(void *)2;return ESP_OK;
}
static bool fail(void){return ++commands==fail_call;}
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t dev,const void *data,size_t n,int timeout){
 assert(dev==(void *)2&&timeout==20);if(fail())return -1;
 const uint8_t *t=data;writes++;
 if(t[0]==0){
  assert(n==3);uint16_t c=t[1]|((uint16_t)t[2]<<8);
  if(c==0x0414||c==0x3672){if(previous_key==0x0414&&c==0x3672)operation=(operation&~6)|4;previous_key=c;}
  else if(c==0xffff){assert(!"capacity correction must not request FULL ACCESS");}
  else if(c==0x90){enters++;if(!enter_timeout)operation|=0x400;}
  else if(c==0x91||c==0x92){
   if(c==0x91){reinits++;settle_left=settle_reads;if(!settle_left)full=((unsigned)block[0]<<8)|block[1];design=((unsigned)block[2]<<8)|block[3];}
   else exits++;
   if(!exit_timeout)operation&=~0x400;
  }else if(c==0x30)operation=(operation&~6)|6;
  else assert(!"unexpected control command");
 }else if(t[0]==0x3e){assert((n==3||n==7)&&t[1]==0x9d&&t[2]==0x92);if(n==7){memcpy(pending,block,32);memcpy(pending,t+3,4);}}
 else if(t[0]==0x40){assert(n==33);memcpy(pending,t+1,32);}
 else if(t[0]==0x60){
  unsigned sum=0x9d+0x92;for(unsigned i=0;i<4;i++)sum+=pending[i];
  assert(n==3&&t[1]==(uint8_t)(255-(sum&255))&&t[2]==8);
  assert(operation&0x400);memcpy(block,pending,32);commits++;
  if(corrupt_once){block[0]^=1;corrupt_once=false;}
 }else assert(!"no OTP/chemistry/charger writes allowed");
 return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t dev,const void *tx,size_t n,void *rx,size_t length,int timeout){
 assert(dev==(void *)2&&n==1&&timeout==20);if(fail())return -1;
 uint8_t reg=*(const uint8_t *)tx,*out=rx;
 if(reg==0x3e){
  assert(length==36);out[0]=bad_echo?0x9e:0x9d;out[1]=0x92;memcpy(out+2,block,32);
  unsigned sum=0;for(unsigned i=0;i<34;i++)sum+=out[i];
  out[34]=(uint8_t)(255-(sum&255));if(bad_checksum)out[34]^=1;
  out[35]=bad_length?35:36;return ESP_OK;
 }
 assert(length==2);uint16_t value=0;
 switch(reg){
 case 0x3a:value=operation;break;case 0x0a:value=status;break;
 case 0x3c:value=design;break;case 0x12:if(settle_left&&!--settle_left)full=((unsigned)block[0]<<8)|block[1];value=full;break;
 case 0x2c:value=26;break;case 0x08:value=4157;break;
 case 0x06:value=3034;break;case 0x10:value=771;break;
 case 0x0c:value=(uint16_t)-7;break;case 0x14:value=(uint16_t)-3;break;
 default:assert(!"unexpected standard register");
 }
 out[0]=value&255;out[1]=value>>8;return ESP_OK;
}
static void delay(unsigned ms){waited+=ms;assert(ms<=2000);}
static void reset(void){
 status=8;operation=0x24;design=full=3000;
 writes=commits=enters=reinits=exits=waited=commands=fail_call=0;
 previous_key=0;settle_reads=settle_left=0;enter_timeout=exit_timeout=bad_checksum=bad_echo=bad_length=corrupt_once=false;
 for(unsigned i=0;i<32;i++)block[i]=(uint8_t)(i*7+11);
 block[0]=block[2]=0x0b;block[1]=block[3]=0xb8;
 memcpy(original,block,32);
}
int main(void){
 pp_battery_profile_report r;
 assert(pp_battery_monitor_configure_500mah(delay,&r)==-1);
 assert(pp_battery_monitor_init((void *)1));
 reset();assert(pp_battery_monitor_configure_500mah(NULL,&r)==-1&&writes==0);
 for(unsigned security=1;security<=3;security++){
  reset();operation=(operation&~6)|(security<<1);
  assert(pp_battery_monitor_configure_500mah(delay,&r)==1);
  assert(design==500&&full==500&&r.updated&&r.block_saved);
  assert(commits==1&&enters==1&&reinits==1&&waited>=2000&&!(operation&0x400));
  assert(!memcmp(block+4,original+4,28)&&!memcmp(r.before,original,32));
  if(security==3)assert((operation&6)==6);
  unsigned before=writes;full=465;
  assert(pp_battery_monitor_configure_500mah(delay,&r)==0&&writes==before&&full==465);
 }
 reset();settle_reads=15;assert(pp_battery_monitor_configure_500mah(delay,&r)==1&&full==500&&waited>=3500);
 reset();settle_reads=40;assert(pp_battery_monitor_configure_500mah(delay,&r)==-1&&r.stage==8&&waited<6000&&!(operation&0x400));
 reset();status=0;assert(pp_battery_monitor_configure_500mah(delay,&r)==0&&writes==0);
 reset();operation&=~32;assert(pp_battery_monitor_configure_500mah(delay,&r)==-1&&writes==0);
 reset();operation|=0x400;assert(pp_battery_monitor_configure_500mah(delay,&r)==1&&exits==1&&design==500);
 for(unsigned kind=0;kind<3;kind++){
  reset();bad_checksum=kind==0;bad_echo=kind==1;bad_length=kind==2;
  assert(pp_battery_monitor_configure_500mah(delay,&r)==-1&&commits==0&&exits==1&&!(operation&0x400));
 }
 reset();enter_timeout=true;assert(pp_battery_monitor_configure_500mah(delay,&r)==-1&&commits==0&&waited<=4000&&exits==1);
 reset();corrupt_once=true;assert(pp_battery_monitor_configure_500mah(delay,&r)==-1&&r.rollback_verified&&commits==2&&!memcmp(block,original,32)&&!(operation&0x400));
 reset();exit_timeout=true;assert(pp_battery_monitor_configure_500mah(delay,&r)==-1&&waited<=6200&&!r.updated);
 /* Fail every individual bus transfer; cleanup must remain bounded. */
 reset();assert(pp_battery_monitor_configure_500mah(delay,&r)==1);unsigned total=commands;
 for(unsigned i=1;i<=total;i++){
  reset();fail_call=i;assert(pp_battery_monitor_configure_500mah(delay,&r)==-1);
  assert(waited<=6100&&!(operation&0x400));
  assert(!memcmp(block+4,original+4,28));
 }
 reset();pp_battery_diagnostics d;assert(pp_battery_monitor_diagnostics(&d));
 assert(d.voltage_mv==4157&&d.temperature_dk==3034&&d.current_ma==-7&&d.average_ma==-3&&d.design_mah==3000&&writes==0);
 for(unsigned i=1;i<=10;i++){
  reset();fail_call=i;memset(&d,0x5a,sizeof(d));pp_battery_diagnostics before=d;
  assert(!pp_battery_monitor_diagnostics(&d)&&!memcmp(&d,&before,sizeof(d))&&writes==0);
 }
 puts("PASS: 500mAh RAM capacity transaction, all access modes, BE block checksum/echo/length, 28 preserved bytes, rollback, every bus failure, bounded mode exit, learned capacity preserved, signed diagnostics; no OTP/charger/chemistry writes");
}
