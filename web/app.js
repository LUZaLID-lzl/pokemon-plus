import {sortWarehouse,warehouseSortLabels} from './warehouse-sort.mjs?v=warehousesort2';
import {drawBattleTypes,drawTypeIcon,battleArtTypes,battleTypeNames} from './battle-types.mjs?v=petinfotypes1';
import {shop,shopEnabled,shopReelView,shopReelAdvance} from './shop.mjs?v=shopreel2';
import {trainingDiscovery} from './training-events.mjs?v=variety1';
import {createTrainingJob,trainingDurationMs} from './training-job.mjs?v=autosupply1-trainingxp2-immunity1-variety1';
import {itemDrop,itemDropDuration} from './item-drop.mjs?v=itemdrop1';
import {talkKinds,talkContext,itemTalk,pickTalk} from './pet-dialogue.mjs?v=pettalk1';
import {battleEntryState,battleEntryPaint,battleEntryDuration} from './battle-entrance.mjs?v=entry1';
import {drawBattleHp,drawBattleBall,drawBattleRelations} from './battle-hud.mjs?v=relations3-traininghud1';
import {trainingScenes,trainingApproachMs} from './training.mjs?v=trainingxp2-immunity1-variety1';
import {trainingScenery,trainingThemes,trainingPosition,trainingBattleElements,trainingExitRows} from './training-scenery.mjs?v=trainingsmooth1';
import {hospitalScenery} from './hospital-scenery.mjs?v=care1';
import {createHospital,hospitalFall,hospitalSlot,hospitalRounds} from './hospital.mjs?v=variety1';
import {createHomeConflict,sampleHomeConflict} from './home-conflict.mjs?v=homeconflict1';
import {advanceEnergy,energyBiscuit} from './pet-energy.mjs?v=variety1';
import {createFishing,fishWeight,fishWeightTotal} from './fishing-preview.mjs?v=variety1';
import {fishCatalog} from './fish-catalog.js?v=fishingdex1';
import {createHomeBehavior,homeKinds,homeScenePatrol} from './home-behavior.mjs?v=nestwalk3';
import {groundProject,routeSample,groundAdvance} from './home-navigation.mjs?v=nestwalk3';
import {habitatPets,hatchPool,encounterPool} from './habitats.js?v=regions2';
import {createHudSettings} from './hud-settings.js?v=autosupply1-batterybag1';
import {appMenuPages,appMenuLabels,appMenuPosition,createSceneMenu} from './menu-preview.mjs?v=variety1';
import {createLegendPreview,isLegend,legendIds} from './legend-preview.mjs?v=legendstyle2';
import {createWorkspace} from './workspace.js?v=switcharrows1-workspace3-skills316-ready-legend1';
import {createWildPreview,highestWildPartner} from "./wild-preview.mjs?v=variety1";
import {createHatchPreview} from './hatch-preview.js?v=traininggrowth2-eggtime1-scenenests1';
import {eggGlow} from './egg-glow.mjs?v=eggdot1';
import {sampleIntro,INTRO_MS} from './intro.js?v=1';
import {createDailyPreview,tarotNames,tarotEffects} from './daily.js?v=variety1';
import {createEvolutionDemo,evolutionStep} from './evolution-choice.js?v=autosupply1-warehouseidle1-shoplottery1';
import {createSceneMotion} from './scene-motion.js?v=pondwind1';
import {renderFishingScenery} from './fishing-scenery.js?v=pondwind1';
import {lakePatrol,lakeApproach,lakeDrop,lakeInFront,lakeNearestPhase} from './sacred-lake.js?v=lake3';
import {sampleSceneEffects,drawSceneEffects} from './scene-effects.js?v=scene-fx14';
import {createAnimationPlayer} from './animation-player.js?v=0211-pets136-copy1';
import {collectionItems,createCollectionDemo,ownsAchievement} from './collection.js?v=achievements1';
const assetRevision='warehousesort2-settingsmenu1-autosupply1-petcard1-petinfotypes1-warehouseidle1-trainingxp2-travelmotion2-shopreel2-relations3-relations2-shoppolish2-relations1-battletypes1-shoplottery1-immunity1-shoppurchase1-completionqueue2-variety1-batterybolt1-scene83fpsrestore1-petpatrolrestore1-sceneusehome1-scenerevert120ms1-ambient33ms1-trainingsmooth1-trainingresult2-pettalk1-hospitalpace1-boothome1-bag4cols1-itemdrop1-trainingheader1-scrolltraining2-entryall1-entry1-traininghud1-training5-nestwalk3-batterycharge1-hospitalcare1-hospitaljoy1-hospital1-holdhome1-shopprice1-pondright1-pondabove1-pondicons1-pondwind1-bagorder1-fishingdex1-bagpages1-itemgrid1-essence1-wildauto1-lowername1-fishingopen1-homeconflict1-travelconfirm1-hatchreveal1-navlayout1-warehouse9-switchpos1-switcharrows1-sceneblur1-scenepanorama1-dexorder1-eggtime1-apporder2-fishing2-scenereward1-sleep1-menubg2-travelready2-warehouseicon2-appspacing2-scenenests6-eggdot1-grid9-gymbusy1-homeinteract2-travelsettlement1-patdaily20-1-rosterreturn1-regions1-gymrecommend1-achievements1-hatchrandom1-batterybag1-settingsver1-battle15x1-wildresult3-apporder1-gymicon1-capturepixel1-warehouse1-0320-menustyle1-giftfind1-legendstyle2-pets151-scenes-clock38-unowned-collection-travel3-ready1-intro5-home1-gymstart1-scenefx3-branches2-vermilion1-gymstamp2-scenery2-citytitle1-doorteam1-doorfloor2-petinfo3-celadon1-fuchsia1-saffron1-cinnabar1-viridian1-skills151b-hatchicon1-toast1-wild3-fxvisible1-weatherfull1-ambientfull2-cometmaple2-skycomet1-lake13b-homespace1-fxchance30-settingsswitch1-treeheight1-treecherry1-treedepth1-cavebats1-tumbleweed3-skycloud1-skills316-level60-ready';
const {version:appVersion}=await fetch(`version.json?v=${assetRevision}`).then(r=>{if(!r.ok)throw Error('Version unavailable');return r.json()});
document.querySelector('footer span').firstChild.textContent=`v${appVersion} 已同步 `;
let ambientStart=performance.now();
const scenery=createSceneMotion(await fetch(`scene-motion.json?v=${assetRevision}`).then(r=>{if(!r.ok)throw Error('Scene motion unavailable');return r.json()}));
const sceneEffects=await fetch(`scene-effects.json?v=${assetRevision}`).then(r=>{if(!r.ok)throw Error('Scene effects unavailable');return r.json()});
let ambientSeed=crypto.getRandomValues(new Uint32Array(1))[0];
let ambientPreview=new URLSearchParams(location.search).get('weather')==='1';
// Simulator readings only; the device continues to use the BQ27220 gauge.
const batteryParams=new URLSearchParams(location.search);
const batteryPercent=batteryParams.has('battery')?Number(batteryParams.get('battery')):72;
const batteryValid=Number.isInteger(batteryPercent)&&batteryPercent>=0&&batteryPercent<=100;
const batteryCharging=batteryValid&&batteryParams.get('charging')==='1'&&batteryPercent<100;
const batteryBoltRows=['00010','00110','01100','11111','00110','01100','01000'];
let batteryChargeElapsed=0,batteryLast=performance.now();
function drawBattery(){
 const real=performance.now(),delta=real-batteryLast;batteryLast=real;
 if(document.hidden||!hudSettings.state.showBattery)return;
 if(batteryCharging&&!motion.matches)batteryChargeElapsed=(batteryChargeElapsed+delta%1800)%1800;
 const fill=batteryCharging?(motion.matches?10:Math.floor(batteryChargeElapsed/300)*2):batteryValid&&batteryPercent>0?1+Math.floor(batteryPercent*9/100):0;
 const value=batteryValid?batteryPercent+'%':'--',tint=!batteryValid?'#89979e':batteryCharging?'#e8d492':batteryPercent<=20?'#f29a8e':'#b2dfbe';
 ctx.font='500 12px "Noto Sans SC",sans-serif';
 const w=ctx.measureText(value).width,x=(360-22-w)/2;
 box(x-4,331,w+30,18,'rgba(10,37,48,.67)',7);
 ctx.strokeStyle=tint;ctx.lineWidth=1;ctx.strokeRect(x+.5,335.5,14,8);ctx.fillStyle=tint;
 ctx.fillRect(x+15,337,2,5);if(fill)ctx.fillRect(x+2,337,fill,5);
 if(batteryCharging){
  for(let pass=0;pass<2;pass++){ctx.fillStyle=pass?'#fff0ab':'#0a2530';const edge=pass?0:1;for(let y=0;y<7;y++){const row=batteryBoltRows[y],a=row.indexOf('1'),b=row.lastIndexOf('1');ctx.fillRect(Math.floor(x)+5+a-edge,336+y,b-a+1+edge*2,1)}}
 }
 ctx.fillStyle=tint;
 ctx.textAlign='left';ctx.fillText(value,x+22,344);
}
const manifest=await fetch(`manifest.json?v=${assetRevision}`).then(r=>{if(!r.ok)throw Error('无法读取资源清单');return r.json()});
const movementName=dex=>[41,42,49].includes(dex)?'飞行':[70,71].includes(dex)?'悬移':[72,73].includes(dex)?'游动':[74,81,82].includes(dex)?'悬浮':[86,87].includes(dex)?'滑行':[88,89,90,91].includes(dex)?'挪动':[92,93].includes(dex)?'悬浮':dex===95?'盘行':[98,99].includes(dex)?'横移':[100,101].includes(dex)?'轻滚':dex===102?'倾摇':[109,110].includes(dex)?'悬浮':dex===114?'藤足挪动':[116,117,118,119,129,130,131].includes(dex)?'游动':[120,121].includes(dex)?'星臂游动':'移动';
const base='../sdcard/pokemon-plus/';let filter='all',scene=0,pose=0,previewPets=[0,1,2,3],coins=240;const imgCache=new Map();
function load(path){if(!imgCache.has(path)){const im=new Image();im.src=base+path+`?v=${assetRevision}`;imgCache.set(path,im)}return imgCache.get(path)}
// Warm only pets that are actually shown. Loading the whole growing dex at
// startup can exhaust browser requests before the action strip has loaded.
const warmedPets=new Set();
function preloadPet(id){if(warmedPets.has(id)||!manifest.pets[id])return;warmedPets.add(id);for(const f of manifest.pets[id].frames)load(f)}
for(const i of [0,1,2,3,4,14])load(`icons-small/${i}.png`);
const trainingItemIcon=(item,i)=>item.summaryImage||(i<6?`icons-small/${i}.png`:item.image.replace('.png','-small.png'));
manifest.items.forEach((item,i)=>{if(shopEnabled(i)){load(trainingItemIcon(item,i));load(`shop/${i}.png`)}});load('shop/lottery.png');load('shop/coin.png');load('items/tree-essence-small.png');
for(const id of previewPets)preloadPet(id);for(const s of manifest.scenes)load(s.image);
for(const size of [32,36,40])for(let frame=0;frame<8;frame++)load(`effects/tumble-${size}-${frame}.png`);
for(const type of ['grass','fire','water'])load(`types-large/${type}.png`);
const canvas=document.querySelector('#device'),ctx=canvas.getContext('2d');ctx.imageSmoothingEnabled=false;
const motion=window.matchMedia('(prefers-reduced-motion: reduce)');
const stage=document.querySelector('.device');
const shot=document.querySelector('#device-shot');
const hotspots=document.querySelector('#device-hotspots');
const pageNames={training:'历练',hospital:'宠物医院',fishing:'像素钓鱼',warehouse:'宠物仓库',individual:'个体信息',gift:'发现藏品',legend:'神兽挑战',intro:'草地初遇',animation:'宠物动作模板',home:'庭院',wildalert:'野生相遇',wild:'野生相遇',wildfight:'野外对战',wildcaught:'对战获胜',wildcapture:'捕捉动画',wildsuccess:'捕捉成功',wildduplicate:'重复捕捉',wildlost:'伙伴逃走',apps:'APP',bag:'BAG',shop:'SHOP',status:'宠物信息',travel:'旅行',battle:'尼比市',gym:'尼比道馆',team:'出战编队',gymfight:'道馆战斗',gymwin:'挑战收获',badges:'徽章展馆',collection:'收藏品',scenes:'换场景',tarot:'每日塔罗',settings:'设置',starter:'选择伙伴',steps:'计步器',pokedex:'宠物图鉴'};
let gymCity=0;
function gymRecommendedLevel(index){const key=['pewter','cerulean','vermilion','celadon','fuchsia','saffron','cinnabar','viridian'][index];return Math.max(...manifest.gyms[key].enemies.map(p=>p.level))}
let gymClearedPreview=false;
const gymCities=[{city:'尼比市',type:'岩石',prefix:'gym',eyebrow:'PEWTER CITY · FIRST GYM',team:'小拳石 Lv.10 · 隆隆石 Lv.11 · 大岩蛇 Lv.12'},{city:'华蓝市',type:'水',prefix:'cerulean',eyebrow:'CERULEAN CITY · SECOND GYM',team:'可达鸭 Lv.18 · 海星星 Lv.19 · 宝石海星 Lv.20'},{city:'枯叶市',type:'电',prefix:'vermilion',eyebrow:'VERMILION CITY · THIRD GYM',team:'霹雳电球 Lv.26 · 皮卡丘 Lv.27 · 雷丘 Lv.28'},{city:'玉虹市',type:'草',prefix:'celadon',eyebrow:'CELADON CITY · FOURTH GYM',team:'蔓藤怪 Lv.34 · 大食花 Lv.35 · 霸王花 Lv.36'},{city:'浅红市',type:'毒',prefix:'fuchsia',eyebrow:'FUCHSIA CITY · FIFTH GYM',team:'瓦斯弹 Lv.42 · 臭臭泥 Lv.43 · 双弹瓦斯 Lv.44'},{city:'金黄市',type:'超能',prefix:'saffron',eyebrow:'SAFFRON CITY · SIXTH GYM',team:'魔墙人偶 Lv.48 · 勇基拉 Lv.49 · 胡地 Lv.50'},{city:'红莲岛',type:'火',prefix:'cinnabar',eyebrow:'CINNABAR ISLAND · SEVENTH GYM',team:'九尾 Lv.53 · 烈焰马 Lv.54 · 风速狗 Lv.55'},{city:'常青市',type:'地面',prefix:'viridian',eyebrow:'VIRIDIAN CITY · FINAL GYM',team:'尼多后 Lv.58 · 尼多王 Lv.59 · 钻角犀兽 Lv.60'}];
let travelPreviewStage='journey',travelPreviewDestination=0;
function nextTravelDestination(){
 const count=manifest.travel.destinations.length;let roll;
 do{roll=crypto.getRandomValues(new Uint32Array(1))[0]}while(roll>=Math.floor(4294967296/count)*count);
 travelPreviewDestination=roll%count;
}

const wildPreview=createWildPreview({storage:localStorage,onSpawn:()=>chooseLocalEncounter()?wildPreviewPet.id:null});
let wildPreviewX=236;
function wildPartnerIndex(){return highestWildPartner(previewIndividuals.map(id=>evolutionDemo.state.pets.find(p=>p.id===id)))}
let wildPreviewPet=manifest.pets[wildPreview.encounter]||manifest.pets.find(p=>p.nationalId===16);preloadPet(wildPreviewPet.id);
function chooseLocalEncounter(){const pool=encounterPool(manifest.habitats,manifest.pets,scene);if(!pool.length)return false;const owned=new Set(evolutionDemo.state.pets.map(p=>manifest.pets[p.art]?.species)),missing=pool.filter(p=>!owned.has(p.species)),eligible=missing.length&&Math.random()<.5?missing:pool;wildPreviewPet=eligible[Math.floor(Math.random()*eligible.length)];preloadPet(wildPreviewPet.id);return true}
const wildMarkPixels=['01110','12221','12221','12221','12221','01110','00000','01110','12221','01110'];
const wildCounter=document.querySelector('#wild-countdown');
function placeWildCountdown(){
 if(wildCounter.hidden)return;
 const r=(page==='wild'?canvas:shot).getBoundingClientRect(),p=stage.getBoundingClientRect();
 const receipt=wildCounter.dataset.receipt==='true';
 Object.assign(wildCounter.style,{left:`${r.left-p.left+r.width*(receipt?190:90)/360}px`,top:`${r.top-p.top+r.height*(receipt?303:204)/360}px`,width:`${r.width*(receipt?68:180)/360}px`,height:`${r.height*(receipt?18:20)/360}px`,fontSize:`${r.width*12/360}px`});
}
new ResizeObserver(placeWildCountdown).observe(stage);
const screenshots={gift:'gift-pending.png',legend:'legend-locked.png',wild:'wild-pending.png',wildfight:'wild-battle.png',wildcaught:'wild-caught.png',wildcapture:'wild-capture.gif',wildsuccess:'wild-success.png',wildduplicate:'wild-duplicate.png',wildlost:'wild-lost.png',steps:'activity-steps.png',apps:'page-1.png',bag:'page-2.png',shop:'page-3.png',travel:'page-5.png',battle:'gym-map.png',gym:'gym-detail.png',team:'gym-team.png',gymfight:'gym-battle.png',gymwin:'gym-win.png',badges:'gym-badges.png',collection:'page-8.png',scenes:'page-9.png'};
let page='home',homeElapsed=0,homeLast=performance.now(),talkAt=18000,talkUntil=0,talkPet=0,patUntil=0,patPet=-1,starterChoice=1,dexPage=0,petDexPage=0;
const homeClocks=new Map(),homeOrigins=[];
let homeBehavior=createHomeBehavior(ambientSeed^0x6a09e667,0,scene),homeAwayAt=0,homeWelcomeAt=-120000,welcomeDue=false;
const homeConflict=createHomeConflict(ambientSeed^0xbb67ae85),conflictOrigins=[];
const moveTypes=['电','草','火','水','一般','虫','毒','飞行','地面','岩石','妖精','超能力','格斗','冰','钢','幽灵','恶','龙'];
const conflictMoves=manifest.pets.map(p=>manifest.skills.filter(m=>m.familyId===p.species).map(m=>({...m,type:moveTypes.indexOf(m.type==='超能'?'超能力':m.type)})));
function conflictPets(){return previewPets.map((art,i)=>{const p=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[i]);return {...p,moves:conflictMoves[art],health:p?.health??0,mood:p?.mood??80,energy:p?.energy??80}})}
function releaseConflict(){for(const p of [homeConflict.state.attacker,homeConflict.state.target]){const m=homeOrigins[p];if(m){m.ox=m.x;m.oy=m.y;m.start=homeElapsed-(scene===12?lakeNearestPhase(m.x,m.y):0)}}}
function beginConflict(){const c=homeConflict.state;[c.attacker,c.target].forEach((p,i)=>{conflictOrigins[i]={x:homeOrigins[p].x,y:homeOrigins[p].y};homeBehavior.cancel(p);homeClocks.delete(p)});homeSpeech(c.attacker,conflictMoves[previewPets[c.attacker]][c.slot].name,-1,700)}
function updateConflict(now,actors){
 const mask=actors.reduce((m,p,i)=>p.available&&!homeBehavior.pets[i].kind?m|(1<<i):m,0);
 try{const event=homeConflict.tick(conflictPets(),Math.round(now),page==='home'&&!previewDrop&&!document.hidden&&!motion.matches,mask,(slot,health,mood)=>evolutionDemo.homeHit(previewIndividuals[slot],health,mood));
 if(event==='begin')beginConflict();if(event==='hit')homeSpeech(homeConflict.state.target,`健康-${homeConflict.state.healthLoss} 心情-${homeConflict.state.moodLoss}`,-1,1400);
 if(['end','cancel'].includes(event))releaseConflict();
 }catch(e){releaseConflict();homeConflict.pause(Math.round(now));toast(e.message)}
 if(homeConflict.state.active){actors[homeConflict.state.attacker].available=false;actors[homeConflict.state.target].available=false}
}
document.addEventListener('visibilitychange',()=>{if(document.hidden){releaseConflict();homeConflict.pause(Math.round(homeElapsed));homeLast=performance.now()}});
function homeActors(){return previewPets.map((id,i)=>{const m=homeOrigins[i]||petRects[i]||{x:89,y:128,left:false};const individual=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[i]);return {x:Math.round(m.x??m.ox??89),y:Math.round(m.y??m.oy??128),left:!!m.left,mood:individual?.mood??80,energy:individual?.energy??80,available:!evolutionDemo.trainingBusy(individual?.id)&&!individual?.sleeping&&!(previewDrop&&i===previewDrop.target)&&pointer?.pet!==i&&page!=='wild'}})}
function drawHomeSymbol(p){
 if(!p.symbol)return;
 const shapes=[['0110110','1111111','1111111','0111110','0011100','0001000','0000000'],['0001110','0001010','0001000','0001000','0111000','1111000','0110000'],['0011100','0100010','0000100','0001000','0001000','0000000','0001000'],['0100010','0110110','0000000','1100011','0000000','0110110','0100010']];
 ctx.fillStyle=['#f6aabb','#ffdf8a','#c9e7dc','#f18b7a'][p.symbol-1];shapes[p.symbol-1].forEach((row,y)=>[...row].forEach((v,x)=>{if(v==='1')ctx.fillRect(p.x+17+x*2,p.y-16+y*2,2,2)}));
}
function petFrame(id,action,now,slot){const a=manifest.pets[id].animation||manifest.animation;let c=homeClocks.get(slot);if(!c||c.action!==action||c.id!==id){preloadPet(id);c={id,action,start:now};homeClocks.set(slot,c)}const frames=a.actions[action],durations=a.durationsMs?.[action]||frames.map(()=>manifest.animation.frameMs);let elapsed=Math.max(0,now-c.start),total=durations.reduce((a,b)=>a+b,0);if(a.modes?.[action]==='one-shot'&&elapsed>=total)return frames.at(-1);elapsed%=total;let i=0;while(i<frames.length-1&&elapsed>=durations[i])elapsed-=durations[i++];return frames[motion.matches?0:i]}
let bubble='',bubbleIcon=-1,speechKey=null,careSpeech=false,petRects=[],previewDrop=null;
const talkStates=new Map();
const petTalkKey=i=>previewIndividuals[i]??`preview:${i}:${previewPets[i]}`;
function petTalkState(i){const key=petTalkKey(i);if(!talkStates.has(key))talkStates.set(key,{cursors:[],sequence:0});return talkStates.get(key)}
function homeSpeech(pet,words,icon,duration){talkPet=pet;bubble=words;bubbleIcon=icon;careSpeech=false;speechKey=petTalkKey(pet);talkUntil=homeElapsed+duration}
function homeSay(pet,topic,duration=3200){const line=pickTalk(petTalkState(pet).cursors,topic,previewPets[pet]*7+pet*3);homeSpeech(pet,line.text,line.icon,duration)}
let eggDropStart=-itemDropDuration;
function startPreviewDrop(item,target=0){
 if(previewDrop){toast('宠物正忙着呢');return false}
 if(evolutionDemo.trainingBusy(previewIndividuals[target])){toast('伙伴正在历练');return false}
 talkUntil=0;
 dialog.close();openPage('home');draw();
 const spot=Math.floor(Math.random()*24),drop=scene===12?lakeDrop(spot):{x:70+(spot%6)*40,y:145+Math.floor(spot/6)*22},ground=groundProject(scene,drop.x-8,drop.y-16);
 previewDrop={item,target,x:ground.x+8,y:ground.y+16,start:homeElapsed,from:petRects[target]||{x:89,y:128}};
 // Entering home can schedule an idle line before the drop owns the scene.
 talkUntil=0;return true;
}
let chosen=null;
try { const id=Number(localStorage.getItem('pokemon-plus.web-starter')); if([1,4,7].includes(id)) chosen=id; } catch {}
const starters=[{dex:1,species:1,name:'妙蛙种子'},{dex:4,species:2,name:'小火龙'},{dex:7,species:3,name:'杰尼龟'}];
const hatchPreview=createHatchPreview(manifest.sceneNests);
let bagPage=0,bagLegendPreview=false;
let ballStock=0,captureCommitted=false,captureTimer,wildShopReturn=false;
const legendPreview=createLegendPreview();const sceneMenu=createSceneMenu(manifest.scenes.length,{getProsperity:()=>evolutionDemo.state.dexHistory.length});let appPreviewPage=0;
function showLegend(phase){try{legendPreview.show(phase);if(phase==='bag'){bagLegendPreview=true;openPage('bag')}else openPage('legend')}catch(e){toast(e.message)}}
function legendBag(){if(!legendPreview.unlocked){toast('先切换八枚徽章预览');return}showLegend('bag')}

