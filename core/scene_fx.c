#include "scene_fx.h"
#include "scene_fx_config.h"
unsigned pp_scene_fx_capacity(unsigned scene) {
  return scene<PP_SCENE_RECIPE_COUNT?pp_scene_fx_configs[scene].count:0;
}
unsigned pp_scene_fx_part_limit(unsigned scene) {
  return scene<PP_SCENE_RECIPE_COUNT && (scene==6 || pp_scene_fx_configs[scene].kind==8 || pp_scene_fx_configs[scene].kind==9 || pp_scene_fx_configs[scene].kind==10 || pp_scene_fx_configs[scene].kind==11)?7:3;
}
static uint32_t mix(uint32_t x) {
  x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;x^=x>>16;return x;
}
static bool overlaps(const pp_scene_particle *p,int x,int y,int w,int h) {
  return p->x<x+w && p->x+p->w>x && p->y<y+h && p->y+p->h>y;
}
bool pp_scene_fx_safe(const pp_scene_particle *p) {
  if(p->w<1 || p->h<1)return false;
  /* Protect the actual HUD and navigation, rather than an entire horizontal band. */
  if(!p->full_screen && (overlaps(p,80,22,200,90) || overlaps(p,98,258,164,70)))return false;
  int radius=p->full_screen?176:172;
  for(unsigned corner=0;corner<4;corner++) {
    int dx=p->x+(corner&1?p->w:0)-180,dy=p->y+(corner&2?p->h:0)-180;
    if(dx*dx+dy*dy>radius*radius)return false;
  }
  return true;
}
unsigned pp_scene_fx_parts(const pp_scene_particle *p,pp_scene_rect out[PP_SCENE_FX_PARTS]) {
  int w=p->w,h=p->h;unsigned a=p->opacity,n=1;
  out[0]=(pp_scene_rect){0,0,w,h,a};
  switch(p->shape) {
    case PP_FX_TUMBLE_0: case PP_FX_TUMBLE_1: case PP_FX_TUMBLE_2: case PP_FX_TUMBLE_3:
    case PP_FX_TUMBLE_4: case PP_FX_TUMBLE_5: case PP_FX_TUMBLE_6: case PP_FX_TUMBLE_7:
      return 0; /* Native preloaded bitmap sprites, never rectangle substitutes. */
    case PP_FX_BAT_UP: case PP_FX_BAT_FLAT: case PP_FX_BAT_DOWN: {
      /* Connected13x9 pixel silhouette; three wing poses, mirrored about body. */
      int mid_y=p->shape==PP_FX_BAT_UP?1:p->shape==PP_FX_BAT_FLAT?3:4;
      int tip_y=p->shape==PP_FX_BAT_UP?0:p->shape==PP_FX_BAT_FLAT?2:6;
      int mid_h=p->shape==PP_FX_BAT_FLAT?2:3;
      int blocks[7][4]={{6,2,1,5},{3,3,3,2},{1,mid_y,3,mid_h},
        {0,tip_y,2,2},{7,3,3,2},{9,mid_y,3,mid_h},{11,tip_y,2,2}};
      n=7;
      for(unsigned j=0;j<n;j++) {
        int x=blocks[j][0]*w/13,y=blocks[j][1]*h/9;
        int rw=(blocks[j][0]+blocks[j][2])*w/13-x;
        int rh=(blocks[j][1]+blocks[j][3])*h/9-y;
        out[j]=(pp_scene_rect){x,y,rw>0?rw:1,rh>0?rh:1,j?a*9/10:a};
      }
      break;
    }
    case PP_FX_STAR:n=2;out[0]=(pp_scene_rect){0,h/2,w,w>=7?2:1,a};out[1]=(pp_scene_rect){w/2,0,w>=7?2:1,h,a};break;
    case PP_FX_LEAF:n=2;out[0]=(pp_scene_rect){0,h/2,w-2,h-h/2,a};out[1]=(pp_scene_rect){2,0,w-2,h/2+1,a*4/5};break;
    case PP_FX_RIPPLE:n=3;out[0]=(pp_scene_rect){2,0,w-4,1,a};out[1]=(pp_scene_rect){0,1,2,h-1,a*2/3};out[2]=(pp_scene_rect){w-2,1,2,h-1,a*2/3};break;
    case PP_FX_LAKE_RING:
      /* A hollow, horizontally foreshortened ring; no translucent filled disc. */
      n=6;
      out[0]=(pp_scene_rect){3,0,w-6,1,a};out[1]=(pp_scene_rect){3,h-1,w-6,1,a};
      out[2]=(pp_scene_rect){1,1,2,h-2,a*2/3};out[3]=(pp_scene_rect){w-3,1,2,h-2,a*2/3};
      out[4]=(pp_scene_rect){0,2,1,h-4,a/2};out[5]=(pp_scene_rect){w-1,2,1,h-4,a/2};
      break;
    case PP_FX_METEOR:n=3;out[0]=(pp_scene_rect){w-3,h-2,3,2,a};out[1]=(pp_scene_rect){w/3,h/3,w/2,2,a*2/3};out[2]=(pp_scene_rect){0,0,w/2,1,a/3};break;
    case PP_FX_CLOUD: {
      /* Three scalloped puffs and a translucent base on one chunky20x10 grid. */
      static const int blocks[7][5]={{1,6,17,3,55},{0,5,5,3,70},{2,3,5,4,85},
        {7,0,6,2,85},{5,2,10,6,95},{13,3,5,5,80},{16,5,4,3,65}};
      n=7;
      for(unsigned j=0;j<n;j++) {
        const int *b=blocks[j];
        out[j]=(pp_scene_rect){b[0]*w/20,b[1]*h/10,b[2]*w/20,b[3]*h/10,a*b[4]/100};
      }
      break;
    }
    case PP_FX_PETAL:n=3;out[0]=(pp_scene_rect){1,0,w-2,h,a*3/4};out[1]=(pp_scene_rect){0,1,w,h-2,a};out[2]=(pp_scene_rect){w/2,h-1,1,1,a/2};break;
    case PP_FX_GLOW:n=3;out[0]=(pp_scene_rect){0,0,w,h,a/6};out[1]=(pp_scene_rect){2,2,w-4,h-4,a/2};out[2]=(pp_scene_rect){w/2,h/2,2,2,a};break;
    case PP_FX_MAPLE: {
      /* Seven connected pixel blocks: centre point, side lobes, taper and stem. */
      static const int blocks[7][4]={{4,0,1,3},{3,1,3,6},{0,2,2,3},{7,2,2,3},{1,4,7,3},{2,7,5,1},{4,8,1,3}};
      n=7;
      for(unsigned j=0;j<n;j++) {
        const int *b=blocks[j];int x=b[0]*w/9,y=b[1]*h/11;
        int right=(b[0]+b[2])*w/9,bottom=(b[1]+b[3])*h/11;
        out[j]=(pp_scene_rect){x,y,right>x?right-x:1,bottom>y?bottom-y:1,j==6?a*3/4:a};
      }
      break;
    }
    case PP_FX_COMET:
      n=7;
      for(unsigned j=0;j<6;j++)out[j]=(pp_scene_rect){(int)j*6,(int)j*2,8,j<2?2:3,a*(j+1)/7};
      out[6]=(pp_scene_rect){36,12,4,4,a};
      break;
    case PP_FX_RAIN:n=2;out[0]=(pp_scene_rect){1,0,w-1,h/2,a*2/3};out[1]=(pp_scene_rect){0,h/2,w-1,h-h/2,a};break;
    default:break;
  }
  for(unsigned i=0;i<n;i++){out[i].x+=p->x;out[i].y+=p->y;}
  return n;
}
static unsigned breathe(uint32_t age,unsigned period,unsigned offset) {
  unsigned t=(age%period+offset%period)%period;
  unsigned v=t<period/2?t*510/period:(period-t)*510/period;
  return v*v/255;
}
static unsigned edge_alpha(const pp_scene_particle *p) {
  int far=0;
  for(unsigned c=0;c<4;c++) {
    int x=p->x+(c&1?p->w:0)-180,y=p->y+(c&2?p->h:0)-180;
    int r=x*x+y*y;if(r>far)far=r;
  }
  if(p->full_screen)return far<170*170?255:(176*176-far)*255/(176*176-170*170);
  unsigned a=far<157*157?255:(172*172-far)*255/(172*172-157*157);
  const int holes[2][4]={{80,22,200,90},{98,258,164,70}};
  for(unsigned i=0;i<2;i++) {
    const int *h=holes[i];int dx=p->x>=h[0]+h[2]?p->x-h[0]-h[2]:h[0]-p->x-p->w;
    int dy=p->y>=h[1]+h[3]?p->y-h[1]-h[3]:h[1]-p->y-p->h;
    int gap=dx>dy?dx:dy;
    if(gap<14 && gap>=0 && (unsigned)gap*255/14<a)a=(unsigned)gap*255/14;
  }
  return a;
}
unsigned pp_scene_fx_sample(unsigned scene,uint32_t seed,uint32_t elapsed,bool preview,
                            pp_scene_particle out[PP_SCENE_FX_MAX]) {
  if(scene>=PP_SCENE_RECIPE_COUNT)return 0;
  const pp_scene_fx_config *c=&pp_scene_fx_configs[scene];
  uint32_t weather=mix(seed^(scene*2654435761u)^((elapsed/PP_SCENE_FX_PERIOD)*2246822519u));
  unsigned start=1200+weather%3000,phase=elapsed%PP_SCENE_FX_PERIOD;
  unsigned burst=0;
  if(preview)burst=255;
  else if(weather%100<c->chance && phase>=start && phase<start+c->duration) {
    unsigned event_age=phase-start,remaining=c->duration-event_age;
    burst=event_age<1200?event_age*255/1200:remaining<1200?remaining*255/1200:255;
  }
  /* All particle overlays share the event gate. Untriggered rounds stay quiet;
   * separate scenery materials continue moving without consuming game RNG. */
  if(!burst)return 0;
  unsigned age=2000u+elapsed;
  unsigned fade=elapsed<800?elapsed*255/800:255;
  unsigned used=0;
  for(unsigned i=0;i<c->count && i<PP_SCENE_FX_MAX;i++) {
    uint32_t r=mix(seed^(scene*3266489917u)^(i*668265263u));
    unsigned speed=c->speed*(75+r%60)/100;
    unsigned phase=((uint64_t)age*speed/1000+r%360)%360;
    unsigned density=burst;
    unsigned glow=breathe(age,3200+(r%5)*640,r);
    unsigned wave=((uint64_t)age+r%7000)%8000;
    int t=(int)(wave<4000?wave:8000-wave)/20-100;
    int sway=(6+(int)(r%9))*t*(30000-t*t)/2000000;
    int x=20+(i*313u/c->count+(r>>8)%17)%320;
    pp_scene_particle p={.x=x+sway,.y=(int)phase-8,.w=2,.h=2,.color=c->color,.opacity=130,.shape=PP_FX_DOT,.full_screen=scene!=5 && scene!=10};
    pp_scene_anchor anchor={x,180};
    if(pp_scene_anchor_counts[scene])anchor=pp_scene_anchors[scene][(r>>3)%pp_scene_anchor_counts[scene]];
    switch(c->kind) {
      case 0: case 10: {
        unsigned layer=i%3;
        phase=((uint64_t)age*(speed*(layer+3)/4)/1000+i*149+r%23)%408;
        p.x=(int)(((uint64_t)age*(4+r%6)/1000+i*73+(r>>8)%29+360+sway)%360);
        p.y=(int)phase-24;
        unsigned flutter=breathe(age,1600+r%4*400,r);
        p.w=layer==0?4:layer==1?6:8;p.h=3+(int)(flutter/85);
        p.shape=scene==1?PP_FX_PETAL:PP_FX_LEAF;
        p.opacity=layer==0?125:layer==1?185:225;
        if(scene==3 && i%3!=2){p.w=2+r%2;p.h=2;p.shape=PP_FX_DOT;p.opacity=135;}
        if(c->kind==10) {
          p.shape=PP_FX_MAPLE;p.w=layer==0?7:9;p.h=flutter<128?10:11;
          p.color=r%3==0?0xe66f41:r%3==1?0xf5b450:0xc74b37;
        }
        if(scene==1)p.color=r%3==0?0xffd8e5:r%3==1?0xefa6c5:0xf2bacd;
        break;
      }
      case 1:
        p.x=anchor.x-1;p.y=anchor.y-1;p.w=9+glow/85;p.h=3;
        p.shape=PP_FX_RIPPLE;p.opacity=60+glow*185/255;
        if(i%4==0){p.w=p.h=3+r%2*2;p.shape=PP_FX_STAR;}
        break;
      case 2: {
        if(i<8) {
          p.x=anchor.x-1;p.y=anchor.y-1;p.w=p.h=3+r%2*2;
          p.shape=PP_FX_STAR;p.opacity=65+glow*160/255;density=burst;
        } else {
          unsigned layer=i%3;
          phase=((uint64_t)age*(speed*(layer+2)/3)/1000+(i-8)*149+r%23)%408;
          p.y=(int)phase-24;
          p.x=(int)(((i-8)*73+(r>>8)%29+360+sway/(3-(int)layer))%360);
          p.full_screen=true;
          p.w=p.h=layer==2?7:layer==1?4:3;
          if(layer==2)p.shape=PP_FX_STAR;
          p.color=layer==0?0xb2cede:layer==1?0xe0edf5:0xf2f8fc;
          p.opacity=layer==0?145:layer==1?205:240;density=burst;
        }
        break;
      }
      case 3: {
        if(scene==6 && i>=36) {
          unsigned slot=i-36,bat_age=preview?elapsed:elapsed%PP_SCENE_FX_PERIOD-start;
          unsigned flight=((uint64_t)bat_age+14000-slot*280)%14000;
          if(flight>=6200)continue;
          unsigned wing=((bat_age+r%400)/100)%4;
          p.shape=wing==0?PP_FX_BAT_UP:wing==2?PP_FX_BAT_DOWN:PP_FX_BAT_FLAT;
          p.w=slot<4?17:23;p.h=slot<4?11:15;
          p.x=(int)(flight*420/6200)-32;
          if(mix(seed^0x614ba71u)&1)p.x=360-p.x-p.w;
          unsigned arc=flight<3100?flight:6200-flight;
          p.y=126+(int)(slot%3)*23+(int)(slot/3)*4-(int)(arc*24/3100)+sway/4;
          p.color=slot<4?0xa79acb:0x1b1834;p.opacity=slot<4?210:250;
          break;
        }
        p.x=anchor.x-1;p.y=anchor.y-1;p.w=p.h=5+2*(glow/100);
        p.shape=PP_FX_STAR;p.opacity=55+glow*200/255;
        p.color=i%3==0?0xdcc5ff:i%3==1?0xb9e5ff:0xa8f3ef;
        break;
      }
      case 4:
        phase=((uint64_t)age*(5+r%7)/1000+i*149+r%23)%408;
        p.x=(int)((i*73+(r>>8)%29+360+sway)%360);p.y=384-(int)phase;
        p.w=p.h=5+(i%3)*2;p.shape=PP_FX_GLOW;
        p.color=r%3?0xb9e8a6:0xe6ce8c;p.opacity=40+glow*210/255;
        break;
      case 5:
        p.x=anchor.x-1;p.y=anchor.y-1;p.w=p.h=r%3?3:5;
        p.shape=PP_FX_STAR;p.opacity=45+glow*200/255;
        if(i%8==0) {
          unsigned streak=(age+r%15000)%15000;if(streak>1800)continue;
          p.x=anchor.x-20+(int)streak/20;p.y=anchor.y-15+(int)streak/45;
          p.w=28;p.h=11;p.shape=PP_FX_METEOR;
          if(p.y+p.h>138)continue;
          unsigned life=streak<300?streak*255/300:(1800-streak)*255/1500;
          p.opacity=life*230/255;
        }
        break;
      case 6:
        if(scene==9 && i>=37) {
          unsigned slot=i-37;
          unsigned event_age=preview?elapsed:elapsed%PP_SCENE_FX_PERIOD-start;
          unsigned roll=((uint64_t)event_age+18000-slot*3200)%18000;
          if(roll>=8800)continue;
          unsigned reverse=mix(seed^0x9a511u)&1;
          unsigned turn=roll/180%8;if(reverse)turn=7-turn;
          p.w=p.h=slot==0?32:slot==1?40:36;
          p.x=(int)(roll*424/8800)-32;
          if(reverse)p.x=360-p.x-p.w;
          unsigned hop_phase=(roll+slot*211)%960;
          unsigned hop=(hop_phase<480?hop_phase:960-hop_phase)*5/480;
          p.y=228+(int)slot*10-p.h-(int)hop;
          p.shape=(pp_scene_shape)(PP_FX_TUMBLE_0+turn);
          p.color=0x866037;p.opacity=255;
          break;
        }
        p.x=anchor.x-10+sway;p.y=anchor.y-1+sway/4;
        p.w=i%4==0?10:2+r%3;p.h=i%4==0?1:2;
        p.opacity=50+glow*150/255;
        break;
      case 7:
        if(i<8) {p.x=anchor.x-1;p.y=anchor.y-1;p.w=7+glow/64;p.h=3;p.shape=PP_FX_RIPPLE;p.opacity=65+glow*155/255;density=burst;}
        else {phase=((uint64_t)age*speed/1000+(i-8)*149+r%23)%408;p.y=(int)phase-24;p.x=(int)(((i-8)*73+(r>>8)%29+360-phase/12)%360);p.full_screen=true;p.w=3;p.h=14+r%8;p.shape=PP_FX_RAIN;p.opacity=i%3==0?220:165;density=burst;}
        break;
      case 9: {
        if(i>=8) {
          p.x=anchor.x-1;p.y=anchor.y-1;p.w=p.h=r%3?3:5;
          p.shape=PP_FX_STAR;p.opacity=45+glow*200/255;density=burst;
          break;
        }
        unsigned streak=(age+r%10000)%10000;if(streak>=3200)continue;
        p.x=(int)(streak*420/3200)-48;
        p.y=10+(int)((r>>8)%30)+(int)(streak*78/3200);
        p.w=40;p.h=16;p.shape=PP_FX_COMET;
        p.color=i%2?0xc9e9ff:0xe8e4ff;
        unsigned life=streak<200?streak*255/200:streak>2800?(3200-streak)*255/400:255;
        p.opacity=(170+r%55)*life/255;
        break;
      }
      case 11: {
        if(i<28) {
          unsigned life=(age+r%5200)%5200;
          p.w=12+(int)(life*24/5200);p.h=6+(int)(life*6/5200);
          p.x=anchor.x-p.w/2;p.y=anchor.y-p.h/2;
          p.shape=PP_FX_LAKE_RING;p.opacity=(5200-life)*150/5200;
          density=burst;
        } else {
          unsigned life=(age+r%10000)%10000;
          int from_x=105+(int)((r>>10)%150),from_y=132+(int)((r>>12)%28);
          int to_x=(r&1?247:68)+(int)((r>>16)%35),to_y=206+(int)((r>>20)%48);
          if(life<6000) {
            p.x=from_x+(to_x-from_x)*(int)life/6000+sway/3;
            p.y=from_y+(to_y-from_y)*(int)life/6000;
            p.w=5+(int)(r%2)*2;p.h=4+(int)(glow/128);
            p.shape=PP_FX_LEAF;p.color=r&1?0x91ddff:0x67c9f2;
            p.opacity=life<400?life*225/400:225;
          } else if(life<8000) {
            unsigned landing=life-6000;
            p.w=12+(int)(landing*26/2000);p.h=6+(int)(landing*6/2000);
            p.x=to_x-p.w/2;p.y=to_y-p.h/2;
            p.shape=PP_FX_LAKE_RING;p.opacity=(2000-landing)*195/2000;
          } else continue;
          density=burst;
        }
        break;
      }
      case 8: {
        unsigned layer=i%3,drift=c->speed+layer*8+r%5;
        p.w=40+(int)layer*20;p.h=p.w/2;
        p.x=(int)(((uint64_t)age*drift/1000+i*71+(r>>8)%73)%480)-100;
        p.y=96+(int)((i*47+(r>>16)%45)%180)+sway/8;
        p.shape=PP_FX_CLOUD;p.color=layer==0?0xc4dfed:layer==1?0xe2eff5:0xf4f8f2;
        p.opacity=layer==0?115:layer==1?150:180;
        break;
      }
    }
    /* Airborne trajectories wrap beyond the display; material accents breathe
     * in place. UI is painted later, so no rectangular exclusion holes. */
    if(scene==8 && p.y+p.h>138)continue;
    if(pp_scene_fx_safe(&p)) {
      p.opacity=p.opacity*density/255*fade/255*edge_alpha(&p)/255;
      if(p.x>105 && p.x<250 && p.y>124 && p.y<244)p.opacity=p.opacity*7/8;
      if(p.opacity)out[used++]=p;
    }
  }
  return used;
}
