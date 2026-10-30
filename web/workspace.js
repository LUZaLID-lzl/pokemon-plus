// Resource browsing is separate from the native-coordinate firmware preview.
export function createWorkspace({manifest,resources,base,revision,resourceCard,detail,showAnimation,onFilter,renderCollectionToolbar,getCollectionQuality,previewPet}) {
 const $=s=>document.querySelector(s);
 const overview=$('.overview'),library=$('.library'),grid=$('#resource-grid'),inspector=$('#pet-inspector'),search=$('#search');
 const names={all:'其他资源',pets:'宠物与技能',animations:'动作模板',scenes:'生活场景',travel:'旅行风景',items:'照料道具',collection:'收藏图鉴'};
 let workspace='preview',filter='all',pageIndex=0,selected=null,availability='all',mobileExpanded=false;
 const queries=new Map(),positions=new Map();
 const skills=manifest.skills;
 const typeNames={NORMAL:'一般',GRASS:'草',POISON:'毒',FIRE:'火',FLYING:'飞行',WATER:'水',BUG:'虫',ELECTRIC:'电',GROUND:'地面',FAIRY:'妖精',FIGHTING:'格斗',PSYCHIC:'超能',ROCK:'岩石',STEEL:'钢',ICE:'冰',GHOST:'幽灵',DRAGON:'龙',DARK:'恶'};
 const petTypes=pet=>(manifest.petTypes?.[pet.nationalId]||[]).map(t=>typeNames[t]||t);
 const pets=resources.filter(x=>x.type==='pets').map(x=>({...x,runtimeIntegrated:true}));
 // Catalog-only forms remain distinct from playable partners.
 const known=new Set(pets.map(x=>x.nationalId));
 for(const move of skills)move.nationalIds.forEach((dex,i)=>{
  if(known.has(dex))return;known.add(dex);
  pets.push({type:'pets',id:`catalog-${dex}`,nationalId:dex,name:move.familyNames[i],species:move.familyId,runtimeIntegrated:false,
   externalImage:true,image:`../assets/animation-templates/${String(dex).padStart(3,'0')}/idle-00.png`});
 });
 pets.sort((a,b)=>a.nationalId-b.nationalId);
 const movesFor=pet=>skills.filter(s=>s.nationalIds.includes(pet.nationalId)).sort((a,b)=>a.slot-b.slot);
 const url=x=>(x.externalImage?x.image:base+x.image)+`?v=${revision}`;
 const node=(tag,cls,text)=>{const e=document.createElement(tag);if(cls)e.className=cls;if(text!==undefined)e.textContent=text;return e};
 const button=(text,fn,cls='')=>{const b=node('button',cls,text);b.type='button';b.onclick=fn;return b};
 const image=(src,alt,cls)=>{const im=node('img',cls);im.src=src;im.alt=alt;im.loading='lazy';return im};
 function route(hash){if(location.hash!==hash)history.replaceState(null,'',hash)}
 function switchWorkspace(next,{updateRoute=true}={}){
  const changed=workspace!==next;workspace=next;document.body.dataset.workspace=next;if(changed)window.scrollTo({top:0,behavior:'instant'});
  overview.hidden=next!=='preview';library.hidden=next==='preview';
  $('#workspace-name').textContent=next==='preview'?'界面预览':names[next==='pets'?'pets':'all'];
  document.querySelectorAll('button[data-workspace="preview"]').forEach(b=>{b.classList.toggle('active',next==='preview');b.setAttribute('aria-pressed',String(next==='preview'));if(next==='preview')b.setAttribute('aria-current','page');else b.removeAttribute('aria-current')});
  document.querySelectorAll('aside [data-filter],.workspace-tabs [data-filter]').forEach(b=>{
   const active=next==='pets'?b.dataset.filter==='pets':next==='resources'&&b.dataset.filter==='all';
   b.classList.toggle('active',active);b.setAttribute('aria-pressed',String(active));if(active)b.setAttribute('aria-current','page');else b.removeAttribute('aria-current');
  });
  if(updateRoute)route(next==='preview'?'#preview':next==='pets'?'#pets':filter==='all'?'#resources':'#'+filter);
 }
 function setFilter(next){
  if(next==='skills')next='pets';
  queries.set(filter,search.value);positions.set(filter,pageIndex);
  filter=names[next]?next:'all';onFilter(filter);search.value=queries.get(filter)||'';pageIndex=positions.get(filter)||0;
  switchWorkspace(filter==='pets'?'pets':'resources');render();
 }
 document.querySelectorAll('button[data-workspace="preview"]').forEach(b=>b.onclick=()=>switchWorkspace('preview'));
 document.querySelectorAll('[data-filter]').forEach(b=>b.onclick=()=>setFilter(b.dataset.filter));
 $('.brand').onclick=e=>{e.preventDefault();switchWorkspace('preview')};
 document.addEventListener('workspace-preview',()=>switchWorkspace('preview'));
 search.oninput=()=>{pageIndex=0;queries.set(filter,search.value);render()};
 const clear=button('清除',()=>{search.value='';search.dispatchEvent(new Event('input'));search.focus()},'search-clear');clear.setAttribute('aria-label','清除搜索');$('.search').append(clear);
 const tools=node('div','pet-tools');
 const hint=node('span','browse-hint','全部伙伴');
 const status=node('select');status.setAttribute('aria-label','伙伴接入状态');
 for(const [value,label] of [['all','全部伙伴'],['runtime','可用伙伴'],['pending','待接入']]){const o=node('option','',label);o.value=value;status.append(o)}
 status.onchange=()=>{availability=status.value;pageIndex=0;render()};tools.append(hint,status);$('.filters').after(tools);
 // Move all preview controls into the right third, retaining existing handlers and IDs.
 const right=$('.overview-right'),control=node('section','preview-control-card');control.setAttribute('aria-label','模拟器控制台');
 control.append(node('h2','','页面'));
 const device=$('.device-panel');
 for(const e of [...device.children])if(e.matches('.preview-pages,.scene-name,.scene-switch,.scene-effect-controls'))control.append(e);
 right.prepend(control);control.querySelector('.preview-pages').classList.add('page-shortcuts');
 const sceneLabel=node('div','control-section-label','场景');control.querySelector('.scene-name').before(sceneLabel);
 function selectPet(pet,{scroll=false}={}){
  selected=pet;if(scroll&&matchMedia('(max-width: 760px)').matches)mobileExpanded=true;renderInspector();updateSelection();
  if(scroll&&matchMedia('(max-width: 760px)').matches)inspector.scrollIntoView({behavior:matchMedia('(prefers-reduced-motion: reduce)').matches?'instant':'smooth',block:'start'});
 }
 function updateSelection(){for(const c of grid.querySelectorAll('[data-dex]')){const active=Number(c.dataset.dex)===selected?.nationalId;c.classList.toggle('selected',active);c.setAttribute('aria-pressed',String(active))}}
 function renderInspector(){
  inspector.replaceChildren();inspector.hidden=filter!=='pets'||!selected||(matchMedia('(max-width: 760px)').matches&&!mobileExpanded);if(inspector.hidden)return;
  const close=button('收起详情 ↑',()=>{mobileExpanded=false;inspector.hidden=true},'inspector-collapse');inspector.append(close);
  const pet=selected,moves=movesFor(pet),family=pets.filter(x=>moves[0]?.nationalIds.includes(x.nationalId));
  const head=node('div','pet-inspector-head');head.append(image(url(pet),pet.name,'inspector-portrait'));
  const title=node('div');title.append(node('span','eyebrow',`NO. ${String(pet.nationalId).padStart(3,'0')}`),node('h2','',pet.name),node('span','integration '+(!pet.runtimeIntegrated?'pending':''),pet.runtimeIntegrated?'已接入':'待接入'));head.append(title);inspector.append(head);
  if(pet.acquisition==='legendary-egg')inspector.append(node('p','detail-meta','八枚徽章 → 神兽挑战 → 背包领蛋 → 庭院孵化'));
  const familyBox=node('div','family-chips');familyBox.setAttribute('aria-label','进化家族');
  for(const x of family){const b=button(x.name,()=>selectPet(x),x.nationalId===pet.nationalId?'active':'');b.setAttribute('aria-pressed',String(x.nationalId===pet.nationalId));familyBox.append(b)}
  inspector.append(familyBox);
  const heading=node('div','inspector-heading');heading.append(node('h3','','伙伴技能'),node('span','','4'));inspector.append(heading);
  const moveGrid=node('div','pet-moves'),player=node('div','skill-player');
  player.setAttribute('aria-label','技能动画预览');
  function playMove(move){
   player.replaceChildren();for(const b of moveGrid.children)b.setAttribute('aria-pressed',String(Number(b.dataset.move)===move.id));
   const im=image((matchMedia('(prefers-reduced-motion: reduce)').matches?base+move.icon:`../docs/screenshots/skill-${move.id}.gif`)+`?v=${revision}`,move.name+' · 共享固件技能预览','skill-animation');
   const power=move.formPowers.find(x=>x.nationalId===pet.nationalId);
   player.append(im,node('strong','',move.name),node('small','',`${move.species} · 招式演示`),node('p','',`威力 ${power.power} · 形态倍率 ${power.percent}%`),node('p','',move.description));
   if(matchMedia('(prefers-reduced-motion: reduce)').matches)player.append(button('播放技能动画',()=>{im.src=`../docs/screenshots/skill-${move.id}.gif?v=${revision}`},'play-skill'));
   const a=node('a','skill-source','资料 ↗');a.href=move.source;a.target='_blank';a.rel='noopener noreferrer';player.append(a,button('放大预览',()=>detail({...resources.find(x=>x.type==='skills'&&x.id===move.id),selectedNationalId:pet.nationalId}),'skill-enlarge'));
  }
  for(const move of moves){const b=button('',()=>playMove(move),'pet-move');b.dataset.move=move.id;
   b.append(image(base+move.icon+'?v='+revision,'',''),node('strong','',move.name),node('small','',`${move.type} · 威力 ${move.formPowers.find(x=>x.nationalId===pet.nationalId).power}`));moveGrid.append(b)}
  inspector.append(moveGrid,player);if(moves[0])playMove(moves[0]);
  const actions=node('div','pet-inspector-actions');
  if(pet.runtimeIntegrated){actions.append(button('放入庭院预览',()=>previewPet(pet),'primary'));if(pet.keyPoses===48)actions.append(button('查看动作',()=>showAnimation(pet.nationalId)));actions.append(button('进化详情',()=>detail(pet)))}
  else{const a=node('a','offline-template','动作预览 ↗');a.href=`../docs/animation-${String(pet.nationalId).padStart(3,'0')}/index.html`;a.target='_blank';a.rel='noopener noreferrer';actions.append(a)}
  inspector.append(actions);
 }
 function petCard(pet){
  const c=button('',()=>selectPet(pet,{scroll:true}),'pet-card');c.dataset.dex=pet.nationalId;c.setAttribute('aria-label','查看'+pet.name+'的技能');
  const art=node('div','pet-card-art');art.append(node('span','dex',String(pet.nationalId).padStart(3,'0')),image(url(pet),pet.name,''));
  if(!pet.runtimeIntegrated)art.append(node('span','pending-dot','待接入'));
  const info=node('div','pet-card-info');info.append(node('strong','',pet.name),node('small','',petTypes(pet).join(' / ')||'4 技能'));
  c.append(art,info);return c;
 }
 function render(){
  const query=search.value.trim().toLowerCase();clear.hidden=!query;
  const isPet=filter==='pets';grid.className=isPet?'grid compact-pets':'grid compact-resources';
  $('.library-layout').classList.toggle('with-inspector',isPet);$('.filters').hidden=isPet;tools.hidden=!isPet;
  document.querySelectorAll('.filters [data-filter]').forEach(b=>{b.classList.toggle('active',b.dataset.filter===filter);b.setAttribute('aria-pressed',String(b.dataset.filter===filter))});
  search.placeholder=isPet?'搜索伙伴、编号、技能…':'搜索资源名称、编号…';
  let list;
  if(isPet){list=pets.filter(p=>{
   const moves=movesFor(p);const family=skills.find(s=>s.nationalIds.includes(p.nationalId));
   const text=[p.name,String(p.nationalId).padStart(3,'0'),...petTypes(p),...(family?.familyNames||[]),...moves.flatMap(m=>[m.name,m.english,m.type])].join(' ').toLowerCase();
   return text.includes(query)&&(availability==='all'||(availability==='runtime'?p.runtimeIntegrated:!p.runtimeIntegrated));
  });hint.textContent=`${pets.filter(p=>p.runtimeIntegrated).length} 可用 · ${pets.filter(p=>!p.runtimeIntegrated).length} 待接入`;
  }else list=resources.filter(x=>!['pets','skills'].includes(x.type)&&(filter==='all'||filter===x.type)&&(x.name+' '+String(x.nationalId??x.id+1).padStart(3,'0')+' '+x.info).toLowerCase().includes(query));
  if(filter==='collection'){
   // Share the selected rarity with the existing firmware-preview toolbar.
   const q=getCollectionQuality();
   if(q!==6)list=list.filter(x=>x.quality===q);
  }
  // Keep the selection visible and relevant after a search or status change.
  if(isPet&&list.length&&!list.some(p=>p.nationalId===selected?.nationalId))selectPet(list[0]);
  if(isPet&&!list.length){selected=null;inspector.replaceChildren();inspector.hidden=true}
  if(!isPet)inspector.hidden=true;else if(selected)inspector.hidden=matchMedia('(max-width: 760px)').matches&&!mobileExpanded;
  const size=isPet?36:filter==='collection'?48:24,total=Math.ceil(list.length/size);pageIndex=Math.max(0,Math.min(pageIndex,total-1));
  $('#library-heading').firstChild.textContent=names[filter]+' ';$('#resource-count').textContent=`${list.length} ${isPet?'位':'项'}`;
  $('#empty').hidden=list.length>0;$('#empty').replaceChildren(node('strong','','没有找到匹配的资源'),button('重置筛选',()=>{search.value='';availability='all';status.value='all';pageIndex=0;render()}));
  grid.replaceChildren(...list.slice(pageIndex*size,(pageIndex+1)*size).map(x=>isPet?petCard(x):resourceCard(x)));updateSelection();
  renderCollectionToolbar();
  const pager=$('#resource-pagination');pager.replaceChildren();pager.hidden=total<=1;
  if(total>1){const change=n=>{pageIndex=n;positions.set(filter,n);render();$('.library-title').scrollIntoView({block:'start',behavior:'instant'})};
   const prev=button('<',()=>change(pageIndex-1));prev.className='switch-arrow';prev.setAttribute('aria-label','上一页');prev.disabled=pageIndex===0;
   const next=button('>',()=>change(pageIndex+1));next.className='switch-arrow';next.setAttribute('aria-label','下一页');next.disabled=pageIndex===total-1;
   const label=node('span','',`${pageIndex+1} / ${total} · ${pageIndex*size+1}–${Math.min((pageIndex+1)*size,list.length)}`);label.setAttribute('role','status');pager.append(prev,label,next);
  }
 }
 function restoreRoute(){
  const hash=location.hash.slice(1);
  if(hash==='animation'){switchWorkspace('preview',{updateRoute:false});showAnimation();route('#animation')}
  else if(hash==='pets'||hash==='skills')setFilter('pets');else if(hash==='resources')setFilter('all');else if(names[hash])setFilter(hash);else switchWorkspace('preview');
 }
 window.addEventListener('hashchange',restoreRoute);
 matchMedia('(max-width: 760px)').addEventListener('change',()=>{if(filter==='pets')renderInspector()});
 switchWorkspace('preview',{updateRoute:false});
 return {render,setFilter,restoreRoute};
}