let introSeen=false,introStart=null;
try{introSeen=localStorage.getItem('pokemon-plus.web-intro-seen')==='1'}catch{}
const introPets=starters.map(s=>manifest.pets.find(p=>p.nationalId===s.dex));
const introImages=[load(manifest.scenes[0].image),...['grass','fire','water'].map(t=>load(`types-large/${t}.png`)),...introPets.flatMap(p=>p.frames.flatMap(f=>[load(f),load(f.replace('pets/','pets-left/'))]))];
for(const im of introImages)im.fetchPriority='high';
await document.fonts.load('800 24px PokemonIntro').catch(()=>{});
function finishIntro(){introSeen=true;try{localStorage.setItem('pokemon-plus.web-intro-seen','1')}catch{}openPage('starter')}
function raisedIntroText(s,y,size,alpha,titleStyle=true){
 const layer=(dx,dy,color,opacity=255)=>{ctx.globalAlpha=alpha*opacity/(255*255);text(s,180+dx,y+dy,size,color,800,'PokemonIntro')};
 if(titleStyle){
  const offsets=[[-1,-1],[0,-1],[1,-1],[-1,0],[1,0],[-1,1],[0,1],[1,1]];
  layer(4,5,'#091b28',180);
  for(const [x,y]of offsets)layer(3+x,3+y,'#142635');
  layer(3,3,'#53446e');layer(2,2,'#716092');layer(1,1,'#9880bd');
  for(const [x,y]of offsets)layer(x,y,'#263447');
  layer(-1,-1,'#fff0ff');layer(0,0,'#d5c6ff');
 }else {layer(1,2,'#14392f',230);layer(0,1,'#5c8470');layer(0,0,'#e2eadb')}
 ctx.globalAlpha=1;
}
function drawIntro(){
 ctx.fillStyle='#080e17';ctx.fillRect(0,0,360,360);
 if(!introImages.every(im=>im.complete&&im.naturalWidth)){drawFooterText('>>',116,296,128,32);return}
 if(introStart===null)introStart=performance.now();
 const age=performance.now()-introStart;
 if(age>=(motion.matches?1400:INTRO_MS)){finishIntro();return}
 const s=sampleIntro(motion.matches?9500:age);
 image(manifest.scenes[0].image,0,0,360,360);ctx.fillStyle='#080e176e';ctx.fillRect(0,0,360,360);
 raisedIntroText('POKEMON',68,32,s.title);
 raisedIntroText('PLUS',101,24,s.title);
 s.pets.forEach((p,i)=>{
  const pet=introPets[i],a=pet.animation||manifest.animation,action=['idle','walk','happy','eat','sleep','attack'][p.pose/4],frames=a.actions[action],durations=a.durationsMs[action];
  let elapsed=p.elapsed,total=durations.reduce((a,b)=>a+b,0),f=0;
  if(a.modes[action]==='one-shot'&&elapsed>=total)f=frames.length-1;
  else {elapsed%=total;while(f<frames.length-1&&elapsed>=durations[f])elapsed-=durations[f++]}
  ctx.globalAlpha=p.opacity/255;image(pet.frames[frames[motion.matches?0:f]].replace('pets/',p.left?'pets-left/':'pets/'),p.x,p.y,48,48);
  ctx.globalAlpha=s.icons/255;image(`types-large/${['grass','fire','water'][i]}.png`,p.x+8,125,32,32);
 });
 if(!motion.matches)for(const p of s.particles){if(!p.opacity)continue;ctx.globalAlpha=p.opacity/255;const color=['#9cdb95','#ffc078','#9bd8ef'][p.type];if(p.type===2){ctx.strokeStyle=color;ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(p.x+.5,p.y+.5,p.size-1,p.size-1,3);ctx.stroke()}else box(p.x,p.y,p.size,p.size,color,1)}
 raisedIntroText('一起开始冒险吧',260,20,s.caption,false);
 ctx.globalAlpha=s.fade/255;ctx.fillStyle='#080e17';ctx.fillRect(0,0,360,360);ctx.globalAlpha=1;
 drawFooterText('>>',116,296,128,32);
}

if(chosen) previewPets=[starters.find(p=>p.dex===chosen).species];
const browserOwned=new Set();
try{const saved=JSON.parse(localStorage.getItem('pokemon-plus.web-owned-forms')||'[]');if(Array.isArray(saved))for(const id of saved)if(Number.isInteger(id)&&manifest.pets[id])browserOwned.add(id)}catch{}
function unlockBrowserPet(id){const pet=manifest.pets[id];for(const p of manifest.pets)if(p.species===pet.species&&(!p.stage||(pet.species===67?p.stage===pet.stage:p.stage<=pet.stage)))browserOwned.add(p.id);try{localStorage.setItem('pokemon-plus.web-owned-forms',JSON.stringify([...browserOwned]))}catch{}}
if(chosen)unlockBrowserPet(starters.find(p=>p.dex===chosen).species);
let demoStorage;try{demoStorage=localStorage}catch{demoStorage={getItem(){return null},setItem(){throw Error('浏览器无法保存，请允许本地存储后重试')}}}
const today=()=>{const d=new Date();return d.getFullYear()*10000+(d.getMonth()+1)*100+d.getDate()};
const hudSettings=createHudSettings(demoStorage);
const daily=createDailyPreview({storage:demoStorage,sceneCount:manifest.scenes.length});
try{daily.advance(today());if(daily.state.random)scene=daily.state.scene}catch{}
let selectedPet=0;
const evolutionDemo=createEvolutionDemo({storage:demoStorage,roster:manifest.pets,chains:manifest.evolution.chains});
coins=evolutionDemo.state.coins;
const collectionDemo=createCollectionDemo({storage:demoStorage});
let collectionQuality=6;
const ownsCollection=id=>id===583?evolutionDemo.state.heart:id>=584?ownsAchievement(id,{ownedForms:browserOwned,roster:manifest.pets}):collectionDemo.has(id);
const collectionList=()=>collectionItems(manifest.collection,collectionQuality);
const collectionInfo=x=>`${manifest.collection.qualities[x.quality].name} · ${x.source}${x.scene===null?'':' · '+manifest.scenes[x.scene].name}`;
let previewIndividuals=[];
if(!evolutionDemo.state.pets.length&&chosen){try{evolutionDemo.add(starters.find(p=>p.dex===chosen).species)}catch{}}
try{evolutionDemo.advanceCare()}catch{}
setInterval(()=>{try{evolutionDemo.advanceCare();coins=evolutionDemo.state.coins}catch{}},5000);
function syncCompanions(){const pets=evolutionDemo.state.active.map(id=>evolutionDemo.state.pets.find(p=>p.id===id)).filter(Boolean);if(pets.length){previewPets=pets.map(p=>p.art);previewIndividuals=pets.map(p=>p.id)}homeClocks.clear();}
syncCompanions();
let warehousePage=0,warehouseSort=0,individualId=null,individualFromWarehouse=false;
const warehousePets=()=>sortWarehouse(evolutionDemo.warehouse(),evolutionDemo.state.active,warehouseSort);
const warehousePageSize=9;
const warehouseCardX=(i,count)=>(360-Math.min(3,count-Math.floor(i/3)*3)*78+6)/2+i%3*78;
function renamePreview(){
 const pet=evolutionDemo.state.pets.find(p=>p.id===individualId);if(!pet)return;
 const out=document.querySelector('#detail-content');out.replaceChildren();const title=document.createElement('h2');title.textContent='命名';
 const input=document.createElement('input');input.type='text';input.value=pet.nickname.toLowerCase();input.setAttribute('aria-label','宠物名字');input.placeholder=manifest.pets[pet.art].name;input.maxLength=8;input.pattern='[a-z]{0,8}';input.autocapitalize='none';input.spellcheck=false;input.setAttribute('aria-description','仅限8个小写字母');input.addEventListener('input',()=>{input.value=input.value.toLowerCase().replace(/[^a-z]/g,'').slice(0,8)});
 const save=document.createElement('button');save.textContent='保存';save.onclick=()=>{try{evolutionDemo.rename(pet.id,input.value);dialog.close();openPage('individual')}catch(e){toast(e.message)}};
 out.append(title,input,save);dialog.showModal();input.focus();
}
function releasePreview(){
 const pet=evolutionDemo.state.pets.find(p=>p.id===individualId);if(!pet)return;
 const out=document.querySelector('#detail-content');out.replaceChildren();const title=document.createElement('h2');title.textContent='确认放生 '+(pet.nickname||manifest.pets[pet.art].name)+'？';
 const yes=document.createElement('button');yes.textContent='确认放生';yes.onclick=()=>{try{evolutionDemo.release(pet.id);dialog.close();openPage('warehouse')}catch(e){toast(e.message)}};
 const no=document.createElement('button');no.textContent='取消';no.onclick=()=>dialog.close();out.append(title,yes,no);dialog.showModal();
}
const navSpans=[[7,8],[6,9],[5,9],[4,9],[3,13],[2,14],[1,15],[0,15],[0,15],[1,15],[2,14],[3,13],[4,9],[5,9],[6,9],[7,8]];
function navCell(x,y){return y>=0&&y<16&&x>=navSpans[y][0]&&x<=navSpans[y][1]}
function drawNavArrow(x,y,next){
 ctx.save();for(let pass=0;pass<2;pass++)for(let row=0;row<16;row++)for(let col=navSpans[row][0];col<=navSpans[row][1];col++){const edge=!navCell(col-1,row)||!navCell(col+1,row)||!navCell(col,row-1)||!navCell(col,row+1);ctx.fillStyle=!pass?'#112e36':edge?'#285448':!navCell(col,row-2)?'#e2ffe5':!navCell(col,row+2)?'#69b69a':'#a9e4bf';ctx.fillRect(x+6+(next?15-col:col)*2,y+5+row*2+(pass?0:2),2,2)}ctx.restore();
}
function drawWarehouseSortIcon(){
 const mode=warehouseSort,cell=(x,y)=>{
  if(x<0||x>=16||y<0||y>=16)return false;
  const arrow=x>=2&&x<=4&&y>=1&&y<=10||y===10&&x<=6||y===11&&x>=1&&x<=5||y===12&&x>=2&&x<=4||y===13&&x===3;
  if(x<8)return arrow;
  if(mode===2)return (x<=10||x>=13)&&(y>=2&&y<=4||y>=10&&y<=12);
  return y>=1&&y<=3||y>=6&&y<=8&&x<=(mode===1?13:15)||y>=11&&y<=13&&x<=(mode===1?11:15);
 };
 for(let pass=0;pass<2;pass++)for(let y=0;y<16;y++)for(let x=0;x<16;x++)if(cell(x,y)){
  const edge=!cell(x-1,y)||!cell(x+1,y)||!cell(x,y-1)||!cell(x,y+1);
  ctx.fillStyle=!pass?'#112e36':edge?'#285448':!cell(x,y-2)?'#e2ffe5':!cell(x,y+2)?'#69b69a':'#a9e4bf';ctx.fillRect(214+x*2,293+y*2+(pass?0:2),2,2);
 }
}
function drawSwitchArrow(next){drawNavArrow(next?298:18,158,next)}
function drawFooterText(s,x=116,y=260,w=128,h=32){
 ctx.save();const rows=Math.floor(h/2);for(let pass=0;pass<2;pass++)for(let row=0;row<rows;row++){const edge=Math.min(row,rows-1-row),inset=edge===0?8:edge===1?4:edge===2?2:0,py=y+row*2+(pass?0:2);ctx.fillStyle=!pass?'#112e36':row===rows-1?'#397d6c':'#81cbb0';ctx.fillRect(x+inset,py,w-inset*2,2);if(pass&&row>0&&row<rows-1){ctx.fillStyle=row===1?'#3c7462':'#28564e';ctx.fillRect(x+inset+2,py,w-inset*2-4,2)}}text(s,x+w/2,y+h/2+5,w<96?12:14,'#e5e9e2');ctx.restore();
}
function bagItems(){
 const entries=manifest.items.map((item,i)=>({...item,id:i,count:i===8?evolutionDemo.state.treeEssences:(evolutionDemo.state.trainingItems?.[i]||0)+(i===7?ballStock:i===6?hatchPreview.boosts:i<5?3:0)}));
 if(bagLegendPreview)for(let i=0;i<5;i++){const pet=manifest.pets.find(p=>p.nationalId===legendIds[i]);entries.push({id:manifest.items.length+i,legend:i,name:pet.name+'的蛋',image:`avatars/${pet.id}-0.png`,count:1})}
 return entries.filter(item=>item.id!==2&&item.count>0);
}
function itemGrid(){
 const capacity=page==='bag'?12:9,entries=page==='bag'?bagItems():shop.grid.map(i=>i===null?{id:-1,name:'抽奖',price:shop.drawPrice,lottery:true}:{...manifest.items[i],id:i}),pages=Math.max(1,Math.ceil(entries.length/capacity));
 bagPage=Math.min(bagPage,pages-1);const start=page==='bag'?bagPage*capacity:0;
 return {pages,entries:entries.slice(start,start+capacity)};
}
function itemCardPosition(i){const bag=page==='bag',cols=bag?4:3;return {x:64+(i%cols)*(bag?60:80),y:(bag?106:98)+Math.floor(i/cols)*(bag?62:68),w:bag?52:72,h:bag?56:64}}
let shopSpin=null;
document.addEventListener('visibilitychange',()=>{if(shopSpin)shopSpin.last=performance.now()});
function buyShopItem(item){if(shopSpin?.active)return;try{coins=evolutionDemo.shopBuy(item,coins);draw();toast(`购买${manifest.items[item].name}x1`)}catch(e){toast(e.message)}}
function startShopDraw(){
 if(shopSpin?.active)return;
 try{const item=evolutionDemo.shopDraw(coins);coins=evolutionDemo.state.coins;shopSpin={item,elapsed:0,last:null,active:true};openPage('shop',{reveal:false})}catch(e){toast(e.message)}
}
function updateShopSpin(){
 if(!shopSpin?.active)return;
 const now=performance.now();if(shopSpin.last===null){shopSpin.last=now;return}const delta=now-shopSpin.last;shopSpin.last=now;
 if(document.hidden||page!=='shop')return;
 shopSpin.elapsed=shopReelAdvance(shopSpin.item,shopSpin.elapsed,delta);
 if(shopReelView(shopSpin.item,shopSpin.elapsed).done){shopSpin.active=false;toast(`抽中${manifest.items[shopSpin.item].name}x1`);queueMicrotask(()=>openPage('shop',{reveal:false}))}
}
function drawItemGrid(){
 updateShopSpin();
 drawMenuBackground();text(page==='bag'?'BAG':'SHOP',180,54,18,'#f3ecdc');
 text(String(coins),190,81,14,'#83c6b3');image('hud/coin.png',157,68,16,16);
 const {entries,pages}=itemGrid();const reel=page==='shop'&&shopSpin?shopReelView(shopSpin.item,shopSpin.elapsed):null;
 if(page==='bag'&&!entries.length)text('背包空空的',180,184,14,'#91adaf');
 if(page==='bag'&&pages>1){text(`${bagPage+1} / ${pages}`,180,97,12,'#799b9c');drawSwitchArrow(false);drawSwitchArrow(true)}
 entries.forEach((item,slot)=>{
  const {x,y,w,h}=itemCardPosition(slot),egg=item.legend!==undefined;
  const lit=reel?.visible&&reel.slot===slot;
  if(item.lottery){box(x,y,w,h,'#544127',8);ctx.strokeStyle='#cda962';ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(x+.5,y+.5,w-1,h-1,8);ctx.stroke();image('shop/lottery.png',x+10,y+1,52,44);ctx.font='500 10px "Noto Sans SC",sans-serif';const price=String(shop.drawPrice),width=ctx.measureText(price).width,left=x+(72-15-width)/2;image('shop/coin.png',left,y+51,12,12);text(price,left+15+width/2,y+61,10,'#ffdb83');return}
  let fill='#173b40';if(lit){const a=200;fill=`rgb(${Math.round((180*a+23*(255-a))/255)},${Math.round((154*a+59*(255-a))/255)},${Math.round((80*a+64*(255-a))/255)})`}
  if(page==='bag'||lit){ctx.save();if(lit)ctx.globalAlpha=.43;box(x,y,w,h,fill,8);ctx.restore();ctx.strokeStyle=lit?'#ffdb83':egg?'#716a97':item.id===8?'#8dbd7a':'#386264';ctx.lineWidth=lit?2:1;ctx.beginPath();ctx.roundRect(x+.5,y+.5,w-1,h-1,8);ctx.stroke()}
  ctx.globalAlpha=page==='bag'&&!item.count?0.32:1;image(page==='shop'?`shop/${item.id}.png`:item.image,x+(w-(page==='shop'||egg?40:32))/2,y+(egg?0:page==='bag'?4:2),page==='shop'||egg?40:32,page==='shop'||egg?40:32);ctx.globalAlpha=1;
  if(page==='bag')text('×'+item.count,x+w/2,y+52,12,item.count?'#c9e6cf':'#718c90');
  else {ctx.font='500 10px "Noto Sans SC",sans-serif';const nameWidth=ctx.measureText(item.name).width,price=String(item.price),priceWidth=ctx.measureText(price).width;text(item.name,x+4+nameWidth/2,y+59,10,'#d8e5df');text(price,x+68-priceWidth/2,y+59,10,'#e9cf83')}
 });returnButton();
}
function drawWarehouse(){
 drawMenuBackground();text(pageNames[page],180,54,18,'#f2e6cd');
 if(page==='warehouse'){
  const pets=warehousePets(),pages=Math.max(1,Math.ceil(pets.length/warehousePageSize));warehousePage%=pages;text(`${pets.length}只    ${warehousePage+1} / ${pages}`,180,74,12,'#94b5b4');
  if(!pets.length)text('仓库空空的',180,165,14,'#91adaf');
  pets.slice(warehousePage*warehousePageSize,warehousePage*warehousePageSize+warehousePageSize).forEach((p,i)=>{const x=warehouseCardX(i,Math.min(warehousePageSize,pets.length-warehousePage*warehousePageSize)),y=84+Math.floor(i/3)*68;box(x,y,72,66,'#173d40',12);if(evolutionDemo.state.active.includes(p.id)){ctx.save();ctx.strokeStyle='#9bd9b1';ctx.lineWidth=2;ctx.beginPath();ctx.roundRect(x+1,y+1,70,64,11);ctx.stroke();ctx.restore()}image(`avatars/${p.art}-0.png`,x+16,y+1,40,40);text(shortName(p.nickname||manifest.pets[p.art].name,5),x+36,y+47,12,'#dce9d5');text(`Lv.${p.level}`,x+36,y+62,12,'#95c8af')});
  if(pages>1){drawSwitchArrow(false);drawSwitchArrow(true)}
  drawWarehouseSortIcon();
 }else{
  const p=evolutionDemo.state.pets.find(p=>p.id===individualId);if(!p)return;
  text('性格 · '+evolutionDemo.natureNames[p.nature],180,74,12,'#94b5b4');
  drawPetIdentityCard(p.art,p,p.level);
  const names=['HP','攻击','防御','特攻','特防','速度'],axes=[0,0,1,3,4,2],up=Math.floor(p.nature/5),down=p.nature%5;
  names.forEach((name,i)=>{const x=78+i%2*112,y=176+Math.floor(i/2)*27,change=i&&up!==down?(axes[i]===up?'+':axes[i]===down?'-':''):'';text(`${name}${change}  ${p.ivs[i]}`,x+46,y+14,12,'#c9ddd4');box(x+3,y+22,86,3,'#27434a',2);box(x+3,y+22,86*p.ivs[i]/31,3,'#8ac6a5',2)});
  const stored=!evolutionDemo.state.active.includes(p.id);const controls=stored?[[76,'命名'],[146,'放生'],[216,evolutionDemo.state.active.length<5?'上场':'替换']]:individualFromWarehouse&&evolutionDemo.state.active.length>1?[[108,'命名'],[188,'收回']]:[[144,'命名']];controls.forEach(([x,label])=>{drawFooterText(label,controls.length===1?116:x,260,controls.length===1?128:stored?68:72,32)});
 }
 if(page==='warehouse')drawNavArrow(158,288,false);else returnButton();
}
function drawPetIdentityCard(art,individual,demoLevel,openDetails=false){
 const pet=manifest.pets[art];
  box(64,76,232,90,'#132630',18);
  ctx.strokeStyle='#2a4650';ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(64.5,76.5,231,89,18);ctx.stroke();
  box(76,84,52,52,'#244044',15);image(pet.frames[0],80,86,48,48);
  ctx.font='500 18px "Noto Sans SC",sans-serif';ctx.fillStyle='#e4eee4';ctx.textAlign='left';ctx.fillText(shortName(individual?.nickname||pet.name,8),144,103);
  box(144,110,44,20,'#25483f',6);text(`Lv.${demoLevel}`,166,124,12,'#b6dfcc');
  ctx.font='500 12px "Noto Sans SC",sans-serif';ctx.fillStyle='#8aa6ac';ctx.textAlign='right';ctx.fillText((individual?.xp??0)+' / '+(demoLevel>=60?1:20*demoLevel),280,124);ctx.textAlign='center';
  box(144,134,136,4,'#29414b',2);box(144,134,136*Math.min(1,(individual?.xp??0)/(demoLevel>=60?1:20*demoLevel)),4,'#c2d69b',2);
  drawPetTypes(art,144,144);
  if(openDetails)text('›',285,103,18,'#8cb7aa');
}
function drawPetTypes(art,x,y){
 for(const [i,type] of (battleArtTypes[art]??[]).entries()){
  const left=x+i*64;drawTypeIcon(ctx,type,left,y);
  ctx.font='500 12px "Noto Sans SC",sans-serif';ctx.fillStyle='#b6cbce';ctx.textAlign='left';ctx.fillText(battleTypeNames[type],left+20,y+13);ctx.textAlign='center';
 }
}
function shortName(s,n){return [...s].length>n?[...s].slice(0,n-1).join('')+'…':s;}

