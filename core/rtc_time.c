#include "rtc_time.h"

static unsigned month_days(unsigned year, unsigned month) {
  static const unsigned days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  return days[month-1] + (month==2 && year%4==0);
}
static int unbcd(uint8_t raw, uint8_t mask) {
  if ((raw & ~mask) || (raw & 15)>9 || (raw>>4)>9) return -1;
  return (raw>>4)*10+(raw&15);
}
static uint8_t bcd(unsigned n) { return (n/10)*16+n%10; }

bool pp_rtc_decode(uint8_t control, const uint8_t raw[7], int64_t *epoch) {
  /* Reject stopped/test/12-hour clocks and oscillator-stop (seconds bit 7).
   * A plausible date alone does not establish validity after loss of power. */
  if (!raw || !epoch || (control & 0xa2)) return false;
  int sec=unbcd(raw[0],0x7f), min=unbcd(raw[1],0x7f);
  int hour=unbcd(raw[2],0x3f), day=unbcd(raw[3],0x3f);
  int month=unbcd(raw[5],0x1f), yy=unbcd(raw[6],0xff);
  if (sec<0 || sec>59 || min<0 || min>59 || hour<0 || hour>23 ||
      day<1 || month<1 || month>12 || yy<25 || yy>99 || raw[4]>6 ||
      (unsigned)day>month_days(2000+yy,month)) return false;
  int64_t days=0;
  for (unsigned y=2000;y<2000u+yy;y++) days+=365+(y%4==0);
  for (int m=1;m<month;m++) days+=month_days(2000+yy,m);
  int64_t value=INT64_C(946684800)+(days+day-1)*86400+hour*3600+min*60+sec;
  if (value<PP_CLOCK_MIN_EPOCH || value>=PP_CLOCK_MAX_EPOCH) return false;
  *epoch=value;
  return true;
}

bool pp_rtc_encode(int64_t epoch, uint8_t raw[7]) {
  if (!raw || epoch<PP_CLOCK_MIN_EPOCH || epoch>=PP_CLOCK_MAX_EPOCH) return false;
  int64_t days=(epoch-INT64_C(946684800))/86400;
  unsigned weekday=(days+6)%7; /* 2000-01-01 was Saturday. */
  unsigned year=2000, month=1;
  while (days>=365+(year%4==0)) { days-=365+(year%4==0);year++; }
  while (days>=month_days(year,month)) { days-=month_days(year,month);month++; }
  unsigned seconds=epoch%86400;
  raw[0]=bcd(seconds%60);raw[1]=bcd(seconds/60%60);raw[2]=bcd(seconds/3600);
  raw[3]=bcd(days+1);raw[4]=weekday;raw[5]=bcd(month);raw[6]=bcd(year-2000);
  return true;
}
