#include "game.h"
#include "balance_config.h"
#include "pet_energy.h"
#include "scene_layout.h"
#include "scene_nests.h"
_Static_assert(PP_SCENE_NEST_COUNT==PP_SCENES,"one egg nest per scene");
#include "scene_nests.h"
#include "legacy_game_v1.h"
#include "legacy_game_v2.h"
#include "legacy_game_v3.h"
#include "legacy_game_v4.h"
#include "legacy_game_v5.h"
#include "legacy_game_v6.h"
#include "legacy_game_v7.h"
#include "legacy_game_v8.h"
#include "legacy_game_v9.h"
#include "legacy_game_v10.h"
#include "legacy_game_v11.h"
#include "legacy_game_v12.h"
#include "legacy_game_v13.h"
#include "legacy_game_v14.h"
#include "legacy_game_v15.h"
#include "legacy_game_v16.h"
#include "moves.h"
#include "pet_types.h"
#include "pet_habitats.h"
#include "collection_catalog.h"
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
const pp_dex_entry pp_pokedex[PP_DEX_COUNT] = { PP_DEX_ENTRIES };
const char *const pp_pet_names[] = {"皮卡丘", "妙蛙种子", "小火龙", "杰尼龟", "绿毛虫", "独角虫", "波波", "小拉达", "烈雀", "阿柏蛇", "穿山鼠", "尼多兰", "尼多朗", "皮皮", "六尾", "胖丁", "超音蝠", "走路草", "派拉斯", "毛球", "地鼠", "喵喵", "可达鸭", "猴怪", "卡蒂狗", "蚊香蝌蚪", "凯西", "腕力", "喇叭芽", "玛瑙水母", "小拳石", "小火马", "呆呆兽", "小磁怪", "大葱鸭", "嘟嘟", "小海狮", "臭泥", "大舌贝", "鬼斯", "大岩蛇", "催眠貘", "大钳蟹", "霹雳电球", "蛋蛋", "卡拉卡拉", "飞腿郎", "艾比郎", "大舌头", "瓦斯弹", "独角犀牛", "吉利蛋", "蔓藤怪", "袋兽", "墨海马", "角金鱼", "海星星", "魔墙人偶", "飞天螳螂", "迷唇姐", "电击兽", "鸭嘴火兽", "凯罗斯", "肯泰罗", "鲤鱼王", "拉普拉斯", "百变怪", "伊布", "多边兽", "菊石兽", "化石盔", "化石翼龙", "卡比兽", "急冻鸟", "闪电鸟", "火焰鸟", "迷你龙", "超梦", "梦幻"};
const char *const pp_scene_names[] = {
    "清晨庭院", "樱花小院", "晴日海岸", "林间瀑布", "秋日木林", "雪落小镇",
    "水晶洞穴", "蘑菇秘境", "星空山丘", "沙漠绿洲", "雨中村庄", "天空岛", "神树镜湖"};
const char *const pp_tarot_names[] = {
    "太阳 · 活力", "星星 · 希望", "月亮 · 想象", "世界 · 圆满",
    "力量 · 勇气", "恋人 · 陪伴", "节制 · 平衡", "命运 · 惊喜"};