function specimen(art){return evolutionDemo.state.pets.find(p=>previewIndividuals.includes(p.id)&&p.art===art)||evolutionDemo.state.pets.find(p=>p.art===art)||evolutionDemo.add(art)}
function evolutionControls(out,x){
 const pending=manifest.evolution.chains.find(c=>c.species===x.species);
 if(!pending||(pending.branches?!pending.branches.some(b=>b.fromNationalId===x.nationalId):pending.nationalIds.at(-1)===x.nationalId))return;
 const current=evolutionDemo.state.pets.find(p=>previewIndividuals.includes(p.id)&&p.art===x.id)||evolutionDemo.state.pets.find(p=>p.art===x.id&&!p.locked)||evolutionDemo.state.pets.find(p=>p.art===x.id);const owned=current?.locked;
 const note=document.createElement('p');note.className='detail-meta';note.textContent=owned?'永久不进化 · 纯净之心已收藏':'浏览器试玩';out.append(note);
 const b=document.createElement('button');b.textContent=owned?'已选择永久不进化':'体验进化选择'+(evolutionStep(pending,x.stage).item?' · '+evolutionStep(pending,x.stage).item:'');b.disabled=!!owned;
 b.onclick=()=>{try{const pet=current||specimen(x.id),offers=evolutionDemo.choices(pet.id);let target=offers.length===1?offers[0].next.nationalId:null;out.replaceChildren();const title=document.createElement('h2');title.textContent='成长的选择 · '+x.name;out.append(title);const art=document.createElement('img');art.src=base+x.frames[0];art.alt=x.name;art.className='choice-pet';out.append(art);const warning=document.createElement('p');warning.textContent='保持原样后，这只伙伴将永久不再进化，并获得收藏品「纯净之心」。';out.append(warning);const actions=document.createElement('div');actions.className='evolution-actions';
 const decide=decline=>{try{const updated=evolutionDemo.choose(pet.id,decline,coins,target);coins=evolutionDemo.state.coins;previewPets=[updated.art];previewIndividuals=[updated.id];if(!decline)unlockBrowserPet(updated.art);out.replaceChildren();const h=document.createElement('h2');h.textContent=decline?'守护最初的自己':manifest.pets[updated.art].name;const words=document.createElement('p');words.textContent=decline?'成长，也可以是守护最初的自己。':'每一次成长，都有你的陪伴。';const im=document.createElement('img');im.src=`../docs/screenshots/${decline?`evolution-decline-${x.species}-${x.stage}`:`evolution-${x.species}-${pending.branches?target:x.stage}`}.gif?v=${assetRevision}`;im.alt=decline?'保持原样与纯净之心赠礼动画':'伙伴进化动画';const result=document.createElement('p');result.textContent=decline?'已获得收藏品「纯净之心」 · 这只伙伴将永久保持当前形态':'新形态已解锁';const done=document.createElement('button');done.textContent='回到庭院';done.onclick=()=>{dialog.close();openPage('home')};out.append(h,words,im,result,done);render()}catch(e){toast(e.message)}};
 const accept=document.createElement('button');const updateAccept=()=>{const offer=offers.find(o=>o.next.nationalId===target);accept.disabled=!offer;accept.textContent=offer?'进化为'+offer.next.name+(offer.cost?` · ${offer.cost}金币`:''):'请选择进化方向'};if(offers.length>1){const options=document.createElement('div');options.className='evolution-actions';for(const offer of offers){const option=document.createElement('button');option.textContent=offer.step.item+' → '+offer.next.name;option.onclick=()=>{target=offer.next.nationalId;updateAccept();for(const b of options.children)b.setAttribute('aria-pressed',String(b===option))};options.append(option)}out.append(options)}updateAccept();accept.onclick=()=>decide(false);
 const decline=document.createElement('button');decline.textContent='保持原样 · 永久不进化';decline.onclick=()=>decide(true);actions.append(accept,decline);out.append(actions);
 const back=document.createElement('button');back.textContent='←';back.className='evolution-back';back.setAttribute('aria-label','返回宠物信息');back.onclick=()=>{dialog.close();detail(x)};out.prepend(back);
 }catch(e){toast(e.message)}};out.append(b);
}
const silhouettes=new Map();
function silhouette(path,x,y){const im=load(path);if(!im.complete||!im.naturalWidth)return;if(!silhouettes.has(path)){const c=document.createElement('canvas');c.width=c.height=40;const t=c.getContext('2d');t.imageSmoothingEnabled=false;t.drawImage(im,0,0,40,40);t.globalCompositeOperation='source-in';t.fillStyle='#4c5a68';t.fillRect(0,0,40,40);silhouettes.set(path,c)}ctx.drawImage(silhouettes.get(path),x,y,40,40)}
function drawPetDex(){
 drawMenuBackground();text('宠物图鉴',180,56,18);
 const list=[...manifest.pets].sort((a,b)=>a.nationalId-b.nationalId);
 list.slice(petDexPage*10,petDexPage*10+10).forEach((p,i)=>{const row=i<3?0:i<7?1:2,col=i-(row===0?0:row===1?3:7),w=row===1?52:62,x=(row===1?69:81)+col*(row===1?57:66),y=82+row*68,ax=x+(w-40)/2,owned=browserOwned.has(p.id),avatar=`avatars/${p.id}-0.png`;box(x,y,w,62,owned?'#183b41':'#111f2b',10);ctx.strokeStyle=owned?'#578d79':'#233644';ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(x+.5,y+.5,w-1,61,10);ctx.stroke();if(owned)image(avatar,ax,y+2,40,40);else{silhouette(avatar,ax,y+2);text('?',x+w/2,y+30,18,'#b9c6cc')}text(owned?p.name:String(p.nationalId).padStart(3,'0'),x+w/2,y+57,12,owned?'#d0e6d7':'#778a99')});
 if(list.length>10){text(`${petDexPage+1} / ${Math.ceil(list.length/10)}`,180,291,12,'#6e8a9b');drawSwitchArrow(false);drawSwitchArrow(true)}returnButton();
}

function drawSceneMenu(){
 ctx.clearRect(0,0,360,360);image('ui/menu-bg.png',0,0,360,360);const selected=sceneMenu.selected,state=sceneMenu.state;
 text(manifest.scenes[selected].name,180,86,18,'#f3e7cc');
 ctx.save();ctx.beginPath();ctx.rect(0,108,360,144);ctx.clip();image(`scenes/${selected}.png`,0,0,360,360);ctx.restore();
 drawSwitchArrow(false,158);drawSwitchArrow(true,158);box(102,226,156,20,'#102b34da',10);text(`${String(selected+1).padStart(2,'0')} / 13 · ${state==='active'?'使用中':state==='owned'?'已解锁':'未解锁'}`,180,241,12,'#c5d9c7');
 ctx.globalAlpha=state==='owned'?1:.5;drawFooterText(state==='active'?'使用中':state==='owned'?'使用':`需解锁${sceneMenu.required}种宠物`,88,264,184,28);ctx.globalAlpha=1;returnButton();
}
function drawSceneBackground(){
 const elapsed=page==='animation'?performance.now():homeElapsed;
 if(motion.matches&&!ambientPreview || !scenery.draw(ctx,load(manifest.scenes[scene].image),scene,elapsed))image(manifest.scenes[scene].image,0,0,360,360);
 if(!motion.matches||ambientPreview)drawSceneEffects(ctx,sampleSceneEffects(sceneEffects,scene,ambientSeed,ambientPreview?performance.now()-ambientStart:page==='animation'?performance.now():homeElapsed,ambientPreview),image);
}
function updateSceneEffectControls(){
 const effect=sceneEffects[scene];
 document.querySelector('#scene-name').textContent=manifest.scenes[scene].name;
 document.querySelector('#scene-effect-name').textContent=effect.name;
 document.querySelector('#scene-effect-icon').textContent=['❧','≈','❄','✧','✦','☄','≋','☂','☁','☄','🍁','≈'][effect.kind];
 document.querySelector('#scene-effect-play').setAttribute('aria-pressed',String(ambientPreview));
 document.querySelector('#scene-effect-play-icon').textContent=ambientPreview?'Ⅱ':'▶';
 document.querySelector('#scene-effect-play-label').textContent=ambientPreview?'停止演示':'播放特效';
}
function image(path,x,y,w,h){const im=load(path);if(im.complete&&im.naturalWidth)ctx.drawImage(im,x,y,w,h)}
function drawMenuBackground(){ctx.fillStyle='#09141e';ctx.fillRect(0,0,360,360);image(manifest.menuBackground.image,0,0,360,360)}
function text(s,x,y,size=14,color='#d4dce5',weight=500,family='Noto Sans SC'){ctx.fillStyle=color;ctx.font=`${weight} ${size}px "${family}",sans-serif`;ctx.textAlign='center';ctx.fillText(s,x,y)}
function box(x,y,w,h,color,r=8){ctx.fillStyle=color;ctx.beginPath();ctx.roundRect(x,y,w,h,r);ctx.fill()}
function returnButton(){drawNavArrow(158,298,false)}
function drawStarter(){
 ctx.fillStyle='#080e17';ctx.fillRect(0,0,360,360);text('选择伙伴',180,72,22);text('选一位伙伴，一起开始冒险吧',180,101,14,'#8fa9b7');
 starters.forEach((p,i)=>{const x=30+i*102;box(x,112,96,136,p.dex===starterChoice?'#25576a':'#172e40',14);if(p.dex===starterChoice){ctx.strokeStyle='#a1d5bd';ctx.lineWidth=2;ctx.strokeRect(x+1,113,94,134)}image(`types-large/${['grass','fire','water'][i]}.png`,x+32,120,32,32);image(`starters-large/${String(p.dex).padStart(3,'0')}.png`,x+8,158,80,80)});
 text(starters.find(p=>p.dex===starterChoice).name,180,269,18,'#ffdb93');drawFooterText('出发',116,296,128,32);
}
const settingsSlides=new Map();
let settingsSection=0;
const settingsTitles=['设置','显示设置','场景设置','历练设置'];
function settingsSwitch(label,id,on,y){
 const now=performance.now(),target=on?1:0;let slide=settingsSlides.get(id);
 if(!slide)settingsSlides.set(id,slide={from:target,to:target,start:now});
 let t=Math.min(1,(now-slide.start)/160),v=slide.from+(slide.to-slide.from)*(t*t*(3-2*t));
 if(slide.to!==target){slide.from=v;slide.to=target;slide.start=now}
 if(motion.matches)v=target;
 box(66,y,228,42,'#132d37',14);
 ctx.textAlign='left';ctx.font='500 14px "Noto Sans SC",sans-serif';ctx.fillStyle='#d4e4e3';ctx.fillText(label,82,y+27);
 box(226,y+8,48,26,on?'#68b99b':'#344c59',13);
 ctx.fillStyle='#f0f6ef';ctx.beginPath();ctx.arc(239+22*v,y+21,10,0,Math.PI*2);ctx.fill();
}
function drawDaily(){
 drawMenuBackground();text(page==='settings'?settingsTitles[settingsSection]:pageNames[page],180,56,18);
 if(page==='settings'){
  if(!settingsSection){
   for(let i=1;i<=3;i++){const y=100+(i-1)*56;box(66,y,228,44,'#132d37',14);ctx.textAlign='left';ctx.font='500 14px "Noto Sans SC",sans-serif';ctx.fillStyle='#d4e4e3';ctx.fillText(settingsTitles[i],82,y+28);ctx.strokeStyle='#9cc9b8';ctx.lineWidth=2;ctx.beginPath();ctx.moveTo(270,y+17);ctx.lineTo(275,y+22);ctx.lineTo(270,y+27);ctx.stroke()}
   text(`v${appVersion}`,180,281,12,'#8196a5');
  }else if(settingsSection===1){
   settingsSwitch('显示时间','time',hudSettings.state.showTime,104);
   settingsSwitch('显示金币','coins',hudSettings.state.showCoins,152);
   settingsSwitch('显示电量','battery',hudSettings.state.showBattery,200);
  }else if(settingsSection===2)settingsSwitch('每日随机场景','random',daily.state.random,104);
  else settingsSwitch('历练自动补给','supply',hudSettings.state.trainingAutoSupply,104);
 }

 if(page==='tarot'){const drawn=daily.state.tarotDay===today();image('icons-large/12.png',140,76,80,80);text(drawn?tarotNames[daily.state.card]:'?',180,181,14,'#eac3e1');text(drawn?tarotEffects[daily.state.card]:'每日一牌 · 当天有效',180,208,12,'#a6c6c4');text(drawn?'今日生效 · 跨日结束':'翻牌获 10 金币',180,229,12,'#7c97a4');ctx.globalAlpha=drawn?.5:1;drawFooterText(drawn?'明日再来':'翻牌',116,260,128,32);ctx.globalAlpha=1}
 if(page==='status'){
  const art=previewPets[selectedPet]??previewPets[0],pet=manifest.pets[art];
  const individual=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[selectedPet]);
  const chain=manifest.evolution.chains.find(c=>c.species===pet.species),step=evolutionStep(chain,pet.stage);
  const canEvolve=chain&&(chain.branches?chain.branches.some(b=>b.fromNationalId===pet.nationalId):chain.nationalIds.at(-1)!==pet.nationalId);
  const demoLevel=individual?.level||Math.max(1,chain?.steps?.[pet.stage-1]?.level||1);
  drawPetIdentityCard(art,individual,demoLevel,true);
  // Preset browser care values illustrate the device layout; no device save is read.
  const names=['饱腹','清洁','健康','心情','精力'],icons=[0,1,2,3,14],values=[individual?.food??80,individual?.clean??85,individual?.health??100,individual?.mood??85,individual?.energy??85],colors=['#e3b979','#79bcd6','#e78d9f','#b69bde','#e8d58d'];
  for(let i=0;i<5;i++){
   const x=70+i*45;image(`icons-small/${icons[i]}.png`,x+8,172,24,24);
   text(names[i],x+20,210,12,'#8da6aa');text(values[i],x+20,230,14,colors[i]);
   box(x+4,236,32,4,'#1b3440',2);box(x+4,236,32*values[i]/100,4,colors[i],2);
  }
  if(individual?.locked||!canEvolve)text(individual?.locked?'已选择永久不进化':'最终形态',180,252,12,'#96b7b0');
  else drawFooterText(step.item?`${step.item} · 200币`:`Lv.${step.level}  可突破`)
  drawFooterText('',104,294,36,36);
  ctx.fillStyle='#b9e8d4';
  for(const [x,y,w,h]of [[0,4,18,2],[12,0,2,2],[14,2,2,2],[14,6,2,2],[12,8,2,2],[0,14,18,2],[4,10,2,2],[2,12,2,2],[2,16,2,2],[4,18,2,2]])ctx.fillRect(113+x,302+y,w,h);
  returnButton();
  if(previewPets.length<5){drawFooterText('',220,294,36,36);text('+',238,318,20,'#b9e8d4')}
  return;
 }
 returnButton();
}
function choosePreviewPet(add=false){
 const out=document.querySelector('#detail-content');out.replaceChildren();const h=document.createElement('h2');h.textContent=add?'添伙伴':'更换宠物';out.append(h);
 let list=evolutionDemo.state.pets.map(p=>({id:p.id,art:p.art}));
 for(let i=0;i<previewPets.length;i++){const art=previewPets[i];if(!list.some(p=>p.art===art))list.push({id:previewIndividuals[i]||null,art})}
 if(add)list=list.filter(p=>p.id?!previewIndividuals.includes(p.id):!previewPets.includes(p.art));
 const grid=document.createElement('div');grid.className='roster-grid';
 list.forEach(p=>{const b=document.createElement('button'),im=document.createElement('img');im.src=base+manifest.pets[p.art].frames[0];im.alt='';b.append(im,manifest.pets[p.art].name);b.onclick=()=>{const active=p.id?previewIndividuals.indexOf(p.id):previewPets.indexOf(p.art),target=selectedPet;
 if(add){if(previewPets.length>=5)return;previewPets.push(p.art);previewIndividuals.push(p.id)}
 else if(active>=0){[previewPets[target],previewPets[active]]=[previewPets[active],previewPets[target]];[previewIndividuals[target],previewIndividuals[active]]=[previewIndividuals[active],previewIndividuals[target]]}else{previewPets[target]=p.art;previewIndividuals[target]=p.id}
 try{evolutionDemo.setActive(previewIndividuals);syncCompanions()}catch{}dialog.close();openPage('home');toast('伙伴已换上场')};grid.append(b)});
 if(!add&&previewIndividuals.length>1){const store=document.createElement('button');store.textContent='收入仓库';store.onclick=()=>{try{evolutionDemo.setActive(previewIndividuals.filter((_,i)=>i!==selectedPet));syncCompanions();selectedPet=0;dialog.close();openPage('warehouse')}catch(e){toast(e.message)}};out.append(store)}out.append(grid);const tip=document.createElement('p');tip.textContent=list.length?'仅列出当前演示伙伴和已创建的试玩伙伴。':'还没有候补伙伴';out.append(tip);dialog.showModal();
}

