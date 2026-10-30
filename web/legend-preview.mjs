// Native screenshot scenarios only. Never awards browser pets or writes storage.
export const legendIds = Object.freeze([144,145,146,150,151]);
export const isLegend = id => legendIds.includes(id);
export function createLegendPreview() {
 let unlocked=false, selected=0, phase='list';
 return {
  get unlocked(){return unlocked}, get selected(){return selected}, get phase(){return phase},
  toggle(){unlocked=!unlocked;phase='list'},
  select(index){if(!Number.isInteger(index)||index<0||index>=legendIds.length)throw Error('无效目标');selected=index;phase='detail'},
  show(next){
   if(!['list','cleared','detail','party','battle','won','lost','bag','egg','hatched'].includes(next))throw Error('无效预览');
   if(!unlocked && ['cleared','party','battle','won','lost','bag','egg','hatched'].includes(next))throw Error('先集齐八枚徽章');
   phase=next;
  },
  file(reduced=false){
   if(phase==='list')return `legend-${unlocked?'list':'locked'}.png`;
   if(phase==='bag')return 'legend-bag.png';
   if(phase==='cleared')return 'legend-cleared.png';
   return `legend-${selected}-${phase==='detail'&&!unlocked?'locked-detail':phase}.${phase==='battle'&&!reduced?'gif':'png'}`;
  }
 };
}
