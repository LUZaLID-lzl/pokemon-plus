"""Export reachable UI states from the shared renderer for visual layout review."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import html,json,subprocess,tempfile
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/screenshots/ui-layout';OUT.mkdir(exist_ok=True)
BASE=[str(ROOT/'preview/build/pokemon_plus'),'--assets',str(ROOT/'sdcard/pokemon-plus'),'--demo','--intro-seen']
names={0:'庭院',1:'APP',2:'背包',3:'商店',4:'伙伴状态',5:'旅行选宠',8:'收藏品',9:'场景选择',10:'塔罗牌',11:'设置',12:'藏品详情',13:'初选伙伴',14:'计步器',15:'宠物图鉴',16:'宠物仓库',18:'馆主详情',19:'出战编队',20:'徽章展馆',24:'神兽列表',25:'神兽详情',26:'个体值',27:'命名键盘',28:'放生确认',29:'替换伙伴',31:'钓鱼'}
jobs=[(f'page-{p}',name,['--page',str(p)])for p,name in names.items()]
jobs += [(a,name,['--activity',a])for a,name in [('travel','旅途中'),('travel-ready','旅行完成待领取'),('travel-receipt','旅行收获'),('gym-team','五只伙伴编队'),('gym-empty','伙伴不足'),('gym-win','道馆首次获胜'),('gym-rematch','道馆再次获胜'),('evolution-choice','进化选择'),('evolution-decline','保留形态')]]
jobs += [(f'wild-{a}',name,['--wild',a])for a,name in [('pending','野生相遇'),('battle','野外战斗'),('caught','捕捉确认'),('success','捕捉成功'),('lost','野生伙伴离开')]]
jobs += [('hatch','孵化揭晓',['--legend','hatched','--legend-index','3']),('intro','片头',['--page','22','--at-ms','9500'])]
for i,prefix in enumerate(['gym','cerulean','vermilion','celadon','fuchsia','saffron','cinnabar','viridian']):
 city=['尼比','华蓝','枯叶','玉虹','浅红','金黄','红莲','常青'][i]
 for phase,label in [('map','地图'),('detail','详情'),('cleared','击败')]:jobs.append((prefix+'-'+phase,city+label,['--activity',prefix+'-'+phase]))
def capture(job):
 key,label,args=job
 with tempfile.TemporaryDirectory(prefix='pp-ui-layout-') as d:
  target=Path(d)/'frame.ppm';subprocess.run(BASE+args+['--screenshot',str(target)],check=True,capture_output=True)
  Image.open(target).save(OUT/(key+'.png'))
with ThreadPoolExecutor(max_workers=4) as pool:list(pool.map(capture,jobs))
for group in range((len(jobs)+7)//8):
 sheet=Image.new('RGB',(1440,780),'#e8eee2');draw=ImageDraw.Draw(sheet)
 for i,(key,label,args)in enumerate(jobs[group*8:group*8+8]):
  x=i%4*360;y=i//4*390;sheet.paste(Image.open(OUT/(key+'.png')),(x,y));draw.text((x+12,y+365),key,fill='#285448')
 sheet.save(OUT/f'overview-{group}.png')
items=''.join(f'<figure><img src="screenshots/ui-layout/{key}.png" width="360" height="360" loading="lazy"><figcaption>{html.escape(label)}</figcaption></figure>'for key,label,args in jobs)
page='''<!doctype html><html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Pokémon Plus · 逐页布局复核</title><style>body{margin:0;background:#eef3e9;color:#294b42;font:16px system-ui}header{max-width:1100px;margin:32px auto;padding:0 24px}h1{font-size:26px}p{line-height:1.8}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,360px));justify-content:center;gap:24px;padding:0 16px 48px}figure{margin:0;background:#fffdf6;border-radius:20px;overflow:hidden}img{display:block;width:100%;height:auto;image-rendering:pixelated;background:#080e17}figcaption{padding:14px 18px}</style><header><h1>逐页布局复核</h1><p>主按钮统一 32px 高，文字居中；返回箭头统一下移；网格末行居中，文字完整显示。以下均由当前共享 LVGL 渲染器导出。旅行收获与进化揭晓为动画中间状态，完成按钮随动画结束出现。</p></header><main>'''+items+'</main></html>'
(ROOT/'docs/ui-layout-review.html').write_text(page)
(OUT/'index.json').write_text(json.dumps([{'id':k,'label':l,'args':a}for k,l,a in jobs],ensure_ascii=False,indent=2))
print(f'PASS: {len(jobs)} reachable UI states exported for visual review')

# Keep the compact review sheets in sync with the same current renderings.
for filename,keys,labels in [
 ('nav-controls-preview.png',['page-1','page-16','page-19','page-26','page-27','hatch'],['APP','WAREHOUSE','PARTY','DETAILS','KEYBOARD','HATCH']),
 ('encounter-layout-preview.png',['page-27','wild-pending','page-31'],['LOWERCASE NAME','WILD ENCOUNTER','FISHING'])
]:
 sheet=Image.new('RGB',(1080,390*((len(keys)+2)//3)),'#e8eee2');draw=ImageDraw.Draw(sheet)
 for i,(key,label)in enumerate(zip(keys,labels)):
  x=i%3*360;y=i//3*390;sheet.paste(Image.open(OUT/(key+'.png')),(x,y));draw.text((x+12,y+365),label,fill='#285448')
 sheet.save(ROOT/'docs/screenshots'/filename)
# These generic pages use the exact same demo seed and arguments.
for key in ['page-27','page-31']:
 Image.open(OUT/(key+'.png')).save(ROOT/'docs/screenshots'/(key+'.png'))