function drawCollection(){
 drawMenuBackground();text('收藏品',180,56,18);
 const list=collectionList(),pages=Math.ceil(list.length/64);dexPage%=pages;
 const q=collectionQuality===6?'全部':manifest.collection.qualities[collectionQuality].name;
 text(`${q} ${list.filter(x=>ownsCollection(x.id)).length}/${list.length}   ${dexPage+1}/${pages}`,180,73,12,'#87a6b9');
 ['全','普','精','稀','史','传','特'].forEach((name,i)=>{const quality=i?i-1:6,c=quality===6?'#d6e3ed':'#'+manifest.collection.qualities[quality].color;ctx.fillStyle=c;if(quality===collectionQuality){ctx.globalAlpha=.2;ctx.fillRect(74+i*31,81,28,24);ctx.globalAlpha=1}text(name,88+i*31,98,12,c)});
 list.slice(dexPage*64,dexPage*64+64).forEach((x,i)=>{const px=80+i%8*25,py=111+Math.floor(i/8)*22,owned=ownsCollection(x.id);ctx.globalAlpha=owned?1:.25;image(x.smallImage,px+1,py-1,20,20);ctx.globalAlpha=1;ctx.fillStyle='#'+manifest.collection.qualities[x.quality].color;ctx.fillRect(px+(owned?7:10),py+19,owned?8:3,2)});
 if(pages>1){drawSwitchArrow(false);drawSwitchArrow(true)}returnButton();
}
const template=await createAnimationPlayer({canvas,revision:assetRevision,drawScene:()=>{ctx.clearRect(0,0,360,360);drawSceneBackground()}});
const clampHomeY=y=>Math.round(Math.max(115,Math.min(scene===12?214:208,y)));
let fishingMode=0,fishingPage=0,fishingDetail=0;
function newFishing(){return createFishing(Math.floor(performance.now()),{onCatch:(id,reward)=>{const granted=evolutionDemo.catchFish(id,reward,coins);coins=evolutionDemo.state.coins;return granted}})}
let fishingPreview=newFishing();
const fishingScenery=createSceneMotion({tickMs:33,scenes:[[]]},(_,t,base,out)=>renderFishingScenery(t,base,out));
let fishingAmbient=0,fishingLast=performance.now();
const fishingPartner=()=>evolutionDemo.state.pets.find(p=>p.id===evolutionDemo.state.fishingPartner);
const fishingReserves=()=>evolutionDemo.state.pets.filter(p=>!evolutionDemo.state.active.includes(p.id)&&!evolutionDemo.trainingBusy(p.id));
function showFishingMode(mode){fishingLast=performance.now();fishingMode=mode;fishingPreview.screen(mode===0&&!document.hidden,performance.now());openPage('fishing',{reveal:false})}
document.addEventListener('visibilitychange',()=>{if(page==='fishing')fishingPreview.screen(!document.hidden&&fishingMode===0,performance.now());fishingLast=performance.now()});
function drawFishingMenu(){
 ctx.fillStyle='#102d3b';ctx.fillRect(0,0,360,360);
 const colors=['#a5d6c0','#8ccae0','#cda9e5','#f0ca75'];
 if(fishingMode===3){const f=fishCatalog[fishingDetail];text('鱼图鉴',180,52,18,'#e5eada');text(`No.${String(f.id+1).padStart(2,'0')} · ${['常见','少见','稀有','珍奇'][f.rarity]}`,180,78,12,colors[f.rarity]);image(f.image,132,100,96,64);text(f.name,180,192,18,'#e5eada');text(`捕获 ${evolutionDemo.state.fishCaught[f.id]} 次 · ${Math.max(2,Math.floor(f.coins/6))} 基础金币`,180,220,12,'#b9cdb7');text(`普通概率 ${(f.weight/10).toFixed(1)}%`,180,244,12,'#9bc1b3');text(`精准概率 ${(Math.floor(fishWeight(f.id,2)*1000/fishWeightTotal(2))/10).toFixed(1)}%`,180,267,12,'#f0d295');returnButton();return}
 const pets=fishingReserves(),total=fishingMode===1?pets.length:fishCatalog.length,pages=Math.ceil(total/6);fishingPage=Math.min(fishingPage,Math.max(0,pages-1));
 text(fishingMode===1?'选择陪钓伙伴':'鱼图鉴',180,52,18,'#e5eada');text(fishingMode===1?`仓库伙伴 · ${pages?fishingPage+1:0} / ${pages}`:`已发现 ${evolutionDemo.state.fishCaught.filter(Boolean).length} / ${fishCatalog.length} · ${fishingPage+1} / ${pages}`,180,77,12,'#9bc1b3');
 if(!total){text('仓库暂无伙伴',180,157,14,'#c9ddd0');text('可在仓库收回场景伙伴',180,182,12,'#9bc1b3')}
 for(let i=0;i<6&&fishingPage*6+i<total;i++){const n=fishingPage*6+i,cols=Math.min(3,total-fishingPage*6-Math.floor(i/3)*3),x=(360-cols*78+6)/2+i%3*78,y=94+Math.floor(i/3)*82;box(x,y,72,76,'#173d40',12);
  if(fishingMode===1){const p=pets[n];image(manifest.pets[p.art].frames[0],x+12,y,48,48);text(shortName(p.nickname||manifest.pets[p.art].name,5),x+36,y+59,12,'#e4eddb');text(p.id===evolutionDemo.state.fishingPartner?'陪钓中':`Lv.${p.level}`,x+36,y+73,12,'#9bd8b4');if(p.id===evolutionDemo.state.fishingPartner){ctx.strokeStyle='#9bd8b4';ctx.lineWidth=2;ctx.strokeRect(x+1,y+1,70,74)}}
  else{const f=fishCatalog[n],seen=evolutionDemo.state.fishCaught[n];ctx.globalAlpha=seen?1:.3;image(`fishing/thumb-${n}.png`,x+12,y+5,48,32);ctx.globalAlpha=1;text(seen?f.name:'???',x+36,y+56,12,colors[f.rarity]);text(`${seen}次`,x+36,y+72,12,'#94baac')}
 }
 if(pages>1){drawSwitchArrow(false);drawSwitchArrow(true)}if(fishingMode===1&&fishingPartner())drawFooterText('暂不陪钓');returnButton();
}
function fishingHits(){
 if(!fishingMode){hit('返回APP',158,298,44,44,()=>openPage('apps'));const b=hit('抛竿或收竿',116,260,128,32,()=>{try{fishingPreview.press(performance.now());drawFishing()}catch(e){toast(e.message)}});b.dataset.fishingAction='true';hit('选择陪钓伙伴',298,136,44,44,()=>{fishingPage=0;showFishingMode(1)});hit('打开鱼图鉴',298,180,44,44,()=>{fishingPage=0;showFishingMode(2)});return}
 hit('返回钓鱼',158,298,44,44,()=>showFishingMode(fishingMode===3?2:0));if(fishingMode===3)return;
 const pets=fishingReserves(),total=fishingMode===1?pets.length:fishCatalog.length,pages=Math.ceil(total/6);
 for(let i=0;i<6&&fishingPage*6+i<total;i++){const n=fishingPage*6+i,cols=Math.min(3,total-fishingPage*6-Math.floor(i/3)*3),x=(360-cols*78+6)/2+i%3*78,y=94+Math.floor(i/3)*82;hit(fishingMode===1?`陪钓：${pets[n].nickname||manifest.pets[pets[n].art].name}`:`鱼图鉴 ${n+1}：${fishCatalog[n].name}`,x,y,72,76,()=>{if(fishingMode===1){try{evolutionDemo.selectFishingPartner(pets[n].id);showFishingMode(0)}catch(e){toast(e.message)}}else{fishingDetail=n;showFishingMode(3)}})}
 if(pages>1)for(const [x,step]of [[18,-1],[298,1]])hit(step<0?'上一页':'下一页',x,158,44,44,()=>{fishingPage=(fishingPage+step+pages)%pages;showFishingMode(fishingMode)});
 if(fishingMode===1&&fishingPartner())hit('暂不陪钓',116,260,128,32,()=>{evolutionDemo.selectFishingPartner(null);showFishingMode(0)});
}
function pondText(s,x,y,size,color){ctx.save();ctx.font=`500 ${size}px "Noto Sans SC",sans-serif`;ctx.textAlign='center';ctx.strokeStyle='#102d3b';ctx.lineWidth=2;ctx.strokeText(s,x,y);text(s,x,y,size,color);ctx.restore()}
function drawFishing(){
 if(fishingMode){drawFishingMenu();return}
 const real=performance.now();fishingAmbient+=Math.max(0,real-fishingLast);fishingLast=real;
 const f=fishingPreview.tick(real),now=motion.matches?1200:fishingAmbient,pet=fishingPartner();
 ctx.clearRect(0,0,360,360);fishingScenery.draw(ctx,load('fishing/pond.png'),0,now);box(126,18,108,20,'#153b3ed7',10);text(`${coins} 金币`,180,32,12,'#e8c57c');
 image('fishing/partner-icon.png',304,142,32,32);image('fishing/dex-icon.png',304,186,32,32);
 box(76,169,30,4,'#153e3c',2);if(pet){const form=manifest.pets[pet.art],p=f.caught?'happy':f.phase===2?'attack':'idle';image(form.frames[petFrame(form.id,p,now,'fishing')],68,125,48,48)}
 const bend=f.phase===2?Math.floor(now/120)%3*2:0,bob=f.phase===1?Math.floor(now/350)%3:f.phase===2?Math.floor(now/100)%5:0;
 box(100,157,8,18,'#776441',2);ctx.beginPath();ctx.moveTo(103,158);ctx.lineTo(146-bend,119+bend);ctx.strokeStyle='#163942';ctx.lineWidth=5;ctx.stroke();ctx.strokeStyle='#cfb374';ctx.lineWidth=2;ctx.stroke();ctx.beginPath();ctx.moveTo(146-bend,119+bend);ctx.lineTo(216,151+bob);ctx.strokeStyle='#e9e3c8';ctx.lineWidth=1;ctx.stroke();
 if(f.caught){box(166,108,108,86,'#0d313cb4',40);image(fishCatalog[f.fish].image,172,119,96,64)}else{box(211,150+bob,10,14,'#f1e8cd',2);box(211,150+bob,10,7,'#e77668',2)}
 pondText(f.phase===0?'准备抛竿':f.phase===1?'等待咬钩…':f.phase===2?'咬钩了！':f.caught?`${fishCatalog[f.fish].name} +${f.reward} 金币`:'鱼溜走了',180,225,14,f.caught?'#f3d18d':'#d5ede7');
 box(76,238,208,16,'#061d27',5);ctx.globalAlpha=f.phase===2?1:.24;box(78+(f.target||50)*204/100-37,240,74,12,'#499e80',2);box(78+(f.target||50)*204/100-8,240,16,12,'#b5e5a0',1);ctx.globalAlpha=1;if(f.phase===2)box(78+f.needle*204/100-2,234,4,24,'#fff0be',1);
 pondText(f.phase===2?'绿色区域收竿':f.phase===1?'水下有鱼在靠近':'',180,204,12,'#d5e9de');returnButton();ctx.globalAlpha=f.phase===1?.6:1;drawFooterText(f.phase===0?'抛竿':f.phase===1?'等待咬钩':f.phase===2?'收竿':'再来一次');ctx.globalAlpha=1;
 const button=hotspots.querySelector('[data-fishing-action]');if(button)button.disabled=f.phase===1;
}
let trainingScene=0,trainingMode=0,trainingSelected=[],trainingList=0,trainingLast=performance.now(),trainingWalkTime=0,trainingAmbient=0,trainingError='',trainingPhase='',trainingHp=[0,0],trainingActors=['',''],trainingEntryTimes=[1000,1000];
const newTraining=()=>createTrainingJob({manifest,autoSupply:()=>hudSettings.state.trainingAutoSupply,temperance:()=>daily.state.tarotDay===today()&&daily.state.card===6,getSupplyInventory:()=>({items:manifest.items.map((_,i)=>i===8?evolutionDemo.state.treeEssences:evolutionDemo.state.trainingItems?.[i]||0)}),getPets:()=>evolutionDemo.state.pets,getActiveIds:()=>evolutionDemo.state.active,commit:(snapshot,reward)=>{evolutionDemo.trainingCommit(snapshot,{...reward,baseCoins:coins});coins=evolutionDemo.state.coins},saved:evolutionDemo.state.training,getInventory:()=>({coins,items:manifest.items.map((_,i)=>i===8?evolutionDemo.state.treeEssences:(evolutionDemo.state.trainingItems?.[i]||0)+(i===7?ballStock:i===6?hatchPreview.boosts:i<5?3:0))}),seed:crypto.getRandomValues(new Uint32Array(1))[0]});
let training=newTraining(),trainingWall=training.job.wallAt,trainingWasBusy=training.job.status===1;
function trainingJobTick(){
 if(training.job.status!==1)return;
 const now=Date.now(),delta=Math.max(0,Math.min(60000,now-trainingWall));
 try{training.advance(delta);trainingWall+=delta;trainingWalkTime=training.job.walkMs+(['search','approach'].includes(training.state.phase)?training.job.pendingMs:0);trainingAmbient=training.job.ambientMs+training.job.pendingMs;trainingError='';
  if(trainingWasBusy&&training.job.status!==1){trainingWasBusy=false;syncCompanions();if(page==='home')openPage('home');toast('历练结束，伙伴已回到庭院')}
 }catch(e){trainingError='保存失败，请重试';toast(e.message)}
}
setInterval(trainingJobTick,1000);
document.addEventListener('visibilitychange',()=>{trainingJobTick();if(document.hidden&&training.job.status===1){try{training.checkpoint()}catch(e){toast(e.message)}}});
const dialog=document.querySelector('#detail');
let trainingNotice=null,trainingNoticeClosing=false;
function trainingNoticeClose(kind,index){try{if(kind)training.ackNotice(kind,index);trainingNotice=null;trainingNoticeClosing=true;dialog.close()}catch(e){toast(e.message)}}
function trainingGrowthOffers(id){try{return evolutionDemo.choices(id).filter(o=>o.step.trigger!=='level'||o.pet.level>=o.step.level)}catch{return []}}
function trainingGrowthTick(){
 if(document.hidden||!['home','training'].includes(page)||dialog.open||trainingNotice||trainingNoticeClosing||!training.pendingGrowth)return;
 const j=training.job;
 for(let i=0;i<training.state.party.length;i++){
  if(j.endLevels[i]<=j.startLevels[i])continue;
  const p=evolutionDemo.state.pets.find(p=>p.id===training.state.party[i].id);if(!p)continue;
  const form=manifest.pets[p.art],out=document.querySelector('#detail-content');
  const title=s=>{const h=document.createElement('h2');h.textContent=s;out.append(h)};
  const picture=()=>{const im=document.createElement('img');im.src=base+form.frames[0];im.alt=p.nickname||form.name;im.className='choice-pet';out.append(im)};
  const button=(name,run)=>{const b=document.createElement('button');b.textContent=name;b.onclick=run;out.append(b);return b};
  if(!(j.growthSeen&(1<<i))){
   out.replaceChildren();trainingNotice={kind:'growth',index:i};title('历练升级 · '+(p.nickname||form.name));picture();const line=document.createElement('p');line.textContent=`Lv.${j.startLevels[i]} → Lv.${j.endLevels[i]}`;out.append(line);button('继续',()=>trainingNoticeClose('growth',i));dialog.showModal();return;
  }
  if(!(j.evolutionSeen&(1<<i))){
   const offers=trainingGrowthOffers(p.id);if(!offers.length){try{training.ackNotice('evolution',i)}catch(e){toast(e.message);return}continue}
   out.replaceChildren();trainingNotice={kind:'evolution',index:i};title('成长的选择 · '+(p.nickname||form.name));picture();let target=offers.length===1?offers[0].next.nationalId:null;
   const warning=document.createElement('p');warning.textContent='保持原样后，这只伙伴将永久不再进化，并获得收藏品「纯净之心」。';out.append(warning);
   const decide=decline=>{try{
    const updated=evolutionDemo.choose(p.id,decline,coins,target);coins=evolutionDemo.state.coins;syncCompanions();if(!decline)unlockBrowserPet(updated.art);
    out.replaceChildren();title(decline?'守护最初的自己':manifest.pets[updated.art].name);const im=document.createElement('img');im.src=`../docs/screenshots/${decline?`evolution-decline-${form.species}-${form.stage}`:`evolution-${form.species}-${offers.length>1?target:form.stage}`}.gif?v=${assetRevision}`;im.alt=decline?'保持原样与纯净之心赠礼动画':'伙伴进化动画';out.append(im);
    trainingNotice={kind:'result',index:i,id:p.id};button('继续',()=>trainingNoticeClose(trainingGrowthOffers(p.id).length?null:'evolution',i));
   }catch(e){toast(e.message)}};
   const yes=button(offers[0].cost?'进化 · 200金币':'进化',()=>decide(false));yes.disabled=target===null;
   if(offers.length>1)for(const o of offers)button(`${o.step.item} → ${o.next.name}`,()=>{target=o.next.nationalId;yes.disabled=false});
   button('保持原样 · 永久',()=>decide(true));button('稍后再说',()=>trainingNoticeClose('evolution',i));dialog.showModal();return;
  }
 }
}
function trainingRefresh(){hotspots.replaceChildren();trainingHits()}
function trainingBack(){if(trainingMode===2||trainingMode===4){try{training.checkpoint()}catch(e){toast(e.message);return}openPage('apps')}else if(trainingMode===1){trainingMode=0;trainingRefresh()}else openPage('apps')}
function trainingExit(){try{training.finish();trainingWasBusy=false;trainingMode=4;syncCompanions();trainingRefresh();toast('历练已结算，伙伴已回到庭院')}catch(e){toast('保存失败，请重试退出')}}
function trainingHits(){
 hit(trainingMode===2?'后台历练':trainingMode===4?'返回APP':trainingMode===1?'返回场景选择':'返回APP',158,trainingMode===2?288:298,44,44,trainingBack);
 if(trainingMode===2)hit('退出历练并立即结算',208,288,44,44,trainingExit);
 if(trainingMode===4){hit('再次历练',116,260,128,32,()=>{trainingMode=0;trainingRefresh()})}
 else if(trainingMode===0){
  for(const [x,d]of [[18,-1],[298,1]])hit(d>0?'下一个历练场景':'上一个历练场景',x,158,44,44,()=>{trainingScene=(trainingScene+d+6)%6;trainingRefresh()});
  hit('选择历练伙伴',116,260,128,32,()=>{trainingMode=1;trainingList=0;trainingError='';trainingRefresh()});
 }else if(trainingMode===1){
  const pets=evolutionDemo.state.active.map(id=>evolutionDemo.state.pets.find(p=>p.id===id)),pages=Math.max(1,Math.ceil(pets.length/9));trainingList%=pages;
  pets.slice(trainingList*9,trainingList*9+9).forEach((p,i)=>{const n=Math.min(9,pets.length-trainingList*9),row=Math.floor(i/3),cols=Math.min(3,n-row*3),x=(360-cols*76+4)/2+i%3*76,y=88+row*54;
   hit((p.nickname||manifest.pets[p.art].name)+' Lv.'+p.level+(evolutionDemo.state.active.includes(p.id)?' · 场景内':' · 仓库'),x,y,72,52,()=>{trainingError='';const at=trainingSelected.indexOf(p.id);if(at>=0)trainingSelected.splice(at,1);else if(trainingSelected.length<3)trainingSelected.push(p.id);else trainingError='最多选择3位伙伴';trainingRefresh()});
  });
  if(pages>1)for(const [x,d]of [[18,-1],[298,1]])hit(d>0?'下一页伙伴':'上一页伙伴',x,158,44,44,()=>{trainingList=(trainingList+d+pages)%pages;trainingRefresh()});
  hit('进入历练',116,260,128,32,()=>{try{if(trainingSelected.some(id=>!evolutionDemo.state.active.includes(id)))throw Error('仅限庭院内的伙伴');if(trainingSelected.includes(evolutionDemo.state.fishingPartner))throw Error('请先更换陪钓伙伴');training.start(trainingScene,trainingSelected);trainingWall=Date.now();trainingWasBusy=true;trainingMode=2;trainingActors=['',''];trainingEntryTimes=[1000,1000];trainingWalkTime=trainingAmbient=0;trainingLast=performance.now();trainingError='';trainingPhase=training.state.phase;trainingRefresh()}catch(e){trainingError=e.message;trainingRefresh()}});
 }
}
const trainingBlurCache=new Map();
function trainingBackground(blurred){
 const path=`training/${trainingScene}.png`,source=load(path);
 if(!blurred){
  box(0,0,360,360,'#102b34',0);
  if(source.complete&&source.naturalWidth){
   const moving=['search','approach','alert'].includes(training.state.phase),offset=moving?Math.floor((motion.matches?0:trainingWalkTime)/25)%360:0;
   ctx.save();ctx.beginPath();ctx.rect(0,108,360,120);ctx.clip();
   ctx.drawImage(source,0,54,180,60,-offset,108,360,120);
   if(offset)ctx.drawImage(source,0,54,180,60,360-offset,108,360,120);
   ctx.restore();
  }return
 }
 if(!source.complete||!source.naturalWidth)return;
 if(!trainingBlurCache.has(trainingScene)){
  const layer=document.createElement('canvas');layer.width=layer.height=360;const c=layer.getContext('2d');
  c.imageSmoothingEnabled=false;c.drawImage(source,0,0,360,360);
  c.filter='blur(8px)';c.drawImage(layer,-6,-6,372,372);c.filter='none';
  c.fillStyle='rgba(10,33,38,.25)';c.fillRect(0,0,360,360);trainingBlurCache.set(trainingScene,layer);
 }
 ctx.drawImage(trainingBlurCache.get(trainingScene),0,0);
}
function trainingExitIcon(){
 const cell=(x,y)=>x>=0&&x<16&&y>=0&&y<16&&(trainingExitRows[y]&(1<<x));
 for(let pass=0;pass<2;pass++)for(let y=0;y<16;y++)for(let x=0;x<16;x++)if(cell(x,y)){
  const edge=!cell(x-1,y)||!cell(x+1,y)||!cell(x,y-1)||!cell(x,y+1);
  ctx.fillStyle=!pass?'#112e36':edge?'#285448':!cell(x,y-2)?'#e2ffe5':!cell(x,y+2)?'#69b69a':'#a9e4bf';ctx.fillRect(214+x*2,293+y*2+(pass?0:2),2,2);
 }
}
function trainingElapsedLabel(ms){const seconds=Math.ceil(ms/1000);if(seconds<60)return seconds+'秒';const minutes=Math.ceil(seconds/60);return minutes%60?minutes+'分钟':minutes/60+'小时'}
function trainingProgress(){const seconds=Math.max(0,Math.ceil((trainingDurationMs-training.job.elapsedMs)/1000)),time=[Math.floor(seconds/3600),Math.floor(seconds/60)%60,seconds%60].map(n=>String(n).padStart(2,'0')).join(':');text(`${time} · 金币 +${training.job.coins} · 物品 ${training.job.items.reduce((a,b)=>a+b,0)}`,180,279,12,'#d4dbb8')}
function trainingSceneryDraw(time,front){trainingScenery(trainingScene,time,front,(_,x,y,w,h,c,a)=>{ctx.globalAlpha=a/255;box(x,y,w,h,'#'+c.toString(16).padStart(6,'0'),0)});ctx.globalAlpha=1}