const char *const pp_tarot_effects[] = {
  "活动精力消耗 -10%", "计步经验 +10%", "休息恢复 +20%",
  "旅行金币 +20%", "挑战伤害 +10%", "摸头心情 +4 · 经验 +2",
  "道具恢复 +15%", "每千步金币 +2"
};
bool pp_tarot_active(const pp_game *g, unsigned card) {
  return g->clock_ready && g->day && g->tarot_effect_day == g->day &&
         g->tarot_day == g->day && g->tarot_card == card;
}
static unsigned travel_coins(const pp_game *g) {
  /* Derive from completed events so existing in-flight saves also receive
   * the new amounts, without rewriting their save layout. */
  unsigned events=g->travel_events>6?6:g->travel_events;
  unsigned coins=PP_TRAVEL_BASE_COINS+events*PP_TRAVEL_EVENT_COINS;
  return coins+(pp_tarot_active(g,3)?coins/5:0);
}
static unsigned recovery(const pp_game *g, unsigned value) {
  return value+(pp_tarot_active(g,6)?(value*15+99)/100:0);
}
const pp_item pp_items[] = {PP_BALANCE_ITEMS};
static uint32_t random_next(pp_game *g) {
  uint32_t x = g->rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return g->rng = x ? x : 1;
}
static uint8_t add(uint8_t a, unsigned b) {
  return a + b > 100 ? 100 : (uint8_t)(a + b);
}
static uint8_t sub(uint8_t a, unsigned b) { return a > b ? a - b : 0; }
static void log_line(pp_game *g, const char *text) {
  memmove(g->log[0], g->log[1], sizeof(g->log[0]) * 3);
  snprintf(g->log[3], 96, "%s", text);
  g->revision++;
}
static void new_pet(pp_pet *p, unsigned species) {
  *p = (pp_pet){.species = species,
                .food = 80,
                .clean = 85,
                .health = 100,
                .mood = 80,
                .energy = 85};
}
const char *const pp_nature_names[PP_NATURES]={
 "勤奋","怕寂寞","勇敢","固执","顽皮","大胆","坦率","悠闲","淘气","乐天",
 "胆小","急躁","认真","爽朗","天真","内敛","慢吞吞","冷静","害羞","马虎",
 "温和","温顺","自大","慎重","浮躁"
};
static void new_identity(pp_game *g,pp_identity *id) {
  memset(id,0,sizeof(*id));id->nature=random_next(g)%PP_NATURES;
  for(unsigned i=0;i<6;i++)id->iv[i]=random_next(g)%32;
}
unsigned pp_warehouse_count(const pp_game *g){return g->reserve_count+g->overflow_count;}
bool pp_warehouse_get(const pp_game *g,unsigned n,pp_stored_pet *p) {
  if(n>=pp_warehouse_count(g)||!p)return false;
  memset(p,0,sizeof(*p));
  if(n<g->reserve_count){p->pet=g->reserves[n];p->growth=g->reserve_growth[n];p->identity=g->reserve_identity[n];p->declined=(g->reserve_declined>>n)&1;p->pats_today=g->reserve_pats_today[n];p->balance=g->reserve_balance[n];}
  else *p=g->overflow[n-g->reserve_count];
  return true;
}
static void warehouse_set(pp_game *g,unsigned n,const pp_stored_pet *p) {
  if(n<g->reserve_count){g->reserves[n]=p->pet;g->reserve_growth[n]=p->growth;g->reserve_identity[n]=p->identity;g->reserve_pats_today[n]=p->pats_today;g->reserve_balance[n]=p->balance;
    g->reserve_declined=(g->reserve_declined&~(1u<<n))|((unsigned)p->declined<<n);}
  else g->overflow[n-g->reserve_count]=*p;
}
static unsigned storage_budget=UINT_MAX;
void pp_set_storage_budget(unsigned bytes){storage_budget=bytes;}
static bool warehouse_append(pp_game *g,const pp_stored_pet *p) {
  if(g->reserve_count==PP_RESERVES && (uint64_t)pp_save_size(g)+sizeof(pp_stored_pet)>storage_budget)return false;
  if(g->reserve_count<PP_RESERVES){unsigned n=g->reserve_count++;warehouse_set(g,n,p);return true;}
  if(g->overflow_count==g->overflow_capacity){
    if(g->overflow_count>UINT32_MAX/sizeof(*g->overflow)-16)return false;
    unsigned cap=g->overflow_count+16;
    void *data=realloc(g->overflow,(size_t)cap*sizeof(*g->overflow));if(!data)return false;
    g->overflow=data;g->overflow_capacity=cap;
  }
  g->overflow[g->overflow_count++]=*p;return true;
}
static void warehouse_remove(pp_game *g,unsigned n) {
  unsigned total=pp_warehouse_count(g);
  if(g->fishing_partner>n+1)g->fishing_partner--;
  else if(g->fishing_partner==n+1)g->fishing_partner=0;
  for(unsigned i=n;i+1<total;i++){pp_stored_pet p;pp_warehouse_get(g,i+1,&p);warehouse_set(g,i,&p);}
  if(g->overflow_count)g->overflow_count--;
  else {g->reserve_count--;g->reserve_declined&=(1u<<g->reserve_count)-1;
    memset(&g->reserves[g->reserve_count],0,sizeof(pp_pet));memset(&g->reserve_growth[g->reserve_count],0,sizeof(pp_growth));
    memset(&g->reserve_identity[g->reserve_count],0,sizeof(pp_identity));g->reserve_pats_today[g->reserve_count]=0;memset(&g->reserve_balance[g->reserve_count],0,sizeof(pp_pet_balance));}
}
unsigned pp_stored_art(const pp_stored_pet *p){
 for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==p->pet.species&&pp_pokedex[d].stage==p->growth.stage)return pp_pokedex[d].art;
 return pp_base_art(p->pet.species);
}
const char *pp_stored_name(const pp_stored_pet *p){
 if(p->identity.nickname[0])return p->identity.nickname;
 for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].species==p->pet.species&&pp_pokedex[d].stage==p->growth.stage)return pp_pokedex[d].name;
 return pp_pet_names[p->pet.species];
}
const char *pp_pet_name(const pp_game *g,unsigned n){return n<g->count&&g->identity[n].nickname[0]?g->identity[n].nickname:pp_form_name(g,n);}
static void remember_roster(pp_game *g){
 for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_dex_owned(g,d))g->dex_history[d/8]|=1u<<(d%8);
}
static bool nickname_valid(const char *s);
static bool identity_valid(const pp_identity *id){
 if(id->nature>=PP_NATURES||!memchr(id->nickname,0,PP_NAME_BYTES))return false;
 for(unsigned i=0;i<6;i++)if(id->iv[i]>31)return false;
 return nickname_valid(id->nickname);
}
/* Validate UTF-8 and count Unicode scalars; controls and invisible separators are rejected. */
static bool nickname_valid(const char *s){
 if(!s)return false;
 unsigned bytes=0,count=0;
 while(s[bytes]){
  unsigned char c=s[bytes];unsigned n=1,cp=c;
  if(c>=0xc2&&c<=0xdf){n=2;cp=c&31;}else if(c>=0xe0&&c<=0xef){n=3;cp=c&15;}
  else if(c>=0xf0&&c<=0xf4){n=4;cp=c&7;}else if(c>=128)return false;
  if(bytes+n>=PP_NAME_BYTES||++count>8)return false;
  for(unsigned k=1;k<n;k++){unsigned char t=s[bytes+k];if((t&0xc0)!=0x80)return false;cp=(cp<<6)|(t&63);}
  if(cp<32||cp==127||(n==2&&cp<128)||(n==3&&cp<2048)||(n==4&&cp<65536)||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff)||cp==0x200b||cp==0x2028||cp==0x2029)return false;
  bytes+=n;
 }
 return true;
}
pp_result pp_rename(pp_game *g,bool warehouse,unsigned n,const char *name){
 /* Keep legacy save names readable; new names use lowercase ASCII only. */
 if(!name)return PP_INVALID;
 unsigned length=0;for(const unsigned char *p=(const unsigned char *)name;*p;p++){
  if(++length>8||!(*p>='a'&&*p<='z'))return PP_INVALID;
 }
 if(warehouse){pp_stored_pet p;if(!pp_warehouse_get(g,n,&p))return PP_INVALID;snprintf(p.identity.nickname,PP_NAME_BYTES,"%s",name);warehouse_set(g,n,&p);}
 else {if(n>=g->count)return PP_INVALID;snprintf(g->identity[n].nickname,PP_NAME_BYTES,"%s",name);}
 g->revision++;return PP_OK;
}
void pp_dispose(pp_game *g){free(g->overflow);g->overflow=NULL;g->overflow_count=g->overflow_capacity=0;}
bool pp_clone(pp_game *dest,const pp_game *src){
 if(dest==src)return true;
 pp_stored_pet *data=dest->overflow;unsigned capacity=dest->overflow_capacity;
 if(capacity<src->overflow_count){data=realloc(data,(size_t)src->overflow_count*sizeof(*data));if(!data)return false;capacity=src->overflow_count;}
 if(src->overflow_count)memcpy(data,src->overflow,(size_t)src->overflow_count*sizeof(*data));
 *dest=*src;dest->overflow=data;dest->overflow_capacity=capacity;return true;
}
/* Pointer-free snapshot with a checksum over the complete fixed and variable payload. */
#define PP_SAVE_FIXED ((unsigned)offsetof(pp_game,overflow))
static uint32_t hash_bytes(const void *data,unsigned bytes){const uint8_t *p=data;uint32_t h=2166136261u;for(unsigned i=0;i<bytes;i++)h=(h^p[i])*16777619u;return h;}
static void unlock_scene_reward(pp_game *g);
unsigned pp_save_size(const pp_game *g){return 16+PP_SAVE_FIXED+g->overflow_count*sizeof(pp_stored_pet);}
bool pp_save_write(const pp_game *g,void *data,unsigned bytes,int64_t wall){
 if(!data||bytes!=pp_save_size(g)||!pp_valid(g))return false;
 uint8_t *p=data;memcpy(p+4,&bytes,4);memcpy(p+8,&wall,8);memcpy(p+16,g,PP_SAVE_FIXED);
 if(g->overflow_count)memcpy(p+16+PP_SAVE_FIXED,g->overflow,(size_t)g->overflow_count*sizeof(pp_stored_pet));
 uint32_t hash=hash_bytes(p+4,bytes-4);memcpy(p,&hash,4);return true;
}
bool pp_save_read(pp_game *g,const void *data,unsigned bytes,int64_t *wall){
 _Static_assert(offsetof(pp_game,fish_caught)==offsetof(pp_game_v15,fish_caught),"Frozen v15 fish prefix changed");
 _Static_assert(offsetof(pp_game,training)>=offsetof(pp_game_v15,overflow),"Frozen v15 fixed prefix changed");
 _Static_assert(offsetof(pp_game,fishing_partner)==offsetof(pp_game_v14,overflow),"Frozen v14 fixed prefix changed");
 _Static_assert(offsetof(pp_game,tree_essences)==offsetof(pp_game_v13,overflow),"Frozen v13 fixed prefix changed");
 _Static_assert(offsetof(pp_game,pats_today)==offsetof(pp_game_v12,overflow),"Frozen v12 fixed prefix changed");
 if(!data||bytes<24)return false;
 const uint8_t *p=data;uint32_t expected,size,version;memcpy(&expected,p,4);memcpy(&size,p+4,4);memcpy(&version,p+20,4);
 if(size!=bytes||hash_bytes(p+4,bytes-4)!=expected || (version!=12 && version!=13 && version!=14 && version!=15 && version!=16 && version!=PP_VERSION))return false;
 unsigned fixed=version==12?(unsigned)offsetof(pp_game_v12,overflow):version==13?(unsigned)offsetof(pp_game_v13,overflow):version==14?(unsigned)offsetof(pp_game_v14,overflow):version==15?(unsigned)offsetof(pp_game_v15,overflow):version==16?(unsigned)offsetof(pp_game_v16,overflow):PP_SAVE_FIXED;
 unsigned stride=version==12?sizeof(pp_stored_pet_v12):version<17?sizeof(pp_stored_pet_v16):sizeof(pp_stored_pet);
 if(bytes<16+fixed)return false;
 pp_game candidate={0};
 if(version==16){
  unsigned prefix=offsetof(pp_game,training);memcpy(&candidate,p+16,prefix);
  pp_training_record_v16 old;memcpy(&old,p+16+prefix,sizeof(old));
  unsigned a=offsetof(pp_training_record,party),b=offsetof(pp_training_record_v16,gym);
  memcpy(&candidate.training,&old,a);
  for(unsigned i=0;i<3;i++)memcpy(&candidate.training.party[i],&old.party[i],sizeof(pp_stored_pet_v16));
  memcpy((uint8_t *)&candidate.training+offsetof(pp_training_record,gym),(uint8_t *)&old+b,sizeof(old)-b);
  if(candidate.training.status==1){candidate.training.status=2;candidate.training.phase=0;candidate.training.battling=0;candidate.training.pending_ms=0;
   for(unsigned i=0;i<candidate.training.count;i++){unsigned pick=candidate.training.picks[i];candidate.training.end_levels[i]=pick<PP_PETS?candidate.growth[pick].level:old.party[i].growth.level;}
  }
 }else memcpy(&candidate,p+16,fixed);
 candidate.version=PP_VERSION;
 if(version<17&&candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
 if(16u+fixed+(uint64_t)candidate.overflow_count*stride!=bytes)return false;
 if(candidate.overflow_count){
  candidate.overflow=calloc(candidate.overflow_count,sizeof(pp_stored_pet));if(!candidate.overflow)return false;
  candidate.overflow_capacity=candidate.overflow_count;
  if(version==12)for(unsigned i=0;i<candidate.overflow_count;i++){
   pp_stored_pet_v12 old;memcpy(&old,p+16+fixed+(size_t)i*stride,sizeof(old));
   candidate.overflow[i]=(pp_stored_pet){.pet=old.pet,.growth=old.growth,.identity=old.identity,.declined=old.declined};
  }else if(version<17)for(unsigned i=0;i<candidate.overflow_count;i++)memcpy(&candidate.overflow[i],p+16+fixed+(size_t)i*stride,sizeof(pp_stored_pet_v16));
  else memcpy(candidate.overflow,p+16+fixed,(size_t)candidate.overflow_count*stride);
 }
 if(version==16&&candidate.training.status==2){for(unsigned i=0;i<candidate.training.count;i++){unsigned pick=candidate.training.picks[i];if(pick>=PP_PETS){pp_stored_pet stored;if(pp_warehouse_get(&candidate,pick-PP_PETS,&stored))candidate.training.end_levels[i]=stored.growth.level;}}}
 if(version<17&&(candidate.scenes&PP_SCENE_REQUIRED_MASK)==PP_SCENE_REQUIRED_MASK)candidate.scenes|=1u<<12;
 if(version<17){candidate.wild_steps_left=PP_WILD_INTERVAL_MAX;candidate.wild_queue=candidate.wild.phase?1:0;candidate.wild_visible_left=PP_WILD_NOTICE_SECONDS;candidate.wild_observed_seconds=candidate.seconds;}
 if(!pp_valid(&candidate)){pp_dispose(&candidate);return false;}
 if(candidate.wild.phase==PP_WILD_NONE && (candidate.wild.next_at&PP_WILD_STEP_MARK) &&
    (candidate.wild.next_at&~PP_WILD_STEP_MARK)>PP_WILD_INTERVAL_MAX){
  candidate.wild.next_at=PP_WILD_STEP_MARK|PP_WILD_INTERVAL_MAX;candidate.revision++;
 }
 /* Upgrade old infinite notices once; a saved finite notice keeps its deadline.
  * Old opened dialogs lose their former 15-second timeout. */
 if(version<17&&candidate.wild.phase==PP_WILD_PENDING){
  if(candidate.wild.next_at && (candidate.wild.next_at!=PP_WILD_STEP_MARK || candidate.wild.escape_at==UINT64_MAX)){
   candidate.wild.next_at=PP_WILD_STEP_MARK;candidate.wild.escape_at=candidate.seconds+PP_WILD_NOTICE_SECONDS;candidate.revision++;
  }else if(!candidate.wild.next_at && candidate.wild.escape_at!=UINT64_MAX){
   candidate.wild.escape_at=UINT64_MAX;candidate.revision++;
  }
 }
 pp_shop_retire_legacy(&candidate);
 unlock_scene_reward(&candidate);
 pp_dispose(g);*g=candidate;if(wall)memcpy(wall,p+8,8);return true;
}
static bool import_individuals(pp_game *g){
 if(!pp_valid(g))return false;
 uint32_t original_rng=g->rng;
 g->starter_chosen=!(g->count==1&&!g->reserve_count&&g->pets[0].species==0&&!g->growth[0].stage);
 for(unsigned i=0;i<g->count;i++)new_identity(g,&g->identity[i]);
 for(unsigned i=0;i<g->reserve_count;i++)new_identity(g,&g->reserve_identity[i]);
 remember_roster(g);g->rng=original_rng;return true;
}
const pp_legend_def pp_legends[PP_LEGEND_COUNT]={
  {144,60,"急冻鸟"},{145,60,"闪电鸟"},{146,60,"火焰鸟"},{150,60,"超梦"},{151,60,"梦幻"}
};
bool pp_is_legendary(unsigned dex) {return (dex>=144 && dex<=146) || dex==150 || dex==151;}
unsigned pp_legend_dex(unsigned legend) {
  if(legend>=PP_LEGEND_COUNT)return PP_DEX_COUNT;
  for(unsigned d=0;d<PP_DEX_COUNT;d++)if(pp_pokedex[d].national_id==pp_legends[legend].national_id)return d;
  return PP_DEX_COUNT;
}
static bool legend_species(unsigned species) {
  for(unsigned i=0;i<PP_LEGEND_COUNT;i++)if(pp_pokedex[pp_legend_dex(i)].species==species)return true;
  return false;
}
bool pp_legend_unlocked(const pp_game *g) {return g->gym.badges==((1u<<PP_GYM_COUNT)-1);}
bool pp_habitat_has(unsigned scene,unsigned species) {
 return scene<12 && species<PP_SPECIES && (pp_habitat_masks[species]&(1u<<scene));
}
/* Ordinary eggs unlock missing local nonlegendary base species first. */
static unsigned hatch_species(pp_game *g) {
  bool owned[PP_SPECIES] = {0};
  for (unsigned i=0; i<g->count; i++) owned[g->pets[i].species] = true;
  for (unsigned i=0;i<pp_warehouse_count(g);i++){pp_stored_pet p;pp_warehouse_get(g,i,&p);owned[p.pet.species]=true;}
  unsigned missing[PP_SPECIES],pool[PP_SPECIES],count=0,total=0;
  for(unsigned s=0;s<PP_SPECIES;s++)if(!legend_species(s) && (g->scene>=12 || pp_habitat_has(g->scene,s))){pool[total++]=s;if(!owned[s])missing[count++]=s;}
  return count ? missing[random_next(g)%count] : pool[random_next(g)%total];
}
bool pp_dex_owned(const pp_game *g, unsigned entry) {
  if (entry>=PP_DEX_COUNT || pp_needs_starter(g)) return false;
  if(g->dex_history[entry/8]&(1u<<(entry%8)))return true;
  const pp_dex_entry *e=&pp_pokedex[entry];
  for (unsigned p=0; p<g->count; p++)
    if (g->pets[p].species==e->species && (!e->stage || (e->species==67?g->growth[p].stage==e->stage:g->growth[p].stage>=e->stage))) return true;
  for (unsigned p=0;p<pp_warehouse_count(g);p++) {
    pp_stored_pet pet;pp_warehouse_get(g,p,&pet);
    if(pet.pet.species==e->species && (!e->stage || (e->species==67?pet.growth.stage==e->stage:pet.growth.stage>=e->stage)))return true;
  }
  return false;
}
unsigned pp_dex_count(const pp_game *g) {
  unsigned n=0;
  for (unsigned i=0;i<PP_DEX_COUNT;i++) n+=pp_dex_owned(g,i);
  return n;
}
void pp_dex_order(const pp_game *g, unsigned order[PP_DEX_COUNT]) {
  (void)g;
  /* The generated catalog is in National Pokedex order, regardless of ownership. */
  for (unsigned i=0;i<PP_DEX_COUNT;i++) order[i]=i;
}
void pp_init(pp_game *g, uint32_t seed) {
  memset(g, 0, sizeof(*g));
  g->magic = 0x504b504c;
  g->version = PP_VERSION;
  g->rng = seed ? seed : 1;
  g->coins = 240;
  g->count = 1;
  g->traveler = -1;
  g->scenes = 1;g->wild_steps_left=PP_WILD_INTERVAL_MIN+random_next(g)%(PP_WILD_INTERVAL_MAX-PP_WILD_INTERVAL_MIN+1);g->wild_visible_left=PP_WILD_NOTICE_SECONDS;
  for (unsigned i = 0; i < PP_PETS; i++)
    g->growth[i].level = 1;
  g->inventory[0] = 3;
  g->inventory[1] = 2;
  /* Slot 2 stays reserved for legacy save compatibility. */
  new_pet(&g->pets[0], 0);
  new_identity(g,&g->identity[0]);
  log_line(g, "欢迎回家，今天也一起生活吧。");
}
bool pp_needs_starter(const pp_game *g) {
  return !g->starter_chosen && !pp_warehouse_count(g) && g->count == 1 && g->pets[0].species == 0 && !g->growth[0].stage;
}
pp_result pp_choose_starter(pp_game *g, unsigned national_id) {
  unsigned species = national_id == 1 ? 1 : national_id == 4 ? 2 : national_id == 7 ? 3 : 0;
  if (!species || !pp_needs_starter(g))
    return PP_INVALID;
  if (g->battling || g->drop_active || g->traveler >= 0)
    return PP_BUSY;
  new_pet(&g->pets[0], species);
  g->starter_chosen=1;new_identity(g,&g->identity[0]);
  g->growth[0] = (pp_growth){.level = 1};
  g->revision++;
  return PP_OK;
}
static void cap_legacy_levels(pp_game *g) {
  for(unsigned i=0;i<PP_PETS+(unsigned)g->reserve_count && i<PP_PETS+PP_RESERVES;i++) {
    pp_growth *p=i<PP_PETS?&g->growth[i]:&g->reserve_growth[i-PP_PETS];
    if(p->level>=PP_MAX_LEVEL && p->level<=100) {p->level=PP_MAX_LEVEL;p->xp=0;}
  }
}
static unsigned pp_stage_level(unsigned species, unsigned stage) {
  if(species==67) return stage<=3?1:101;
  unsigned minimum=1;bool exists=false;
  for (unsigned i=0;i<PP_EVOLUTIONS;i++)
    if (pp_evolutions[i].species==species && pp_evolutions[i].from_stage+1u<=stage) {
      if(pp_evolutions[i].from_stage+1u==stage)exists=true;
      if(pp_evolutions[i].level>minimum)minimum=pp_evolutions[i].level;
    }
  return exists?minimum:101;
}
unsigned pp_base_art(unsigned species) {
  for (unsigned i=0;i<PP_DEX_COUNT;i++)
    if(pp_pokedex[i].species==species && !pp_pokedex[i].stage) return pp_pokedex[i].art;
  return 0;
}
/* Save v9 stores ongoing gym HP. Accept the exact pre-60 balance as well so
 * completed/active old battles survive an upgrade; rematches use current teams. */
static bool gym_saved_max(unsigned gym,unsigned foe,unsigned hp) {
  static const unsigned short legacy[8][3]={{124,142,164},{160,174,196},{200,216,240},{258,276,296},{310,334,356},{318,334,356},{340,360,386},{340,360,390}};
  return gym<8 && foe<3 && (hp==pp_gyms[gym].team[foe].hp || hp==legacy[gym][foe]);
}
bool pp_valid(const pp_game *g) {
  if (g->magic != 0x504b504c || g->version != PP_VERSION || !g->rng ||
      g->reserve_count>PP_RESERVES || g->random_scene>1 ||
      (g->reserve_declined & ~((1u<<g->reserve_count)-1)) ||
      (g->reserve_declined && !pp_has(g,PP_PURE_HEART)) ||
      g->random_scene_day>g->day || g->tarot_effect_day>g->day ||
      !g->count || g->count > PP_PETS || g->scene >= PP_SCENES ||
      !(g->scenes & 1) || !(g->scenes & (1u << g->scene)) || g->traveler < -1 ||
      g->traveler >= (int)g->count || g->tarot_card >= 8 || g->digs_today > 6 ||
      g->dig_lives > 3 || g->dig_depth > 20 || g->clock_ready > 1 ||
      g->care_remainder >= 600 || g->steps_today > g->total_steps ||
      g->steps_rewarded > g->steps_today / 100u ||
      (g->evolution_declined & ~((1ULL << g->count) - 1)) ||
      (g->evolution_declined && !pp_has(g, PP_PURE_HEART)))
    return false;
  if(g->gift.active_seconds>=PP_GIFT_INTERVAL || g->gift.pending>1 ||
     (g->gift.pending && (g->gift.item>=PP_TRAVEL_COLLECTION || g->gift.dex>=PP_DEX_COUNT || !pp_has(g,g->gift.item))))return false;
  if(g->fishing_partner>pp_warehouse_count(g)||g->wild_queue>3||g->wild_steps_left>PP_WILD_INTERVAL_MAX||g->wild_visible_left>PP_WILD_NOTICE_SECONDS||g->wild_delay>60||g->wild_active_seconds>=1200||g->pat_coins_today>25||g->rematches_today>2)return false;
  for(unsigned i=0;i<g->count;i++)if(g->pet_balance[i].xp_fraction>=1000000||g->pet_balance[i].energy_fraction>=3600||g->pet_balance[i].battles>=20||g->pet_balance[i].energy_discount>=100)return false;
  const pp_training_record *tr=&g->training;
  if(tr->status>2||tr->growth_seen>7||tr->evolution_seen>7)return false;
  if(tr->status){
   if(tr->scene>=6||tr->count<1||tr->count>3||tr->pending_ms>=40||tr->elapsed_ms%40||tr->elapsed_ms+tr->pending_ms>PP_TRAINING_LEGACY_DURATION_MS||tr->phase>5||!tr->rng||!tr->loot_rng||tr->awarded>1||tr->speed_remainder>1||tr->last_item>PP_SHOP_ITEMS)return false;
   for(unsigned i=0;i<tr->count;i++){
    unsigned pick=tr->picks[i];if((tr->status==1||pp_training_growth_pending(g)) && (pick<PP_PETS?pick>=g->count:pick-PP_PETS>=pp_warehouse_count(g)))return false;
    for(unsigned j=0;j<i;j++)if(pick==tr->picks[j])return false;
    const pp_stored_pet *tp=&tr->party[i];
    if(tp->growth.stage>(tp->pet.species==67?3:2)||tp->pet.sleeping>1||tp->declined>1||tp->pet.food>100||tp->pet.clean>100||tp->pet.health>100||tp->pet.mood>100||tp->pet.energy>100)return false;
    if(!tr->start_levels[i]||tr->start_levels[i]>60||tr->end_levels[i]<tr->start_levels[i]||tr->end_levels[i]>60)return false;
    if(tr->party[i].growth.level<1||tr->party[i].growth.level>60||tr->party[i].pet.species>=PP_SPECIES||!identity_valid(&tr->party[i].identity))return false;
    if(tr->foes[i].art>=PP_DEX_COUNT||tr->foes[i].level>60)return false;
   }
   if((tr->status==1 && (!tr->phase||tr->elapsed_ms==PP_TRAINING_LEGACY_DURATION_MS)) || (tr->status==2 && (tr->phase||tr->pending_ms)) || tr->loot_checks!=tr->elapsed_ms/60000 || tr->walk_ms>tr->elapsed_ms || tr->ambient_ms!=tr->elapsed_ms || tr->battling>1)return false;
   if(tr->phase==2||tr->phase==3){
    if(tr->gym.mode!=PP_GYM_COUNT+1 || !tr->battle_rng || tr->battle_pet>=tr->count || tr->gym.result>2 || (tr->phase==2)!=!!tr->battling)return false;
    for(unsigned i=0;i<tr->count;i++)if(tr->gym.team[i]!=i || tr->gym.hp[i]<0 || tr->gym.max_hp[i]<=0 || tr->gym.hp[i]>tr->gym.max_hp[i] || tr->gym.enemy_hp[i]<0 || tr->gym.enemy_max[i]<=0 || tr->gym.enemy_hp[i]>tr->gym.enemy_max[i] || !tr->foes[i].level)return false;
   }
   if(tr->gym.ally>=tr->count||tr->gym.enemy>=tr->count||tr->battle_round>100||tr->clock_ms>=2000||tr->loot_checks>180)return false;
  }

  if(g->starter_chosen>1 || (g->overflow_count && (!g->overflow || g->reserve_count!=PP_RESERVES)) || g->overflow_count>g->overflow_capacity)return false;
  for(unsigned i=0;i<PP_PETS;i++)if(g->pats_today[i]>PP_PAT_DAILY_LIMIT)return false;
  for(unsigned i=0;i<PP_RESERVES;i++)if(g->reserve_pats_today[i]>PP_PAT_DAILY_LIMIT)return false;
  for(unsigned i=0;i<g->count;i++)if(!identity_valid(&g->identity[i]))return false;
  for(unsigned i=0;i<g->reserve_count;i++)if(!identity_valid(&g->reserve_identity[i])||g->reserve_balance[i].xp_fraction>=1000000||g->reserve_balance[i].energy_fraction>=3600||g->reserve_balance[i].battles>=20||g->reserve_balance[i].energy_discount>=100)return false;
  for(unsigned i=0;i<g->overflow_count;i++){
    const pp_stored_pet *p=&g->overflow[i];
    if(p->balance.xp_fraction>=1000000||p->balance.energy_fraction>=3600||p->balance.battles>=20||p->balance.energy_discount>=100||p->pats_today>PP_PAT_DAILY_LIMIT||!identity_valid(&p->identity)||p->declined>1||(p->declined&&!pp_has(g,PP_PURE_HEART))||p->pet.species>=PP_SPECIES||p->pet.food>100||p->pet.clean>100||p->pet.health>100||p->pet.mood>100||p->pet.energy>100||p->pet.sleeping>1||!p->growth.level||p->growth.level>PP_MAX_LEVEL||p->growth.stage>(p->pet.species==67?3:2)||(p->growth.stage&&p->growth.level<pp_stage_level(p->pet.species,p->growth.stage))||p->growth.xp>=pp_xp_needed(p->growth.level)||(p->growth.level==PP_MAX_LEVEL&&p->growth.xp))return false;
  }
  const pp_legend_state *l=&g->legend;
  if(l->active>PP_LEGEND_COUNT || l->hatching>PP_LEGEND_COUNT || l->defeated>31 || (l->hatching && !g->egg_end))return false;
  bool legendary_state=l->active || l->hatching || l->defeated;
  for(unsigned i=0;i<PP_LEGEND_COUNT;i++)legendary_state|=l->eggs[i]!=0;
  if(legendary_state && !pp_legend_unlocked(g))return false;
  const pp_wild_state *w=&g->wild;
  if(w->phase==PP_WILD_NONE && (w->next_at&PP_WILD_STEP_MARK) &&
     (w->next_at&~PP_WILD_STEP_MARK)>PP_WILD_LEGACY_INTERVAL_MAX)return false;
  if(l->active && (w->phase<PP_WILD_BATTLE || w->dex!=pp_legend_dex(l->active-1) || w->level!=pp_legends[l->active-1].level))return false;
  if(w->phase>PP_WILD_LOST || w->outcome>PP_WILD_CAPTURE_READY)return false;
  if(w->outcome==PP_WILD_EGG && (!l->active || w->phase!=PP_WILD_WON || w->reward_coins || w->reward_xp))return false;
  if(l->active && w->phase==PP_WILD_WON && (w->outcome!=PP_WILD_EGG || !(l->defeated&(1u<<(l->active-1)))))return false;
  if(w->phase) {
    if(w->dex>=PP_DEX_COUNT || !w->level || w->level>PP_MAX_LEVEL || w->scene>=PP_SCENES ||
       (pp_pokedex[w->dex].stage && w->level<pp_stage_level(pp_pokedex[w->dex].species,pp_pokedex[w->dex].stage)) ||
       g->gym.mode)return false;
    if(w->phase==PP_WILD_PENDING && (!w->escape_at || g->battling || w->outcome))return false;
    if(w->phase==PP_WILD_PENDING && w->next_at==PP_WILD_STEP_MARK && w->escape_at!=UINT64_MAX && w->escape_at!=PP_WILD_NOTICE_PAUSED &&
       w->escape_at>g->seconds && w->escape_at-g->seconds>PP_WILD_NOTICE_SECONDS)return false;
    if(w->phase==PP_WILD_PENDING && w->next_at!=PP_WILD_STEP_MARK && w->next_at>g->seconds &&
       (w->next_at-g->seconds>5 || w->escape_at-w->next_at!=15u))return false;
    if(w->phase>=PP_WILD_BATTLE && g->battle_pet>=g->count)return false;
    if(w->phase==PP_WILD_BATTLE && (!g->battling || w->outcome))return false;
    if(w->phase>=PP_WILD_WON && g->battling)return false;
    if(w->phase==PP_WILD_WON && (w->outcome==PP_WILD_NO_REWARD || g->enemy_hp))return false;
    if(w->phase!=PP_WILD_WON && (w->reward_coins || w->reward_xp || w->outcome))return false;
    if(w->phase==PP_WILD_WON&&!l->active){bool fresh=w->reward_coins==8&&w->reward_xp<=44;bool old=(w->reward_coins==25||w->reward_coins==105)&&(w->reward_xp==30+8u*(w->level-1)||w->reward_xp==90+8u*(w->level-1)||w->reward_xp==30||w->reward_xp==90);if(!fresh&&!old)return false;}
  }
  if (g->battling &&
      (g->battle_pet >= g->count || ((!g->gym.mode) && (g->battle_hp <= 0 || g->enemy_hp <= 0))))
    return false;
  if ((g->gym.badges & ~((1ULL<<PP_GYM_COUNT)-1)) ||
      (g->gym.badges & (g->gym.badges+1)) || g->gym.mode>PP_GYM_COUNT ||
      g->gym.result>2 || g->gym.first_clear>1) return false;
  if (g->gym.mode) {
    if (!pp_gym_unlocked(g,g->gym.mode-1)) return false;
    if (g->gym.ally>=3 || g->gym.enemy>=3 || (g->gym.entered & ~7u) ||
        (g->battling && g->gym.result) || (g->gym.reward_item>=PP_TRAVEL_COLLECTION&&g->gym.reward_item!=PP_COLLECTION)) return false;
    for(unsigned i=0;i<3;i++) {
      if(g->gym.team[i]>=g->count || g->gym.hp[i]<0 || g->gym.hp[i]>g->gym.max_hp[i] ||
         g->gym.max_hp[i]<=0 || g->gym.enemy_hp[i]<0 || g->gym.enemy_hp[i]>g->gym.enemy_max[i] ||
         !gym_saved_max(g->gym.mode-1,i,g->gym.enemy_max[i])) return false;
      for(unsigned j=0;j<i;j++) if(g->gym.team[i]==g->gym.team[j])return false;
    }
    if(g->battle_pet!=g->gym.team[g->gym.ally] || g->battle_hp!=g->gym.hp[g->gym.ally] ||
       g->enemy_hp!=g->gym.enemy_hp[g->gym.enemy])return false;
  }
  if (g->drop_active &&
      ((g->drop_item >= PP_ITEMS - 1 && g->drop_item != PP_TREE_ESSENCE) || g->drop_target >= g->count ||
       g->drop_active > PP_DROP_SPOTS + 1))
    return false;
  for (unsigned i = 0; i < PP_PETS+(unsigned)g->reserve_count; i++) {
    const pp_pet *p = i<PP_PETS? &g->pets[i]: &g->reserves[i-PP_PETS];
    const pp_growth *growth = i<PP_PETS? &g->growth[i]: &g->reserve_growth[i-PP_PETS];
    if (!growth->level || growth->level > PP_MAX_LEVEL || growth->stage > (p->species==67?3:2) ||
        (growth->stage && growth->level < pp_stage_level(p->species, growth->stage)) ||
        growth->xp >= pp_xp_needed(growth->level) ||
        (growth->level == PP_MAX_LEVEL && growth->xp))
      return false;
    if (p->species >= PP_SPECIES || p->food > 100 || p->clean > 100 ||
        p->health > 100 || p->mood > 100 || p->energy > 100 || p->sleeping > 1)
      return false;
  }
  for (unsigned i = 0; i < 4; i++)
    if (!memchr(g->log[i], 0, 96))
      return false;
  return true;
}
uint32_t pp_checksum(const pp_game *g) {
 uint32_t h=hash_bytes(g,PP_SAVE_FIXED);
 const uint8_t *p=(const uint8_t *)g->overflow;
 for(size_t i=0;i<(size_t)g->overflow_count*sizeof(pp_stored_pet);i++)h=(h^p[i])*16777619u;
 return h;
}
const char *pp_message(pp_result r) {
  static const char *s[] = {
      "完成啦",           "金币不足",     "背包里没有了",
      "伙伴栏已经满啦",   "它正忙着呢",   "至少两只宠物才能旅行",
      "今天的次数用完啦", "还没到时间哦", "当前无法操作", "已选择永久不进化", "请选择三只可出战伙伴", "场上还没有宠物蛋", "需要购买咕噜球", "存储空间不足，请稍后重试"};
  return (unsigned)r <= PP_STORAGE_FULL ? s[r] : s[PP_INVALID];
}
bool pp_has(const pp_game *g, unsigned id) {
  /* These milestones are monotonic in actual play: badges never disappear,
   * and releasing/swapping pets preserves the historical dex. No save resize
   * or second claim flag is needed, including when loading completed old saves. */
  if(id==PP_GYM_CROWN)return g->gym.badges==((1u<<PP_GYM_COUNT)-1);
  if(id==PP_DEX_TOME)return pp_dex_count(g)==PP_DEX_COUNT;
  return id < PP_STORED_COLLECTION && (g->collected[id / 8] & (1u << (id % 8)));
}
unsigned pp_collected(const pp_game *g) {
  unsigned n = 0;
  for (unsigned i = 0; i < PP_COLLECTION; i++)
    n += pp_has(g, i);
  return n;
}
unsigned pp_collection_total(unsigned quality) {
  if (quality == PP_QUALITY_ALL) return PP_COLLECTION;
  return quality < PP_QUALITY_COUNT ? pp_qualities[quality].count : 0;
}
unsigned pp_collection_owned(const pp_game *g, unsigned quality) {
  unsigned count = 0;
  for (unsigned i=0; i<PP_COLLECTION; i++)
    if ((quality==PP_QUALITY_ALL || pp_collectibles[i].quality==quality) && pp_has(g,i)) count++;
  return count;
}
unsigned pp_collection_id(unsigned quality, unsigned index) {
  if (quality > PP_QUALITY_ALL) return PP_COLLECTION;
  /* Special gifts lead; all original travel IDs keep their relative order. */
  for (unsigned slot=0; slot<PP_COLLECTION; slot++) {
    unsigned id=slot<PP_SPECIAL_COLLECTION?PP_TRAVEL_COLLECTION+slot:slot-PP_SPECIAL_COLLECTION;
    if (quality!=PP_QUALITY_ALL && pp_collectibles[id].quality!=quality) continue;
    if (!index--) return id;
  }
  return PP_COLLECTION;
}
unsigned pp_pat_remaining(const pp_game *g,unsigned p) {
  return p<g->count && g->pats_today[p]<PP_PAT_DAILY_LIMIT?PP_PAT_DAILY_LIMIT-g->pats_today[p]:0;
}
pp_result pp_pat(pp_game *g, unsigned p) {
  if (p >= g->count)
    return PP_INVALID;
  if (pp_training_busy(g,p) || (int)p == g->traveler)
    return PP_BUSY;
  if(!pp_pat_remaining(g,p))return PP_DAILY_LIMIT;
  g->pats_today[p]++;
  g->pets[p].mood = add(g->pets[p].mood, pp_tarot_active(g,5)?4:2);
  if(g->pats_today[p]<=5){if(g->pat_coins_today<25){if(g->coins<UINT32_MAX)g->coins++;g->pat_coins_today++;}pp_gain_xp(g,p,pp_tarot_active(g,5)?2:1);}
  log_line(g, "摸摸头，心情 +2，金币 +1。");
  return PP_OK;
}
unsigned pp_pat_all(pp_game *g) {
  if(pp_needs_starter(g)) return 0;
  unsigned n=0;
  for(unsigned i=0;i<g->count;i++) if(pp_pat(g,i)==PP_OK) n++;
  return n;
}
static bool roster_busy(const pp_game *g) {
  return (g->training.status==1||pp_training_growth_pending(g))||pp_needs_starter(g)||g->traveler>=0||g->battling||g->drop_active||g->wild.phase;
}
pp_result pp_replace_pet(pp_game *g,unsigned slot,unsigned reserve) {
  pp_stored_pet next;
  if(slot>g->count||slot>=PP_PETS||!pp_warehouse_get(g,reserve,&next))return PP_INVALID;
  /* A fixed-slot swap leaves the training participants and their receipt indices intact.
   * Adding/removing slots still waits until the running job has finished. */
  if((g->training.status==1 && (slot==g->count||pp_training_busy(g,slot)||pp_training_busy(g,PP_PETS+reserve)))||pp_training_growth_pending(g)||g->fishing_partner==reserve+1)return PP_BUSY;
  /* Replacing one slot does not move any other pet's index. Completed wild
   * receipts and unrelated travel/item use must not lock the whole courtyard. */
  if(pp_needs_starter(g) || g->battling || g->wild.phase==PP_WILD_BATTLE ||
     g->traveler==(int)slot || (g->drop_active && g->drop_target==slot))return PP_BUSY;
  remember_roster(g);pp_gym_leave(g);
  if(slot==g->count){warehouse_remove(g,reserve);g->count++;}
  else {pp_stored_pet old={.pet=g->pets[slot],.growth=g->growth[slot],.identity=g->identity[slot],.pats_today=g->pats_today[slot],.balance=g->pet_balance[slot],.declined=(g->evolution_declined>>slot)&1};warehouse_set(g,reserve,&old);}
  g->pets[slot]=next.pet;g->growth[slot]=next.growth;g->identity[slot]=next.identity;g->pats_today[slot]=next.pats_today;g->pet_balance[slot]=next.balance;
  g->evolution_declined=(g->evolution_declined&~(1ULL<<slot))|((uint64_t)next.declined<<slot);
  log_line(g,"伙伴已加入庭院。");return PP_OK;
}
pp_result pp_reserve_pet(pp_game *g,unsigned slot) {
  if(slot>=g->count)return PP_INVALID;
  if(roster_busy(g))return PP_BUSY;
  if(g->count<=1)return PP_NEED_TWO;
  pp_stored_pet p={.pet=g->pets[slot],.growth=g->growth[slot],.identity=g->identity[slot],.pats_today=g->pats_today[slot],.balance=g->pet_balance[slot],.declined=(g->evolution_declined>>slot)&1};
  if(!warehouse_append(g,&p))return PP_STORAGE_FULL;
  remember_roster(g);pp_gym_leave(g);
  for(unsigned i=slot;i+1<g->count;i++){g->pets[i]=g->pets[i+1];g->growth[i]=g->growth[i+1];g->identity[i]=g->identity[i+1];g->pats_today[i]=g->pats_today[i+1];g->pet_balance[i]=g->pet_balance[i+1];}
  g->evolution_declined=(g->evolution_declined&((1ULL<<slot)-1))|((g->evolution_declined>>(slot+1))<<slot);
  g->count--;memset(&g->pets[g->count],0,sizeof(pp_pet));g->growth[g->count]=(pp_growth){.level=1};memset(&g->identity[g->count],0,sizeof(pp_identity));g->pats_today[g->count]=0;memset(&g->pet_balance[g->count],0,sizeof(pp_pet_balance));
  log_line(g,"伙伴已回到仓库。");return PP_OK;
}
pp_result pp_release(pp_game *g,unsigned n){
 if(n>=pp_warehouse_count(g))return PP_INVALID;
 if(g->fishing_partner==n+1)return PP_BUSY;
 if((g->training.status==1||pp_training_growth_pending(g))||pp_needs_starter(g)||g->traveler>=0||g->battling||g->drop_active||g->wild.phase==PP_WILD_PENDING||g->wild.phase==PP_WILD_BATTLE)return PP_BUSY;
 remember_roster(g);warehouse_remove(g,n);log_line(g,"伙伴回到了大自然。");return PP_OK;
}
pp_result pp_fishing_partner_select(pp_game *g,unsigned slot){
 if(slot>=pp_warehouse_count(g))return PP_INVALID;
 if(pp_training_busy(g,PP_PETS+slot))return PP_BUSY;
 if(g->fishing_partner!=slot+1){g->fishing_partner=slot+1;g->revision++;}
 return PP_OK;
}
void pp_fishing_partner_clear(pp_game *g){if(g->fishing_partner){g->fishing_partner=0;g->revision++;}}
static bool grant_pet(pp_game *g,unsigned species,unsigned level,unsigned stage){
 pp_stored_pet p={0};new_pet(&p.pet,species);p.growth=(pp_growth){.level=level,.stage=stage};
 uint32_t rng=g->rng;new_identity(g,&p.identity);
 if(g->count<PP_PETS){unsigned n=g->count++;g->pets[n]=p.pet;g->growth[n]=p.growth;g->identity[n]=p.identity;g->pats_today[n]=0;memset(&g->pet_balance[n],0,sizeof(pp_pet_balance));g->evolution_declined&=~(1ULL<<n);}
 else if(!warehouse_append(g,&p)){g->rng=rng;return false;}
 remember_roster(g);return true;
}
static bool hatch_pet(pp_game *g) {
  uint32_t rng=g->rng;
  unsigned species=g->legend.hatching?pp_pokedex[pp_legend_dex(g->legend.hatching-1)].species:hatch_species(g);
  if(!grant_pet(g,species,1,0)){g->rng=rng;return false;}
  g->legend.hatching=0;return true;
}
unsigned pp_item_count(const pp_game *g,unsigned item) {
  return item<PP_ITEMS?g->inventory[item]:item==PP_HATCH_BOOST?g->hatch_boosts:item==PP_BALL?g->balls:item==PP_TREE_ESSENCE?g->tree_essences:0;
}
bool pp_item_enabled(unsigned item){return item<PP_SHOP_ITEMS && item!=PP_RETIRED_POTION;}
void pp_shop_retire_legacy(pp_game *g){
  unsigned count=g->inventory[PP_RETIRED_POTION];
  if(count){uint64_t coins=(uint64_t)g->coins+(uint64_t)count*pp_items[PP_RETIRED_POTION].price;g->coins=coins>UINT32_MAX?UINT32_MAX:(uint32_t)coins;g->inventory[PP_RETIRED_POTION]=0;g->revision++;}
  if(g->training.items[PP_RETIRED_POTION] || g->training.last_item==PP_RETIRED_POTION+1){g->training.items[PP_RETIRED_POTION]=0;if(g->training.last_item==PP_RETIRED_POTION+1)g->training.last_item=0;g->revision++;}
}
pp_result pp_shop_draw(pp_game *g,unsigned *item){
  static const unsigned weights[PP_SHOP_ITEMS]=PP_SHOP_WEIGHTS;
  if(!item)return PP_INVALID;
  if(g->coins<PP_SHOP_PRICE)return PP_NO_COINS;
  /* Keep published probabilities: reject before charging if any prize is full. */
  for(unsigned i=0;i<PP_SHOP_ITEMS;i++)if(weights[i] && pp_item_count(g,i)==UINT16_MAX)return PP_FULL;
  unsigned total=0;for(unsigned i=0;i<PP_SHOP_ITEMS;i++)total+=weights[i];
  unsigned roll=random_next(g)%total,pick=0;
  while(pick+1<PP_SHOP_ITEMS && roll>=weights[pick]){roll-=weights[pick];pick++;}
  g->coins-=PP_SHOP_PRICE;
  if(pick==PP_HATCH_BOOST)g->hatch_boosts++;
  else if(pick==PP_BALL)g->balls++;
  else if(pick==PP_TREE_ESSENCE)g->tree_essences++;
  else g->inventory[pick]++;
  *item=pick;g->revision++;return PP_OK;
}
pp_result pp_buy(pp_game *g, unsigned item) {
  if(!pp_item_enabled(item))return PP_INVALID;
  if(g->coins<pp_items[item].price)return PP_NO_COINS;
  if(pp_item_count(g,item)==UINT16_MAX)return PP_FULL;
  g->coins-=pp_items[item].price;
  if(item==PP_HATCH_BOOST)g->hatch_boosts++;
  else if(item==PP_BALL)g->balls++;
  else if(item==PP_TREE_ESSENCE)g->tree_essences++;
  else g->inventory[item]++;
  g->revision++;return PP_OK;
}
pp_result pp_use_auto(pp_game *g,unsigned item) {
  if(!pp_item_enabled(item) || !g->count)return PP_INVALID;
  if(!pp_item_count(g,item))return PP_EMPTY;
  if(item>=5 && item!=PP_TREE_ESSENCE)return pp_use(g,item,0);
  if(g->drop_active || g->battling)return PP_BUSY;
  unsigned target=g->count,lowest=501;
  for(unsigned i=0;i<g->count;i++) {
    if(pp_training_busy(g,i)||g->traveler==(int)i)continue;
    const pp_pet *p=&g->pets[i];
    unsigned value=item==PP_TREE_ESSENCE?(unsigned)p->food+p->clean+p->health+p->mood+p->energy:item==0?p->food:item==1?p->clean:item==2?p->health:item==3?p->mood:p->energy;
    if(value<lowest){lowest=value;target=i;}
  }
  return target<g->count?pp_use(g,item,target):PP_BUSY;
}
pp_result pp_use(pp_game *g, unsigned item, unsigned p) {
  if(!pp_item_enabled(item))return PP_INVALID;
  if (item >= PP_SHOP_ITEMS || p >= g->count)
    return PP_INVALID;
  if (!pp_item_count(g,item))
    return PP_EMPTY;
  if(item==PP_BALL)return PP_NOT_READY;
  if(item==PP_HATCH_BOOST) {
    if(!g->egg_end)return PP_NO_EGG;
    if(pp_needs_starter(g) || g->battling)return PP_BUSY;
    if(!hatch_pet(g))return PP_STORAGE_FULL;
    g->egg_end=0;g->egg_shakes=0;g->hatch_boosts--;
    log_line(g,"使用孵化药剂，蛋孵化啦！");return PP_OK;
  }
  if (item == 5) {
    if (g->egg_end)
      return PP_BUSY;
    g->egg_end = g->seconds + PP_EGG_MIN_SECONDS + (random_next(g) % 121u) * 60u;
    g->egg_shakes = 0;g->legend.hatching=0;
    log_line(g, "蛋已放进庭院，摇一摇，等待孵化。");
  } else {
    if (pp_training_busy(g,p) || g->drop_active || g->traveler == (int)p || g->battling)
      return PP_BUSY;
    const pp_pet *pet=&g->pets[p];unsigned need=item==PP_TREE_ESSENCE?500-pet->food-pet->clean-pet->health-pet->mood-pet->energy:item==0?100-pet->food:item==1?100-pet->clean:item==2?100-pet->health:item==3?100-pet->mood:100-pet->energy;
    if(!need)return PP_NOT_READY;
    g->drop_active = 2 + random_next(g) % PP_DROP_SPOTS;
    g->drop_item = item;
    g->drop_target = p;
    g->drop_end = g->seconds + 4;
    log_line(g, "道具放在地上，宠物正在走过来。");
  }
  if(item==PP_TREE_ESSENCE)g->tree_essences--;
  else g->inventory[item]--;
  g->revision++;
  return PP_OK;
}
void pp_drop_position(const pp_game *g, int *x, int *y) {
  if (g->drop_active < 2 || g->drop_active > PP_DROP_SPOTS + 1) {
    *x = 164; *y = 234; /* Existing v1-v3 in-flight drops keep their position. */
    return;
  }
  unsigned spot = g->drop_active - 2;
  if(g->scene==12) {
    *x=pp_lake_shore_x(spot);*y=140+(int)(spot/4)*16;return;
  }
  *x = 70 + (int)(spot % 6) * 40;
  *y = 145 + (int)(spot / 6) * 22;
}
void pp_egg_position(const pp_game *g, int *x, int *y) {
  pp_scene_egg_position(g->scene,x,y);
}
/* Destination is a stable pseudorandom function of the persisted trip deadline.
 * It never follows courtyard changes, advances RNG, or needs a save migration. */
typedef struct { const char *name, *activities[3], *encounters[2]; } pp_travel_theme;
static const pp_travel_theme travel_themes[PP_TRAVEL_DESTINATIONS]={PP_TRAVEL_THEME_ENTRIES};
unsigned pp_travel_destination(const pp_game *g) {
  uint64_t h=g->travel_end+UINT64_C(0x9e3779b97f4a7c15);
  h=(h^(h>>30))*UINT64_C(0xbf58476d1ce4e5b9);
  h=(h^(h>>27))*UINT64_C(0x94d049bb133111eb);
  return (unsigned)((h^(h>>31))%PP_TRAVEL_DESTINATIONS);
}
const char *pp_travel_name(unsigned destination) {
  return travel_themes[destination<PP_TRAVEL_DESTINATIONS?destination:0].name;
}
pp_travel_status pp_travel_describe(const pp_game *g) {
  if (g->traveler < 0 || g->traveler >= (int)g->count)
    return (pp_travel_status){0};
  unsigned scene = pp_travel_destination(g);
  uint64_t remaining = g->travel_end > g->seconds ? g->travel_end - g->seconds : 0;
  if (remaining > PP_TRAVEL_SECONDS) remaining = PP_TRAVEL_SECONDS;
  unsigned elapsed = PP_TRAVEL_SECONDS - (unsigned)remaining;
  unsigned phase = elapsed / 300 % 3;
  return (pp_travel_status){.active=true, .ready=!remaining,
    .walking=remaining && phase==0, .remaining=(unsigned)remaining,
    .events=g->travel_events, .pending_coins=travel_coins(g),
    .activity=remaining ? travel_themes[scene].activities[phase] : "结束旅行回来了",
    .event=g->travel_events ? travel_themes[scene].encounters[(g->travel_events-1)%2] : NULL};
}
pp_result pp_travel(pp_game *g, unsigned p) {
  if (p >= g->count)
    return PP_INVALID;
  if (g->count < 2)
    return PP_NEED_TWO;
  if(g->pets[p].health<30||g->pets[p].food<20||g->pets[p].energy<20||g->pets[p].sleeping)return PP_NOT_READY;
  if (pp_training_busy(g,p) || g->traveler >= 0 || g->battling || g->drop_active)
    return PP_BUSY;
  g->pets[p].food=sub(g->pets[p].food,6);pp_spend_energy(g,p,15);g->traveler = p;
  g->travel_end = g->seconds + PP_TRAVEL_SECONDS;
  g->travel_events = 0;
  g->travel_reward = 0;
  g->travel_xp_snapshot=g->growth[p].level<PP_MAX_LEVEL?pp_xp_needed(g->growth[p].level)/5:0;
  log_line(g, "背上小包出发，三小时后见！");
  return PP_OK;
}
unsigned pp_travel_xp(const pp_game *g) {
  if(g->traveler<0 || g->traveler>=(int)g->count)return 0;
  return g->travel_xp_snapshot;
}
pp_result pp_claim_travel(pp_game *g,pp_travel_reward *out) {
  if (g->traveler < 0 || g->traveler >= (int)g->count)
    return PP_INVALID;
  if (g->seconds < g->travel_end)
    return PP_NOT_READY;
  unsigned pet=(unsigned)g->traveler;
  pp_travel_reward reward={.pet=pet,.destination=pp_travel_destination(g),
    .coins=travel_coins(g),.before=g->growth[pet]};
  unsigned missing=0,id=PP_TRAVEL_COLLECTION;
  for (unsigned i=0;i<PP_TRAVEL_COLLECTION;i++) missing+=!pp_has(g,i);
  if (missing) {
    /* Uniform among missing travel items; an empty pool consumes no RNG. */
    uint32_t roll,limit=UINT32_MAX-(UINT32_MAX % missing);
    do {roll=random_next(g)-1u;} while (roll>=limit);
    unsigned pick=roll % missing;
    for (unsigned i=0;i<PP_TRAVEL_COLLECTION;i++) if (!pp_has(g,i)) {
      if (!pick--) {id=i;break;}
    }
    g->collected[id / 8] |= 1u << (id % 8);
  }
  g->coins += reward.coins;
  pp_gain_xp(g, pet, pp_travel_xp(g));
  reward.item=id;reward.after=g->growth[pet];
  /* Actual credited progress, including level-ups and the Lv60 cap. */
  reward.xp=10u*reward.after.level*(reward.after.level-1)+reward.after.xp
    -(10u*reward.before.level*(reward.before.level-1)+reward.before.xp);
  g->pets[g->traveler].mood = add(g->pets[g->traveler].mood, 10);
  g->traveler = -1;
  g->travel_end = 0;g->travel_xp_snapshot=0;
  char message[96];
  if (id<PP_TRAVEL_COLLECTION)
    snprintf(message,sizeof(message),"新收藏 · %s · %s",pp_qualities[pp_collectibles[id].quality].name,pp_collectibles[id].name);
  else snprintf(message,sizeof(message),"旅行藏品已集齐，获得金币和经验。");
  log_line(g,message);
  if(out)*out=reward;
  return PP_OK;
}
pp_result pp_claim(pp_game *g) { return pp_claim_travel(g,NULL); }
/* Branch keys identify mutually exclusive forms, not growth ranks. */
static unsigned growth_rank(const pp_game *g,unsigned p) {
  return g->pets[p].species==67 ? !!g->growth[p].stage : g->growth[p].stage;
}
pp_result pp_battle(pp_game *g, unsigned p) {
  if (p >= g->count)
    return PP_INVALID;
  if (pp_training_busy(g,p) || g->battling || g->wild.phase || g->traveler == (int)p || g->drop_active)
    return PP_BUSY;
  if(g->pets[p].health<30||g->pets[p].energy<3||g->pets[p].sleeping)return PP_NOT_READY;
  pp_spend_energy(g,p,3);g->gym.mode = 0;
  g->battling = 1;
  g->battle_round = 0;
  g->battle_pet = p;
  g->battle_hp = 70 + 5*g->growth[p].level + g->pets[p].health / 2 + growth_rank(g,p) * 10;
  g->enemy_hp = 60 + 3*g->growth[p].level + (g->stage % 10) * 2;
  g->battle_max_hp = g->battle_hp;
  g->enemy_max_hp = g->enemy_hp;
  g->battle_next = g->seconds + PP_BATTLE_TURN_SECONDS;
  log_line(g, "挑战开始，宠物将自动使用技能。");
  return PP_OK;
}
/* First gym is deliberately fixed: rematches never scale above level 12. */
const pp_gym_enemy pp_pewter_team[3] = {
  {"小拳石",10,73,124,48,48}, {"隆隆石",11,74,142,53,55}, {"大岩蛇",12,94,164,57,64}
};
static const pp_gym_enemy cerulean_team[3] = {
  {"可达鸭",18,53,180,72,61}, {"海星星",19,119,194,77,67}, {"宝石海星",20,120,218,84,76}
};
static const pp_gym_enemy vermilion_team[3] = {
  {"霹雳电球",26,99,236,95,80}, {"皮卡丘",27,0,254,100,85}, {"雷丘",28,25,280,110,93}
};
static const pp_gym_enemy celadon_team[3] = {
  {"蔓藤怪",34,113,313,121,114}, {"大食花",35,70,333,133,111}, {"霸王花",36,44,355,139,122}
};
static const pp_gym_enemy fuchsia_team[3] = {
  {"瓦斯弹",42,108,383,146,128}, {"臭臭泥",43,88,410,155,138}, {"双弹瓦斯",44,109,435,161,147}
};
static const pp_gym_enemy saffron_team[3] = {
  {"魔墙人偶",48,121,413,163,150}, {"勇基拉",49,63,431,173,134}, {"胡地",50,64,456,185,144}
};
static const pp_gym_enemy cinnabar_team[3] = {
  {"九尾",53,37,462,179,160}, {"烈焰马",54,77,498,191,169}, {"风速狗",55,58,531,201,176}
};
static const pp_gym_enemy viridian_team[3] = {
  {"尼多后",58,30,493,188,171}, {"尼多王",59,33,531,201,177}, {"钻角犀兽",60,111,585,219,198}
};
const pp_gym_def pp_gyms[PP_GYM_COUNT] = {
  {"尼比道馆","尼比市","小刚","岩石系","灰色徽章",
   "brock","boulder","badge-display","map","arena",pp_pewter_team,300,20,12,0,75,20},
  {"华蓝道馆","华蓝市","小霞","水系","蓝色徽章",
   "misty","cascade","cascade-display","cerulean-map","cerulean-arena",cerulean_team,450,25,18,0,65,28},
  {"枯叶道馆","枯叶市","马志士","电系","橙色徽章",
   "surge","thunder","thunder-display","vermilion-map","vermilion-arena",vermilion_team,600,30,27,0,55,35},
  {"玉虹道馆","玉虹市","莉佳","草系","彩虹徽章",
   "erika","rainbow","rainbow-display","celadon-map","celadon-arena",celadon_team,750,35,33,0,45,40},
  {"浅红道馆","浅红市","阿桔","毒系","粉红徽章",
   "koga","soul","soul-display","fuchsia-map","fuchsia-arena",fuchsia_team,900,40,39,0,35,45},
  {"金黄道馆","金黄市","娜姿","超能系","金色徽章",
   "sabrina","marsh","marsh-display","saffron-map","saffron-arena",saffron_team,1050,45,45,0,25,50},
  {"红莲道馆","红莲岛","夏伯","火系","深红徽章",
   "blaine","volcano","volcano-display","cinnabar-map","cinnabar-arena",cinnabar_team,1200,50,51,0,20,50},
  {"常青道馆","常青市","坂木","地面系","绿色徽章",
   "giovanni","earth","earth-display","viridian-map","viridian-arena",viridian_team,1350,55,54,0,15,50}
};
static struct {pp_game *game;const pp_gym_def *def;unsigned count;} encounter;
bool pp_battle_encounter(const pp_game *g){return g && encounter.game==g;}
unsigned pp_battle_team_count(const pp_game *g){return pp_battle_encounter(g)?encounter.count:3;}
void pp_battle_encounter_close(pp_game *g){if(encounter.game==g)memset(&encounter,0,sizeof(encounter));}
pp_result pp_battle_encounter_start(pp_game *g,const pp_gym_def *def,unsigned count){
  if(!g||!def||!def->team||!count||count>3||count>g->count)return PP_INVALID;
  if(encounter.game && encounter.game!=g)return PP_BUSY;
  encounter.game=g;encounter.def=def;encounter.count=count;
  memset(&g->gym,0,sizeof(g->gym));g->gym.mode=PP_GYM_COUNT+1;g->gym.entered=1;
  for(unsigned i=0;i<count;i++){
    g->gym.team[i]=i;g->gym.hp[i]=g->gym.max_hp[i]=pp_gym_hp(g,i);
    g->gym.enemy_hp[i]=g->gym.enemy_max[i]=def->team[i].hp;
  }
  g->battle_pet=0;g->battle_hp=g->battle_max_hp=g->gym.hp[0];
  g->enemy_hp=g->enemy_max_hp=g->gym.enemy_hp[0];g->battle_round=0;
  g->battle_next=g->seconds+PP_BATTLE_TURN_SECONDS;g->battling=1;return PP_OK;
}
const pp_gym_def *pp_gym_current(const pp_game *g) {
  if(pp_battle_encounter(g))return encounter.def;
  return &pp_gyms[g->gym.mode && g->gym.mode<=PP_GYM_COUNT?g->gym.mode-1:0];
}
unsigned pp_gym_recommended_level(unsigned gym) {
  if(gym>=PP_GYM_COUNT)return 0;
  unsigned level=0;
  for(unsigned i=0;i<PP_GYM_TEAM;i++)if(pp_gyms[gym].team[i].level>level)level=pp_gyms[gym].team[i].level;
  return level;
}
bool pp_gym_unlocked(const pp_game *g,unsigned gym) {
  return gym<PP_GYM_COUNT && (!gym || (g->gym.badges & ((1ULL<<gym)-1))==((1ULL<<gym)-1));
}
static const pp_move gym_moves[PP_GYM_COUNT][4] = {{
  {"落石","岩石","投掷岩石",PP_ROCK, 32}, {"撞击","一般","撞向对手",PP_NORMAL, 44},
  {"岩石封锁","岩石","封锁对手",PP_ROCK, 58}, {"挖洞","地面","钻地攻击",PP_GROUND, 74}
}, {
  {"水枪","水","喷射水流",PP_WATER, 32}, {"高速星星","一般","星形光芒",PP_NORMAL, 44},
  {"泡沫光线","水","喷射泡沫",PP_WATER, 58}, {"精神强念","超能","念力攻击",PP_PSYCHIC, 74}
}, {
  {"电击","电","释放电流",PP_ELECTRIC, 32}, {"电光一闪","一般","快速冲击",PP_NORMAL, 44},
  {"十万伏特","电","强力电流",PP_ELECTRIC, 58}, {"高速星星","一般","星形光芒",PP_NORMAL, 74}
}, {
  {"飞叶快刀","草","锋利叶片",PP_GRASS, 32}, {"溶解液","毒","喷射酸液",PP_POISON, 44},
  {"魔法叶","草","叶片追击",PP_GRASS, 58}, {"撞击","一般","撞向对手",PP_NORMAL, 74}
}, {
  {"污泥攻击","毒","污泥冲击",PP_POISON, 32}, {"撞击","一般","撞向对手",PP_NORMAL, 44},
  {"污泥炸弹","毒","污泥爆发",PP_POISON, 58}, {"溶解液","毒","喷射酸液",PP_POISON, 74}
}, {
  {"念力","超能","念力冲击",PP_PSYCHIC, 32}, {"高速星星","一般","星形光芒",PP_NORMAL, 44},
  {"幻象光线","超能","幻象攻击",PP_PSYCHIC, 58}, {"精神强念","超能","念力爆发",PP_PSYCHIC, 74}
}, {
  {"火花","火","火苗攻击",PP_FIRE, 32}, {"高速星星","一般","星形光芒",PP_NORMAL, 44},
  {"火焰轮","火","烈焰冲击",PP_FIRE, 58}, {"喷射火焰","火","喷射烈焰",PP_FIRE, 74}
}, {
  {"泥巴射击","地面","泥块攻击",PP_GROUND, 32}, {"猛撞","一般","全力撞击",PP_NORMAL, 44},
  {"岩石封锁","岩石","岩石攻击",PP_ROCK, 58}, {"地震","地面","大地震动",PP_GROUND, 74}
}};
const pp_move *pp_battle_move(const pp_game *g,unsigned side,unsigned slot) {
  if(side && pp_battle_encounter(g)){
    unsigned art=encounter.def->team[g->gym.enemy].art;
    for(unsigned i=0;i<PP_DEX_COUNT;i++)if(pp_pokedex[i].art==art)return pp_move_at(pp_pokedex[i].species,slot%4);
  }
  if(side && g->wild.phase>=PP_WILD_BATTLE)return pp_move_at(pp_pokedex[g->wild.dex].species,slot%4);
  if(side && !g->gym.mode)return pp_move_at((g->pets[g->battle_pet].species+1)%PP_SPECIES,slot%4);
  return g->gym.mode && side ? &gym_moves[g->gym.mode-1][slot%4] : pp_move_at(g->pets[g->battle_pet].species,slot%4);
}
unsigned pp_battle_move_power(const pp_game *g,unsigned side,unsigned slot) {
  if(slot>=4 || g->battle_pet>=g->count)return 0;
  if(side && pp_battle_encounter(g)){
    unsigned art=encounter.def->team[g->gym.enemy].art;
    for(unsigned i=0;i<PP_DEX_COUNT;i++)if(pp_pokedex[i].art==art)return pp_move_form_power(pp_pokedex[i].national_id,slot);
  }
  if(side && g->gym.mode)return pp_battle_move(g,side,slot)->power;
  unsigned species=g->pets[g->battle_pet].species;
  if(side && g->wild.phase>=PP_WILD_BATTLE)return pp_move_form_power(pp_pokedex[g->wild.dex].national_id,slot);
  unsigned art=side?pp_base_art((species+1)%PP_SPECIES):pp_art_id(g,g->battle_pet);
  for(unsigned i=0;i<PP_DEX_COUNT;i++)if(pp_pokedex[i].art==art)return pp_move_form_power(pp_pokedex[i].national_id,slot);
  return 0;
}
unsigned pp_gym_hp(const pp_game *g,unsigned p) {
  return pp_care_hp(g,p);
}
/* Fold the actual defender types; immunity takes precedence over weakness. */
unsigned pp_gym_effectiveness(const pp_game *g,bool enemy,unsigned slot) {
  pp_move_type move=pp_battle_move(g,enemy,slot)->type;
  unsigned art=enemy?pp_art_id(g,g->battle_pet):g->wild.phase>=PP_WILD_BATTLE?pp_pokedex[g->wild.dex].art:pp_gym_current(g)->team[g->gym.enemy].art;
  if(art>=PP_DEX_COUNT)return 100;
  unsigned scale=100;
  for(unsigned i=0;i<2;i++) {
    int type=pp_pet_types[art][i];
    if(type<0)continue;
    unsigned factor=100;
    if(move==PP_ROCK) {
      if(type==PP_FIRE || type==PP_ICE || type==PP_FLYING || type==PP_BUG)factor=200;
      if(type==PP_FIGHTING || type==PP_GROUND || type==PP_STEEL)factor=50;
    } else if(move==PP_GROUND) {
      if(type==PP_FLYING)factor=0;
      if(type==PP_FIRE || type==PP_ELECTRIC || type==PP_POISON || type==PP_ROCK || type==PP_STEEL)factor=200;
      if(type==PP_GRASS || type==PP_BUG)factor=50;
    } else if(move==PP_NORMAL) {
      if(type==PP_GHOST)factor=0;
      if(type==PP_ROCK || type==PP_STEEL)factor=50;
    } else if(move==PP_WATER) {
      if(type==PP_FIRE || type==PP_GROUND || type==PP_ROCK)factor=200;
      if(type==PP_WATER || type==PP_GRASS || type==PP_DRAGON)factor=50;
    } else if(move==PP_ELECTRIC) {
      if(type==PP_GROUND)factor=0;
      if(type==PP_WATER || type==PP_FLYING)factor=200;
      if(type==PP_ELECTRIC || type==PP_GRASS || type==PP_DRAGON)factor=50;
    } else if(move==PP_GRASS) {
      if(type==PP_WATER || type==PP_GROUND || type==PP_ROCK)factor=200;
      if(type==PP_FIRE || type==PP_GRASS || type==PP_POISON || type==PP_FLYING || type==PP_BUG || type==PP_STEEL || type==PP_DRAGON)factor=50;
    } else if(move==PP_POISON) {
      if(type==PP_STEEL)factor=0;
      if(type==PP_GRASS || type==PP_FAIRY)factor=200;
      if(type==PP_POISON || type==PP_GROUND || type==PP_ROCK || type==PP_GHOST)factor=50;
    } else if(move==PP_FIRE) {
      if(type==PP_GRASS || type==PP_ICE || type==PP_BUG || type==PP_STEEL)factor=200;
      if(type==PP_FIRE || type==PP_WATER || type==PP_ROCK || type==PP_DRAGON)factor=50;
    } else if(move==PP_FLYING) {
      if(type==PP_GRASS || type==PP_FIGHTING || type==PP_BUG)factor=200;
      if(type==PP_ELECTRIC || type==PP_ROCK || type==PP_STEEL)factor=50;
    } else if(move==PP_ICE) {
      if(type==PP_GRASS || type==PP_GROUND || type==PP_FLYING || type==PP_DRAGON)factor=200;
      if(type==PP_FIRE || type==PP_WATER || type==PP_ICE || type==PP_STEEL)factor=50;
    } else if(move==PP_BUG) {
      if(type==PP_GRASS || type==PP_PSYCHIC || type==PP_DARK)factor=200;
      if(type==PP_FIRE || type==PP_FIGHTING || type==PP_POISON || type==PP_FLYING || type==PP_GHOST || type==PP_STEEL || type==PP_FAIRY)factor=50;
    } else if(move==PP_PSYCHIC) {
      if(type==PP_POISON || type==PP_FIGHTING)factor=200;
      if(type==PP_DARK)factor=0;
      if(type==PP_PSYCHIC || type==PP_STEEL)factor=50;
    } else if(move==PP_STEEL) {
      if(type==PP_ROCK || type==PP_ICE || type==PP_FAIRY)factor=200;
      if(type==PP_FIRE || type==PP_WATER || type==PP_ELECTRIC || type==PP_STEEL)factor=50;
    } else if(move==PP_FAIRY) {
      if(type==PP_FIGHTING || type==PP_DARK || type==PP_DRAGON)factor=200;
      if(type==PP_FIRE || type==PP_POISON || type==PP_STEEL)factor=50;
    } else if(move==PP_FIGHTING) {
      if(type==PP_NORMAL || type==PP_ROCK || type==PP_ICE || type==PP_STEEL || type==PP_DARK)factor=200;
      if(type==PP_POISON || type==PP_FLYING || type==PP_PSYCHIC || type==PP_BUG || type==PP_FAIRY)factor=50;
      if(type==PP_GHOST)factor=0;
    } else if(move==PP_GHOST) {
      if(type==PP_PSYCHIC || type==PP_GHOST)factor=200;
      if(type==PP_DARK)factor=50;
      if(type==PP_NORMAL)factor=0;
    } else if(move==PP_DARK) {
      if(type==PP_PSYCHIC || type==PP_GHOST)factor=200;
      if(type==PP_DARK || type==PP_FIGHTING || type==PP_FAIRY)factor=50;
    } else if(move==PP_DRAGON) {
      if(type==PP_DRAGON)factor=200;
      if(type==PP_STEEL)factor=50;
      if(type==PP_FAIRY)factor=0;
    } else if(!enemy) {
      /* Remaining outgoing attacks against the actual gym defender types. */
      if((move==PP_FLYING || move==PP_STEEL) && type==PP_ELECTRIC)factor=50;
      if(move==PP_ELECTRIC && type==PP_GROUND)factor=0;
      if((move==PP_ELECTRIC || move==PP_GRASS) && type==PP_WATER)factor=200;
      if((move==PP_FIRE || move==PP_ICE || move==PP_STEEL) && type==PP_WATER)factor=50;
      if((move==PP_BUG || move==PP_GHOST || move==PP_DARK) && type==PP_PSYCHIC)factor=200;
      if(move==PP_FIGHTING && (type==PP_PSYCHIC || type==PP_POISON))factor=50;
      if((move==PP_WATER || move==PP_GRASS) && (type==PP_ROCK || type==PP_GROUND))factor=200;
      if((move==PP_FIGHTING || move==PP_STEEL || move==PP_GROUND) && type==PP_ROCK)factor=200;
      if(move==PP_ICE && type==PP_GROUND)factor=200;
      if((move==PP_FIRE || move==PP_FLYING || move==PP_POISON) && type==PP_ROCK)factor=50;
      if(move==PP_POISON && type==PP_GROUND)factor=50;
    }
    scale=scale*factor/100;
  }
  /* Compact-game curve: immunities 0, quarter 35, half 65, double 150, quadruple 200. */
  return !scale?0:scale<=25?35:scale<=50?65:scale>=400?200:scale>=200?150:100;
}
unsigned pp_gym_damage(const pp_game *g,bool enemy,unsigned slot,unsigned variance) {
  unsigned p=g->battle_pet,l=g->growth[p].level;
  const pp_gym_enemy *foe=&pp_gym_current(g)->team[g->gym.enemy];
  unsigned stat=pp_battle_move(g,enemy,slot)->type;
  bool special=stat==PP_FIRE||stat==PP_WATER||stat==PP_GRASS||stat==PP_ELECTRIC||stat==PP_ICE||stat==PP_PSYCHIC||stat==PP_DRAGON||stat==PP_DARK;
  unsigned atk=enemy?foe->attack:pp_individual_stat(g,p,special?3:1);
  unsigned def=enemy?pp_individual_stat(g,p,special?4:2):foe->defense;
  unsigned level=enemy?foe->level:l;
  unsigned damage=(12+level)*atk/(3*def)+4;
  unsigned percent=pp_gym_effectiveness(g,enemy,slot);
  unsigned power=pp_battle_move_power(g,enemy,slot);
  damage=(unsigned)((uint64_t)damage*power*percent*(90+variance%11)/550000);
  if(!enemy && pp_tarot_active(g,4))damage+=(damage+9)/10;
  return percent && power && !damage?1:damage;
}
pp_result pp_gym_start(pp_game *g,const unsigned team[3]) {
  return pp_gym_start_at(g,0,team);
}
pp_result pp_gym_start_at(pp_game *g,unsigned gym,const unsigned team[3]) {
  if(!team || gym>=PP_GYM_COUNT)return PP_INVALID;
  if(!pp_gym_unlocked(g,gym))return PP_NOT_READY;
  /* A trip or item use occupies one partner, not the whole roster.  Keep the
   * other healthy partners available for a gym team. */
  if(g->battling || g->wild.phase==PP_WILD_BATTLE || pp_needs_starter(g))return PP_BUSY;
  /* An opened encounter or uncaught partner needs an explicit player choice.
   * Finished receipts must not leave the whole gym permanently busy. */
  if(pp_wild_opened(g) || (g->wild.phase==PP_WILD_WON && g->wild.outcome==PP_WILD_CAPTURE_READY))return PP_BUSY;
  if(g->count<3)return PP_NEED_THREE;
  for(unsigned i=0;i<3;i++) {
    if(team[i]>=g->count)return PP_INVALID;
    for(unsigned j=0;j<i;j++)if(team[i]==team[j])return PP_INVALID;
    if(pp_training_busy(g,team[i]) || team[i]==(unsigned)g->traveler ||
       (g->drop_active && team[i]==g->drop_target))return PP_BUSY;
    if(g->pets[team[i]].sleeping || g->pets[team[i]].health<30 || g->pets[team[i]].energy<5)return PP_NOT_READY;
  }
  /* Validate the entire team before dismissing an untouched notice or closing
   * an already-settled receipt. No reward or capture opportunity is discarded. */
  if(g->wild.phase==PP_WILD_PENDING)pp_wild_dismiss(g);
  else if(g->wild.phase>=PP_WILD_WON)pp_wild_clear_result(g);
  pp_gym_state *b=&g->gym;
  uint64_t badges=b->badges;uint32_t wins=b->wins;
  memset(b,0,sizeof(*b));b->badges=badges;b->wins=wins;b->mode=gym+1;b->entered=1;
  for(unsigned i=0;i<3;i++) {
    b->team[i]=team[i];b->hp[i]=b->max_hp[i]=pp_gym_hp(g,team[i]);
    b->enemy_hp[i]=b->enemy_max[i]=pp_gyms[gym].team[i].hp;
    pp_spend_energy(g,team[i],5);g->pets[team[i]].food=sub(g->pets[team[i]].food,1);
  }
  g->battle_pet=team[0];g->battle_hp=b->hp[0];g->battle_max_hp=b->max_hp[0];
  g->enemy_hp=b->enemy_hp[0];g->enemy_max_hp=b->enemy_max[0];
  g->battle_round=0;g->battle_next=g->seconds+PP_BATTLE_TURN_SECONDS;g->battling=1;
  char line[96];snprintf(line,sizeof(line),"%s · 对手派出了%s！",pp_gyms[gym].title,pp_gyms[gym].team[0].name);log_line(g,line);return PP_OK;
}
void pp_gym_leave(pp_game *g) { if(!g->battling && g->gym.mode){g->gym.mode=0;g->revision++;} }
static unsigned gym_random(pp_game *g,unsigned bound) {
  /* Xorshift visits 1..UINT32_MAX; use a multiple-sized one-based domain. */
  uint32_t limit=UINT32_MAX-UINT32_MAX%bound,r;
  do {r=random_next(g);}while(r>limit);
  return (r-1)%bound;
}
static void gym_finish(pp_game *g,bool win) {
  pp_gym_state *b=&g->gym;g->battling=0;b->result=win?1:2;
  const pp_gym_def *def=pp_gym_current(g);
  if(pp_battle_encounter(g)){b->reward_xp=win?def->win_xp:def->loss_xp;g->revision++;return;}
  uint64_t badge=1ULL<<(b->mode-1);
  unsigned enemy_level=0,defeated=0;for(unsigned i=0;i<3;i++){enemy_level+=def->team[i].level;if(!b->enemy_hp[i])defeated++;}enemy_level/=3;
  bool first=win&&!(b->badges&badge);b->reward_xp=0;
  for(unsigned i=0;i<3;i++){unsigned pet=b->team[i],level=g->growth[pet].level;
   unsigned xp=pp_award_battle_xp(g,pet,level,enemy_level,defeated,3,win,3);
   if(first){unsigned before=10u*g->growth[pet].level*(g->growth[pet].level-1)+g->growth[pet].xp;unsigned bonus=level<PP_MAX_LEVEL?pp_xp_needed(level)*30/100:0;pp_gain_xp(g,pet,bonus);xp+=10u*g->growth[pet].level*(g->growth[pet].level-1)+g->growth[pet].xp-before;}
   if(!i){b->reward_xp=xp;}
   if(!b->hp[i])g->pets[pet].health=sub(g->pets[pet].health,1);
  }
  if(win) {
    b->first_clear=!(b->badges&badge);b->badges|=badge;b->wins++;
    bool collectible=b->first_clear||g->rematches_today==0;
    b->reward_coins=b->first_clear?def->first_coins:g->rematches_today<2?20+(b->mode-1)*5:0;if(!b->first_clear&&g->rematches_today<2)g->rematches_today++;
    g->coins=UINT32_MAX-g->coins<b->reward_coins?UINT32_MAX:g->coins+b->reward_coins;
    b->reward_item=PP_COLLECTION;if(collectible){
    unsigned roll=gym_random(g,100),quality=roll<def->rare_percent?2:roll<def->rare_percent+def->epic_percent?3:4;
    unsigned n=pp_collection_total(quality),index=gym_random(g,n);
    b->reward_item=pp_collection_id(quality,index);
    g->collected[b->reward_item/8]|=1u<<(b->reward_item%8);}
    char reward[96];snprintf(reward,sizeof(reward),"%s · %s · 金币 +%u",b->first_clear?"首通成功":"道馆再胜",def->badge,(unsigned)b->reward_coins);log_line(g,reward);
    if(b->reward_item<PP_COLLECTION){char line[96];snprintf(line,sizeof(line),"%s · %s",pp_qualities[pp_collectibles[b->reward_item].quality].name,pp_collectibles[b->reward_item].name);log_line(g,line);}
  } else {char line[96];snprintf(line,sizeof(line),"挑战惜败 · 已出场伙伴经验 +%u",b->reward_xp);log_line(g,line);}
}
static void gym_tick(pp_game *g) {
  pp_gym_state *b=&g->gym;
  /* At most one visible turn per call: offline catch-up cannot skip the roster. */
  if(!g->battling || g->seconds<g->battle_next)return;
  g->battle_next=g->seconds+PP_BATTLE_TURN_SECONDS;
  if(!g->battle_hp || !g->enemy_hp) {
    if(!g->enemy_hp) b->enemy++;
    if(!g->battle_hp) b->ally++;
    g->battle_pet=b->team[b->ally];b->entered|=1u<<b->ally;
    g->battle_hp=b->hp[b->ally];g->battle_max_hp=b->max_hp[b->ally];
    g->enemy_hp=b->enemy_hp[b->enemy];g->enemy_max_hp=b->enemy_max[b->enemy];
    log_line(g,"新的伙伴上场！");return;
  }
  if(pp_battle_encounter(g) && g->battle_round>=100){gym_finish(g,false);return;}
  unsigned slot=g->battle_round++%4;
  unsigned hit=pp_gym_damage(g,false,slot,random_next(g));
  g->enemy_hp=g->enemy_hp>(int)hit?g->enemy_hp-hit:0;b->enemy_hp[b->enemy]=g->enemy_hp;
  if(!g->enemy_hp) {
    log_line(g,"对方伙伴倒下了！");
    if(b->enemy+1==pp_battle_team_count(g))gym_finish(g,true);
    return;
  }
  hit=pp_gym_damage(g,true,slot,random_next(g));
  g->battle_hp=g->battle_hp>(int)hit?g->battle_hp-hit:0;b->hp[b->ally]=g->battle_hp;
  g->revision++;
  if(!g->battle_hp) {
    log_line(g,"伙伴倒下了，下一位准备上场！");
    if(b->ally+1==pp_battle_team_count(g))gym_finish(g,false);
  }
}
/* Wild receipts are persisted with the grant, never granted by the UI. */
/* Count online, visible garden activity, never offline pp_tick catch-up. */
void pp_gift_scene(pp_game *g,bool active,unsigned seconds) {
  if(!active || !seconds || seconds>5 || g->gift.pending || g->wild.phase ||
     g->battling || g->gym.mode || g->drop_active || g->digging || pp_needs_starter(g))return;
  unsigned pets[PP_PETS],count=0,missing=0;
  for(unsigned p=0;p<g->count;p++)if(!pp_training_busy(g,p) && g->traveler!=(int)p && !g->pets[p].sleeping && g->pets[p].health>=30)pets[count++]=p;
  if(!count)return;
  for(unsigned id=0;id<PP_TRAVEL_COLLECTION;id++)missing+=!pp_has(g,id);
  if(!missing)return;
  g->gift.active_seconds+=seconds;
  if(g->gift.active_seconds<PP_GIFT_INTERVAL)return;
  g->gift.active_seconds-=PP_GIFT_INTERVAL;
  g->revision++;
  if(gym_random(g,100)>=PP_GIFT_CHANCE)return;
  unsigned pick=gym_random(g,missing),id=0;
  for(;id<PP_TRAVEL_COLLECTION;id++)if(!pp_has(g,id) && !pick--)break;
  unsigned pet=pets[gym_random(g,count)],art=pp_art_id(g,pet),dex=0;
  for(;dex<PP_DEX_COUNT;dex++)if(pp_pokedex[dex].art==art)break;
  if(dex==PP_DEX_COUNT)return;
  g->collected[id/8]|=1u<<(id%8);
  g->gift.item=id;g->gift.dex=dex;g->gift.pending=1;
  char line[96];snprintf(line,sizeof(line),"%s找到%s，送给主人！",pp_form_name(g,pet),pp_collectibles[id].name);log_line(g,line);
}
void pp_gift_ack(pp_game *g) {
  if(!g->gift.pending)return;
  g->gift.pending=0;g->revision++;
}
static void wild_cooldown(pp_game *g){if(!g->wild_steps_left)g->wild_steps_left=PP_WILD_INTERVAL_MIN+random_next(g)%(PP_WILD_INTERVAL_MAX-PP_WILD_INTERVAL_MIN+1);g->wild.next_at=PP_WILD_STEP_MARK|g->wild_steps_left;}
static void wild_charge(pp_game *g){if(g->wild_queue>=3)return;g->wild_queue++;g->wild_active_seconds=0;g->wild_steps_left=PP_WILD_INTERVAL_MIN+random_next(g)%(PP_WILD_INTERVAL_MAX-PP_WILD_INTERVAL_MIN+1);g->revision++;}
static void wild_consume(pp_game *g){if(g->wild_queue)g->wild_queue--;g->wild_delay=60;g->wild_visible_left=PP_WILD_NOTICE_SECONDS;}
unsigned pp_wild_steps_remaining(const pp_game *g){return g->wild_queue>=3?0:g->wild_steps_left;}
bool pp_wild_eligible(const pp_game *g,unsigned p) {
  return p<g->count && !pp_training_busy(g,p) && g->traveler!=(int)p && !g->pets[p].sleeping && g->pets[p].health>=30 && g->pets[p].energy>=3;
}
unsigned pp_wild_default_pet(const pp_game *g) {
  unsigned best=PP_PETS;
  for(unsigned p=0;p<g->count;p++)if(pp_wild_eligible(g,p) &&
      (best==PP_PETS || g->growth[p].level>g->growth[best].level))best=p;
  return best;
}
unsigned pp_wild_alert_remaining(const pp_game *g){return g->wild.phase==PP_WILD_PENDING&&g->wild.next_at?g->wild_visible_left:0;}
pp_result pp_wild_reveal(pp_game *g) {
  if(g->wild.phase!=PP_WILD_PENDING || !g->wild.next_at)return PP_NOT_READY;
  if(!pp_wild_alert_remaining(g)){pp_wild_dismiss(g);return PP_NOT_READY;}
  g->wild.next_at=0;g->wild.escape_at=UINT64_MAX;g->revision++;
  return PP_OK;
}
bool pp_wild_opened(const pp_game *g) {
  return g->wild.phase==PP_WILD_PENDING && !g->wild.next_at;
}
void pp_wild_dismiss(pp_game *g) {
  if(g->wild.phase!=PP_WILD_PENDING)return;
  g->wild.phase=PP_WILD_NONE;g->wild.escape_at=0;wild_consume(g);wild_cooldown(g);
  log_line(g,"野生伙伴离开了。");
}
void pp_wild_clear_result(pp_game *g) {
  if(g->wild.phase<PP_WILD_WON)return;
  if(!g->legend.active){wild_consume(g);}
  memset(&g->wild,0,sizeof(g->wild));g->legend.active=0;wild_cooldown(g);g->revision++;
}
void pp_wild_scene(pp_game *g,bool active) {
 unsigned elapsed=g->seconds>=g->wild_observed_seconds?(unsigned)(g->seconds-g->wild_observed_seconds):0;g->wild_observed_seconds=g->seconds;if(elapsed>5)elapsed=0;
 if(!active||g->scene>=12||g->gift.pending||g->battling||g->gym.mode||g->drop_active||g->digging||pp_needs_starter(g))return;
 if(g->wild.phase==PP_WILD_PENDING){if(g->wild.next_at&&elapsed){g->wild_visible_left=elapsed>=g->wild_visible_left?0:g->wild_visible_left-elapsed;if(!g->wild_visible_left)pp_wild_dismiss(g);}return;}
 if(g->wild.phase)return;
 if(g->wild_queue<3){g->wild_active_seconds+=elapsed;if(g->wild_active_seconds>=1200)wild_charge(g);}
 if(g->wild_delay){g->wild_delay=elapsed>=g->wild_delay?0:g->wild_delay-elapsed;return;}
 if(!g->wild_queue)return;
  unsigned level=0;
  for(unsigned p=0;p<g->count;p++)if(pp_wild_eligible(g,p) && g->growth[p].level>level)level=g->growth[p].level;
  if(!level)return;

  unsigned candidates[PP_DEX_COUNT],count=0;
  for(unsigned d=0;d<PP_DEX_COUNT;d++) {
    unsigned min=pp_pokedex[d].stage?pp_stage_level(pp_pokedex[d].species,pp_pokedex[d].stage):1;
    if(min<=level && pp_habitat_has(g->scene,pp_pokedex[d].species) && !pp_is_legendary(pp_pokedex[d].national_id))candidates[count++]=d;
  }
  if(!count)return;
  unsigned missing[PP_DEX_COUNT],nmissing=0;bool owned[PP_SPECIES]={0};for(unsigned i=0;i<g->count;i++)owned[g->pets[i].species]=true;for(unsigned i=0;i<pp_warehouse_count(g);i++){pp_stored_pet pet;pp_warehouse_get(g,i,&pet);owned[pet.pet.species]=true;}
  for(unsigned i=0;i<count;i++)if(!owned[pp_pokedex[candidates[i]].species])missing[nmissing++]=candidates[i];
  pp_wild_state *w=&g->wild;w->dex=nmissing&&random_next(g)%2?missing[random_next(g)%nmissing]:candidates[random_next(g)%count];
  const pp_dex_entry *e=&pp_pokedex[w->dex];
  unsigned min=e->stage?pp_stage_level(e->species,e->stage):1;
  unsigned floor=level>3?level-3:1;if(floor<min)floor=min;
  w->level=floor+random_next(g)%(level-floor+1);w->scene=g->scene;
  w->phase=PP_WILD_PENDING;
  w->next_at=PP_WILD_STEP_MARK;
  w->escape_at=UINT64_MAX;g->wild_visible_left=PP_WILD_NOTICE_SECONDS;
  w->outcome=w->reward_coins=w->reward_xp=0;
  log_line(g,"草丛中出现了野生伙伴！");
}
void pp_wild_screen(pp_game *g,bool screen_on){(void)g;(void)screen_on;}
pp_result pp_wild_start(pp_game *g,unsigned p) {
  if(p>=g->count)return PP_INVALID;
  if(!pp_wild_opened(g) || pp_is_legendary(pp_pokedex[g->wild.dex].national_id))return PP_NOT_READY;
  if(g->battling || g->drop_active)return PP_BUSY;
  if(!pp_wild_eligible(g,p))return PP_NOT_READY;
  g->gym.mode=0;g->wild.phase=PP_WILD_BATTLE;g->wild.escape_at=0;
  g->battling=1;g->battle_pet=p;g->battle_round=0;
  g->battle_hp=g->battle_max_hp=pp_gym_hp(g,p);
  g->enemy_hp=g->enemy_max_hp=56+5*g->wild.level+6*!!pp_pokedex[g->wild.dex].stage;
  g->battle_next=g->seconds+PP_BATTLE_TURN_SECONDS;pp_spend_energy(g,p,3);
  log_line(g,"野外对战开始，获胜后收服伙伴！");return PP_OK;
}
pp_result pp_legend_start(pp_game *g,unsigned legend,unsigned pet) {
  if(legend>=PP_LEGEND_COUNT || pet>=g->count)return PP_INVALID;
  if(!pp_legend_unlocked(g))return PP_NOT_READY;
  if(roster_busy(g) || g->digging)return PP_BUSY;
  if(!pp_wild_eligible(g,pet))return PP_NOT_READY;
  if(g->legend.eggs[legend]==UINT16_MAX)return PP_FULL;
  pp_gym_leave(g);
  g->legend.active=legend+1;
  g->wild=(pp_wild_state){.phase=PP_WILD_BATTLE,.dex=pp_legend_dex(legend),.level=pp_legends[legend].level,.scene=g->scene};
  g->battling=1;g->battle_pet=pet;g->battle_round=0;
  g->battle_hp=g->battle_max_hp=pp_gym_hp(g,pet);
  g->enemy_hp=g->enemy_max_hp=356;g->battle_next=g->seconds+PP_BATTLE_TURN_SECONDS;
  pp_spend_energy(g,pet,5);
  log_line(g,"神兽挑战开始！获胜获得对应的蛋。");return PP_OK;
}
pp_result pp_legend_egg_use(pp_game *g,unsigned legend) {
  if(legend>=PP_LEGEND_COUNT)return PP_INVALID;
  if(!g->legend.eggs[legend])return PP_EMPTY;
  if(roster_busy(g) || g->egg_end || g->digging)return PP_BUSY;
  g->legend.eggs[legend]--;g->legend.hatching=legend+1;
  g->egg_end=g->seconds+PP_EGG_MIN_SECONDS+(random_next(g)%121u)*60u;g->egg_shakes=0;
  char line[96];snprintf(line,sizeof(line),"%s的蛋已放入庭院，摇一摇开始孵化。",pp_legends[legend].name);log_line(g,line);return PP_OK;
}
static void wild_finish(pp_game *g,bool win) {
  pp_wild_state *w=&g->wild;g->battling=0;w->phase=win?PP_WILD_WON:PP_WILD_LOST;
  if(!win){log_line(g,g->legend.active?"神兽挑战结束，调整伙伴后再来吧。":"野生伙伴逃走了，照顾好伙伴再出发吧。");return;}
  if(g->legend.active) {
    unsigned n=g->legend.active-1;
    g->legend.eggs[n]++;g->legend.defeated|=1u<<n;w->outcome=PP_WILD_EGG;
    char line[96];snprintf(line,sizeof(line),"%s的蛋已放入背包。",pp_legends[n].name);log_line(g,line);return;
  }
  w->outcome=PP_WILD_CAPTURE_READY;
  w->reward_coins=PP_WILD_BASE_COINS;w->reward_xp=pp_award_battle_xp(g,g->battle_pet,g->growth[g->battle_pet].level,w->level,1,1,true,2);
  g->coins=UINT32_MAX-g->coins<w->reward_coins?UINT32_MAX:g->coins+w->reward_coins;

  log_line(g,"对战获胜！使用咕噜球带伙伴回家。");
}
pp_result pp_wild_capture(pp_game *g){
 if(g->wild.phase!=PP_WILD_WON||g->wild.outcome!=PP_WILD_CAPTURE_READY||g->legend.active)return PP_INVALID;
 if(!g->balls)return PP_NO_BALL;
 const pp_dex_entry *e=&pp_pokedex[g->wild.dex];
 if(!grant_pet(g,e->species,g->wild.level,e->stage))return PP_STORAGE_FULL;
 g->balls--;g->wild.outcome=PP_WILD_CAUGHT;log_line(g,"捕捉成功！伙伴加入了。");return PP_OK;
}
/* The five nature axes follow Attack, Defense, Speed, Sp. Attack, Sp. Defense. */
unsigned pp_individual_stat(const pp_game *g,unsigned p,unsigned stat){
 if(p>=g->count||stat>=6)return 0;
 unsigned l=g->growth[p].level,rank=growth_rank(g,p),base;
 if(stat==0)base=64+6*l+6*rank;else if(stat==1||stat==3)base=22+3*l+4*rank;
 else if(stat==5)base=20+2*l;else base=16+2*l+3*rank;
 const pp_identity *id=&g->identity[p];base+=id->iv[stat]*l/60;
 if(stat){static const unsigned axis[]={0,0,1,3,4,2};unsigned up=id->nature/5,down=id->nature%5;
   if(up!=down){if(axis[stat]==up)base=base*110/100;else if(axis[stat]==down)base=base*90/100;}}
 return base;
}
static unsigned wild_damage(const pp_game *g,bool enemy,unsigned slot,unsigned variance) {
  unsigned player=g->growth[g->battle_pet].level,foe=g->wild.level;
  unsigned level=enemy?foe:player,deflevel=enemy?player:foe;
  unsigned type=pp_battle_move(g,enemy,slot)->type;
  bool special=type==PP_FIRE||type==PP_WATER||type==PP_GRASS||type==PP_ELECTRIC||type==PP_ICE||type==PP_PSYCHIC||type==PP_DRAGON||type==PP_DARK;
  unsigned atk=enemy?22+3*level:pp_individual_stat(g,g->battle_pet,special?3:1);
  unsigned def=enemy?pp_individual_stat(g,g->battle_pet,special?4:2):16+2*deflevel;
  unsigned damage=(12+level)*atk/(3*def)+4;
  unsigned percent=pp_gym_effectiveness(g,enemy,slot);
  unsigned power=pp_battle_move_power(g,enemy,slot);
  damage=(unsigned)((uint64_t)damage*power*percent*(90+variance%11)/550000);
  if(!enemy && pp_tarot_active(g,4))damage+=(damage+9)/10;
  return percent && power && !damage?1:damage;
}
static void wild_tick(pp_game *g) {

  if(g->wild.phase!=PP_WILD_BATTLE || !g->battling || g->seconds<g->battle_next)return;
  /* Match visible three-second turns; never fast-forward an unseen battle. */
  g->battle_next=g->seconds+PP_BATTLE_TURN_SECONDS;
  unsigned slot=g->battle_round++%4,hit=wild_damage(g,false,slot,random_next(g));
  g->enemy_hp=g->enemy_hp>(int)hit?g->enemy_hp-hit:0;
  g->revision++;
  if(!g->enemy_hp){wild_finish(g,true);return;}
  hit=wild_damage(g,true,slot,random_next(g));
  g->battle_hp=g->battle_hp>(int)hit?g->battle_hp-hit:0;
  if(!g->battle_hp || g->battle_round>=60)wild_finish(g,false);
}
pp_result pp_dig_start(pp_game *g) {
  if (!g->clock_ready)
    return PP_NOT_READY;
  if (g->digging)
    return PP_BUSY;
  if (g->digs_today >= 6)
    return PP_DAILY_LIMIT;
  g->digs_today++;
  g->digging = 1;
  g->dig_lives = 3;
  g->dig_depth = 0;
  g->dig_reward = 0;
  g->revision++;
  return PP_OK;
}
pp_result pp_dig_stop(pp_game *g) {
  if (!g->digging)
    return PP_INVALID;
  g->coins += g->dig_reward;
  g->digging = 0;
  log_line(g, "挖宝结束，金币已收入背包。");
  return PP_OK;
}
pp_result pp_shake(pp_game *g) {
  bool changed = false;
  if (g->egg_end) {
    if (g->egg_shakes < 10)
      g->egg_shakes++;
    changed = true;
  }
  if (g->digging) {
    if (random_next(g) % 4 == 0) {
      g->dig_lives--;
      log_line(g, "遇到塌方，生命 -1。");
    } else {
      g->dig_depth++;
      g->dig_reward += 1+(g->dig_depth%5==0?3:0);
      log_line(g, "又深一层，宝藏更多啦！");
    }
    if (!g->dig_lives || g->dig_depth >= 20)
      pp_dig_stop(g);
    changed = true;
  }
  if (changed) {
    g->revision++;
    return PP_OK;
  }
  return PP_INVALID;
}
pp_result pp_tarot(pp_game *g) {
  if (!g->clock_ready)
    return PP_NOT_READY;
  if (g->tarot_day == g->day)
    return PP_DAILY_LIMIT;
  g->tarot_day = g->day;
  g->tarot_card = random_next(g) % 8;
  g->tarot_effect_day = g->day;
  g->coins += 10;
  log_line(g, pp_tarot_effects[g->tarot_card]);
  return PP_OK;
}
/* Keep existing ownership, and grant the collection reward without selecting it. */
unsigned pp_scene_required(unsigned scene){static const uint8_t n[13]=PP_BALANCE_SCENES;return scene<PP_SCENES?n[scene]:UINT_MAX;}
unsigned pp_prosperity(const pp_game *g){return pp_dex_count(g);}
static void unlock_scene_reward(pp_game *g){unsigned n=pp_prosperity(g),before=g->scenes;for(unsigned i=0;i<PP_SCENES;i++)if(n>=pp_scene_required(i))g->scenes|=1u<<i;if(g->scenes!=before)g->revision++;}
pp_result pp_scene(pp_game *g,unsigned scene){
 if(scene>=PP_SCENES)return PP_INVALID;
 unlock_scene_reward(g);
 if(!(g->scenes&(1u<<scene)))return PP_NOT_READY;
 g->scene=scene;g->revision++;return PP_OK;
}
static void daily_scene(pp_game *g) {
  if(!g->random_scene||!g->clock_ready||!g->day||g->random_scene_day>=g->day) return;
  unsigned choices[PP_SCENES],n=0;
  for(unsigned i=0;i<PP_SCENES;i++) if((g->scenes&(1u<<i)) && i!=g->scene) choices[n++]=i;
  if(n)g->scene=choices[random_next(g)%n];
  g->random_scene_day=g->day;g->revision++;
}
pp_result pp_random_scene(pp_game *g,bool enabled) {
  g->random_scene=enabled;daily_scene(g);g->revision++;return PP_OK;
}
/* Care shares its saved remainder across live and offline updates. */
static void care_advance(pp_game *g,uint32_t dt,uint64_t end){
 unsigned bounded=dt>28800?28800:dt,total=g->care_remainder+bounded,steps=total/600;
 g->care_remainder=total%600;uint64_t tick=end/600;
 for(unsigned i=0;i<g->count+pp_warehouse_count(g);i++){
  pp_stored_pet stored;pp_pet *p;pp_pet_balance *balance;
  if(i<g->count){p=&g->pets[i];balance=&g->pet_balance[i];}else{pp_warehouse_get(g,i-g->count,&stored);p=&stored.pet;balance=&stored.balance;}
  if(i<g->count&&!pp_training_busy(g,i)){unsigned food_ticks=0,other_ticks=0;for(unsigned k=0;k<steps;k++){food_ticks+=((tick-k)%6==0);other_ticks+=((tick-k)%12==0);}
   p->food=sub(p->food,food_ticks);p->clean=sub(p->clean,other_ticks);p->mood=sub(p->mood,other_ticks);
   if(dt>5){if(p->food<20&&p->food+food_ticks>=20)p->food=20;if(p->clean<20&&p->clean+other_ticks>=20)p->clean=20;if(p->mood<30&&p->mood+other_ticks>=30)p->mood=30;}
  }
  bool busy=pp_training_busy(g,i<g->count?i:PP_PETS+i-g->count)||(i<g->count&&(int)i==g->traveler)||(g->battling&&i<g->count&&g->battle_pet==i);
  if(!busy)pp_energy_restore(&p->energy,&p->sleeping,&balance->energy_fraction,bounded,pp_tarot_active(g,2));
  if(i>=g->count)warehouse_set(g,i-g->count,&stored);
 }
 if(dt)g->revision++;
}
static void care_apply(pp_game *g,pp_pet *p,unsigned id) {
  const pp_item *item=&pp_items[id];
  p->food=add(p->food,recovery(g,item->food));
  p->clean=add(p->clean,recovery(g,item->clean));
  p->health=add(p->health,recovery(g,item->health));
  p->mood=add(p->mood,recovery(g,item->mood));
  if(id==4){p->energy=add(p->energy,recovery(g,25));pp_energy_wake(p->energy,&p->sleeping);}
  if(id==PP_TREE_ESSENCE){p->food=p->clean=p->health=p->mood=p->energy=100;pp_energy_wake(p->energy,&p->sleeping);}
}
unsigned pp_training_supply(pp_game *g,unsigned pet) {
  if(pet>=g->count||!pp_training_busy(g,pet))return 0;
  static const unsigned thresholds[5]=PP_TRAINING_SUPPLY_THRESHOLDS;
  pp_pet *p=&g->pets[pet];uint8_t *fields[]={&p->food,&p->clean,&p->health,&p->mood,&p->energy};unsigned used=0;
  for(unsigned i=0;i<5;i++)if(pp_item_enabled(i)){
    while((*fields[i]<thresholds[i]||(i==4&&p->sleeping))&&g->inventory[i]){
      g->inventory[i]--;care_apply(g,p,i);used++;
    }
  }
  bool low=p->sleeping;for(unsigned i=0;i<5;i++)low|=*fields[i]<thresholds[i];
  if(low&&g->tree_essences){g->tree_essences--;care_apply(g,p,PP_TREE_ESSENCE);used++;}
  if(used){g->revision++;log_line(g,"历练自动补给，背包物品已使用。");}
  return used;
}
static void care_complete(pp_game *g) {
  care_apply(g,&g->pets[g->drop_target],g->drop_item);
  g->drop_active=0;
  log_line(g,"宠物使用了道具，状态恢复啦。");
}
void pp_tick(pp_game *g, uint32_t dt, uint32_t day) {
  unlock_scene_reward(g);
  g->clock_ready = day != 0 && day >= g->day;
  if (day && day > g->day) {
    if (g->day) {
      g->steps_today = g->steps_rewarded = g->steps_coins = g->steps_xp = 0;g->pat_coins_today=0;g->fish_today=0;g->rematches_today=0;
      memset(g->pats_today,0,sizeof(g->pats_today));
      memset(g->reserve_pats_today,0,sizeof(g->reserve_pats_today));
      for(unsigned i=0;i<g->overflow_count;i++)g->overflow[i].pats_today=0;
    }
    g->day = day;
    g->digs_today = 0;
    g->revision++;
  }
  daily_scene(g);
  if (!dt)
    return;
  uint64_t before=g->seconds;
  g->seconds += dt;
  /* A pending biscuit completes at its saved deadline, before any subsequent
   * offline care boundaries. Never apply hours of sleep then add it last. */
  if(g->drop_active && g->seconds>=g->drop_end) {
    uint32_t until=g->drop_end>before?(uint32_t)(g->drop_end-before):0;
    care_advance(g,until,before+until);
    care_complete(g);
    care_advance(g,dt-until,g->seconds);
  } else care_advance(g,dt,g->seconds);
  if (g->egg_end && g->seconds >= g->egg_end && g->egg_shakes >= 1) {
    if(!hatch_pet(g))return;
    g->egg_end = 0;
    log_line(g, "蛋孵化啦！新的伙伴加入了庭院。");
  }
  if (g->traveler >= 0) {
    uint64_t start = g->travel_end - PP_TRAVEL_SECONDS;
    unsigned e = (unsigned)((g->seconds > g->travel_end ? PP_TRAVEL_SECONDS
                                                        : g->seconds - start) /
                            1800);
    if (e > g->travel_events) {
      unsigned gained = (e - g->travel_events) * PP_TRAVEL_EVENT_COINS;
      g->travel_reward = e * PP_TRAVEL_EVENT_COINS;
      g->travel_events = e;
      char line[96];
      snprintf(line,sizeof(line),"%s · 旅途金币 +%u",pp_travel_describe(g).event,gained);
      log_line(g, line);
    }
  }
  if(g->wild.phase){wild_tick(g);return;}
  if (g->gym.mode) { gym_tick(g); return; }
  unsigned turns = 0;
  while (g->battling && g->seconds >= g->battle_next && turns++ < 60) {
    g->battle_next += PP_BATTLE_TURN_SECONDS;
    unsigned slot = g->battle_round++ % PP_MOVE_SLOTS;
    unsigned hit = (10 + g->growth[g->battle_pet].level/2 + random_next(g) % 9) * pp_battle_move_power(g,0,slot) / 55;
    if(pp_tarot_active(g,4)) hit+=(hit+9)/10;
    g->enemy_hp -= hit;
    char line[96];
    snprintf(line, sizeof(line), "%s使用%s，造成 %u HP 伤害",
             pp_pet_names[g->pets[g->battle_pet].species],
             pp_move_at(g->pets[g->battle_pet].species, slot)->name, hit);
    log_line(g, line);
    if (g->enemy_hp <= 0) {
      g->enemy_hp = 0;
      g->battling = 0;
      g->stage++;
      g->coins += 4;
      pp_award_battle_xp(g,g->battle_pet,g->growth[g->battle_pet].level,g->growth[g->battle_pet].level,1,1,true,1);
      log_line(g, "挑战成功！金币 +4，下一关已解锁。");
      break;
    }
    g->battle_hp -= (6 + g->growth[g->battle_pet].level/3 + random_next(g) % 7) * pp_battle_move_power(g,1,slot)/55;
    if (g->battle_hp <= 0) {
      g->battle_hp = 0;
      g->battling = 0;
      
      log_line(g, "这次惜败，已获得训练经验，再来挑战吧。");
    }
  }
}

unsigned pp_care_hp(const pp_game *g,unsigned pet){unsigned hp=pp_individual_stat(g,pet,0),h=g->pets[pet].health;return hp*(h>=80?100:h>=60?90:80)/100;}
unsigned pp_battle_xp(const pp_pet *p,unsigned level,unsigned enemy){unsigned effective=enemy<level+3?enemy:level+3;unsigned base=1+PP_BALANCE_XP_TENTHS*effective/10;unsigned gap=level>enemy+10?20:level>enemy+5?50:100;
 unsigned food=p->food>=80?105:p->food<40?80:100,mood=p->mood>=80?105:p->mood<40?90:100,care=food*mood;if(care>11000)care=11000;return base*gap*care;}
unsigned pp_award_battle_xp(pp_game *g,unsigned pet,unsigned level,unsigned enemy,unsigned defeated,unsigned count,bool win,unsigned multiplier){
 if(pet>=g->count||!count)return 0;
 unsigned before=10u*g->growth[pet].level*(g->growth[pet].level-1)+g->growth[pet].xp;
 uint64_t value=(uint64_t)pp_battle_xp(&g->pets[pet],level,enemy)*multiplier;
 if(!win){value=value*defeated/(5*count);}
 value+=g->pet_balance[pet].xp_fraction;
 unsigned xp=value/1000000;g->pet_balance[pet].xp_fraction=value%1000000;
 pp_gain_xp(g,pet,xp);if(g->growth[pet].level==PP_MAX_LEVEL)g->pet_balance[pet].xp_fraction=0;
 g->revision++;
 return 10u*g->growth[pet].level*(g->growth[pet].level-1)+g->growth[pet].xp-before;
}

void pp_spend_energy(pp_game *g,unsigned pet,unsigned amount){if(pet>=g->count)return;unsigned cost=amount*100;if(pp_tarot_active(g,0))cost=cost*90/100;cost+=g->pet_balance[pet].energy_discount;g->pet_balance[pet].energy_discount=cost%100;g->pets[pet].energy=sub(g->pets[pet].energy,cost/100);if(g->pets[pet].energy<20)g->pets[pet].sleeping=1;g->revision++;}
void pp_training_cost(pp_game *g,unsigned pet,bool fainted){if(pet>=g->count)return;pp_pet *p=&g->pets[pet];pp_spend_energy(g,pet,1);unsigned n=++g->pet_balance[pet].battles;if(n%10==0)p->food=sub(p->food,1);if(n==20){p->clean=sub(p->clean,1);g->pet_balance[pet].battles=0;}if(fainted)p->health=sub(p->health,1);}
pp_result pp_hospital_treat(pp_game *g,unsigned pet){if(pet>=g->count)return PP_INVALID;if(pp_training_busy(g,pet)||g->traveler==(int)pet||g->battling||g->drop_active)return PP_BUSY;if(g->pets[pet].health==100)return PP_NOT_READY;if(g->coins<20)return PP_NO_COINS;g->coins-=20;g->pets[pet].health=100;g->revision++;return PP_OK;}
unsigned pp_training_xp(unsigned level,bool win) { return win?(1+PP_BALANCE_XP_TENTHS*level/10)*PP_TRAINING_XP_MULTIPLIER:0; }
unsigned pp_xp_needed(unsigned level) { return level >= PP_MAX_LEVEL ? 1 : 20 * level; }
void pp_gain_xp(pp_game *g, unsigned pet, unsigned xp) {
  if (pet >= g->count || !xp)
    return;
  pp_growth *p = &g->growth[pet];
  if (p->level >= PP_MAX_LEVEL)
    return;
  uint64_t total = (uint64_t)p->xp + xp;
  while (p->level < PP_MAX_LEVEL && total >= pp_xp_needed(p->level)) {
    total -= pp_xp_needed(p->level);
    p->level++;
  }
  p->xp = p->level == PP_MAX_LEVEL ? 0 : total;
  g->revision++;
}
pp_result pp_roster_gain_xp(pp_game *g,bool warehouse,unsigned slot,unsigned xp){
  if(!warehouse){if(slot>=g->count)return PP_INVALID;pp_gain_xp(g,slot,xp);return PP_OK;}
  pp_stored_pet p;if(!pp_warehouse_get(g,slot,&p))return PP_INVALID;
  if(p.growth.level>=PP_MAX_LEVEL || !xp)return PP_OK;
  uint64_t total=(uint64_t)p.growth.xp+xp;
  while(p.growth.level<PP_MAX_LEVEL && total>=pp_xp_needed(p.growth.level)){total-=pp_xp_needed(p.growth.level);p.growth.level++;}
  p.growth.xp=p.growth.level==PP_MAX_LEVEL?0:total;warehouse_set(g,slot,&p);g->revision++;return PP_OK;
}
const pp_evolution_template pp_evolutions[PP_EVOLUTIONS] = {{2, 16, 0, 4, "火恐龙"},
                                                {2, 36, 1, 5, "喷火龙"},
 {1,16,0,6,"妙蛙草"}, {1,32,1,7,"妙蛙花"},
 {3,16,0,8,"卡咪龟"}, {3,36,1,9,"水箭龟"},
 {4,7,0,11,"铁甲蛹"}, {4,10,1,12,"巴大蝶"},
 {5,7,0,14,"铁壳蛹"}, {5,10,1,15,"大针蜂"},
 {6,18,0,17,"比比鸟"}, {6,36,1,18,"大比鸟"},
 {7,20,0,20,"拉达"}, {8,20,0,22,"大嘴雀"},
 {9,22,0,24,"阿柏怪"}, {0,0,0,25,"雷丘"},
 {10,22,0,27,"穿山王"}, {11,16,0,29,"尼多娜"}, {11,0,1,30,"尼多后"},
 {12,16,0,32,"尼多力诺"}, {12,0,1,33,"尼多王"}, {13,0,0,35,"皮可西"},
 {14,0,0,37,"九尾"}, {15,0,0,39,"胖可丁"}, {16,22,0,41,"大嘴蝠"},
 {17,21,0,43,"臭臭花"}, {17,0,1,44,"霸王花"},
 {18,24,0,46,"派拉斯特"}, {19,31,0,48,"摩鲁蛾"}, {20,26,0,50,"三地鼠"},
 {21,28,0,52,"猫老大"}, {22,33,0,54,"哥达鸭"},
 {23,28,0,56,"火暴猴"}, {24,0,0,58,"风速狗"},
 {25,25,0,60,"蚊香君"}, {25,0,1,61,"蚊香泳士"},
 {26,16,0,63,"勇基拉"}, {26,0,1,64,"胡地"},
 {27,28,0,66,"豪力"}, {27,0,1,67,"怪力"},
 {28,21,0,69,"口呆花"}, {28,0,1,70,"大食花"},
 {29,30,0,72,"毒刺水母"}, {30,25,0,74,"隆隆石"}, {30,0,1,75,"隆隆岩"},
 {31,40,0,77,"烈焰马"}, {32,37,0,79,"呆壳兽"}, {33,30,0,81,"三合一磁怪"}, {35,31,0,84,"嘟嘟利"},
 {36,34,0,86,"白海狮"}, {37,38,0,88,"臭臭泥"}, {38,0,0,90,"刺甲贝"}, {39,25,0,92,"鬼斯通"}, {39,0,1,93,"耿鬼"},
 {41,26,0,96,"引梦貘人"}, {42,28,0,98,"巨钳蟹"}, {43,30,0,100,"顽皮雷弹"}, {44,0,0,102,"椰蛋树"}, {45,28,0,104,"嘎啦嘎啦"}, {49,35,0,109,"双弹瓦斯"}, {50,40,0,111,"钻角犀兽"}, {54,32,0,116,"海刺龙"}, {55,33,0,118,"金鱼王"}, {56,0,0,120,"宝石海星"}, {64,20,0,129,"暴鲤龙"}, {69,40,0,138,"多刺菊石兽"}, {70,40,0,140,"镰刀盔"}, {76,30,0,147,"哈克龙"}, {76,55,1,148,"快龙"}};
unsigned pp_evolution_level(const pp_game *g, unsigned pet) {
  if (pet >= g->count || pp_evolution_locked(g, pet))
    return 0;
  for (unsigned i = 0; i < PP_EVOLUTIONS; i++) {
    const pp_evolution_template *t = &pp_evolutions[i];
    if (t->species == g->pets[pet].species &&
        t->from_stage == g->growth[pet].stage)
      return t->level;
  }
  return 0;
}
bool pp_evolution_locked(const pp_game *g, unsigned pet) {
  return pet < g->count && (g->evolution_declined & (1ULL << pet));
}
const char *pp_evolution_stone(const pp_game *g, unsigned pet) {
  if (pet >= g->count) return NULL;
  if(g->pets[pet].species==67 && !g->growth[pet].stage)return "进化石";
  for (unsigned i=0;i<PP_EVOLUTIONS;i++) {
    const pp_evolution_template *t=&pp_evolutions[i];
    if (t->species==g->pets[pet].species && t->from_stage==g->growth[pet].stage && !t->level)
      return t->species==0 ? "雷之石" : (t->species==14 || t->species==24) ? "火之石" : (t->species==17 || t->species==28 || t->species==44) ? "叶之石" : (t->species==25 || t->species==38 || t->species==56) ? "水之石" : (t->species==26 || t->species==27 || t->species==30 || t->species==39) ? "联系之绳" : "月之石";
  }
  return NULL;
}
/* Check the current form and activity before either irreversible choice. */
pp_result pp_evolution_offer(const pp_game *g, unsigned pet) {
  if (pet >= g->count || pp_needs_starter(g)) return PP_INVALID;
  if (pp_evolution_locked(g, pet)) return PP_EVOLUTION_LOCKED;
  bool stone = pp_evolution_stone(g, pet) != NULL;
  unsigned gate = pp_evolution_level(g, pet);
  if (!gate && !stone) return PP_INVALID;
  if (pp_training_busy(g,pet) || g->traveler == (int)pet || (g->battling && (g->gym.mode || g->battle_pet == pet)) ||
      (g->drop_active && g->drop_target == pet)) return PP_BUSY;
  if (gate && g->growth[pet].level < gate) return PP_NOT_READY;
  return PP_OK;
}
pp_result pp_decline_evolution(pp_game *g, unsigned pet) {
  pp_result result = pp_evolution_offer(g, pet);
  if (result != PP_OK) return result;
  /* One transaction: no coin cost, and repeated calls cannot grant again. */
  g->evolution_declined |= 1ULL << pet;
  g->collected[PP_PURE_HEART / 8] |= 1u << (PP_PURE_HEART % 8);
  log_line(g, "守护最初的自己，获得收藏品：纯净之心。");
  return PP_OK;
}
pp_result pp_evolve(pp_game *g, unsigned pet) {
  pp_result result = pp_evolution_offer(g, pet);
  if (result != PP_OK) return result;
  if (!pp_evolution_level(g, pet)) return PP_INVALID; /* Stone never auto-evolves. */
  g->growth[pet].stage++;
  char s[96];
  snprintf(s, sizeof(s), "突破成功，进化为%s！", pp_form_name(g, pet));
  log_line(g, s);
  return PP_OK;
}
bool pp_roster_evolution_preview(const pp_game *g,unsigned pick,pp_game *out){
 pp_stored_pet p;if(pick<PP_PETS){if(pick>=g->count)return false;p=(pp_stored_pet){.pet=g->pets[pick],.growth=g->growth[pick],.identity=g->identity[pick],.declined=(g->evolution_declined>>pick)&1};}
 else if(!pp_warehouse_get(g,pick-PP_PETS,&p))return false;
 if(out==g)return false;
 memset(out,0,sizeof(*out));out->count=1;out->starter_chosen=1;out->traveler=-1;out->coins=g->coins;out->pets[0]=p.pet;out->growth[0]=p.growth;out->identity[0]=p.identity;out->evolution_declined=p.declined;memcpy(out->collected,g->collected,sizeof(out->collected));return true;
}
pp_result pp_roster_evolution_choose(pp_game *g,unsigned pick,bool decline,unsigned branch){
 if(pick<PP_PETS)return decline?pp_decline_evolution(g,pick):branch?pp_evolve_branch(g,pick,branch):pp_evolution_stone(g,pick)?pp_evolve_stone(g,pick):pp_evolve(g,pick);
 if(pp_needs_starter(g))return PP_INVALID;
 if(pp_training_busy(g,pick)||g->fishing_partner==pick-PP_PETS+1||(g->battling&&g->gym.mode))return PP_BUSY;
 /* Core calls are serialized by the game/display lock; keep the large scratch off the task stack. */
 pp_stored_pet p;static pp_game preview;if(!pp_warehouse_get(g,pick-PP_PETS,&p)||!pp_roster_evolution_preview(g,pick,&preview))return PP_INVALID;
 pp_result result=decline?pp_decline_evolution(&preview,0):branch?pp_evolve_branch(&preview,0,branch):pp_evolution_stone(&preview,0)?pp_evolve_stone(&preview,0):pp_evolve(&preview,0);
 if(result!=PP_OK)return result;
 p.pet=preview.pets[0];p.growth=preview.growth[0];p.declined=preview.evolution_declined&1;warehouse_set(g,pick-PP_PETS,&p);g->coins=preview.coins;memcpy(g->collected,preview.collected,sizeof(g->collected));memcpy(g->log,preview.log,sizeof(g->log));g->revision++;return PP_OK;
}
pp_result pp_evolve_thunder_stone(pp_game *g, unsigned pet) {
  if (pet >= g->count || g->pets[pet].species != 0 || g->growth[pet].stage) return PP_INVALID;
  return pp_evolve_stone(g, pet);
}
const char *pp_evolution_branch_name(unsigned branch) {
  static const char *const names[]={"水之石","雷之石","火之石"};
  return branch>=1 && branch<=3?names[branch-1]:NULL;
}
pp_result pp_evolve_branch(pp_game *g, unsigned pet, unsigned branch) {
  if(pet>=g->count || g->pets[pet].species!=67 || !pp_evolution_branch_name(branch))return PP_INVALID;
  pp_result result=pp_evolution_offer(g,pet);
  if(result!=PP_OK)return result;
  if(g->growth[pet].stage)return PP_INVALID;
  if(g->coins<200)return PP_NO_COINS;
  g->coins-=200;g->growth[pet].stage=branch;
  char s[96];snprintf(s,sizeof(s),"使用%s，进化为%s！",pp_evolution_branch_name(branch),pp_form_name(g,pet));log_line(g,s);
  return PP_OK;
}
pp_result pp_evolve_stone(pp_game *g, unsigned pet) {
  if(pet<g->count && g->pets[pet].species==67)return PP_INVALID;
  const char *stone=pp_evolution_stone(g, pet);
  if (!stone) return PP_INVALID;
  pp_result result = pp_evolution_offer(g, pet);
  if (result != PP_OK) return result;
  unsigned species=g->pets[pet].species;
  unsigned price=species==0 ? PP_THUNDER_STONE_PRICE : (species==14 || species==24) ? PP_FIRE_STONE_PRICE : (species==17 || species==28 || species==44) ? PP_LEAF_STONE_PRICE : (species==25 || species==38 || species==56) ? PP_WATER_STONE_PRICE : (species==26 || species==27 || species==30 || species==39) ? PP_LINK_CORD_PRICE : PP_MOON_STONE_PRICE;
  if (g->coins < price) return PP_NO_COINS;
  g->coins -= price;
  g->growth[pet].stage++;
  char s[96];
  snprintf(s,sizeof(s),"使用%s，进化为%s！",stone,pp_form_name(g,pet));
  log_line(g, s);
  return PP_OK;
}
const char *pp_form_name(const pp_game *g, unsigned pet) {
  if (pet >= g->count)
    return "";
  if(g->pets[pet].species==67) {
    static const char *const forms[]={"伊布","水伊布","雷伊布","火伊布"};
    return g->growth[pet].stage<=3?forms[g->growth[pet].stage]:"";
  }
  for (unsigned i=0;i<PP_EVOLUTIONS;i++)
    if (pp_evolutions[i].species==g->pets[pet].species &&
        pp_evolutions[i].from_stage+1==g->growth[pet].stage)
      return pp_evolutions[i].name;
  return pp_pet_names[g->pets[pet].species];
}
unsigned pp_art_id(const pp_game *g, unsigned pet) {
  if (pet >= g->count) return 0;
  if(g->pets[pet].species==67)return 132+g->growth[pet].stage;
  for (unsigned i=0;i<PP_EVOLUTIONS;i++)
    if (pp_evolutions[i].species==g->pets[pet].species &&
        pp_evolutions[i].from_stage+1==g->growth[pet].stage)
      return pp_evolutions[i].art;
  return pp_base_art(g->pets[pet].species);
}
bool pp_import_v1(pp_game *dest, const void *data, unsigned bytes,
                  uint32_t checksum) {
  if (bytes != sizeof(pp_game_v1))
    return false;
  const unsigned char *raw = data;
  uint32_t hash = 2166136261u;
  for (unsigned i = 0; i < bytes; i++)
    hash = (hash ^ raw[i]) * 16777619u;
  if (hash != checksum)
    return false;
  pp_game_v1 old;
  memcpy(&old, data, sizeof(old));
  if (old.version != 1)
    return false;
  pp_game candidate;
  memset(&candidate, 0, sizeof(candidate));
  memcpy(&candidate, &old, offsetof(pp_game, growth));
  candidate.version = PP_VERSION;
  candidate.digging = 0;
  candidate.collected[PP_PURE_HEART / 8] &= ~(1u << (PP_PURE_HEART % 8));
  if (candidate.egg_end>candidate.seconds && candidate.egg_end-candidate.seconds>PP_EGG_MAX_SECONDS)
    candidate.egg_end=candidate.seconds+PP_EGG_MAX_SECONDS;
  for (unsigned i = 0; i < PP_PETS; i++)
    candidate.growth[i].level = 1;
  cap_legacy_levels(&candidate);
  if(!import_individuals(&candidate))return false;
  if (!pp_valid(&candidate))
    return false;
  *dest = candidate;
  return true;
}

/* Automatic hundred-step milestones, paid once as part of the same save. */
static uint32_t sat_add(uint32_t a, uint64_t b) {
  return b > UINT32_MAX-a ? UINT32_MAX : a+(uint32_t)b;
}
unsigned pp_step_coin_rate(const pp_game *g){return g->steps_today>=10000?0:g->steps_today>=5000?10:30;}
unsigned pp_step_xp_rate(const pp_game *g){return g->steps_today>=10000?0:g->steps_today>=5000?2:6;}
void pp_record_steps(pp_game *g, unsigned steps) {
  if (!steps || pp_needs_starter(g)) return;
  unsigned room=UINT32_MAX-g->steps_today;
  if (steps>room) steps=room;
  if (!steps) return;
  if (UINT64_MAX-g->total_steps<steps) return;
  g->total_steps+=steps;
  g->steps_today+=steps;
  unsigned available=steps;while(g->scene<12&&g->wild_queue<3&&available){if(!g->wild_steps_left)wild_cooldown(g);unsigned take=available<g->wild_steps_left?available:g->wild_steps_left;available-=take;g->wild_steps_left-=take;if(!g->wild_steps_left)wild_charge(g);}
  unsigned chunks=g->steps_today/100;uint64_t coins=0,xp=0;
  for(unsigned k=g->steps_rewarded+1;k<=chunks&&k<=100;k++)if(k%10==0){
   coins+=k<=50?30:10;if(pp_tarot_active(g,7))coins+=2;
   for(unsigned i=0;i<g->count;i++){unsigned gain=pp_xp_needed(g->growth[i].level)*(k<=50?6:2)/100;if(k<=50&&gain<20)gain=20;if(pp_tarot_active(g,1))gain+=gain/10;pp_gain_xp(g,i,gain);if(!i)xp+=gain;}
  }
  g->coins=sat_add(g->coins,coins);g->steps_coins=sat_add(g->steps_coins,coins);g->steps_xp=sat_add(g->steps_xp,xp);g->steps_rewarded=chunks;
  g->revision++;
}
bool pp_import_v2(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,total_steps)==sizeof(pp_game_v2),"Frozen v2 prefix changed");
  if (bytes!=sizeof(pp_game_v2)) return false;
  const uint8_t *raw=data;
  uint32_t hash=2166136261u;
  for (unsigned i=0;i<bytes;i++) hash=(hash^raw[i])*16777619u;
  if (hash!=checksum) return false;
  pp_game_v2 old; memcpy(&old,data,sizeof(old));
  if (old.version!=2) return false;
  pp_game candidate={0}; memcpy(&candidate,&old,sizeof(old));
  candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  candidate.digging=0;
  candidate.collected[PP_PURE_HEART / 8] &= ~(1u << (PP_PURE_HEART % 8));
  if (candidate.egg_end>candidate.seconds && candidate.egg_end-candidate.seconds>PP_EGG_MAX_SECONDS)
    candidate.egg_end=candidate.seconds+PP_EGG_MAX_SECONDS;
  cap_legacy_levels(&candidate);
  if(!import_individuals(&candidate))return false;
  if (!pp_valid(&candidate)) return false;
  *dest=candidate; return true;
}

