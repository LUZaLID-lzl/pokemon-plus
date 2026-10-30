// Mirrors core/scene_layout.h. No game or storage mutation.
export function lakePatrol(elapsed){
 const points=[[40,145],[76,116],[138,116],[200,116],[264,145],[272,178],[272,214],[220,214],[156,214],[88,214],[40,214],[40,178]],phase=(elapsed>>>0)%24000,a=Math.floor(phase/2000),b=(a+1)%12,t=phase%2000;
 return {x:points[a][0]+Math.trunc((points[b][0]-points[a][0])*t/2000),y:points[a][1]+Math.trunc((points[b][1]-points[a][1])*t/2000),left:points[b][0]<points[a][0]};
}
export function lakeApproach(fromX,fromY,toX,toY,elapsed){
 elapsed=Math.min(2400,elapsed);if(fromX+24>=80&&fromX+24<=280){const shore=fromX<156?40:272;if(elapsed<800)return {x:fromX+Math.trunc((shore-fromX)*elapsed/800),y:fromY};fromX=shore;elapsed=Math.trunc((elapsed-800)*2400/1600)}if((fromX<156)===(toX<156))return {x:fromX+Math.trunc((toX-fromX)*elapsed/2400),y:fromY+Math.trunc((toY-fromY)*elapsed/2400)};
 const segment=elapsed===2400?2:Math.floor(elapsed/800),t=elapsed===2400?800:elapsed%800,ax=segment<2?fromX:toX,ay=segment?214:fromY,bx=segment?toX:fromX,by=segment<2?214:toY;
 return {x:ax+Math.trunc((bx-ax)*t/800),y:ay+Math.trunc((by-ay)*t/800)};
}
export function lakeInFront(y){return y+42>=254}
export function lakeDrop(spot,egg=false){return {x:[44,60,268,284][spot%4],y:(egg?144:140)+Math.floor(spot/4)*(egg?12:16)}}

export function lakeNearestPhase(x,y){let best=0,distance=1e6;for(let phase=0;phase<24000;phase+=120){const p=lakePatrol(phase),d=(p.x-x)**2+(p.y-y)**2;if(d<distance){distance=d;best=phase}}return best}