const trainingEntryCanvas=document.createElement('canvas');trainingEntryCanvas.width=trainingEntryCanvas.height=48;
function trainingEntryPet(frame,side,hop,entry){
 const source=load(frame);if(!source.complete||!source.naturalWidth)return;
 let sprite=source;
 if(entry.white){const c=trainingEntryCanvas.getContext('2d');c.clearRect(0,0,48,48);c.globalCompositeOperation='source-over';c.drawImage(source,0,0,48,48);c.globalCompositeOperation='source-atop';c.fillStyle=`rgba(255,255,255,${entry.white/255})`;c.fillRect(0,0,48,48);c.globalCompositeOperation='source-over';sprite=trainingEntryCanvas}
 ctx.save();ctx.globalAlpha=entry.opacity/255;
 if(side){ctx.translate(308-hop,160+entry.drop);ctx.scale(-1,1);ctx.drawImage(sprite,0,0,56,56)}else ctx.drawImage(sprite,52+hop,160+entry.drop,56,56);
 ctx.restore();
}
function drawTraining(){
 const now=performance.now(),delta=Math.max(0,Math.floor(now-trainingLast));trainingLast+=delta;const h=training.state,s=trainingScenes[trainingScene];
 if(trainingMode===2){if(training.job.status===2){trainingMode=4;trainingRefresh()}if(trainingPhase!==h.phase){trainingPhase=h.phase;trainingRefresh()}}
 ctx.clearRect(0,0,360,360);trainingBackground(trainingMode<2||trainingMode===4);
 if(trainingMode===4){
  box(112,44,16,1,'#e8c98ea0',0);box(232,44,16,1,'#e8c98ea0',0);text('历练成果',180,47,18,'#ffe8b1');text(`${s.name} · ${trainingElapsedLabel(training.job.elapsedMs)}`,180,68,12,'#c0d3cb');
  const j=training.job,stats=[['战斗','icons-small/9.png',j.status===2?h.rounds:0,'#ecc68b'],['经验','icons-small/14.png','+'+h.totalXp,'#b5e2c7'],['金币','hud/coin-large.png','+'+j.coins,'#ffdb80'],['物品','icons-small/6.png',j.items.reduce((a,b)=>a+b,0),'#b8d7ee']];
  stats.forEach(([name,icon,value,tint],i)=>{const x=64+i%2*120,y=76+Math.floor(i/2)*44;box(x,y,112,40,'#102b39eb',8);ctx.strokeStyle=tint+'34';ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(x+.5,y+.5,111,39,8);ctx.stroke();image(icon,x+8,y+8,24,24);ctx.textAlign='left';ctx.fillStyle='#a4babc';ctx.font='500 12px "Noto Sans SC",sans-serif';ctx.fillText(name,x+40,y+14);ctx.fillStyle=tint;ctx.font=`500 ${String(value).length>6?14:18}px "Noto Sans SC",sans-serif`;ctx.fillText(value,x+40,y+33)});
  box(76,166,208,1,'#a7c5bb41',0);const found=j.items.map((qty,i)=>({qty,i})).filter(p=>p.qty);
  if(found.length){box(52,171,256,83,'#102b39d7',8);found.forEach(({qty,i},n)=>{const row=Math.floor(n/3),cols=Math.min(3,found.length-row*3),x=(360-cols*84)/2+n%3*84,y=174+row*26;image(trainingItemIcon(manifest.items[i],i),x,y+1,24,24);ctx.textAlign='left';ctx.font='500 12px "Noto Sans SC",sans-serif';ctx.fillStyle='#d9e7df';ctx.fillText(manifest.items[i].name,x+26,y+11);ctx.fillStyle='#f5dca3';ctx.fillText('×'+qty,x+26,y+24)})}
  else{image('icons-small/6.png',168,186,24,24);text('途中没有发现物品',180,231,12,'#b4c8c5')}
  drawFooterText('再次历练');returnButton();return;
 }
 if(trainingMode===2&&['fight','result'].includes(h.phase)){
  box(0,0,360,360,'#102b39ed',0);text(s.name,180,52,18,'#ddd8b7');
  ctx.save();ctx.beginPath();ctx.rect(0,108,360,112);ctx.clip();image(`training/${trainingScene}.png`,0,0,360,360);ctx.restore();
  [h.party,h.foes].forEach((team,side)=>{const index=side?h.enemy:h.ally,p=team[index],actor=side+'-'+index+'-'+h.rounds;
   if(trainingActors[side]!==actor){trainingHp[side]=p.maxHp;trainingActors[side]=actor;trainingEntryTimes[side]=0}
   const impact=(h.fx?.at??0)*2.25,ready=trainingEntryTimes.every(t=>t>=battleEntryDuration);
   if(ready&&h.fx&&impact>=(side?360:1260))trainingHp[side]=Math.max(p.hp,trainingHp[side]-p.maxHp*Math.min(delta,250)*2.25/220);
   drawBattleHp(ctx,side,trainingHp[side],p.maxHp);
   drawBattleTypes(ctx,side,p.art);
   text(`${Math.ceil(trainingHp[side])} / ${p.maxHp}`,side?258:102,107,12,'#9fb7bd');team.forEach((pet,i)=>drawBattleBall(ctx,(side?264:52)+i*16,65,i===index?trainingHp[side]>0:pet.hp>0,index===i));
   const fx=h.fx,age=(fx?.at??2000)*2.25,acting=age<900?0:1,attack=fx&&acting===side&&age<1800&&trainingEntryTimes.every(t=>t>=battleEntryDuration),frame=manifest.pets[p.art].frames[petFrame(p.art,trainingHp[side]<=0?'sleep':attack?'attack':'idle',trainingAmbient*2.25,'training-battle-'+side)];
   const hop=motion.matches?0:attack?Math.round(Math.sin(age/180)*4):0;
   const entry=battleEntryState(motion.matches?battleEntryDuration:trainingEntryTimes[side]);trainingEntryPet(frame,side,hop,entry);
   if(!motion.matches)battleEntryPaint(side,trainingEntryTimes[side],(_,x,y,w,h,c,a)=>{ctx.globalAlpha=a/255;box(x,y+64,w,h,'#'+c.toString(16).padStart(6,'0'),0)});ctx.globalAlpha=1;
   trainingEntryTimes[side]=Math.min(battleEntryDuration,trainingEntryTimes[side]+Math.min(delta,250)*1.5);
   text((p.nickname||p.form.name)+' Lv.'+p.level,side?272:90,236,12,'#d4e0d5');
  });
  if(h.fx&&!motion.matches&&trainingEntryTimes.every(t=>t>=battleEntryDuration)){const fx=h.fx,fxAge=fx.at*2.25,reply=fxAge>=900,type=reply?fx.reply?.type:fx.type,age=reply?fxAge-900:fxAge;if(type&&age>=0&&age<720){const map={'电':0,'草':1,'火':2,'水':3},typeIndex=map[type];if(typeIndex!==undefined){const source=reply?280:80,dest=reply?80:280;image(`effects/${typeIndex}-${Math.floor(age/180)}.png`,source+(dest-source)*Math.min(1,age/360)-24,164,48,48)}else{ctx.globalAlpha=Math.max(0,1-age/720);for(let k=0;k<9;k++)box((reply?80:280)+Math.cos(k)*age/15,183+Math.sin(k)*age/18,3,3,'#f7d797',0);ctx.globalAlpha=1}}}
  drawBattleRelations(ctx,manifest,h,trainingEntryTimes.every(t=>t>=battleEntryDuration));
  text(trainingError||h.story,180,264,12,'#e5e0bf');trainingProgress();drawNavArrow(158,288,false);trainingExitIcon();return;
 }
 const theme=trainingThemes[trainingScene],panel='#'+theme[0].toString(16).padStart(6,'0'),accent='#'+theme[1].toString(16).padStart(6,'0');
 box(108,49,18,1,accent+'6e',0);box(234,49,18,1,accent+'6e',0);
 text(trainingMode===1?'选择伙伴':s.name,180,55,18,'#f3edcf');
 if(trainingMode===0){box(110,65,140,20,panel+'aa',10);ctx.strokeStyle=accent+'2d';ctx.lineWidth=1;ctx.beginPath();ctx.roundRect(110.5,65.5,139,19,10);ctx.stroke();text(`Lv.${s.minLevel}–${s.maxLevel} · ${s.pool.length}种`,180,81,12,'#d6e1cb')}
 else text(trainingMode===1?(trainingError||`已选 ${trainingSelected.length} / 3 · 仅庭院伙伴`):`Lv.${s.minLevel}–${s.maxLevel} · ${s.search}`,180,81,12,'#b9cfbe');
 if(trainingMode===0){s.pool.slice(0,6).forEach((dex,i)=>{const p=manifest.pets.find(p=>p.nationalId===dex),x=80+i%3*76,y=99+Math.floor(i/3)*70;box(x-3,y-3,66,66,panel+'87',8);image(p.frames[0],x+6,y,48,48);text(p.name,x+30,y+58,12,'#e0e3ca')});drawFooterText('选择伙伴');drawSwitchArrow(false);drawSwitchArrow(true)}
 else if(trainingMode===1){const pets=evolutionDemo.state.active.map(id=>evolutionDemo.state.pets.find(p=>p.id===id)),n=Math.min(9,pets.length-trainingList*9);pets.slice(trainingList*9,trainingList*9+9).forEach((p,i)=>{const row=Math.floor(i/3),cols=Math.min(3,n-row*3),x=(360-cols*76+4)/2+i%3*76,y=88+row*54,order=trainingSelected.indexOf(p.id)+1;box(x,y,72,52,order?'#477352ee':'#163d43ee',8);image(`avatars/${p.art}-0.png`,x,y-3,40,40);text(p.nickname||manifest.pets[p.art].name,x+36,y+46,12,'#e0e3ca');text('Lv.'+p.level,x+51,y+22,12,order?'#ffedab':'#bad2cd')});if(!pets.length)text('暂无可出战伙伴',180,167,14,'#eed7ac');if(pets.length>9){drawSwitchArrow(false);drawSwitchArrow(true)}drawFooterText('进入历练')}
 else{
  const time=motion.matches?1200:trainingAmbient,walkTime=motion.matches?0:trainingWalkTime;trainingSceneryDraw(time,false);
  const actors=h.party.map((p,i)=>({...trainingPosition(trainingScene,i,h.party.length,walkTime),p,i}));
  for(const {p,i,x,y,left} of actors){
   ctx.fillStyle='rgba(18,37,42,.18)';ctx.beginPath();ctx.ellipse(x+24,y+42,13,2.5,0,0,Math.PI*2);ctx.fill();
   const frame=manifest.pets[p.art].frames[petFrame(p.art,h.phase==='alert'?'idle':'walk',h.phase==='alert'?0:Math.floor(walkTime*.75)+i*170,'training-search-'+i)];image(left?frame.replace('pets/','pets-left/'):frame,x,y,48,48);
  }
  if(['approach','alert'].includes(h.phase)){
   const p=h.foes[0],x=354-Math.floor((h.phase==='alert'?trainingApproachMs:h.approachTime+training.job.pendingMs)*144/trainingApproachMs);
   ctx.fillStyle='rgba(18,37,42,.18)';ctx.beginPath();ctx.ellipse(x+24,196,13,2.5,0,0,Math.PI*2);ctx.fill();
   const frame=manifest.pets[p.art].frames[petFrame(p.art,'idle',time,'training-visible-foe')];image(frame.replace('pets/','pets-left/'),x,154,48,48);
  }
  trainingSceneryDraw(time,true);
  if(!motion.matches&&['search','approach'].includes(h.phase))trainingBattleElements(trainingScene,trainingWalkTime,(_,x,y,w,h,c,a)=>{ctx.globalAlpha=a/255;box(x,y,w,h,'#'+c.toString(16).padStart(6,'0'),0)});
  ctx.globalAlpha=1;
  box(48,229,264,57,panel+'b4',8);text(`战斗 ${h.rounds} 次 · 经验 +${h.totalXp} XP`,180,246,14,'#f3e2b8');const found=trainingDiscovery(training.job,trainingScene,manifest.items);text(found,180,263,12,'#d4dbb8');trainingProgress();
  for(const {p,x,y} of actors){
   const current=evolutionDemo.state.pets.find(pet=>pet.id===p.id)||p;box(x+2,y+45,44,16,panel+'d2',8);text('Lv.'+current.level,x+24,y+57,12,'#ffe9af');
   if(h.phase==='alert'){box(x+14,y-24,20,22,'#ffe6a2',8);box(x+22,y-4,5,5,'#ffe6a2',2);text('!',x+24,y-7,18,'#69422c',700)}
  }

 }
 if(trainingMode===2){drawNavArrow(158,288,false);trainingExitIcon()}else returnButton();
}

let hospitalSelected=0;let hospital=createHospital(1),hospitalLast=performance.now(),hospitalShown=0,hospitalPatients=[],hospitalError='',hospitalAmbient=0;
const hospitalFields=['food','clean','health','mood','energy'];
function hospitalRefresh(){hotspots.replaceChildren();hospitalHits()}
function hospitalClaim(){
 try{hospitalPatients=hospital.claim(targets=>evolutionDemo.hospitalRecover(targets));hospitalError='';syncCompanions();hospitalRefresh()}
 catch(e){hospitalError='保存失败，请重试领取';toast(e.message);hospitalRefresh()}
}
function hospitalHits(){
 hit('返回APP',158,298,44,44,()=>openPage('apps'));
 const h=hospital.state;
 if(h.phase!==1&&h.phase!==2){const ids=evolutionDemo.state.active;hospitalSelected%=Math.max(1,ids.length);hit('选择治疗伙伴',88,222,184,28,()=>{hospitalSelected=(hospitalSelected+1)%Math.max(1,ids.length);hospitalRefresh()});hit('治疗 20金币',184,260,108,32,()=>{try{evolutionDemo.hospitalTreat(ids[hospitalSelected]);coins=evolutionDemo.state.coins;hospitalError='健康已恢复';syncCompanions();hospitalRefresh()}catch(e){toast(e.message)}})}
 if(h.phase===1){['左侧轨道','中间轨道','右侧轨道'].forEach((name,i)=>hit(name,68+i*76,104,72,142,()=>hospital.select(i)));return}
 const name=h.phase===2?(h.claimed?'完成':'领取护理'):h.phase===3?'重新开始':'开始护理';
 hit(name,h.phase===2?116:72,260,h.phase===2?128:108,32,()=>{
  if(h.phase===2){if(h.claimed)openPage('apps');else hospitalClaim();return}
  if(previewDrop){toast('伙伴忙碌中，请稍后再来');return}
  if(!evolutionDemo.state.active.some(id=>id!==evolutionDemo.state.traveler)){toast('暂时没有可护理的伙伴');return}
  hospital.start();hospitalLast=performance.now();hospitalShown=1;hospitalError='';hospitalRefresh();
 });
}
const hospitalHeart=['01100110','11111111','11111111','11111111','01111110','00111100','00011000'];
const hospitalCloud=['000011110000','001111111000','011111111100','111111111110','111111111111','011111111110','000010010000','000100100000'];
const hospitalFurnitureColors=hospitalScenery.rectangles.map(r=>'#'+r[4].toString(16).padStart(6,'0'));
function drawHospitalClinic(h){
 const time=motion.matches?1200:hospitalAmbient;
 hospitalScenery.rectangles.forEach(([x,y,w,ht],i)=>box(x,y,w,ht,hospitalFurnitureColors[i],0));
 for(const [path,x,y]of hospitalScenery.supplies)image(path,x,y,32,32);
 ctx.save();ctx.globalAlpha=(120+(Math.floor(time/400)%2)*135)/255;box(280,110,2,2,'#b4f0c9',0);ctx.restore();
 const patients=evolutionDemo.state.active.filter(id=>id!==evolutionDemo.state.traveler).map(id=>evolutionDemo.state.pets.find(p=>p.id===id)).filter(Boolean).slice(0,3);
 patients.forEach((p,i)=>{
  const [px,py]=hospitalScenery.patients[i],action=h.phase===2?'happy':'sleep';
  image(manifest.pets[p.art].frames[petFrame(p.art,action,time,'hospital-patient-'+i)],px,py,48,48);
  for(let k=0;k<3;k++){
   const age=(Math.floor(time)+i*470+k*610)%2400,x=px-12+8+(i*13+k*19)%52,y=py-24+62-Math.floor(age*44/2400),alpha=age<400?Math.floor(age*180/400):age>1800?Math.floor((2400-age)*180/600):180;
   ctx.save();ctx.globalAlpha=alpha/255;
   if(k===1){for(let row=0;row<7;row++)for(let col=0;col<8;col++)if(hospitalHeart[row][col]==='1')box(x+col,y+row,1,1,'#ffc0cb',0)}
   else{box(x+2,y,2,8,k?'#f3e2a4':'#a2edcf',0);box(x,y+3,6,2,k?'#f3e2a4':'#a2edcf',0)}ctx.restore();
  }
 });
}
function drawHospital(){
 const now=performance.now(),delta=Math.max(0,Math.floor(now-hospitalLast));hospitalLast=now;
 hospital.advance(delta);const h=hospital.state;if(h.phase!==1)hospitalAmbient+=delta;
 if(h.phase!==hospitalShown){hospitalShown=h.phase;if(h.phase===2)hospitalClaim();else hospitalRefresh()}
 ctx.clearRect(0,0,360,360);image('hospital/room.png',0,0,360,360);text('宠物医院',180,55,18,'#f4eed9');
 if(h.phase===1){
  box(54,66,252,26,'#123940e1',10);text(`爱心 ${h.score} / 6 · 还剩 ${Math.ceil((hospitalRounds*hospitalSlot-h.elapsed)/1000)} 秒`,180,84,14,'#c3e4d2');
  box(64,98,232,148,'#123940d2',12);
  for(let i=0;i<3;i++)for(let y=108;y<220;y+=12)box(103+i*76,y,2,4,'#426e6b',0);
  if(!h.resolved){const cloud=hospital.cloud(),mask=cloud?hospitalCloud:hospitalHeart,s=cloud?2:3,x0=92+h.lane*76,y0=106+Math.floor(116*(h.elapsed-h.slot*hospitalSlot)/hospitalFall);
   for(let y=0;y<mask.length;y++)for(let x=0;x<mask[y].length;x++)if(mask[y][x]==='1'){
    const edge=!x||x+1===mask[y].length||!y||y+1===mask.length||mask[y][x-1]!=='1'||mask[y][x+1]!=='1'||mask[y-1][x]!=='1'||mask[y+1][x]!=='1';
    box(x0+x*s,y0+y*s,s,s,edge?'#183b43':cloud?'#8998b3':y<3?'#ffb5ac':'#f0808d',0);
   }
  }
  const x=80+h.basket*76;box(x+2,226,44,18,'#173e43',0);box(x,222,48,6,'#e9c986',0);box(x+4,228,40,12,'#8bd3b4',0);box(x+8,230,32,4,'#c9eed5',0);box(x+18,229,12,8,'#f5e3b3',0);
  text('点击轨道移动',180,273,12,'#f4e5bf');
 }else{
  drawHospitalClinic(h);image('hospital/joy.png',132,88,96,128);
  if(h.phase===2){
   box(132,222,96,24,'#123940e1',10);text(h.claimed?'健康已护理':hospitalError,180,239,14,'#f0e6cb');drawFooterText(h.claimed?'完成':'领取护理');
  }else{
   text(hospitalError||'免费健康至60 · 治疗至100',180,84,12,'#eed59c');const p=evolutionDemo.state.pets.find(p=>p.id===evolutionDemo.state.active[hospitalSelected]);drawFooterText(p?`${p.nickname||manifest.pets[p.art].name} · 健康 ${p.health}`:'暂无伙伴',88,222,184,28);drawFooterText(h.phase===3?'重新护理':'免费护理',72,260,108,32);drawFooterText('治疗 20金币',184,260,108,32);
  }
 }
 returnButton();
}
document.addEventListener('visibilitychange',()=>{hospitalLast=trainingLast=performance.now()});

