#include "training.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void init(pp_game *g,pp_training *t){
 pp_init(g,123);assert(pp_choose_starter(g,1)==PP_OK);g->count=3;
 for(unsigned i=0;i<3;i++){g->pets[i]=(pp_pet){.species=i+1,.food=90,.clean=90,.health=90,.mood=90,.energy=100};g->growth[i]=(pp_growth){.level=12};for(unsigned k=0;k<6;k++)g->identity[i].iv[k]=15;}
 pp_training_init(t,123);unsigned picks[]={0,1};assert(pp_training_job_start(t,g,0,picks,2)==PP_OK);
 memset(g->inventory,0,sizeof(g->inventory));g->tree_essences=0;
}
static void reload(pp_game *g,pp_training *t){
 pp_training_checkpoint(t,g);assert(pp_valid(g));size_t size=pp_save_size(g);void *blob=malloc(size);assert(blob&&pp_save_write(g,blob,size,100));
 pp_game copy={0};assert(pp_save_read(&copy,blob,size,NULL));free(blob);pp_training_close(t);pp_dispose(g);*g=copy;assert(pp_training_restore(t,g));
}
static void samples(void){
 const unsigned levels[]={0,19,20,29,30,39,40,59,60,100};
 for(unsigned tarot=0;tarot<2;tarot++)for(unsigned sleep=0;sleep<2;sleep++)for(unsigned stock=0;stock<4;stock++)for(unsigned v=0;v<10;v++){
  pp_game g={0};pp_training t;init(&g,&t);unsigned value=levels[v];
  g.pets[0]=(pp_pet){.species=1,.food=value,.clean=value,.health=value,.mood=value,.energy=value,.sleeping=sleep};
  for(unsigned i=0;i<5;i++)g.inventory[i]=stock;g.tree_essences=stock==3;
  g.clock_ready=1;g.day=g.tarot_day=g.tarot_effect_day=1;g.tarot_card=tarot?6:0;
  unsigned used=pp_training_supply(&g,0);pp_pet *p=&g.pets[0];
  printf("[%u,%u,%u,%u,%u,[%u,%u,%u,%u,%u,%u],[%u,%u,%u,%u,%u,%u]]\n",tarot,sleep,stock,value,used,p->food,p->clean,p->health,p->mood,p->energy,p->sleeping,g.inventory[0],g.inventory[1],g.inventory[2],g.inventory[3],g.inventory[4],g.tree_essences);
  pp_training_close(&t);pp_dispose(&g);
 }
}
int main(int argc,char **argv){
 if(argc>1&&!strcmp(argv[1],"--trace")){samples();return 0;}
 pp_game g={0};pp_training t;init(&g,&t);
 g.pets[0].food=g.pets[0].clean=g.pets[0].mood=40;g.pets[0].health=30;g.pets[0].energy=20;
 for(unsigned i=0;i<5;i++)g.inventory[i]=5;g.tree_essences=2;
 assert(!pp_training_supply(&g,0));uint32_t checksum=pp_checksum(&g);assert(!pp_training_supply(&g,2));assert(!pp_training_supply(&g,99));assert(pp_checksum(&g)==checksum);
 g.pets[0].food=39;assert(pp_training_supply(&g,0)==1&&g.pets[0].food==79&&g.inventory[0]==4);
 g.pets[0].energy=19;g.pets[0].sleeping=1;assert(pp_training_supply(&g,0)==2&&g.pets[0].energy==69&&!g.pets[0].sleeping&&g.inventory[4]==3);
 g.pets[0].health=29;assert(pp_training_supply(&g,0)==1&&g.pets[0].health==100&&g.tree_essences==1&&g.inventory[2]==5);
 g.inventory[2]=0;reload(&g,&t);checksum=pp_checksum(&g);assert(!pp_training_supply(&g,0)&&pp_checksum(&g)==checksum);
 pp_training_close(&t);pp_dispose(&g);
 // A failed wake-up ends the job; disabled mode leaves the stock alone.
 for(unsigned enabled=0;enabled<2;enabled++){
  init(&g,&t);g.inventory[4]=1;g.pets[0].energy=19;g.pets[0].sleeping=1;
  pp_training_job_advance_auto(&t,&g,40,enabled);
  assert(g.inventory[4]==(enabled?0:1));if(enabled)assert(g.training.status==2&&g.pets[0].energy==44&&g.pets[0].sleeping);
  pp_training_close(&t);pp_dispose(&g);
 }
 // Shared stock goes to team order, and cannot affect the idle pet or buy items.
 init(&g,&t);g.coins=1000;g.inventory[0]=1;g.pets[0].food=g.pets[1].food=g.pets[2].food=19;
 pp_training_job_advance_auto(&t,&g,40,true);assert(g.training.status==2&&g.pets[0].food==59&&g.pets[1].food==19&&g.pets[2].food==19&&g.coins==1000&&!g.inventory[0]);
 pp_training_close(&t);pp_dispose(&g);
 // Whole-hour catch-up and foreground slices, with save/reload, consume identically.
 pp_training_record expected={0};pp_pet pets[3];pp_growth growth[3];uint16_t inventory[PP_ITEMS];unsigned essence=0,coins=0;
 for(unsigned variant=0;variant<2;variant++){
  init(&g,&t);for(unsigned i=0;i<5;i++)g.inventory[i]=i==2?0:100;g.tree_essences=30;g.pets[0].food=39;
  unsigned spans[]={17,51,233,1000,59999},n=0;
  while(g.training.status==1){pp_training_job_advance_auto(&t,&g,variant?spans[n++%5]:60000,true);if(variant&&n%101==0)reload(&g,&t);}
  assert(g.training.elapsed_ms==PP_TRAINING_DURATION_MS&&g.inventory[0]<100&&g.tree_essences<30);
  if(!variant){expected=g.training;memcpy(pets,g.pets,sizeof(pets));memcpy(growth,g.growth,sizeof(growth));memcpy(inventory,g.inventory,sizeof(inventory));essence=g.tree_essences;coins=g.coins;}
  else{assert(!memcmp(&expected,&g.training,sizeof(expected)));assert(!memcmp(pets,g.pets,sizeof(pets)));assert(!memcmp(growth,g.growth,sizeof(growth)));assert(!memcmp(inventory,g.inventory,sizeof(inventory)));assert(essence==g.tree_essences&&coins==g.coins);}
  checksum=pp_checksum(&g);pp_training_job_advance_auto(&t,&g,3600000,true);assert(pp_checksum(&g)==checksum);
  pp_training_close(&t);pp_dispose(&g);
 }
 puts("PASS: supply thresholds, ordinary-before-essence, sleeping biscuits, shared stock, no purchases, idle isolation, disabled, one-hour slicing/reload and no duplicate use");
 return 0;
}
