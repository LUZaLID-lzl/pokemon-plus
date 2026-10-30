// Shared choreography with core/intro.c. All coordinates are native 360px.
export const INTRO_MS=11200;
const ramp=(t,a,b)=>Math.max(0,Math.min(1,(t-a)/(b-a)));
const mix=(a,b,t)=>Math.round(a+(b-a)*t);
export function sampleIntro(t){
 const startX=[26,282,250],startY=[155,143,245],meetX=[95,177,235],meetY=[166,166,190],starts=[650,1800,2950],ends=[2350,3450,4500];
 const pets=starts.map((start,i)=>{
  const entry=ramp(t,start,ends[i]),line=ramp(t,6850,7850);
  const p={x:mix(mix(startX[i],meetX[i],entry),84+i*72,line),y:mix(mix(startY[i],meetY[i],entry),172,line),opacity:255*ramp(t,start,start+300),left:i>0&&t<6850,pose:t<ends[i]||(t>=6850&&t<7850)?4:0};
  let since=t<6850?start:6850;
  if(t>=7850+i*240&&t<8900+i*240){p.pose=8;since=7850+i*240}
  if(i===2&&t>=4500&&t<5200){p.pose=20;since=4500}
  if(i===1&&t>=5300&&t<6000){p.pose=8;since=5300}
  if(i===0&&t>=6000&&t<6700){p.pose=8;since=6000}
  p.elapsed=Math.max(0,t-since);
  if(i===1&&t>=5200&&t<5650)p.y-=Math.trunc(9*Math.sin((t-5200)*Math.PI/450));
  return p;
 });
 const particles=Array.from({length:18},(_,i)=>{
  const group=Math.floor(i/6),n=i%6,begin=4500+group*700+n*70;
  if(t<begin||t>=begin+1150)return {opacity:0};
  const u=(t-begin)/1150;
  return {type:group===0?2:group===1?1:0,opacity:205*Math.sin(u*Math.PI),size:group===0?5+(n%3)*2:3+(n%2)*2,x:mix(group===0?246:group===1?200:116,group===0?179:group===1?147:207,u)+Math.trunc(4*Math.sin(u*6+n)),y:mix(group===0?190:group===1?169:168,118+(n%3)*9,u)-Math.trunc(10*Math.sin(u*Math.PI))};
 });
 return {pets,particles,fade:255*(1-ramp(t,0,650)+ramp(t,10600,11200)),title:255*ramp(t,350,1000),caption:255*ramp(t,8100,8650),icons:255*ramp(t,7800,8350),finished:t>=INTRO_MS};
}
