import {nestAnchors} from './scene-nest-anchors.mjs?v=nestwalk2';
// Integer mirror of core/home_navigation.h. No state, allocation in firmware or RNG.
export const nestBounds=scene=>{const [x,y]=nestAnchors[scene]||nestAnchors[0];return [x-34,y-28,x+58,y+4]};
export const nestBlocked=(scene,x,y)=>{const [l,t,r,b]=nestBounds(scene);return x>=l&&x<=r&&y>=t&&y<=b};
const valid=(scene,x,y)=>x>=(scene===12?40:48)&&x<=272&&y>=115&&y<=(scene===12?214:208);
export function groundProject(scene,x,y){
 x=Math.max(scene===12?40:48,Math.min(272,x));y=Math.max(115,Math.min(scene===12?214:208,y));
 if(!nestBlocked(scene,x,y))return {x,y};
 const [l,t,r,b]=nestBounds(scene),candidates=[[x,t-1],[x,b+1],[l-1,y],[r+1,y]];let best=1e6,bx=x,by=y;
 for(const [cx,cy] of candidates)if(valid(scene,cx,cy)){const d=Math.abs(cx-x)+Math.abs(cy-y);if(d<best){best=d;bx=cx;by=cy}}
 return {x:bx,y:by};
}
const cross=(ax,ay,bx,by,cx,cy)=>(bx-ax)*(cy-ay)-(by-ay)*(cx-ax);
function edgesCross(ax,ay,bx,by,cx,cy,dx,dy){
 if(Math.max(ax,bx)<Math.min(cx,dx)||Math.max(cx,dx)<Math.min(ax,bx)||Math.max(ay,by)<Math.min(cy,dy)||Math.max(cy,dy)<Math.min(ay,by))return false;
 const a=cross(ax,ay,bx,by,cx,cy),b=cross(ax,ay,bx,by,dx,dy),c=cross(cx,cy,dx,dy,ax,ay),d=cross(cx,cy,dx,dy,bx,by);
 return (a<=0&&b>=0||a>=0&&b<=0)&&(c<=0&&d>=0||c>=0&&d<=0);
}
export function segmentClear(scene,x,y,tx,ty){
 const [l,t,r,b]=nestBounds(scene);
 return !nestBlocked(scene,x,y)&&!nestBlocked(scene,tx,ty)&&![[l,t,r,t],[r,t,r,b],[r,b,l,b],[l,b,l,t]].some(edge=>edgesCross(x,y,tx,ty,...edge));
}
function nestRoute(scene,x,y,tx,ty){
 ({x,y}=groundProject(scene,x,y));({x:tx,y:ty}=groundProject(scene,tx,ty));
 if(segmentClear(scene,x,y,tx,ty))return [[x,y],[tx,ty]];
 const [l,t,r,b]=nestBounds(scene),nodes=[[x,y],[tx,ty],[l-2,t-2],[r+2,t-2],[r+2,b+2],[l-2,b+2]],dist=[0,1e6,1e6,1e6,1e6,1e6],previous=Array(6).fill(-1),used=Array(6).fill(false);
 for(let i=2;i<6;i++)nodes[i]=[Math.max(scene===12?40:48,Math.min(272,nodes[i][0])),Math.max(115,Math.min(scene===12?214:208,nodes[i][1]))];
 for(let step=0;step<6;step++){
  let a=-1;for(let i=0;i<6;i++)if(!used[i]&&(a<0||dist[i]<dist[a]))a=i;
  if(a<0||dist[a]===1e6||a===1)break;used[a]=true;
  for(let j=0;j<6;j++)if(!used[j]&&valid(scene,...nodes[j])&&segmentClear(scene,...nodes[a],...nodes[j])){
   const cost=dist[a]+Math.abs(nodes[a][0]-nodes[j][0])+Math.abs(nodes[a][1]-nodes[j][1]);
   if(cost<dist[j]){dist[j]=cost;previous[j]=a}
  }
 }
 if(dist[1]===1e6)return [[x,y]];
 const reverse=[];for(let a=1;a>=0&&reverse.length<6;a=previous[a])reverse.push(nodes[a]);return reverse.reverse();
}
export function homeRoute(scene,x,y,tx,ty){
 ({x,y}=groundProject(scene,x,y));({x:tx,y:ty}=groundProject(scene,tx,ty));
 const base=[[x,y]],points=[[x,y]];
 if(scene===12&&!((y<=132&&ty<=132)||(y>=214&&ty>=214)||((x<=56&&tx<=56)||(x>=256&&tx>=256)))){
  const shore=x<156?40:272,target=tx<156?40:272;base.push([shore,y],[shore,214],[target,214],[target,ty]);
 }
 base.push([tx,ty]);for(let i=1;i<base.length;i++)points.push(...nestRoute(scene,...base[i-1],...base[i]).slice(1));return points;
}
export const routeLength=points=>points.slice(1).reduce((sum,p,i)=>sum+Math.abs(p[0]-points[i][0])+Math.abs(p[1]-points[i][1]),0);
export function routeSample(scene,x,y,tx,ty,elapsed,duration,left=false){
 return pathSample(scene,homeRoute(scene,x,y,tx,ty),elapsed,duration,left);
}
export function pathSample(scene,points,elapsed,duration,left=false){
 let traveled=Math.trunc(routeLength(points)*Math.min(elapsed,duration)/(duration||1));
 for(let i=1;i<points.length;i++){
  const dx=points[i][0]-points[i-1][0],dy=points[i][1]-points[i-1][1],span=Math.abs(dx)+Math.abs(dy);if(dx)left=dx<0;
  if(span&&traveled<span)return {...groundProject(scene,points[i-1][0]+Math.trunc(dx*traveled/span),points[i-1][1]+Math.trunc(dy*traveled/span)),left};traveled-=span;
 }
 const last=points.at(-1);return {x:last[0],y:last[1],left};
}
export function groundAdvance(scene,x,y,tx,ty,step,left){
 if(segmentClear(scene,x,y,tx,ty))return {x:tx,y:ty,left};
 return routeSample(scene,x,y,tx,ty,step,routeLength(homeRoute(scene,x,y,tx,ty)),left);
}
