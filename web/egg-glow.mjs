/* Same bounded, draw-only dot field as core/egg_glow.h. Coordinates are
 * relative to the egg; the upper shell emits on a3px lattice. */
export function eggGlow(elapsed,legendary=false){
 const palette=legendary?[0xe5d2ff,0xc3eaff,0xf5eaff]:[0xffedbd,0xffdc83,0xfff5d9],dots=[];
 for(let i=0;i<18;i++){
  const age=((elapsed>>>0)%2400+i*137%2400)%2400;
  if(age>=1800)continue;
  const opacity=age<240?Math.floor(age*210/240):Math.floor((1800-age)*210/1560);
  if(!opacity)continue;
  const step=Math.floor(age/120),lane=i*7%9-4;
  dots.push({x:15+lane*3+Math.trunc(lane*step/10)*3,y:9-(step+i%3)*3,size:step<8?3:2,opacity,color:palette[i%3]});
 }
 return dots;
}