function draw(){
 trainingJobTick();trainingGrowthTick();
 if(page!=='home'&&page!=='wildalert' || document.hidden || !hudSettings.state.showBattery || document.querySelector('.overview').hidden)batteryLast=performance.now();
 if(document.querySelector('.overview').hidden){homeLast=hospitalLast=trainingLast=performance.now();return}
 if(page==='scenes'){if(!document.hidden)drawSceneMenu();return}
 if(page==='training'){if(!document.hidden)drawTraining();else trainingLast=performance.now();return}
 if(page==='hospital'){if(!document.hidden)drawHospital();else hospitalLast=performance.now();return}
 if(page==='intro'){if(!document.hidden)drawIntro();return}
 if(page==='animation'){if(!document.hidden)template.draw(performance.now());return}
 if(page==='fishing'){if(!document.hidden)drawFishing();return}
 if(document.hidden||page!=='home'&&page!=='wild'&&page!=='wildalert'&&page!=='starter'&&page!=='collection'&&page!=='pokedex'&&page!=='settings'&&page!=='tarot'&&page!=='status'&&page!=='warehouse'&&page!=='individual'&&page!=='bag'&&page!=='shop')return;
 ctx.clearRect(0,0,360,360);
 if(page==='bag'||page==='shop'){drawItemGrid();return}
 if(page==='warehouse'||page==='individual'){drawWarehouse();return}
 if(page==='settings'||page==='tarot'||page==='status'){drawDaily();return}
 if(page==='pokedex'){drawPetDex();return}if(page==='starter'){drawStarter();return}if(page==='collection'){drawCollection();return}
 const real=performance.now(),homeDelta=Math.max(0,real-homeLast);homeElapsed+=homeDelta;homeLast=real;
 const now=homeElapsed;
 drawSceneBackground();
 if(!homeConflict.state.active&&!previewDrop&&now>=talkUntil&&now>=talkAt&&previewPets.length){talkPet=(talkPet+1)%previewPets.length;talkAt=now+32000+talkPet*2900;const p=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[talkPet]);if(!p?.sleeping&&!evolutionDemo.trainingBusy(p?.id))homeSay(talkPet,talkContext(scene,p?.food??80,p?.clean??80,p?.health??80,p?.mood??80,p?.energy??80,petTalkState(talkPet).sequence++))}
 if(previewDrop&&now-previewDrop.start>=4000){const {target,item}=previewDrop,m=homeOrigins[target];if(m){m.ox=m.x;m.oy=m.y;m.start=now-(scene===12?lakeNearestPhase(m.x,m.y):0)}previewDrop=null;patPet=target;patUntil=now+1400;homeSay(target,itemTalk(item),3600);careSpeech=true}
 const positions=[[89,128],[238,179],[163,227],[189,80],[65,213]];
 if(homeBehavior.scene!==scene)homeBehavior=createHomeBehavior(homeBehavior.rng,now,scene);
 // Initialize actors at the same clamped ground positions as the native renderer.
 previewPets.forEach((id,i)=>{if(!homeOrigins[i]||homeOrigins[i].id!==id){homeBehavior.clear();const {x,y}=groundProject(scene,positions[i][0],clampHomeY(positions[i][1])),p=homeScenePatrol(scene,i*2900,i,false,x,y,false);homeOrigins[i]={id,ox:x,oy:y,x:p.x,y:p.y,start:now-i*2900,left:p.left,tired:false,sleeping:false}}});
 const actors=homeActors();updateConflict(now,actors);
 if(welcomeDue&&!previewDrop&&page==='home'){
  const candidates=actors.map((p,i)=>({i,d:Math.abs(p.x-156)+Math.abs(p.y-198),available:p.available&&p.energy>=30})).filter(p=>p.available).sort((a,b)=>a.d-b.d);
  if(candidates.length){if(homeBehavior.request(now,homeKinds.welcome,candidates[0].i,0,156,198,actors))homeSay(candidates[0].i,talkKinds.welcome);welcomeDue=false;homeWelcomeAt=real}
 }
 const interactions=homeBehavior.tick(now,actors,page==='home'&&!previewDrop&&!motion.matches);
 petRects=previewPets.map((id,i)=>{
  const m=homeOrigins[i],individual=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[i]);
  if(evolutionDemo.trainingBusy(individual?.id))return {x:-100,y:-100,w:0,h:0,id};
  const sleeping=!!individual?.sleeping,tired=actors[i].energy<30;
  if(m.tired!==tired||m.sleeping!==sleeping){m.ox=m.x;m.oy=m.y;m.start=now-(scene===12?lakeNearestPhase(m.x,m.y):0);m.tired=tired;m.sleeping=sleeping}
  if(pointer?.pet===i)m.start+=homeDelta;
  const baseline=homeScenePatrol(scene,Math.round(now-m.start),i,tired,m.ox,m.oy,m.left);
  let {x,y,left}=baseline,action=baseline.pose===4&&!motion.matches?'walk':'idle';
  if(motion.matches){x=m.x;y=m.y;left=m.left}
  if(scene===12&&!motion.matches){const p=lakePatrol(now-m.start);x=p.x;y=p.y;left=p.left;action='walk'}
  if(now<patUntil&&(i===patPet||patPet===-2&&patTargets.has(i)))action='happy';
  if(sleeping||pointer?.pet===i){x=m.x;y=m.y;left=m.left;action=sleeping?'sleep':'idle'}
  const interaction=interactions[i];
  if(interaction.active&&(!m.interacting||m.behaviorStart!==homeBehavior.pets[i].start))homeClocks.delete(i);
  if(m.interacting&&!interaction.active&&!interaction.release)Object.assign(interaction,{release:true,x:m.x,y:m.y,left:m.left});
  m.interacting=interaction.active;m.behaviorStart=homeBehavior.pets[i].start;
  if(interaction.active||interaction.release){x=interaction.x;y=interaction.y;left=interaction.left;if(interaction.active)action={0:'idle',4:'walk',8:'happy'}[interaction.pose];m.ox=x;m.oy=y;m.start=now-(scene===12&&!interaction.active?lakeNearestPhase(x,y):0)}
  else if(action==='happy'||pointer?.pet===i){x=m.x;y=m.y;m.ox=x;m.oy=y;m.start=now-(scene===12?lakeNearestPhase(x,y):0)}
  if(previewDrop&&i===previewDrop.target){const elapsed=Math.min(2400,Math.round(now-previewDrop.start));({x,y,left}=routeSample(scene,previewDrop.from.x,previewDrop.from.y,previewDrop.x-8,previewDrop.y-16,elapsed,2400,left));action=elapsed<2400?'walk':'eat'}
  if(scene===12&&!interaction.active&&!(previewDrop&&i===previewDrop.target)&&!motion.matches){const dx=x-m.x,dy=y-m.y,span=Math.max(Math.abs(dx),Math.abs(dy)),limit=Math.max(1,Math.ceil(homeDelta*40/1000));if(span>limit){x=m.x+Math.trunc(dx*limit/span);y=m.y+Math.trunc(dy*limit/span)}}
  if(individual?.sleeping){x=m.x;y=m.y;action='sleep'}
  const conflict=homeConflict.state,side=i===conflict.target?1:0,conflictPet=conflict.active&&(i===conflict.attacker||i===conflict.target),age=now-conflict.started;
  if(conflictPet){x=conflictOrigins[side].x;y=conflictOrigins[side].y;left=x>conflictOrigins[1-side].x;action=side?'idle':'attack';if(side&&conflict.hit&&age<1050)x+=Math.floor(age/65)%2?2:-2}
  ({x,y,left}=groundAdvance(scene,m.x,m.y,x,y,Math.ceil(homeDelta*40/1000)+1,left));
  const path=manifest.pets[id].frames[petFrame(id,action,now,i)].replace('pets/',left?'pets-left/':'pets/');
  m.x=x;m.y=y;m.left=left;
  return {x,y:y-(motion.matches?0:interaction.jump),groundY:y,id,left,path,symbol:conflictPet&&!side&&age<700?4:interaction.symbol,behavior:interaction.kind,hurt:conflictPet&&side&&conflict.hit&&age<1100&&Math.floor(age/80)%2};
 });
 // Same foot-depth rule as firmware; mask preserves gaps between branches.
 const drawPet=p=>{ctx.save();if(p.hurt)ctx.globalAlpha=.55;image(p.path,p.x,p.y,48,48);ctx.restore()};
 const nest=manifest.sceneNests.scenes[scene],nestY=nest.position[1];
 const drawNest=()=>{
  if(hatchPreview.egg){const ground=hatchPreview.position(scene),p=itemDrop(motion.matches?itemDropDuration:now-eggDropStart,ground.x,ground.y,32),wobble=motion.matches?0:[0,1,2,3,2,1,0,-1,-2,-3,-2,-1][Math.floor(now/100)%12];
   ctx.save();for(const dot of eggGlow(motion.matches?600:Math.floor(now),!!hatchPreview.egg.legendary)){ctx.globalAlpha=dot.opacity*p.opacity/65025;ctx.fillStyle='#'+dot.color.toString(16).padStart(6,'0');ctx.fillRect(p.x+wobble+dot.x,p.y+dot.y,dot.size,dot.size)}ctx.restore();
   ctx.save();ctx.globalAlpha=p.opacity/255;image('icons/5.png',p.x+wobble,p.y,32,32);ctx.restore();
  }
  image(nest.front,nest.position[0],nestY+manifest.sceneNests.frontTop,...manifest.sceneNests.frontSize);
 };
 if(scene===12){
  petRects.filter(p=>!lakeInFront(p.groundY)).forEach(drawPet);
  const mask=load('scene-fronts/12.png');
  if(motion.matches&&!ambientPreview){if(mask.complete&&mask.naturalWidth)ctx.drawImage(mask,0,0,360,360)}
  else scenery.drawFront(ctx,mask);
  petRects.filter(p=>lakeInFront(p.groundY)&&p.groundY<nestY).forEach(drawPet);drawNest();
  petRects.filter(p=>lakeInFront(p.groundY)&&p.groundY>=nestY).forEach(drawPet);
 }else{petRects.filter(p=>p.groundY<nestY).forEach(drawPet);drawNest();petRects.filter(p=>p.groundY>=nestY).forEach(drawPet)}
 petRects.forEach(drawHomeSymbol);
 if(homeConflict.state.active){const [a,b]=conflictOrigins;ctx.save();for(const p of sampleHomeConflict(homeConflict.state,Math.round(now),a.x+24,a.y+24,b.x+24,b.y+24)){ctx.globalAlpha=p.opacity/255;ctx.fillStyle='#'+p.color.toString(16).padStart(6,'0');ctx.fillRect(p.x,p.y,p.w,p.h)}ctx.restore()}

 const active=petRects.find(p=>p.behavior),status=document.querySelector('#home-interaction-status');
 const state=homeConflict.state.active?'使出技能':active?['','摸摸回应','呼唤伙伴','好奇探索','开心小跳','伙伴打招呼','结伴散步','并排休息','欢迎回来'][active.behavior]:'点空地可呼唤伙伴';
 if(status.textContent!==state)status.textContent=state;
 if(hudSettings.state.showTime){ ctx.textAlign='center';ctx.font='900 38px "Pixel Clock",sans-serif';const time=new Date().toLocaleTimeString('zh-CN',{hour:'2-digit',minute:'2-digit',hour12:false});ctx.fillStyle='#7d78a7';ctx.fillText(time,181,60);ctx.fillStyle='#d5c6ff';ctx.fillText(time,180,59);}
 if(hudSettings.state.showCoins){ ctx.font='500 18px "Noto Sans SC",sans-serif';const coinX=Math.round((360-28-ctx.measureText(String(coins)).width)/2);image('hud/coin-large.png',coinX,67,24,24);ctx.textAlign='left';ctx.fillStyle='#344866';ctx.fillText(coins,coinX+29,85);ctx.fillStyle='#e0e1ff';ctx.fillText(coins,coinX+28,84);}
 if(previewDrop){const d=previewDrop,p=itemDrop(motion.matches?itemDropDuration:now-d.start,d.x,d.y,24);
  ctx.save();ctx.globalAlpha=p.y>=d.y-8?50/255:25/255;box(d.x+7,d.y+28,18,4,'#20382b',2);
  ctx.globalAlpha=p.opacity/255;image(d.item===8?'items/tree-essence-small.png':`icons-small/${d.item}.png`,p.x,p.y,24,24);ctx.restore();
 }
 if(page!=='wildalert'&&now<talkUntil&&petRects[talkPet]?.w!==0&&petRects[talkPet]&&speechKey===petTalkKey(talkPet)&&(careSpeech||!evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[talkPet])?.sleeping)){
  const p=petRects[talkPet],hasIcon=bubbleIcon>=0,padding=hasIcon?48:16,limit=220-padding;
  ctx.save();ctx.font='500 12px "Noto Sans SC",sans-serif';ctx.textAlign='left';
  const lines=[];let line='';for(const char of bubble){if(line&&ctx.measureText(line+char).width>limit){lines.push(line);line=''}line+=char}lines.push(line);
  const width=Math.max(1,...lines.map(line=>Math.ceil(ctx.measureText(line).width)))+padding,height=Math.max(44,lines.length*16+16);
  const x=Math.max(36,Math.min(324-width,p.x+24-Math.floor(width/2))),y=Math.max(99,p.y-48);
  box(x,y,width,height,'#123c60eb',8);
  if(hasIcon){box(x+5,y+(height-28)/2,30,28,'#9be2da41',7);image(bubbleIcon===8?'items/tree-essence-small.png':`icons-small/${bubbleIcon}.png`,x+8,y+(height-24)/2,24,24)}
  ctx.fillStyle='#f1f0ff';lines.forEach((line,i)=>ctx.fillText(line,x+(hasIcon?40:8),y+(height-lines.length*16)/2+12+i*16));ctx.restore();
 }
 if(page==='wildalert'){
  image(wildPreviewPet.frames[petFrame(wildPreviewPet.id,'idle',now,'wild')],wildPreviewX,186,48,48);
  const markY=150+(Math.floor(real/330)%2?0:3);
  wildMarkPixels.forEach((row,y)=>[...row].forEach((pixel,x)=>{if(pixel!=='0'){ctx.fillStyle=pixel==='1'?'#203c49':'#fff0b0';ctx.fillRect(wildPreviewX+17+x*3,markY+y*3,3,3)}}));
 }
 ['APP','BAG','SHOP'].forEach((caption,i)=>{const x=128+i*52;ctx.font='700 14px PokemonNav, sans-serif';ctx.textAlign='center';ctx.fillStyle='#163145';ctx.fillText(caption,x+1,282);ctx.fillStyle='#e1ddff';ctx.fillText(caption,x,281);ctx.fillStyle='#103b75';ctx.strokeStyle='#63baff';ctx.lineWidth=2;ctx.beginPath();ctx.arc(x,306,22,0,Math.PI*2);ctx.fill();ctx.stroke();image(`nav/${i}.png`,x-12,294,24,24)});
 drawBattery();
 if(page==='wild'){
  ctx.fillStyle='#071923d2';ctx.fillRect(0,0,360,360);
  ctx.save();ctx.beginPath();ctx.arc(180,176,142,0,Math.PI*2);ctx.clip();
  ctx.fillStyle='#eef1e8';ctx.fillRect(38,34,284,284);
  const cap=ctx.createLinearGradient(0,34,0,160);cap.addColorStop(0,'#d47476');cap.addColorStop(1,'#b74b60');ctx.fillStyle=cap;ctx.fillRect(38,34,284,126);
  box(38,155,284,13,'#243e4b',0);
  box(132,52,96,20,'#b85d65',10);text('野生相遇',180,67,12,'#fff0da');text(wildPreviewPet.name,180,100,18,'#fff5df');
  for(const [r,c] of [[44,'#243e4b'],[40,'#f7f1df'],[35,'#dce9df']]){ctx.beginPath();ctx.arc(180,164,r,0,Math.PI*2);ctx.fillStyle=c;ctx.fill()}
  image(wildPreviewPet.frames[0],156,141,48,48);box(212,129,44,20,'#294b57',10);text('Lv.8',234,144,12,'#f5edcf');
  const partner=wildPartnerIndex(),individual=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[partner]);box(112,226,136,24,'#dfe8df',12);text(partner<0?'暂无可出战伙伴':manifest.pets[previewPets[partner]].name+' Lv.'+individual.level,180,242,12,'#42636b');
  
  ctx.restore();ctx.beginPath();ctx.arc(180,176,140,0,Math.PI*2);ctx.strokeStyle='#203c49';ctx.lineWidth=3;ctx.stroke();
  drawFooterText("放走",100,254,72,32);ctx.globalAlpha=wildPartnerIndex()<0?.6:1;drawFooterText("对战",188,254,72,32);ctx.globalAlpha=1;
 }
}
let lastFrame=0,raf;
function animate(time){if(time-lastFrame>=33){draw();lastFrame=time}raf=requestAnimationFrame(animate)}raf=requestAnimationFrame(animate);window.addEventListener('pagehide',()=>cancelAnimationFrame(raf));
function hit(label,x,y,w,h,fn){const b=document.createElement('button');b.type='button';b.className='device-hit';b.setAttribute('aria-label',label);Object.assign(b.style,{left:`${x/3.6}%`,top:`${y/3.6}%`,width:`${w/3.6}%`,height:`${h/3.6}%`});b.onclick=fn;hotspots.append(b);return b}
function openPage(next,{reveal=true,holdHome=false}={}){
 if(next==='settings'&&page!=='settings')settingsSection=0;
 if(shopSpin?.active && next!=='shop')return;
 if(page==='training'&&next!=='training'&&training.job.status===1){try{training.checkpoint()}catch(e){toast(e.message);return}}
 if(next==='training'&&page!=='training'){
  trainingJobTick();trainingMode=training.job.status===1?2:training.job.status===2?4:0;trainingScene=training.state.scene||0;trainingSelected=training.state.party.map(p=>p.id);trainingList=0;trainingLast=performance.now();trainingWalkTime=training.job.walkMs+(['search','approach'].includes(training.state.phase)?training.job.pendingMs:0);trainingAmbient=training.job.ambientMs+training.job.pendingMs;trainingError='';trainingPhase='';trainingActors=[training.state.ally,training.state.enemy].map((index,side)=>['fight','result'].includes(training.state.phase)?side+'-'+index+'-'+training.state.rounds:'');trainingHp=[training.state.party[training.state.ally]?.hp||0,training.state.foes[training.state.enemy]?.hp||0];trainingEntryTimes=[battleEntryDuration,battleEntryDuration];
 }

 batteryLast=performance.now();
 cancelBootPress();
 if(page==='home'&&next!=='home'){releaseConflict();homeConflict.pause(Math.round(homeElapsed))}
 if(next==='hospital'&&page!=='hospital'){hospital=createHospital(crypto.getRandomValues(new Uint32Array(1))[0]);hospitalLast=performance.now();hospitalShown=0;hospitalPatients=[];hospitalError='';hospitalAmbient=0;for(let i=0;i<3;i++)homeClocks.delete('hospital-patient-'+i)}
 if(next==='fishing' && page!=='fishing'){fishingPreview=newFishing();fishingMode=0;fishingAmbient=0;fishingLast=performance.now()}
 clearTimeout(captureTimer);
 if(page==='shop'&&next!=='wildcaught')wildShopReturn=false;
 if(next==='wildcapture'&&motion.matches)next='wildsuccess';
 if(reveal)document.dispatchEvent(new Event('workspace-preview'));
 if(next==='wild'&&wildPreview.phase!=='pending'){if(page==='wildalert'){if(wildPreview.phase==='alert')next='wildalert'}else{if(!chooseLocalEncounter()){toast('此地图没有普通野生伙伴');return}wildPreviewX=[72,156,236].reduce((best,x)=>Math.min(...petRects.map(p=>(x-p.x)**2+(186-p.y)**2))>Math.min(...petRects.map(p=>(best-p.x)**2+(186-p.y)**2))?x:best,72);wildPreview.enter();next='wildalert'}}
 if(!holdHome&&next==='home'&&wildPreview.phase==='pending')next='wild';
 if(!holdHome&&page.startsWith('wild') && next==='home')wildPreview.dismiss();
 if(!holdHome&&next==='home' && wildPreview.phase==='alert')next='wildalert';
 const readyWild=['wildcaught','wildduplicate'].includes(next);
 wildCounter.hidden=!readyWild;wildCounter.dataset.receipt=String(readyWild);
 if(readyWild){wildCounter.textContent=ballStock||evolutionDemo.state.trainingItems[7]?`捕捉 ×${ballStock+evolutionDemo.state.trainingItems[7]}`:'购买咕噜球';wildCounter.dataset.urgent='false'}
 const returning=next==='home'||next==='wildalert',leaving=page==='home'||page==='wildalert';
 if(leaving&&!returning){homeAwayAt=performance.now();homeBehavior.clear()}
 if(returning&&!leaving&&homeAwayAt&&performance.now()-homeAwayAt>=60000&&performance.now()-homeWelcomeAt>=120000)welcomeDue=true;
 homeLast=performance.now();
 if(next==='bag'&&page!=='bag'){bagPage=0;if(page!=='legend')bagLegendPreview=false;}
 if(next==='scenes'&&page!=='scenes')sceneMenu.enter(scene);
 if(next==='intro')introStart=null;
 if(next==='travel'&&page!=='travel'){travelPreviewStage='journey';nextTravelDestination()}
 if(['wildcaught','wildduplicate'].includes(next)&&!['wildcaught','wildduplicate','shop'].includes(page))captureCommitted=false;
 if(next==='pokedex'&&page!=='pokedex')petDexPage=0;
 if(next==='collection'&&page!=='collection')dexPage=0;
 pageNames.battle=gymCities[gymCity].city;
 pageNames.gym=gymCities[gymCity].city+'道馆';
 page=next;document.querySelector('#home-interaction-controls').hidden=page!=='home';
 document.querySelectorAll('[data-home-action]').forEach(b=>{b.disabled=Number(b.dataset.homeAction)>=5&&Number(b.dataset.homeAction)<=7&&previewPets.length<2});
 updateSceneEffectControls();template.setActive(page==='animation');stage.dataset.page=page;hotspots.replaceChildren();canvas.hidden=!!screenshots[page]&&!['collection','wild','bag','shop','scenes'].includes(page);shot.hidden=!canvas.hidden;
 if(!shot.hidden){
 const city=gymCities[gymCity];
 const gymFiles={battle:gymClearedPreview?'cleared':'map',gym:'detail',team:'team',gymfight:motion.matches?'battle':'battle.gif',gymwin:'win',badges:'badges'};
 const gymFile=page==='badges'?(['viridian-badges-pewter.png','viridian-badges-cerulean.png','viridian-badges-vermilion.png','viridian-badges-celadon.png','viridian-badges-fuchsia.png','viridian-badges-saffron.png','viridian-badges-cinnabar.png','viridian-badges.png'][gymCity]):gymFiles[page]?(city.prefix+'-'+gymFiles[page]+(gymFiles[page].endsWith('.gif')?'':'.png')):null;
 const file=(['wildcaught','wildduplicate'].includes(page)&&ballStock?'wild-caught-ball.png':null) || (page==='wildcapture'?'wild-capture.gif':null) || (page==='scenes'?sceneMenu.file:null) || (page==='legend'?legendPreview.file(motion.matches):page==='apps'&&appPreviewPage?'legend-apps.png':null) || (page==='wildfight'&&!motion.matches?'wild-battle.gif':null) || gymFile || (page==='travel'?(travelPreviewStage==='journey'&&!motion.matches?`travel-${travelPreviewDestination}.gif`:travelPreviewStage==='receipt'&&!motion.matches?'travel-settlement.gif':`travel-${travelPreviewDestination}-${travelPreviewStage}.png`):page==='gymfight'&&!motion.matches?'gym-battle.gif':screenshots[page]);
 // Every newly opened combat preview starts at its ball throw, including cached GIFs.
 const playOnceFromStart=file.endsWith('.gif')&&['wildfight','gymfight','legend','wildcapture'].includes(page);
 shot.src='../docs/screenshots/'+file+`?v=${assetRevision}${playOnceFromStart?'&play='+performance.now():''}`;shot.alt=pageNames[page]+' · 共享LVGL预览';
 }
 document.querySelector('#travel-preview-controls').hidden=page!=='travel';
 const legendControls=document.querySelector('#legend-preview-controls');legendControls.hidden=page!=='legend';
 if(page==='legend'){
  const toggle=document.querySelector('#legend-unlocked-preview');toggle.textContent=legendPreview.unlocked?'预览：八枚徽章':'预览：七枚徽章';toggle.setAttribute('aria-pressed',String(legendPreview.unlocked));toggle.onclick=()=>{legendPreview.toggle();openPage('legend')};
  document.querySelectorAll('[data-legend-phase]').forEach(b=>{b.disabled=!legendPreview.unlocked;b.classList.toggle('active',b.dataset.legendPhase===legendPreview.phase);b.onclick=()=>showLegend(b.dataset.legendPhase)});
 }

 const wildPage=['wildalert','wild','wildfight','wildcaught','wildcapture','wildsuccess','wildduplicate','wildlost'].includes(page);
 document.querySelector('#wild-preview-controls').hidden=!wildPage;
 document.querySelectorAll('[data-wild-page]').forEach(b=>{b.classList.toggle('active',b.dataset.wildPage===(page==='wildalert'?'wild':page));b.onclick=()=>openPage(b.dataset.wildPage)});
 const gymPage=['battle','gym','team','gymfight','gymwin','badges'].includes(page);
 document.querySelector('#gym-preview-controls').hidden=!gymPage;
 document.querySelector('#gym-city-controls').hidden=!gymPage;
 const gymInfo=gymCities[gymCity];pageNames.gym=gymInfo.city+'道馆';
 document.querySelector('[data-gym-page="gym"]').textContent=`推荐 Lv.${gymRecommendedLevel(gymCity)}`;
 document.querySelector('#gym-city-label').textContent=gymInfo.city;
 const clearedToggle=document.querySelector('#gym-cleared-preview');clearedToggle.hidden=page!=='battle';clearedToggle.setAttribute('aria-pressed',String(gymClearedPreview));clearedToggle.textContent=gymClearedPreview?'显示未通关':'预览已击败';clearedToggle.onclick=()=>{gymClearedPreview=!gymClearedPreview;openPage('battle')};
 document.querySelectorAll('[data-gym-city]').forEach(b=>{b.classList.toggle('active',Number(b.dataset.gymCity)===gymCity);b.setAttribute('aria-pressed',String(Number(b.dataset.gymCity)===gymCity));b.onclick=()=>{gymCity=Number(b.dataset.gymCity);openPage('battle')}});
 document.querySelectorAll('[data-gym-page]').forEach(b=>{b.classList.toggle('active',b.dataset.gymPage===page);b.onclick=()=>openPage(b.dataset.gymPage)});
 document.querySelector('#travel-destination-label').textContent=manifest.travel.destinations[travelPreviewDestination].name;
 document.querySelector('#travel-again').onclick=()=>{nextTravelDestination();travelPreviewStage='journey';openPage('travel')};
 document.querySelectorAll('[data-travel-stage]').forEach(b=>{b.classList.toggle('active',b.dataset.travelStage===travelPreviewStage);b.setAttribute('aria-pressed',String(b.dataset.travelStage===travelPreviewStage));b.onclick=()=>{travelPreviewStage=b.dataset.travelStage;openPage('travel')}});
 document.querySelector('#preview-page').textContent=pageNames[page];document.querySelector('#preview-kind').textContent=['home','starter','intro','animation','pokedex','collection','status','settings','tarot','warehouse','individual','fishing','hospital','training'].includes(page)?'互动演示':'固件预览';
 document.querySelectorAll('[data-page]').forEach(b=>{b.classList.toggle('active',b.dataset.page===(page==='wildalert'?'wild':page));b.setAttribute('aria-pressed',String(b.dataset.page===(page==='wildalert'?'wild':page)))});
 if(page==='legend'){
  const phase=legendPreview.phase;
  if(phase==='hatched'){
   hit('放入仓库预览',100,296,72,32,()=>openPage('home'));hit('替换伙伴预览',188,296,72,32,()=>openPage('warehouse'));
  }else if(phase==='egg'){
   hit('APP',106,284,44,44,()=>openPage('apps'));hit('BAG',158,284,44,44,legendBag);hit('SHOP',210,284,44,44,()=>openPage('shop'));
  }else{
   const home=['battle','bag'].includes(phase),back=['list','cleared'].includes(phase)?()=>openPage('apps'):home?()=>openPage('home'):()=>showLegend('list');
   hit(['list','cleared'].includes(phase)?'返回APP':home?'返回庭院':'返回神兽列表',158,298,44,44,back);
  }
  if(['list','cleared'].includes(phase))for(let i=0;i<5;i++)hit(manifest.pets.find(p=>p.nationalId===[144,145,146,150,151][i]).name,i<3?47+i*92:93+(i-3)*92,i<3?98:192,82,86,()=>{legendPreview.select(i);openPage('legend')});
  if(['detail','party'].includes(phase)&&legendPreview.unlocked)hit('开始神兽挑战预览',116,260,128,32,()=>showLegend('battle'));
 }
 else if(page==='warehouse'){
  const pets=warehousePets(),pages=Math.max(1,Math.ceil(pets.length/warehousePageSize));warehousePage%=pages;
  hit('返回APP',158,288,44,44,()=>openPage('apps'));
  hit('切换排序：'+warehouseSortLabels[warehouseSort],208,288,44,44,()=>{warehouseSort=(warehouseSort+1)%warehouseSortLabels.length;warehousePage=0;openPage('warehouse');toast(warehouseSortLabels[warehouseSort])});
  pets.slice(warehousePage*warehousePageSize,warehousePage*warehousePageSize+warehousePageSize).forEach((p,i)=>hit((p.nickname||manifest.pets[p.art].name)+' Lv.'+p.level,warehouseCardX(i,Math.min(warehousePageSize,pets.length-warehousePage*warehousePageSize)),84+Math.floor(i/3)*68,72,66,()=>{individualFromWarehouse=true;individualId=p.id;openPage('individual')}));
  if(pages>1)for(const [x,d]of [[18,-1],[298,1]])hit(d>0?'下一页仓库':'上一页仓库',x,158,44,44,()=>{warehousePage=(warehousePage+d+pages)%pages;openPage('warehouse')});
 }else if(page==='individual'){
  const p=evolutionDemo.state.pets.find(p=>p.id===individualId),stored=p&&!evolutionDemo.state.active.includes(p.id);
  hit('返回',158,298,44,44,()=>openPage(individualFromWarehouse?'warehouse':'status'));const canStore=!stored&&individualFromWarehouse&&evolutionDemo.state.active.length>1;hit('命名',stored?76:canStore?108:116,260,stored?68:canStore?72:128,32,renamePreview);
  if(canStore)hit('收回',188,260,72,32,()=>{try{evolutionDemo.setActive(evolutionDemo.state.active.filter(id=>id!==p.id));syncCompanions();selectedPet=0;openPage('warehouse')}catch(e){toast(e.message)}});
  if(stored){hit('放生',146,260,68,32,releasePreview);hit(evolutionDemo.state.active.length<5?'上场':'替换',216,260,68,32,()=>{
   const apply=slot=>{try{const active=[...evolutionDemo.state.active];if(slot===active.length)active.push(p.id);else active[slot]=p.id;evolutionDemo.setActive(active);dialog.close();syncCompanions();openPage('home')}catch(e){toast(e.message)}};
   if(evolutionDemo.state.active.length<5){apply(evolutionDemo.state.active.length);return}
   const out=document.querySelector('#detail-content');out.replaceChildren();const title=document.createElement('h2');title.textContent='替换伙伴';out.append(title);
   evolutionDemo.state.active.forEach((id,slot)=>{const current=evolutionDemo.state.pets.find(p=>p.id===id),button=document.createElement('button');button.textContent=(current.nickname||manifest.pets[current.art].name)+' Lv.'+current.level;button.onclick=()=>apply(slot);out.append(button)});dialog.showModal();
  });}
 }
 else if(page==='gift'){hit('收下藏品预览',116,296,128,32,()=>{openPage('home');toast('藏品预览已结束')})}
 else if(page==='wild'){hit('放走野生伙伴',100,254,72,32,()=>{wildPreview.dismiss();openPage('home')});const fight=hit('确认进入野外对战',188,254,72,32,()=>{if(wildPartnerIndex()<0){toast('暂无可出战伙伴');return}if(wildPreview.confirm())openPage('wildfight');else {openPage('home');toast('野生伙伴已逃走')}});fight.disabled=wildPartnerIndex()<0}
 else if(['wildfight','wildcaught','wildduplicate','wildcapture','wildsuccess','wildlost'].includes(page)){
  if(['wildsuccess','wildlost'].includes(page))hit('完成，返回庭院',116,296,128,32,()=>openPage('home'));
  else if(page==='wildfight')hit('返回庭院',158,298,44,44,()=>openPage('home'));
  if(page==='wildcapture')captureTimer=setTimeout(()=>{if(page==='wildcapture')openPage('wildsuccess',{reveal:false})},2900);
  if(['wildcaught','wildduplicate'].includes(page)){
   hit('放走',100,296,72,32,()=>openPage('home'));
   hit(ballStock||evolutionDemo.state.trainingItems[7]?'使用咕噜球捕捉':'购买咕噜球',188,296,72,32,()=>{if(captureCommitted){toast('本次已捕捉');return}if(!ballStock&&!evolutionDemo.state.trainingItems[7]){wildShopReturn=true;toast('先在商店购买咕噜球');openPage('shop');return}try{const p=evolutionDemo.add(wildPreviewPet.id,8);if(evolutionDemo.state.trainingItems[7])evolutionDemo.useTrainingItem(7);else ballStock--;captureCommitted=true;unlockBrowserPet(p.art);syncCompanions();openPage('wildcapture')}catch(e){toast(e.message)}});
  }
 }
 else if(page==='home'||page==='wildalert'){if(page==='wildalert')hit('查看野生伙伴',wildPreviewX,150,48,84,()=>{if(wildPreview.reveal())openPage('wild',{reveal:false})});hit('APP',106,284,44,44,()=>openPage('apps'));hit('BAG',158,284,44,44,()=>openPage('bag'));hit('SHOP',210,284,44,44,()=>openPage('shop'))}
 else if(page==='training')trainingHits();
 else if(page==='hospital')hospitalHits();
 else if(page==='fishing')fishingHits();
 else if(page==='intro'){hit('跳过片头',116,296,128,32,finishIntro)}
 else if(page==='animation'){}
 else if(page==='settings') {
  hit(settingsSection?'返回设置':'返回APP',158,298,44,44,()=>{if(settingsSection){settingsSection=0;openPage('settings',{reveal:false})}else openPage('apps')});
  if(!settingsSection)for(let i=1;i<=3;i++)hit(settingsTitles[i],66,100+(i-1)*56,228,44,()=>{settingsSection=i;openPage('settings',{reveal:false})});
  const rows=settingsSection===1?[['显示时间',hudSettings.state.showTime,104,()=>hudSettings.toggle('showTime')],['显示金币',hudSettings.state.showCoins,152,()=>hudSettings.toggle('showCoins')],['显示电量',hudSettings.state.showBattery,200,()=>hudSettings.toggle('showBattery')]]:settingsSection===2?[['每日随机场景',daily.state.random,104,()=>{daily.toggle(today());if(daily.state.random)assignScene(daily.state.scene)}]]:settingsSection===3?[['历练自动补给',hudSettings.state.trainingAutoSupply,104,()=>hudSettings.toggle('trainingAutoSupply')]]:[];
  rows.forEach(([name,on,y,change])=>{
   const b=hit(name,66,y,228,42,()=>{try{change();openPage('settings',{reveal:false})}catch(e){toast(e.message)}});b.setAttribute('role','switch');b.setAttribute('aria-checked',String(on));
  });
 }

 else if(page==='tarot') {hit('返回APP',158,298,44,44,()=>openPage('apps'));if(daily.state.tarotDay<today())hit('翻牌',116,260,128,32,()=>{try{daily.draw(today());coins+=10;openPage('tarot');toast('当日增益已开启 · 浏览器试玩')}catch(e){toast(e.message)}})}
 else if(page==='status') {hit('性格与个体值',64,76,232,90,()=>{const p=specimen(previewPets[selectedPet]);individualFromWarehouse=false;individualId=p.id;openPage('individual')});hit('返回庭院',158,298,44,44,()=>openPage('home'));hit('更换宠物',104,294,36,36,()=>choosePreviewPet());if(previewPets.length<5)hit('添伙伴',220,294,36,36,()=>choosePreviewPet(true))}
 else if(page==='starter'){starters.forEach((p,i)=>hit('选择'+p.name,30+i*102,112,96,136,()=>{starterChoice=p.dex;draw()}));hit('确认伙伴，出发',116,296,128,32,()=>{chosen=starterChoice;previewPets=[starters.find(p=>p.dex===chosen).species];try{const p=evolutionDemo.add(previewPets[0]);evolutionDemo.setActive([p.id]);syncCompanions()}catch{}unlockBrowserPet(previewPets[0]);try{localStorage.setItem('pokemon-plus.web-starter',String(chosen))}catch{}openPage('home');toast('已选择伙伴 · 仅保存在此浏览器')})}
 else if(['battle','gym','team','gymfight','gymwin','badges'].includes(page)){
  const back={battle:'apps',gym:'battle',team:'gym',gymfight:'apps',gymwin:'gym',badges:'apps'}[page];
  hit('返回'+pageNames[back],158,298,44,44,()=>openPage(back));
  if(page==='battle'){
   hit(gymInfo.city+gymInfo.type+'道馆',120,114,120,122,()=>openPage('gym'));
   if(!gymClearedPreview){const [x,y]=manifest.gyms.doorTeams.positions[gymCity];hit('推荐等级 '+gymRecommendedLevel(gymCity)+' · 守馆宝可梦',x,y,128,72,()=>openPage('gym'))}
   hit('上一馆：'+gymCities[(gymCity+gymCities.length-1)%gymCities.length].city,18,158,44,44,()=>{gymCity=(gymCity+gymCities.length-1)%gymCities.length;openPage('battle')});
   hit('下一馆：'+gymCities[(gymCity+1)%gymCities.length].city,298,158,44,44,()=>{gymCity=(gymCity+1)%gymCities.length;openPage('battle')});
   hit('进入'+gymInfo.city+'道馆',116,260,128,32,()=>openPage('gym'));
  }
  if(page==='gym')hit('开始战斗',116,260,128,32,()=>openPage('team'));
  if(page==='team')hit('预览三宠道馆战斗',116,260,128,32,()=>openPage('gymfight'));
  if(page==='gymwin')hit('查看徽章展馆',116,260,128,32,()=>openPage('badges'));
  if(page==='badges'){hit('灰色徽章',66,90,48,48,()=>{gymCity=0;openPage('badges')});hit('蓝色徽章',126,90,48,48,()=>{gymCity=1;openPage('badges')});hit('橙色徽章',186,90,48,48,()=>{gymCity=2;openPage('badges')});hit('彩虹徽章',246,90,48,48,()=>{gymCity=3;openPage('badges')});hit('粉红徽章',66,168,48,48,()=>{gymCity=4;openPage('badges')});hit('金色徽章',126,168,48,48,()=>{gymCity=5;openPage('badges')});hit('深红徽章',186,168,48,48,()=>{gymCity=6;openPage('badges')});hit('绿色徽章',246,168,48,48,()=>{gymCity=7;openPage('badges')});hit('前往'+gymInfo.city,116,260,128,32,()=>openPage('gym'));}
 }
 else if(page==='travel'){if(travelPreviewStage==='journey')hit('返回APP',158,298,44,44,()=>openPage('apps'));else if(travelPreviewStage==='ready')hit('确认领取旅行奖励',116,260,128,32,()=>{travelPreviewStage='receipt';openPage('travel')});else hit('完成旅行',116,296,128,32,()=>openPage('home'))}
 else {hit('返回'+(page==='shop'&&wildShopReturn?'野外收获':['apps','bag','shop','status'].includes(page)?'庭院':'APP'),158,298,44,44,()=>{if(page==='shop'&&wildShopReturn){wildShopReturn=false;openPage('wildcaught')}else openPage(['apps','bag','shop','status'].includes(page)?'home':'apps')});
 if(page==='apps'){
  appMenuPages[appPreviewPage].forEach((name,i)=>{const {x,y,size}=appMenuPosition(appPreviewPage,i);hit(appMenuLabels[name],x,y,size,size,()=>name==='legend'?showLegend('list'):openPage(name))});
  for(const [name,x] of [['上一页APP',18],['下一页APP',298]])hit(name,x,158,44,44,()=>{appPreviewPage^=1;openPage('apps')});
 }
 if(page==='scenes'){
  hit('上一个场景',18,158,44,44,()=>{sceneMenu.move(-1);openPage('scenes')});
  hit('下一个场景',298,158,44,44,()=>{sceneMenu.move(1);openPage('scenes')});
  const sceneAction=hit(sceneMenu.state==='active'?'使用中':sceneMenu.state==='owned'?'使用场景':`需解锁${sceneMenu.required}种宠物`,88,264,184,28,()=>{const result=sceneMenu.apply();if(result==='not-ready'){toast(`需要解锁${sceneMenu.required}种宠物`);return}assignScene(sceneMenu.current);ambientStart=performance.now();openPage('home')});sceneAction.disabled=sceneMenu.state!=='owned';
 }
 if(page==='shop'||page==='bag'){
  const {entries,pages}=itemGrid();
  entries.forEach((x,i)=>{const {x:left,y,w,h}=itemCardPosition(i);const b=hit(x.lottery?'抽奖 · '+shop.drawPrice+'金币':(page==='shop'?'购买':'使用')+x.name,left,y,w,h,()=>{if(page==='shop'){if(x.lottery)startShopDraw();else buyShopItem(x.id)}else if(x.legend!==undefined){legendPreview.select(x.legend);showLegend('egg')}else detail({...x,type:'items',info:x.description||`${x.price}金币`})});b.disabled=!!shopSpin?.active});
  if(page==='bag'&&pages>1)for(const [name,x,delta]of [['上一页道具',18,-1],['下一页道具',298,1]])hit(name,x,158,44,44,()=>{bagPage=(bagPage+pages+delta)%pages;openPage('bag')});
 }
 if(page==='pokedex'&&manifest.pets.length>10){const pages=Math.ceil(manifest.pets.length/10);hit('上一页宠物',18,158,44,44,()=>{petDexPage=(petDexPage+pages-1)%pages;draw()});hit('下一页宠物',298,158,44,44,()=>{petDexPage=(petDexPage+1)%pages;draw()})}
 if(page==='collection'){
 ['全部','普通','精良','稀有','史诗','传说','特殊'].forEach((name,i)=>hit(name+'收藏',74+i*31,81,28,24,()=>{collectionQuality=i?i-1:6;dexPage=0;openPage('collection');render()}));
 const list=collectionList(),pages=Math.ceil(list.length/64);dexPage%=pages;
 list.slice(dexPage*64,dexPage*64+64).forEach((x,i)=>hit(x.name,80+i%8*25,111+Math.floor(i/8)*22,23,21,()=>detail({...x,type:'collection',info:collectionInfo(x)})));
 if(pages>1){hit('上一页收藏',18,158,44,44,()=>{dexPage=(dexPage+pages-1)%pages;openPage('collection')});hit('下一页收藏',298,158,44,44,()=>{dexPage=(dexPage+1)%pages;openPage('collection')})}
 } }

 draw();
 if(!wildCounter.hidden)placeWildCountdown();
}
setInterval(()=>{
 try{const before=wildPreview.phase;wildPreview.tick({active:['home','wildalert'].includes(page)&&!document.hidden,enabled:scene<12});if(wildPreview.phase==='alert'&&before!=='alert')openPage('home',{reveal:false});else if(before==='alert'&&wildPreview.phase!=='alert'&&page==='wildalert')openPage('home',{reveal:false});}catch{}
},100);
for(const b of document.querySelectorAll('[data-page]'))b.onclick=()=>openPage(b.dataset.page);
for(const b of document.querySelectorAll('[data-home-action]'))b.onclick=()=>{
 if(page!=='home')return;const input=homeActors(),eligible=input.map((p,i)=>p.available?i:-1).filter(i=>i>=0);
 const kind=Number(b.dataset.homeAction);if(!eligible.length||kind>=5&&kind<=7&&eligible.length<2)return;
 homeBehavior.clear();homeBehavior.request(homeElapsed,kind,eligible[0],eligible[1]??0,220,198,input);draw();
};
document.querySelector('#demo-pets').onclick=()=>{previewIndividuals=[];previewPets=[0,1,2,3];openPage('home')};
function assignScene(n){if(n!==scene){for(const m of homeOrigins){if(!m)continue;const p=groundProject(n,m.x,m.y);Object.assign(m,{x:p.x,y:p.y,ox:p.x,oy:p.y,start:homeElapsed-(n===12?lakeNearestPhase(p.x,p.y):0),interacting:false})}homeBehavior.clear();previewDrop=null}scene=n;}
function setScene(n){assignScene((n+manifest.scenes.length)%manifest.scenes.length);ambientStart=performance.now();openPage(page==='animation'?'animation':'home')}
document.querySelector('#scene-effect-play').onclick=()=>{
 ambientPreview=!ambientPreview;ambientStart=performance.now();
 openPage(page==='animation'?'animation':'home');
};
document.querySelector('#wild-walk').onclick=()=>{const met=wildPreview.walk(100,{enabled:scene<12});if(met)openPage('home',{reveal:false});document.querySelector('#wild-walk-progress').textContent=scene>=12?'此地图无普通遭遇':wildPreview.phase==='alert'?'点击感叹号':`还需 ${wildPreview.stepsRemaining} 步`;};
document.querySelector('#next-scene').onclick=()=>setScene(scene+1);document.querySelector('#previous-scene').onclick=()=>setScene(scene-1);
let toastAnimation;
function toast(s){
 const t=document.querySelector('#toast');
 toastAnimation?.cancel();t.textContent=s;t.style.whiteSpace='pre-line';
 // Re-enter the top layer so notices also remain visible above a detail dialog.
 if(t.matches(':popover-open'))t.hidePopover();
 t.showPopover();
 const still=matchMedia('(prefers-reduced-motion: reduce)').matches;
 const frames=[{opacity:0,transform:'translateY(0)'},{opacity:1,transform:still?'none':'translateY(-2px)',offset:.05625},{opacity:1,transform:still?'none':'translateY(-25px)',offset:.7},{opacity:0,transform:still?'none':'translateY(-36px)'}];
 const animation=t.animate(frames,{duration:3200,easing:'linear',fill:'forwards'});
 toastAnimation=animation;
 animation.finished.then(()=>{if(toastAnimation===animation){t.hidePopover();toastAnimation=null;animation.cancel()}}).catch(()=>{});
}
function patPreview(indices){
 try{
  const ids=indices.map(i=>{const p=evolutionDemo.state.pets.find(p=>p.id===previewIndividuals[i]&&p.art===previewPets[i])||specimen(previewPets[i]);previewIndividuals[i]=p.id;return p.id});
  const bonus=daily.state.tarotDay===today()&&daily.state.card===5;
  const rewarded=new Set(evolutionDemo.pat(ids,today(),bonus?2:1,coins));
  if(!rewarded.size)return;
  coins=evolutionDemo.state.coins;patTargets=new Set(indices.filter((i,n)=>rewarded.has(ids[n])));
  patPet=-2;patUntil=homeElapsed+2000;
  const actors=homeActors();for(const i of patTargets)homeBehavior.request(homeElapsed,homeKinds.pat,i,0,0,0,actors);
  homeSay([...patTargets][0],talkKinds.pat);
  toast(`摸摸 ${patTargets.size} 位伙伴 ♥ · 浏览器试玩`);draw();
 }catch(e){toast(e.message)}
}
let patTargets=new Set();
let pointer;
var bootTimer,bootDown=false;
function cancelBootPress(){bootDown=false;clearTimeout(bootTimer);bootTimer=null}
const boot=document.querySelector('.rim-button');
boot.addEventListener('pointerdown',e=>{
 if(!e.isPrimary||e.button!==0)return;
 e.preventDefault();cancelBootPress();bootDown=true;boot.setPointerCapture(e.pointerId);
 const origin=page;
 bootTimer=setTimeout(()=>{
  if(!bootDown||page!==origin)return;
  cancelBootPress();
  if(origin==='home')patPreview(previewPets.map((_,i)=>i));
  else if(!['intro','starter'].includes(origin)){cancelPetPress();openPage('home',{holdHome:true})}
 },1000);
});
for(const name of ['pointerup','pointercancel','lostpointercapture'])boot.addEventListener(name,cancelBootPress);
window.addEventListener('blur',cancelBootPress);
document.addEventListener('visibilitychange',()=>{if(document.hidden)cancelBootPress()});
function cancelPetPress(){if(pointer)clearTimeout(pointer.timer);pointer=null}
function hitEgg(x,y){const egg=page==='home'&&hatchPreview.egg&&hatchPreview.position(scene);return egg&&x>=egg.x-8&&x<=egg.x+40&&y>=egg.y-8&&y<=egg.y+40}
canvas.addEventListener('pointerdown',e=>{
 if(!['home','wildalert'].includes(page)||pointer)return;
 const r=canvas.getBoundingClientRect(),x=(e.clientX-r.left)*360/r.width,y=(e.clientY-r.top)*360/r.height;
 const pet=hitEgg(x,y)?-1:petRects.findIndex(p=>x>=p.x-5&&x<=p.x+53&&y>=p.y-5&&y<=p.y+53);
 pointer={pet,id:e.pointerId,x,y,held:false};
 if(pet>=0)pointer.timer=setTimeout(()=>{if(!pointer||!['home','wildalert'].includes(page))return;pointer.held=true;selectedPet=pet;openPage('status')},1000);
 canvas.setPointerCapture(e.pointerId);
});
canvas.addEventListener('pointerup',e=>{
 if(!pointer||e.pointerId!==pointer.id)return;
 const {pet,held,x,y}=pointer,r=canvas.getBoundingClientRect(),dx=(e.clientX-r.left)*360/r.width-x,dy=(e.clientY-r.top)*360/r.height-y;
 cancelPetPress();if(held||!['home','wildalert'].includes(page)||dx*dx+dy*dy>12*12)return;
 if(hitEgg(x,y))toast(hatchPreview.notice());
 else if(pet>=0)patPreview([pet]);
 else if(page==='home'&&!previewDrop){const pet=homeBehavior.call(homeElapsed,Math.round(x),Math.round(y),homeActors());if(pet>=0)homeSay(pet,talkKinds.call)}
});
canvas.addEventListener('pointermove',e=>{if(!pointer)return;const r=canvas.getBoundingClientRect(),x=(e.clientX-r.left)*360/r.width,y=(e.clientY-r.top)*360/r.height;if((x-pointer.x)**2+(y-pointer.y)**2>144){clearTimeout(pointer.timer);pointer.held=true}});
for(const name of ['pointercancel','lostpointercapture'])canvas.addEventListener(name,cancelPetPress);
openPage(chosen?'home':introSeen?'starter':'intro');
const resources=[...manifest.travel.destinations.map(x=>({...x,type:'travel',info:'360 × 112 · 旅行随机抵达'})),...manifest.collection.items.map(x=>({...x,type:'collection',info:collectionInfo(x)})),...template.resources,...manifest.pets.map(x=>({...x,type:'pets',info:x.keyPoses===48?'新绘 48 帧 · 6 动作 · 48 × 48':'原画位移 · 24槽位 · 48 × 48',image:x.frames[0]})),...manifest.scenes.map(x=>({...x,type:'scenes',info:`360 × 360 · ${x.unlock?.requiredPets?`繁荣度 ${x.unlock.requiredPets} 解锁`:'初始场景'}`})),...manifest.items.map((x,i)=>({...x,id:i})).filter(x=>shopEnabled(x.id)).map(x=>({...x,type:'items',info:x.effect==='fullRestore'?`${x.price} 金币 · 全部状态恢复`:x.effect==='hatchBoost'?`${x.price} 金币 · 孵化场上的蛋`:x.id===5?`${x.price} 金币 · 2–4 小时 · 优先未拥有`:`${x.price} 金币 · 随机落在庭院`})),...manifest.skills.map((x,i)=>({...x,id:i,type:'skills',info:x.type+' · '+x.species+(x.runtimeIntegrated?' · 已接入':' · 技能就绪，伙伴待接入'),image:x.icon}))];
const labels={animations:'动作模板',all:'资源库',pets:'宠物伙伴',scenes:'生活场景',travel:'旅行风景',items:'照料道具',skills:'战斗技能',collection:'收藏图鉴'};
function setFilter(f){library.setFilter(f)}
function resourceCard(x){const c=document.createElement('button');c.className='card';c.dataset.type=x.type;c.setAttribute('aria-label','查看'+x.name);const art=document.createElement('div');art.className='card-art';if(x.image){const im=document.createElement('img');im.src=(x.externalImage?x.image:base+x.image)+`?v=${assetRevision}`;im.alt=x.name;im.loading='lazy';im.className=x.type==='travel'?'travel-scene':x.type==='scenes'?'scene':'pet';art.append(im);}else{const icon=document.createElement('span');icon.className='glyph';icon.textContent=x.type==='skills'?'ϟ':['●','◇','✚','☆','▣','◉'][x.id];art.append(icon);}
 const id=document.createElement('span');id.className='id';id.textContent=x.nationalId?`DEX / ${String(x.nationalId).padStart(3,'0')}`:`${x.type.slice(0,3).toUpperCase()} / ${String(x.id+1).padStart(3,'0')}`;art.append(id);const tag=document.createElement('span');tag.className='asset-tag';tag.textContent=x.type==='collection'?manifest.collection.qualities[x.quality].name:x.type==='animations'?'动作模板':x.type==='skills'?(x.runtimeIntegrated?'技能动画':'伙伴待接入'):'已接入';if(x.type==='collection'){const color='#'+manifest.collection.qualities[x.quality].color;tag.style.color=color;art.style.borderBottom='3px solid '+color;}art.append(tag);c.append(art);const info=document.createElement('div');info.className='card-info';const row=document.createElement('div');const strong=document.createElement('strong');strong.textContent=x.name;const small=document.createElement('small');small.textContent=labels[x.type]+' ↗';row.append(strong,small);const p=document.createElement('p');p.textContent=x.info;info.append(row,p);c.append(info);c.onclick=()=>detail(x);return c;}
