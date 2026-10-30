// Native120 RGB565 animation, matching core/fishing_scenery.h.
export function pondWater(x,y){if(y<46||y>112)return false;return x>=(y<55?50:y<72?49:y<93?31:22)&&x<=(y<55?94:y<72?107:114)}
const safe=(x,y)=>(x*2-119)**2+(y*2-119)**2<114*114;
const leaf=c=>(c>>5&63)>(c>>11)+8&&(c>>5&63)>(c&31)*2+6;
function mix(c,ink,a){return Math.floor(((c>>11)*(255-a)+(ink>>11)*a)/255)<<11|Math.floor(((c>>5&63)*(255-a)+(ink>>5&63)*a)/255)<<5|Math.floor(((c&31)*(255-a)+(ink&31)*a)/255)}
function dot(out,x,y,c,a,water){if(x<1||x>118||y<1||y>118||!safe(x,y)||(water&&!pondWater(x,y)))return;const n=y*120+x;out[n]=mix(out[n],c,a)}
export function renderFishingScenery(t,base,out){
 t=t>>>0;out.set(base);
 for(let y=2;y<54;y++)for(let x=2;x<118;x++){if(!safe(x,y)||(x>=46&&x<=83&&y>=28))continue;const p=(t+(x<60?0:1700)+y*21)>>>0,phase=p%6400,dx=phase<800?0:phase<2800?1:phase<4000?0:phase<6000?-1:0,n=y*120+x;if(dx&&leaf(base[n])&&leaf(base[n-1])&&leaf(base[n+1]))out[n]=base[n-dx]}
 const centers=[[73,54],[89,70],[95,90]];
 for(let i=0;i<3;i++){const period=3600+i*800,age=((t+i*1300)>>>0)%period,rx=2+Math.floor(age*9/period),ry=1+Math.floor(rx/3),a=125-Math.floor(age*110/period);for(let y=-ry;y<=ry;y++)for(let x=-rx;x<=rx;x++)if(Math.abs(x*x*ry*ry+y*y*rx*rx-rx*rx*ry*ry)<=Math.floor(rx*rx/2))dot(out,centers[i][0]+x,centers[i][1]+y,0x8e99,a,true)}
 for(let i=0;i<3;i++){const age=((t+i*2700)>>>0)%9000,a=age<1500?Math.floor(age*95/1500):age<4500?95:age<6500?Math.floor((6500-age)*95/2000):0,x=i===1?96-Math.floor(age*24/9000):i===2?100-Math.floor(age*13/9000):54+Math.floor(age*32/9000),y=i===2?90:61+i*17;for(let dy=-1;dy<=1;dy++)for(let dx=-3;dx<=3;dx++)if(!dy||(dx>=-1&&dx<=1))dot(out,x+dx,y+dy,0x09e8,a,true);const tail=i?4:-4;dot(out,x+tail,y-1,0x09e8,a,true);dot(out,x+tail,y+1,0x09e8,a,true)}
 for(let i=0;i<5;i++){const period=7600+i*600,age=((t+i*1900)>>>0)%period;let bend=Math.floor(age*12/period)%6;if(bend>3)bend=6-bend;const dx=Math.floor(age*23/period),x=i%2?102-dx+bend:15+dx+bend,y=25+Math.floor(age*76/period),a=age<600?Math.floor(age*220/600):age>period-1800?Math.floor((period-age)*220/1800):220,color=i%2?0xbde9:0x9d4b;dot(out,x,y,color,a,false);dot(out,x+1,y,color,a,false);dot(out,x+(i%2?1:0),y+1,color,a,false);dot(out,x-1,y+1,0x6b66,a,false)}
 return out;
}