bool pp_import_v3(pp_game *dest, const void *data, unsigned bytes, uint32_t checksum) {
  _Static_assert(offsetof(pp_game, evolution_declined) == sizeof(pp_game_v3), "Frozen v3 prefix changed");
  if (bytes != sizeof(pp_game_v3)) return false;
  const uint8_t *raw = data;
  uint32_t hash = 2166136261u;
  for (unsigned i = 0; i < bytes; i++) hash = (hash ^ raw[i]) * 16777619u;
  if (hash != checksum) return false;
  pp_game_v3 old; memcpy(&old, data, sizeof(old));
  if (old.version != 3) return false;
  pp_game candidate = {0}; memcpy(&candidate, &old, sizeof(old));
  candidate.version = PP_VERSION;
  candidate.digging = 0;
  candidate.collected[PP_PURE_HEART / 8] &= ~(1u << (PP_PURE_HEART % 8));
  cap_legacy_levels(&candidate);
  if(!import_individuals(&candidate))return false;
  if (!pp_valid(&candidate)) return false;
  *dest = candidate;
  return true;
}

bool pp_import_v4(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,reserves)==sizeof(pp_game_v4),"Frozen v4 prefix changed");
  if(bytes!=sizeof(pp_game_v4)) return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++) hash=(hash^raw[i])*16777619u;
  if(hash!=checksum) return false;
  pp_game_v4 old;memcpy(&old,data,sizeof(old));if(old.version!=4)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  /* Dig is retired; preserve the old blob without continuing its activity. */
  candidate.digging=0;
  cap_legacy_levels(&candidate);
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  *dest=candidate;return true;
}

