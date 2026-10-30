"""Stable travel IDs, largest-remainder quality quotas, and MobiIoT atlas prompts."""
from pathlib import Path
import json, hashlib
ROOT=Path(__file__).resolve().parents[1]
THEMES=[
('清晨庭院','庭院', '露珠叶片 三叶草 小雏菊 松果 斑点石 树枝 鸟羽 蜗牛壳 小蘑菇 红莓 木纽扣 种子袋 花种瓶 小铲子 浇水壶 园艺手套 花盆 草编环 木风车 野餐篮 蜂蜜罐 苹果 核桃 蒲公英 樱桃 胡萝卜 南瓜 豌豆荚 四叶草 银铃 铃兰 玻璃弹珠 蝴蝶结 木口哨 小鸟笛 花纹瓷杯 彩绘鸟屋 蜂蜡烛 花冠 金色橡果 琥珀叶片 玉石蜗牛 珍珠露滴 翡翠种子 晨光怀表 彩虹风铃 水晶花枝 生命树苗 晨曦王冠'),
('樱花小院','樱庭','樱花瓣 竹叶 白卵石 团扇 和纸鹤 小竹筒 糯米团 茶叶包 竹签 木屐 红绳 花布包 手鞠 纸灯笼 折纸船 茶碗 竹梳 樱花饼 糖葫芦 竹篮 芝麻点心 小饭团 漆盒 竹勺 竹哨 绢帕 铜钱 线轴 花簪 兔子木雕 鲤鱼旗 面具 小鼓 茶壶 风铃 茶点碟 玉兔香囊 绣花鞋 金鱼挂饰 朱漆梳 银花簪 月白瓷瓶 玫瑰折扇 琥珀茶盏 玉樱枝 夜樱灯 蝶纹宝盒 樱雨宝珠 千年樱冠'),
('晴日海岸','海岸','白贝壳 蓝贝壳 扇贝 海螺 珊瑚枝 海玻璃 海星 沙钱 海带 海胆壳 浮木 卵形石 小瓶沙 渔线团 木浮标 小渔网 船锚扣 航海绳 软木塞 盐晶 鲨鱼齿 小椰子 椰叶 沙铲 沙桶 草帽 蓝拖鞋 救生圈 漂流瓶 航海图 海鸥羽 珍珠蚌 水手结 船舵 木帆船 航海罗盘 铜望远镜 珊瑚胸针 银鱼饰 蓝宝石 海蓝耳坠 月光贝 琉璃海豚 海马雕像 珍珠项链 深海水晶 潮汐法螺 海神三叉戟 海洋之心'),
('城镇','街巷','铜纽扣 旧钥匙 小齿轮 螺丝钉 邮票 票根 书签 铅笔 橡皮 墨水瓶 红邮筒牌 面包 羊角包 奶瓶 饼干盒 糖纸 汽水瓶 纸袋 报纸 布零钱包 红围巾 毛线球 针线盒 木衣夹 铁马蹄 煤油灯 街灯模型 小钟 酒杯 咖啡杯 手摇铃 怀表 留声机唱片 小提琴模型 手风琴 铜相框 打字机模型 八音盒 银钥匙 铁皮机器人 黄铜火车 玻璃城堡 宝石袖扣 镀金钢笔 蓝珐琅钟 星纹印章 古董王印 城市勋章 时光齿轮'),
('秋日木林','秋林','枫叶 橡树叶 银杏叶 榛子 栗子 橡果 松针 松塔 树皮 苔藓石 棕羽毛 斑纹菌 干花 枯枝 藤蔓球 野葡萄 树莓 蓝莓 越橘 红枣 野梨 柿子 榴莲壳 木耳 蜂巢片 鹿角尖 猫头鹰羽 草编篮 松脂块 木杯 木勺 橡木哨 树皮卷 叶脉标本 树枝鹿 木刻狐 蘑菇灯 秋叶书 赤铜叶坠 黄金枫叶 琥珀松塔 银鹿角 玛瑙栗子 森林罗盘 火红羽冠 古树年轮牌 丰收号角 森之竖琴 秋之圣杯'),
('雪落小镇','雪乡','雪松枝 冰凌 白羽毛 冰花石 雪花片 松木片 绒毛球 毛线帽 围脖 手套 羊毛袜 小雪球 萝卜鼻 煤块 迷你雪人 雪橇模型 滑雪板 冰鞋 雪地靴 热可可杯 姜饼人 拐杖糖 糖霜饼 冷杉果 小暖炉 木柴捆 蜡烛 台灯 玻璃雪瓶 铃铛 冬青枝 红袜 驯鹿木雕 企鹅木偶 北极熊像 小雪屋 冰钓竿 冰晶铃 银雪花 冰蓝宝石 霜纹胸针 水晶雪兔 极光玻璃 冰羽挂坠 雪莲 冰晶王杖 冬日宝匣 极光宝珠 永霜王冠'),
('水晶洞穴','晶洞','石英碎片 紫晶碎片 黄晶碎片 蓝晶碎片 绿晶碎片 红晶碎片 黑曜石 云母片 方解石 萤石 盐石 硫晶 磁石 铁矿 铜矿 煤晶 水滴石 石笋尖 洞穴菌 蝙蝠牙 古陶片 石锤 矿灯 小铁镐 矿车模型 矿石袋 金砂瓶 晶簇 银矿 金矿 紫水晶柱 黄水晶簇 玫瑰石英 青金石 孔雀石 月光石 虎眼石 石榴石 血玉 原石钻 红宝石冠 蓝宝石柱 翡翠蛋白石 彩虹萤石 星光晶洞 龙鳞矿 心形紫晶 地心宝钻 创世晶核'),
('蘑菇秘境','菌境','红伞菇 蓝伞菇 黄伞菇 紫伞菇 褐伞菇 白斑菇 扁扇菇 金针菇 蘑菇孢子 苔藓球 蕨叶 小树根 腐木 苔藓瓶 绿藤 露水瓶 野蒜 野葱 橙色菌杯 木蝴蝶 蝉蜕 蛹壳 蜘蛛丝团 螺纹木 银杏果 发光苔 灯笼果 猪笼草 捕蝇草 水晶兰 菌丝团 圆木凳 蘑菇伞 叶片雨衣 青蛙雕像 甲虫胸针 蝴蝶标本 萤火灯 菌环戒指 夜光孢子 玉蘑菇 琉璃青蛙 秘境钥匙 灵芝 魔法蘑菇瓶 翡翠藤杖 月光灵菇 精灵花盏 菌之贤者冠'),
('星空山丘','星丘','鹅卵石 野草 花岗岩 小陨石 黑羽 星纹叶 月见草 夜来香 萤石砂 风干花 石片箭头 旧铃 铝星片 草蚱蜢 手工星绳 小旗布 绳结 木风向标 金属哨 小帐篷 模型望远镜 营火石 水壶 饭盒 罗盘片 地图卷 旅行日志 墨蓝羽 星空玻璃 夜光珠 星形饼 许愿牌 铜星章 月亮吊坠 星图盘 天平模型 黄铜星仪 银月铃 紫星砂 星光石 月白水晶 陨铁圆盘 流星胸针 银河瓶 天文仪 北辰宝石 月辉竖琴 星轨沙漏 群星圣冠'),
('沙漠绿洲','绿洲','沙色石 鹅黄砂 红砂 仙人掌刺 仙人掌花 枣椰果 干椰叶 砂岩片 沙漠玫瑰石 盐粒瓶 甲虫壳 蝎尾壳 蜥蜴蜕 蛇皮片 旧陶片 陶水瓶 皮水袋 编绳 布头巾 草凉鞋 青瓷碗 铜杯 粗陶壶 沙铲 小金字塔 骆驼木雕 绿洲树模型 油灯 铜盘 彩色织毯 石刻板 骨笛 贝币 沙漏 绿松石 天青釉瓶 彩珠串 黄铜宝盒 孔雀羽 黄金甲虫 琥珀香料 金面具 翡翠圣蛇 宝石油灯 太阳圆盘 流砂水晶 沙漠权杖 绿洲圣莲 太阳神冠'),
('雨中村庄','雨村','雨滴叶 湿石子 芦苇 荷叶 荷花瓣 莲蓬 芦花 蛙卵石 河蚌 河螺 小水草 竹节 雨靴 小雨伞 蓑衣 竹笠 竹篓 渔篓 钓鱼浮 竹鱼竿 小木舟 船桨 水车模型 木桥模型 青瓦片 土陶碗 稻穗 米袋 青团 粽子 芦苇笛 泥哨 花布巾 蓝印花包 纸鸢 燕子木雕 水滴吊坠 琉璃鱼 白瓷莲 铜雨铃 银竹簪 青玉莲蓬 水晶雨蛙 碧玉荷叶 龙纹瓦当 雨虹宝瓶 听雨古琴 碧水宝珠 甘霖龙冠'),
('天空岛','云岛','白云棉 云纹石 青空羽 风铃草 天蓝花 小浮石 银色丝带 风筝骨 轻木片 云朵饼 云色玻璃 纸飞机 风车叶 蓝绸带 白绒球 鸟蛋壳 鸽羽 草编翼 彩色风筝 滑翔翼模型 热气球模型 小飞艇 天使木偶 羽毛笔 晴天娃娃 飞行护目镜 风向仪 浮岛地图 青鸟木雕 云纹瓷盘 天空罗盘 飞翼徽章 白银羽坠 蓝晶风铃 水晶云马 流云宝匣 云端时钟 星翼项链 天青宝石 彩虹棱镜 风之竖琴 天空权杖 苍穹宝珠 云海圣冠')]
qualities=[dict(id=i,key=k,name=n,color=c,targetPercent=p) for i,(k,n,c,p) in enumerate([
('common','普通','9AAAB8',60),('fine','精良','76C98A',25),('rare','稀有','64B5F6',10),('epic','史诗','C48AF1',4),('legendary','传说','F4C45E',1),('special','特殊','F49CB9',0)])]
# Largest remainder: 350 / 146 / 58 / 23 / 6, sum exactly 583.
counts=[350,146,58,23,6]
# Each theme progresses from everyday objects to elaborate artifacts.
# Allocate exact global quotas while keeping quality tied to object value.
quality_by_id={}
legend_scenes={0,2,5,6,8,11}
start=0
for scene in range(12):
 total=44 if scene==11 else 49
 local=[0,13 if scene<2 else 12,4 if scene>=10 else 5,1 if scene==11 else 2,int(scene in legend_scenes)]
 local[0]=total-sum(local)
 cursor=start
 for q,count in enumerate(local):
  for i in range(cursor,cursor+count):quality_by_id[i]=q
  cursor+=count
 start+=total