const library=createWorkspace({manifest,resources,base,revision:assetRevision,resourceCard,detail,showAnimation,
 onFilter:f=>{filter=f},renderCollectionToolbar,getCollectionQuality:()=>collectionQuality,
 previewPet(x){previewPets=[x.id];previewIndividuals=evolutionDemo.state.pets.filter(p=>p.art===x.id).slice(0,1).map(p=>p.id);homeClocks.clear();openPage('home')}
});
function render(){library.render()}
document.querySelector('.close').onclick=()=>dialog.close();dialog.addEventListener('close',()=>{if(trainingNoticeClosing){trainingNoticeClosing=false;return}const notice=trainingNotice;if(!notice)return;trainingNotice=null;try{if(notice.kind==='result'){if(!trainingGrowthOffers(notice.id).length)training.ackNotice('evolution',notice.index)}else training.ackNotice(notice.kind,notice.index)}catch(e){toast(e.message)}});dialog.addEventListener('click',e=>{if(e.target===dialog){const r=dialog.getBoundingClientRect();if(e.clientX<r.left||e.clientX>r.right||e.clientY<r.top||e.clientY>r.bottom)dialog.close()}});
function detail(x){if(x.type==='collection'){collectionDetail(x);return}const out=document.querySelector('#detail-content');out.replaceChildren();const h=document.createElement('h2');h.textContent=x.name;out.append(h);if(x.type==='animations'){template.openDetail(out,()=>{dialog.close();showAnimation(x.nationalId)},x.nationalId);dialog.showModal();return}if(x.type==='pets'){const preview=document.createElement('button');preview.textContent='放入庭院预览';preview.onclick=()=>{previewPets=[x.id];previewIndividuals=evolutionDemo.state.pets.filter(p=>p.art===x.id).slice(0,1).map(p=>p.id);homeClocks.clear();dialog.close();openPage('home');document.querySelector('.overview').scrollIntoView({behavior:'smooth'})};out.append(preview);const regions=document.createElement('p');regions.textContent='栖息地：'+(manifest.habitats.scenes.filter(s=>habitatPets(manifest.habitats,manifest.pets,s.scene).some(p=>p.id===x.id)).map(s=>s.name).join('、')||'神兽挑战');out.append(regions);const f=document.createElement('div');f.className='frames';for(const path of x.frames){const im=document.createElement('img');im.src=base+path+`?v=${assetRevision}`;im.alt=x.name+' '+['待机',movementName(x.nationalId),'开心','进食','睡眠','攻击'][Math.floor(x.frames.indexOf(path)/(x.keyPoses===48?8:4))]+' '+(x.frames.indexOf(path)%(x.keyPoses===48?8:4)+1);im.title=im.alt;f.append(im)}out.append(f);if(x.keyPoses===48){const b=document.createElement('button');b.textContent='预览动作';b.onclick=()=>{dialog.close();showAnimation(x.nationalId)};out.append(b)}const chain=manifest.evolution.chains.find(c=>c.species===x.species);if(chain){const im=document.createElement('img');if(!chain.branches)im.src=`../docs/screenshots/evolution-${x.species}.gif?v=${assetRevision}-evo2`;im.alt=x.name+'进化演示';if(!chain.branches)out.append(im);const note=document.createElement('p');note.textContent=chain.branches?chain.branches.map(b=>'伊布 + '+b.item+' → '+manifest.pets.find(p=>p.nationalId===b.toNationalId).name).join('；')+'。三条分支互斥，各200金币。':chain.nationalIds.map((dex,i)=>(i?(evolutionStep(chain,i-1).item?evolutionStep(chain,i-1).item+' → ':'Lv.'+chain.levels[i-1]+' → '):'')+manifest.pets.find(p=>p.nationalId===dex).name).join(' · ');out.append(note);}const pending=manifest.evolution.pendingChains?.find(c=>c.species===x.species);if(pending){const note=document.createElement('p');note.textContent=`Lv.${pending.level} → ${String(pending.nationalIds.at(-1)).padStart(3,'0')}（后续批次；当前尚未开放进化）`;out.append(note);}evolutionControls(out,x);}else if(x.image){const im=document.createElement('img');im.src=x.type==='skills'?`../docs/screenshots/skill-${x.id}.gif?v=${assetRevision}`:base+x.image+`?v=${assetRevision}`;im.alt=x.name;out.append(im)}const p=document.createElement('p');p.className='detail-meta';p.textContent=x.type==='skills'?`${x.description} · 威力 ${x.formPowers?.find(f=>f.nationalId===x.selectedNationalId)?.power??x.power}`:x.type==='pets'?'':x.info;out.append(p);if(x.type==='items')itemPreviewControls(out,x);if(x.type==='skills'){const a=document.createElement('a');a.href=x.source;a.target='_blank';a.rel='noopener noreferrer';a.textContent='招式资料 ↗';out.append(a)}if(x.type==='travel'){const b=document.createElement('button');b.textContent='查看旅行风景';b.onclick=()=>{dialog.close();openPage('travel');travelPreviewDestination=x.id;travelPreviewStage='journey';openPage('travel');document.querySelector('.overview').scrollIntoView({behavior:'smooth'})};out.append(b)}if(x.type==='scenes'){const b=document.createElement('button');b.textContent='在圆屏中预览';b.onclick=()=>{setScene(x.id);dialog.close();document.querySelector('.overview').scrollIntoView({behavior:'smooth'})};out.append(b)}dialog.showModal()}
render();draw();

