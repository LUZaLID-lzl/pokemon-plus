#pragma once
#include "scene_nests.h"
#include <stdbool.h>
#include <stdlib.h>

/* Ground footprint expanded by a 48px actor's feet, in sprite top-left
 * coordinates. The body may overlap scenery only with correct depth order. */
#define PP_HOME_ROUTE_POINTS 32
static inline void pp_home_nest_bounds(unsigned scene,int b[4]) {
 if(scene>=PP_SCENE_NEST_COUNT)scene=0;
 int x=pp_scene_nest_positions[scene][0],y=pp_scene_nest_positions[scene][1];
 b[0]=x-34;b[1]=y-28;b[2]=x+58;b[3]=y+4;
}
static inline bool pp_home_nest_blocked(unsigned scene,int x,int y) {
 int b[4];pp_home_nest_bounds(scene,b);
 return x>=b[0]&&x<=b[2]&&y>=b[1]&&y<=b[3];
}
static inline bool pp_home_ground_valid(unsigned scene,int x,int y) {
 return x>=(scene==12?40:48)&&x<=272&&y>=115&&y<=(scene==12?214:208);
}
static inline void pp_home_ground_project(unsigned scene,int *x,int *y) {
 int low=scene==12?40:48,high=scene==12?214:208;
 *x=*x<low?low:*x>272?272:*x;*y=*y<115?115:*y>high?high:*y;
 if(!pp_home_nest_blocked(scene,*x,*y))return;
 int b[4];pp_home_nest_bounds(scene,b);
 int candidates[4][2]={{*x,b[1]-1},{*x,b[3]+1},{b[0]-1,*y},{b[2]+1,*y}};
 int best=1000000,bx=*x,by=*y;
 for(unsigned i=0;i<4;i++)if(pp_home_ground_valid(scene,candidates[i][0],candidates[i][1])) {
  int d=abs(candidates[i][0]-*x)+abs(candidates[i][1]-*y);
  if(d<best){best=d;bx=candidates[i][0];by=candidates[i][1];}
 }
 *x=bx;*y=by;
}
static inline int pp_home_cross(int ax,int ay,int bx,int by,int cx,int cy) {
 return (bx-ax)*(cy-ay)-(by-ay)*(cx-ax);
}
static inline bool pp_home_edges_cross(int ax,int ay,int bx,int by,int cx,int cy,int dx,int dy) {
 if((ax>bx?ax:bx)<(cx<dx?cx:dx)||(cx>dx?cx:dx)<(ax<bx?ax:bx)||
    (ay>by?ay:by)<(cy<dy?cy:dy)||(cy>dy?cy:dy)<(ay<by?ay:by))return false;
 int a=pp_home_cross(ax,ay,bx,by,cx,cy),b=pp_home_cross(ax,ay,bx,by,dx,dy);
 int c=pp_home_cross(cx,cy,dx,dy,ax,ay),d=pp_home_cross(cx,cy,dx,dy,bx,by);
 return ((a<=0&&b>=0)||(a>=0&&b<=0))&&((c<=0&&d>=0)||(c>=0&&d<=0));
}
static inline bool pp_home_segment_clear(unsigned scene,int x,int y,int tx,int ty) {
 int b[4];pp_home_nest_bounds(scene,b);
 if(pp_home_nest_blocked(scene,x,y)||pp_home_nest_blocked(scene,tx,ty))return false;
 return !pp_home_edges_cross(x,y,tx,ty,b[0],b[1],b[2],b[1])&&
        !pp_home_edges_cross(x,y,tx,ty,b[2],b[1],b[2],b[3])&&
        !pp_home_edges_cross(x,y,tx,ty,b[2],b[3],b[0],b[3])&&
        !pp_home_edges_cross(x,y,tx,ty,b[0],b[3],b[0],b[1]);
}
/* Six-node visibility graph, bounded stack storage; no allocation or RNG. */
static inline unsigned pp_home_nest_route(unsigned scene,int x,int y,int tx,int ty,int points[6][2]) {
 pp_home_ground_project(scene,&x,&y);pp_home_ground_project(scene,&tx,&ty);
 if(pp_home_segment_clear(scene,x,y,tx,ty)){
  points[0][0]=x;points[0][1]=y;points[1][0]=tx;points[1][1]=ty;return 2;
 }
 int b[4];pp_home_nest_bounds(scene,b);
 int nodes[6][2]={{x,y},{tx,ty},{b[0]-2,b[1]-2},{b[2]+2,b[1]-2},{b[2]+2,b[3]+2},{b[0]-2,b[3]+2}};
 /* Edge nests can leave only a one-pixel corridor along the lawn boundary. */
 for(unsigned i=2;i<6;i++){
  int low=scene==12?40:48,high=scene==12?214:208;
  nodes[i][0]=nodes[i][0]<low?low:nodes[i][0]>272?272:nodes[i][0];
  nodes[i][1]=nodes[i][1]<115?115:nodes[i][1]>high?high:nodes[i][1];
 }
 int dist[6]={0,1000000,1000000,1000000,1000000,1000000},previous[6]={-1,-1,-1,-1,-1,-1};
 bool used[6]={false};
 for(unsigned step=0;step<6;step++) {
  int a=-1;
  for(int i=0;i<6;i++)if(!used[i]&&(a<0||dist[i]<dist[a]))a=i;
  if(a<0||dist[a]==1000000)break;
  if(a==1)break;
  used[a]=true;
  for(int j=0;j<6;j++)if(!used[j]&&pp_home_ground_valid(scene,nodes[j][0],nodes[j][1])&&
     pp_home_segment_clear(scene,nodes[a][0],nodes[a][1],nodes[j][0],nodes[j][1])) {
   int cost=dist[a]+abs(nodes[a][0]-nodes[j][0])+abs(nodes[a][1]-nodes[j][1]);
   if(cost<dist[j]){dist[j]=cost;previous[j]=a;}
  }
 }
 int reverse[6];unsigned n=0;
 for(int a=1;a>=0&&n<6;a=previous[a])reverse[n++]=a;
 if(dist[1]==1000000){points[0][0]=x;points[0][1]=y;return 1;}
 for(unsigned i=0;i<n;i++){points[i][0]=nodes[reverse[n-1-i]][0];points[i][1]=nodes[reverse[n-1-i]][1];}
 return n;
}
static inline unsigned pp_home_route(unsigned scene,int x,int y,int tx,int ty,int points[PP_HOME_ROUTE_POINTS][2],unsigned *length) {
 pp_home_ground_project(scene,&x,&y);pp_home_ground_project(scene,&tx,&ty);
 int base[7][2]={{x,y}};unsigned n=1;
 if(scene==12&&!((y<=132&&ty<=132)||(y>=214&&ty>=214)||((x<=56&&tx<=56)||(x>=256&&tx>=256)))) {
  int shore=x<156?40:272,target=tx<156?40:272;
  base[n][0]=shore;base[n++][1]=y;base[n][0]=shore;base[n++][1]=214;
  base[n][0]=target;base[n++][1]=214;base[n][0]=target;base[n++][1]=ty;
 }
 base[n][0]=tx;base[n++][1]=ty;
 unsigned count=1;points[0][0]=x;points[0][1]=y;*length=0;
 for(unsigned i=1;i<n;i++) {
  int part[6][2];unsigned size=pp_home_nest_route(scene,base[i-1][0],base[i-1][1],base[i][0],base[i][1],part);
  for(unsigned j=1;j<size&&count<PP_HOME_ROUTE_POINTS;j++) {
   *length+=abs(part[j][0]-points[count-1][0])+abs(part[j][1]-points[count-1][1]);
   points[count][0]=part[j][0];points[count++][1]=part[j][1];
  }
 }
 return count;
}
static inline void pp_home_path_sample(unsigned scene,int points[PP_HOME_ROUTE_POINTS][2],unsigned n,unsigned length,unsigned elapsed,unsigned duration,int *ox,int *oy,bool *left) {
 unsigned traveled=(unsigned)((uint64_t)length*(elapsed>duration?duration:elapsed)/(duration?duration:1));
 for(unsigned i=1;i<n;i++) {
  int dx=points[i][0]-points[i-1][0],dy=points[i][1]-points[i-1][1];unsigned span=abs(dx)+abs(dy);
  if(dx)*left=dx<0;
  if(span&&traveled<span){
   *ox=points[i-1][0]+dx*(int)traveled/(int)span;*oy=points[i-1][1]+dy*(int)traveled/(int)span;
   /* Integer truncation can round an outside tangent one pixel inward. */
   pp_home_ground_project(scene,ox,oy);return;
  }
  traveled-=span;
 }
 *ox=points[n-1][0];*oy=points[n-1][1];
}
static inline void pp_home_route_sample(unsigned scene,int x,int y,int tx,int ty,unsigned elapsed,unsigned duration,int *ox,int *oy,bool *left) {
 int points[PP_HOME_ROUTE_POINTS][2];unsigned length,n=pp_home_route(scene,x,y,tx,ty,points,&length);
 pp_home_path_sample(scene,points,n,length,elapsed,duration,ox,oy,left);
}
/* Protect small depth/rejoin corrections as well as authored routes. */
static inline void pp_home_ground_advance(unsigned scene,int from_x,int from_y,int *x,int *y,unsigned step,bool *left) {
 if(pp_home_segment_clear(scene,from_x,from_y,*x,*y))return;
 int points[PP_HOME_ROUTE_POINTS][2];unsigned length;
 pp_home_route(scene,from_x,from_y,*x,*y,points,&length);
 pp_home_route_sample(scene,from_x,from_y,*x,*y,step,length,x,y,left);
}