bool pp_import_v5(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,gym)==sizeof(pp_game_v5),"Frozen v5 prefix changed");
  if(bytes!=sizeof(pp_game_v5))return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++)hash=(hash^raw[i])*16777619u;
  if(hash!=checksum)return false;
  pp_game_v5 old;memcpy(&old,data,sizeof(old));if(old.version!=5)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  cap_legacy_levels(&candidate);
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  *dest=candidate;return true;
}

bool pp_import_v6(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,hatch_boosts)==sizeof(pp_game_v6),"Frozen v6 prefix changed");
  if(bytes!=sizeof(pp_game_v6))return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++)hash=(hash^raw[i])*16777619u;
  if(hash!=checksum)return false;
  pp_game_v6 old;memcpy(&old,data,sizeof(old));if(old.version!=6)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  cap_legacy_levels(&candidate);
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  *dest=candidate;return true;
}

/* Preserve the complete v7 prefix, including eggs, badge and battle state. */
bool pp_import_v7(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,hatch_boosts)==sizeof(pp_game_v7),"Frozen v7 prefix changed");
  if(bytes!=sizeof(pp_game_v7))return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++)hash=(hash^raw[i])*16777619u;
  if(hash!=checksum)return false;
  pp_game_v7 old;memcpy(&old,data,sizeof(old));if(old.version!=7)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  *dest=candidate;return true;
}