function itemPreviewControls(out,x){
 if(x.effect==='fullRestore'){
  const stock=document.createElement('p');const update=()=>stock.textContent='背包 ×'+evolutionDemo.state.treeEssences;update();out.append(stock);
  const buy=document.createElement('button');buy.textContent='购买 · 250金币';buy.onclick=()=>{try{coins=evolutionDemo.shopBuy(x.id,coins);update();draw();toast(`购买${x.name}x1`)}catch(e){toast(e.message)}};
  const use=document.createElement('button');use.textContent='使用 · 全部恢复';use.onclick=()=>{try{if(previewDrop)throw Error('宠物正忙着呢');const p=evolutionDemo.useEssence(previewIndividuals);startPreviewDrop(8,previewIndividuals.indexOf(p.id));toast((p.nickname||manifest.pets[p.art].name)+'状态全部恢复')}catch(e){toast(e.message)}};out.append(buy,use);return;
 }

 if(x.effect==='captureBall'){
  const b=document.createElement('button');b.textContent=`购买咕噜球 · ${x.price}金币`;b.onclick=()=>{buyShopItem(x.id)};out.append(b);return;
 }
 if(x.id===5 || x.effect==='hatchBoost'){
  const stock=document.createElement('p');const update=()=>stock.textContent=`背包 ×${hatchPreview.boosts+evolutionDemo.state.trainingItems[6]} · ${hatchPreview.egg?'场上有蛋':'场上无蛋'}`;update();out.append(stock);
  const b=document.createElement('button');b.textContent=x.id===5?'放入演示蛋':`购买 · ${x.price}金币`;
  b.onclick=()=>{try{if(x.id===5){if(hatchPreview.egg)throw Error('场上已经有蛋了');if(evolutionDemo.state.trainingItems[5])evolutionDemo.useTrainingItem(5);hatchPreview.place(previewPets.length);eggDropStart=homeElapsed;dialog.close();openPage('home');toast('演示蛋已放入庭院 · 可用孵化药剂')}else{coins=evolutionDemo.shopBuy(x.id,coins);update();toast(`购买${x.name}x1`)}}catch(e){toast(e.message)}};out.append(b);
  if(x.effect==='hatchBoost'){
   const use=document.createElement('button');use.textContent='使用 · 立即孵化';
   use.onclick=()=>{try{
    const pool=hatchPool(manifest.habitats,manifest.pets,scene,evolutionDemo.state.pets.map(p=>manifest.pets[p.art])).map(p=>p.id);
    if(!hatchPreview.egg)throw Error('场上还没有宠物蛋');const found=evolutionDemo.state.trainingItems[6]>0;if(found)evolutionDemo.useTrainingItem(6);hatchPreview.use(previewPets.length,found);
    const id=pool[Math.floor(Math.random()*pool.length)];evolutionDemo.add(id);syncCompanions();preloadPet(id);unlockBrowserPet(id);
    patPet=previewPets.length-1;patUntil=homeElapsed+2600;dialog.close();openPage('home');toast('蛋孵化啦 · 消耗1瓶孵化药剂');
   }catch(e){toast(e.message)}};out.append(use);
  }
  return;
 }
 if(x.id>=5)return;
 if(x.id===4){
  // A temporary visual trial; this never changes companion saves or stock.
  let rest={energy:19,sleeping:true,seconds:0,remainder:0};
  const trial=document.createElement('div');trial.className='sleep-trial';
  const art=document.createElement('img'),meter=document.createElement('progress'),caption=document.createElement('p');
  art.width=96;art.height=96;art.alt='睡眠恢复演示';meter.max=100;
  const pet=manifest.pets[previewPets[selectedPet]??previewPets[0]];
  const redraw=()=>{art.src=base+pet.frames[rest.sleeping?(pet.keyPoses===48?32:16):0]+`?v=${assetRevision}`;meter.value=rest.energy;caption.textContent=`${rest.sleeping?'休息中':'醒来啦'} · ${rest.energy} / 100`};
  const wait=document.createElement('button');wait.textContent='休息 +10分钟';wait.onclick=()=>{rest=advanceEnergy(rest,600);redraw()};
  const biscuit=document.createElement('button');biscuit.textContent='精力饼干 +25';biscuit.onclick=()=>{rest=energyBiscuit(rest);redraw()};
  trial.append(art,meter,caption,wait,biscuit);out.append(trial);redraw();
 }
 const b=document.createElement('button');b.textContent=evolutionDemo.state.trainingItems[x.id]?'使用历练找到的道具':'预览随机落点';
 b.onclick=()=>{try{const target=previewIndividuals.findIndex(id=>!evolutionDemo.trainingBusy(id));if(target<0||previewDrop)throw Error('宠物正忙着呢');const found=evolutionDemo.state.trainingItems[x.id]>0;if(found)evolutionDemo.useTrainingItem(x.id,previewIndividuals[target]);if(startPreviewDrop(x.id,target)){toast(found?'已使用历练找到的道具':'随机落在庭院 · 浏览器演示');document.querySelector('.overview').scrollIntoView({behavior:motion.matches?'instant':'smooth'})}}catch(e){toast(e.message)}};out.append(b);
}
function showAnimation(dex){if(dex)template.selectPet(dex);openPage('animation');history.replaceState(null,'','#animation');document.querySelector('.overview').scrollIntoView({behavior:motion.matches?'instant':'smooth'})}
document.querySelector('#animation-entry').onclick=e=>{e.preventDefault();showAnimation()};
library.restoreRoute();

function renderCollectionToolbar(){
 let bar=document.querySelector('#collection-toolbar');
 if(!bar){bar=document.createElement('div');bar.id='collection-toolbar';document.querySelector('#resource-grid').before(bar)}
 bar.hidden=filter!=='collection';bar.replaceChildren();if(bar.hidden)return;
 
 const tabs=document.createElement('div');tabs.className='collection-quality-tabs';
 [6,0,1,2,3,4,5].forEach(q=>{const b=document.createElement('button'),quality=manifest.collection.qualities[q],list=collectionItems(manifest.collection,q);b.textContent=q===6?`全部 ${manifest.collection.total}`:`${quality.name} ${quality.count}`;b.classList.toggle('active',q===collectionQuality);b.setAttribute('aria-pressed',String(q===collectionQuality));if(quality)b.style.setProperty('--quality','#'+quality.color);b.onclick=()=>{collectionQuality=q;dexPage=0;render();if(page==='collection')openPage('collection',{reveal:false})};tabs.append(b)});bar.append(tabs);
 const summary=document.createElement('p');const list=collectionList();summary.textContent=`已收藏 ${list.filter(x=>ownsCollection(x.id)).length}/${list.length}`;bar.append(summary);
 const claim=document.createElement('button');claim.className='collection-claim';claim.textContent='旅行抽取试玩';claim.onclick=()=>{try{const result=collectionDemo.claim();render();if(page==='collection')openPage('collection',{reveal:false});if(result.id===null){toast('旅行藏品已集齐 · 设备后续旅行仅获得金币和经验');return}const x=manifest.collection.items[result.id];toast(`新收藏 · ${collectionInfo(x)} · ${x.name}`);collectionDetail(x)}catch(e){toast(e.message)}};bar.append(claim);
}
function collectionDetail(x){
 const out=document.querySelector('#detail-content');out.replaceChildren();const quality=manifest.collection.qualities[x.quality];
 const h=document.createElement('h2');h.textContent=x.name;h.style.color='#'+quality.color;
 const im=document.createElement('img');im.src=base+x.image+`?v=${assetRevision}`;im.alt=x.name;im.className='collection-detail-art';
 const p=document.createElement('p');p.textContent=collectionInfo(x)+' · '+(ownsCollection(x.id)?'已收藏':'未获得');
 const description=document.createElement('p');description.textContent=x.description;
 out.append(h,im,p,description);dialog.showModal();
}

setInterval(()=>{try{const old=daily.state.sceneDay;daily.advance(today());if(daily.state.random&&daily.state.sceneDay!==old){assignScene(daily.state.scene);updateSceneEffectControls();ambientStart=performance.now()}if(page==='tarot'||page==='settings')openPage(page,{reveal:false})}catch{}},30000);
