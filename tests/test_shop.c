#include "game.h"
#include "shop_reel.h"
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc,char **argv){
  (void)argv;
  pp_game g={0};pp_init(&g,123);unsigned item=99;
  assert(!g.inventory[2] && !pp_item_enabled(2));
  uint32_t before=pp_checksum(&g);
  assert(pp_buy(&g,2)==PP_INVALID && pp_use(&g,2,0)==PP_INVALID && pp_use_auto(&g,2)==PP_INVALID && pp_checksum(&g)==before);
  g.coins=29;before=pp_checksum(&g);assert(pp_shop_draw(&g,&item)==PP_NO_COINS && item==99 && pp_checksum(&g)==before);
  g.coins=100;before=pp_checksum(&g);assert(pp_shop_draw(&g,NULL)==PP_INVALID && pp_checksum(&g)==before);
  for(unsigned i=0;i<PP_SHOP_ITEMS;i++)if(pp_item_enabled(i)){
    uint16_t *stock=i<PP_ITEMS?&g.inventory[i]:i==PP_HATCH_BOOST?&g.hatch_boosts:i==PP_BALL?&g.balls:&g.tree_essences;
    unsigned old=*stock;*stock=UINT16_MAX;before=pp_checksum(&g);
    assert(pp_shop_draw(&g,&item)==PP_FULL && pp_checksum(&g)==before);*stock=old;
  }
  unsigned counts[9]={0};
  for(unsigned n=0;n<100000;n++){
    g.coins=30;unsigned stock[9];for(unsigned i=0;i<9;i++)stock[i]=pp_item_count(&g,i);
    assert(pp_shop_draw(&g,&item)==PP_OK && item!=2 && !g.coins);
    for(unsigned i=0;i<9;i++)assert(pp_item_count(&g,i)==stock[i]+(i==item));
    counts[item]++;
    if(n%1000==0){for(unsigned i=0;i<6;i++)g.inventory[i]=0;g.hatch_boosts=g.balls=g.tree_essences=0;}
  }
  static const unsigned weights[9]=PP_SHOP_WEIGHTS;
  for(unsigned i=0;i<9;i++){int delta=(int)counts[i]-(int)(weights[i]*1000);assert(abs(delta)<700);}
  g.coins=100;g.inventory[2]=3;g.training.items[2]=2;g.training.last_item=3;
  unsigned bytes=pp_save_size(&g);void *data=malloc(bytes);assert(data&&pp_save_write(&g,data,bytes,0));pp_game copy={0};
  assert(pp_save_read(&copy,data,bytes,NULL)&&copy.coins==175&&!copy.inventory[2]&&!copy.training.items[2]&&!copy.training.last_item);
  before=pp_checksum(&copy);pp_shop_retire_legacy(&copy);assert(pp_checksum(&copy)==before);
  assert(pp_save_write(&copy,data,bytes,0)&&pp_save_read(&g,data,bytes,NULL)&&g.coins==175);
  g.coins=UINT32_MAX-1;g.inventory[2]=65535;pp_shop_retire_legacy(&g);assert(g.coins==UINT32_MAX&&!g.inventory[2]);
  free(data);pp_dispose(&g);pp_dispose(&copy);
  static const unsigned ring[8]=PP_SHOP_RING;
  static const unsigned delays[]={33,66,132,250,1200,0};
  for(unsigned i=0;i<9;i++)if(pp_item_enabled(i))for(unsigned pattern=0;pattern<6;pattern++){
    unsigned elapsed=0,steps=0,off=0,seen=1,frames=0,target=0;
    while(pp_shop_slot(ring[target])!=i)target++;
    pp_shop_reel_view previous=pp_shop_reel_at(i,0);
    while(!previous.done){
      unsigned delta=delays[pattern];if(!delta)delta=(unsigned[]){33,99,450,17,1800,66}[frames%6];
      assert(pp_shop_reel_advance(i,elapsed,0)==elapsed);
      unsigned next=pp_shop_reel_advance(i,elapsed,delta);
      pp_shop_reel_view view=pp_shop_reel_at(i,next);
      if(view.slot!=previous.slot){
        unsigned index=0;while(ring[index]!=previous.slot)index++;
        assert(view.slot==ring[(index+1)%8]&&view.visible&&previous.visible);steps++;
      }
      if(previous.visible&&!view.visible){off++;assert(view.slot==ring[target]&&steps==24+target);}
      if(!view.visible)assert(view.slot==ring[target]);
      for(unsigned index=0;index<8;index++)if(view.slot==ring[index])seen|=1u<<index;
      if(argc>1)printf("%u %u %u %u %u %u %u\n",i,elapsed,delta,next,view.slot,view.visible,view.done);
      elapsed=next;previous=view;assert(++frames<2000);
    }
    assert(seen==255&&steps==24+target&&off==3&&previous.visible&&pp_shop_slot(previous.slot)==i);
    assert(pp_shop_reel_advance(i,elapsed,UINT_MAX)==elapsed);
  }
  puts("PASS: 100000 paid draws and original error/refund/save checks; eight prizes x six render cadences, adjacent clockwise cells, three complete laps, exactly three winner flashes, stall/no-catch-up and final hold");
}