/* v8 is frozen independently of the new wild state. */
bool pp_import_v8(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,wild)==sizeof(pp_game_v8),"Frozen v8 prefix changed");
  if(!data || bytes!=sizeof(pp_game_v8))return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++)hash=(hash^raw[i])*16777619u;
  if(hash!=checksum)return false;
  pp_game_v8 old;memcpy(&old,data,sizeof(old));if(old.version!=8)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  *dest=candidate;return true;
}

/* v9 prefix, including wild encounters and all 151 existing pets, stays frozen. */
bool pp_import_v9(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,legend)==sizeof(pp_game_v9),"Frozen v9 prefix changed");
  if(!data || bytes!=sizeof(pp_game_v9))return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++)hash=(hash^raw[i])*16777619u;
  if(hash!=checksum)return false;
  pp_game_v9 old;memcpy(&old,data,sizeof(old));if(old.version!=9)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  if(candidate.wild.phase && candidate.wild.phase<PP_WILD_WON && pp_is_legendary(pp_pokedex[candidate.wild.dex].national_id)) {
    memset(&candidate.wild,0,sizeof(candidate.wild));candidate.battling=0;candidate.wild.next_at=candidate.seconds+PP_WILD_INTERVAL_MIN;
  }
  *dest=candidate;return true;
}