for q,count in enumerate(counts):
 assert sum(v==q for v in quality_by_id.values())==count
 qualities[q]['count']=count
qualities[5]['count']=3
items=[];batches=[]
for scene,(theme,prefix,words) in enumerate(THEMES):
 names=words.split();assert len(names)==(44 if scene==11 else 49),(scene,len(names))
 batch=[]
 for cell,word in enumerate(names):
  i=len(items);q=quality_by_id[i]
  x=dict(id=i,name=prefix+'·'+word,quality=q,scene=scene,source='旅行归来',description=theme+'旅途中带回的'+word+'。',image=f'collection/{i:03}.png',smallImage=f'collection/{i:03}-small.png',batch=scene,cell=cell)
  items.append(x);batch.append(x)
 rows=[]
 for row in range(7):
  cells=batch[row*7:(row+1)*7]
  rows.append(f'Row {row+1}, left to right: '+ '; '.join(f'{x["name"].split("·")[1]} ({qualities[x["quality"]]["name"]} quality)' for x in cells)+(' ; EMPTY transparent cells to the right' if len(cells)<7 else ''))
 prompt=f'''Create a production pixel-art inventory sprite atlas: exactly 7 columns by 7 rows, equally sized cells on a square canvas, no outer margins. Theme: {theme}, cozy handheld creature-raising game travel souvenirs. Every cell has ONE independent centered object from the ordered list below. Each object occupies at most 65% of cell width/height with large empty gutters. All icons use identical scale and bold readable silhouettes, 32x32 native pixel look, 1px continuous dark navy outline, flat limited 12-color palette per object, crisp square pixels, no antialias, no text, labels, numbers, grid lines, shadows, pedestals, or decorative cell frames. Background must be a perfectly flat uniform pure magenta RGB(255,0,255) for chroma-key extraction. No magenta/pink inside objects. The objects are diverse and match their named shapes, not recolors of the same object. 普通=humble natural/simple object; 精良=well crafted; 稀有=bright blue accent; 史诗=purple crystalline detail; 传说=golden exquisite artifact. Do not add any rarity badge or colored frame. Keep all objects fully inside their own cell and every edge/gutter entirely pure magenta. EXACT ROW ORDER:\n'''+ '\n'.join(rows)
 path=ROOT/f'assets/prompts/collection-v1/{scene:02}.txt';path.parent.mkdir(parents=True,exist_ok=True)
 if not path.exists():path.write_text(prompt+'\n')
 batches.append(dict(id=scene,theme=theme,count=len(batch),prompt=str(path.relative_to(ROOT/'assets')),source=f'source/collection-v1/{scene:02}.png',status='pending'))
assert len(items)==583 and len({i['name'] for i in items})==583
items.append(dict(id=583,name='纯净之心',quality=5,scene=None,source='选择永久不进化',description='成长，也可以是守护最初的自己。',image='collection/pure-heart.png',smallImage='collection/pure-heart-small.png'))
items.extend([{'id': 584, 'name': '八徽之冠', 'quality': 5, 'scene': None, 'source': '八座道馆首次通关', 'description': '集齐八枚徽章，铭记与你并肩走过的挑战。', 'image': 'collection/584.png', 'smallImage': 'collection/584-small.png'}, {'id': 585, 'name': '万象图鉴', 'quality': 5, 'scene': None, 'source': '宠物图鉴全部点亮', 'description': '151个形态，每一次相遇都被珍藏。', 'image': 'collection/585.png', 'smallImage': 'collection/585-small.png'}])
catalog=dict(version=1,total=586,travelSlots=583,classification='number-of-distinct-travel-items',rounding='largest-remainder',dropRule='uniform-unowned-travel-items; completed-pool-gives-coins-and-xp-only; special excluded',qualities=qualities,items=items,batches=batches)
(ROOT/'assets/collection-catalog.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n')
print('586 catalog entries; quality quotas:',counts)
