#include "game.h"
#include "pet_energy.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static pp_game g,live;
static void init(void){pp_init(&g,42);assert(pp_choose_starter(&g,1)==PP_OK);pp_tick(&g,0,20261027);}
static void oracle(uint8_t *e,uint8_t *sleep,unsigned ticks,unsigned first,bool sun,bool moon){
 (void)first;(void)sun;unsigned fraction=0;if(*e<20)*sleep=1;if(*e>=60)*sleep=0;
 for(unsigned i=0;i<ticks*600u&&*e<100;i++){unsigned rate=*sleep?25:10;if(moon)rate=rate*120/100;fraction+=rate;if(fraction>=3600){fraction-=3600;(*e)++;}if(*sleep&&*e>=60)*sleep=0;}
}
int main(int argc,char **argv){
 const unsigned spans[]={0,1,2,3,4,10,63,72,123,1008};
 bool samples=argc>1&&!strcmp(argv[1],"--samples");unsigned cases=0;
 for(unsigned e=0;e<=100;e++)for(unsigned sleeping=0;sleeping<2;sleeping++)
 for(unsigned effect=0;effect<3;effect++)for(unsigned first=1;first<=2;first++)
 for(unsigned n=0;n<sizeof spans/sizeof spans[0];n++){
  uint8_t actual=e,asleep=sleeping,expected=e,esleep=sleeping;
  pp_energy_advance(&actual,&asleep,spans[n],first,effect==1,effect==2);
  oracle(&expected,&esleep,spans[n],first,effect==1,effect==2);
  assert(actual==expected&&asleep==esleep);cases++;
  if(samples)printf("%u %u %u %u %u %u %u\n",e,sleeping,effect,first,spans[n],actual,asleep);
 }
 if(samples)return 0;
 /* Saved fractional recovery gives identical results across arbitrary chunks. */
 for(unsigned energy=0;energy<=100;energy++)for(unsigned sleeping=0;sleeping<2;sleeping++){
  uint8_t a=energy,b=energy,sa=sleeping,sb=sleeping;uint16_t fa=0,fb=0;
  pp_energy_restore(&a,&sa,&fa,28800,false);
  for(unsigned i=0;i<28800;i++)pp_energy_restore(&b,&sb,&fb,1,false);
  assert(a==b&&sa==sb&&fa==fb);
 }
 init();g.pets[0].energy=10;g.pets[0].sleeping=1;pp_tick(&g,7199,g.day);assert(g.pets[0].energy==59&&g.pets[0].sleeping);pp_tick(&g,1,g.day);assert(g.pets[0].energy==60&&!g.pets[0].sleeping);
 for(unsigned e=34;e<=35;e++){init();g.inventory[4]=1;g.pets[0].energy=e;g.pets[0].sleeping=1;assert(pp_use(&g,4,0)==PP_OK);pp_tick(&g,4,g.day);assert(g.pets[0].energy==e+25&&g.pets[0].sleeping==(e==34));}
 pp_dispose(&g);pp_dispose(&live);printf("PASS: %u energy samples, per-second oracle, split recovery and biscuit threshold\n",cases);
}
