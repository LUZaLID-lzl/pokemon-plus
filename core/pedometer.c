#include "pedometer.h"
#include <math.h>
#include <string.h>
unsigned pp_pedometer_sample(pp_pedometer *s,uint64_t ms,float x,float y,float z) {
  float magnitude=sqrtf(x*x+y*y+z*z);
  if (!isfinite(magnitude) || magnitude<1 || magnitude>39.2f) {
    memset(s,0,sizeof(*s)); return 0;
  }
  if (!s->initialized || ms<=s->last_ms || ms-s->last_ms>300) {
    *s=(pp_pedometer){.initialized=true,.last_ms=ms,.settled_ms=ms,.gravity=magnitude}; return 0;
  }
  float dt=(float)(ms-s->last_ms); s->last_ms=ms;
  s->gravity+=dt/(800.0f+dt)*(magnitude-s->gravity);
  s->filtered+=dt/(60.0f+dt)*((magnitude-s->gravity)-s->filtered);
  if (ms-s->settled_ms<1000) return 0;
  if (s->filtered<-.4f) s->armed=true;
  if (!s->armed || s->filtered<1.05f) return 0;
  s->armed=false;
  uint64_t interval=ms-s->candidate_ms;
  if (s->candidate_ms && interval<280) {s->run=0;s->candidate_ms=ms;return 0;}
  unsigned previous=s->run;
  s->run=(!s->candidate_ms||interval>1500)?1:s->run<3?s->run+1:3;
  s->candidate_ms=ms;
  return s->run==3?(previous==2?3:1):0;
}