/* v10 contains every existing pet, legendary egg and battle unchanged. */
bool pp_import_v10(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum) {
  _Static_assert(offsetof(pp_game,gift)==sizeof(pp_game_v10),"Frozen v10 prefix changed");
  if(!data || bytes!=sizeof(pp_game_v10))return false;
  const uint8_t *raw=data;uint32_t hash=2166136261u;
  for(unsigned i=0;i<bytes;i++)hash=(hash^raw[i])*16777619u;
  if(hash!=checksum)return false;
  pp_game_v10 old;memcpy(&old,data,sizeof(old));if(old.version!=10)return false;
  pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
  if(!import_individuals(&candidate))return false;
  if(!pp_valid(&candidate))return false;
  *dest=candidate;return true;
}

bool pp_import_v11(pp_game *dest,const void *data,unsigned bytes,uint32_t checksum){
 _Static_assert(offsetof(pp_game,identity)==sizeof(pp_game_v11),"Frozen v11 prefix changed");
 if(!data||bytes!=sizeof(pp_game_v11)||hash_bytes(data,bytes)!=checksum)return false;
 pp_game_v11 old;memcpy(&old,data,sizeof(old));if(old.version!=11)return false;
 pp_game candidate={0};memcpy(&candidate,&old,sizeof(old));candidate.version=PP_VERSION;
 if(candidate.traveler>=0&&candidate.traveler<(int)candidate.count)candidate.travel_xp_snapshot=candidate.growth[candidate.traveler].level*4;
 if(!import_individuals(&candidate))return false;
 if(!pp_valid(&candidate))return false;
 *dest=candidate;return true;
}
