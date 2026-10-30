#!/usr/bin/env python3
"""Package IDF offsets and a single merged firmware with built-in images."""
from pathlib import Path
import hashlib,json,shutil,subprocess,sys,zipfile
root=Path(__file__).resolve().parents[1]
build=root/'build';dst=root/'dist/firmware-all-in-one'
dst.mkdir(parents=True,exist_ok=True)
args=json.loads((build/'flasher_args.json').read_text())
version=json.loads((build/'project_description.json').read_text())['project_version']
asset_manifest=json.loads((build/'builtin-assets-manifest.json').read_text())
assert asset_manifest['format']=='PPASSET3'
assert asset_manifest['version']==version, (asset_manifest['version'],version)
runtime=json.loads((root/'web/manifest.json').read_text())
form_count=len(runtime['pets']);move_count=len(runtime['skills'])
runtime_move_count=sum(m.get('runtimeIntegrated',True) for m in runtime['skills'])
family_count=len({p['species'] for p in runtime['pets']});chain_count=len(runtime['evolution']['chains'])
assert 'assets.bin' in args['flash_files'].values(),'Built-in resource image missing'
records=[]
for offset,name in args['flash_files'].items():
 source=build/name;target=dst/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy(source,target)
 records.append({'offset':offset,'path':name,'sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'bytes':target.stat().st_size})
for name in ['flasher_args.json','flash_args']:shutil.copy(build/name,dst/name)
merged=dst/'pokemon-plus-all-in-one.bin'
command=[sys.executable,'-m','esptool','--chip','esp32s3','merge_bin','--output',str(merged)]
for key in ['flash_mode','flash_freq','flash_size']:command+=['--'+key,args['flash_settings'][key]]
for item in records:command+=[item['offset'],str(dst/item['path'])]
subprocess.run(command,check=True)
manifest={'project':'pokemon-plus','version':version,'target':'esp32s3','board':'ESP32-S3-Touch-LCD-1.85B','requiresSD':False,'flash_files':records,'flash_settings':args['flash_settings'],'merged':{'path':merged.name,'offset':'0x0','bytes':merged.stat().st_size,'sha256':hashlib.sha256(merged.read_bytes()).hexdigest()}}
(dst/'resource-package.json').write_text(json.dumps({
 'format':asset_manifest['format'], 'version':version,
 'requiredApplicationVersion':version,
 'assets':next(item for item in records if item['path']=='assets.bin')
},indent=2)+'\n')
(dst/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(dst/'README.txt').write_text(f'''Pokemon Plus {version} / ESP32-S3-Touch-LCD-1.85B
内置资源版：不需要 SD 卡，不需要单独复制图片。

单文件刷入（esptool 4.x 命令，在此目录运行）：
python -m esptool --chip esp32s3 --port YOUR_PORT write_flash 0x0 pokemon-plus-all-in-one.bin
esptool 5.x 使用 write-flash；图形刷机工具选择 ESP32-S3，文件地址 0x0。
单文件覆盖 NVS 所在空隙，会清除原存档和 Wi-Fi 配置。

已有存档需保留时，使用包内 IDF 原始分区文件：
python -m esptool --chip esp32s3 --port YOUR_PORT write_flash @flash_args
该方式同样内置资源，无需 SD 卡，保留 NVS 分区。
不需要执行 erase_flash。仅适用于以上板型（16MB Flash）。
本版应用预留1.5MiB、assets起点0x190000/大小0xE70000；应用起点及NVS地址保持。
资源包使用 PPASSET3 格式，版本严格绑定应用版本。resource-package.json 记录资源版本、格式、所需应用版本和 SHA256；启动时会拒绝旧格式、损坏包或不同版本的 assets.bin。资源位置已更新，分区表、应用与assets必须使用本包同版文件，不可混用旧版。

内含宠物动作、场景、图标、技能特效、计步器、尼比、华蓝、枯叶、玉虹、浅红、金黄、红莲及常青道馆。
APP道馆入口经关都地图进入八馆，三宠编队接力，按徽章顺序解锁。
道馆每场胜利按敌队均级计算逐场经验，账号前两次再战胜利领取20–55金币；首通额外30%等级经验条，满级显示实际到账0。
八枚徽章在APP展馆永久展示。地图已通关道馆上方按岩石／水／电／草／毒／超能／火／地面主题显示街机风格击败图片，左右两侧箭头循环切馆。未通关时馆主与三只守馆宝可梦站在门前，点击即可进入；获得对应徽章后隐藏门口队伍。地图标题为城市名与馆主名。等级上限60；旧版进行中道馆保留原HP，重赛使用新等级和经验；超限旧等级迁移为60，保留进化/个体决定/藏品。
板载RTC自动恢复时间；USB／联网校时后自动写回，电池为RTC持续供电时脱离电脑及关机重开均可保时；完全没电后需重新校时。
宠物蛋随机2–4小时；计步每1000步结算，前5000步每档30金币，5001–10000步每档10金币，超过10000步无金币经验。
场景按图鉴不同形态数量（繁荣度）免费解锁：1/3/6/10/15/22/30/40/52/66/82/100/125。重复个体不加繁荣度，旧已购场景保留。
13个场景均有结合地形重绘的孵蛋窝，蛋固定在当前场景的窝位；神树镜湖位于树根。切图或返回菜单不重抽蛋位置，原孵化期限与存档保持。
商店孵化药剂150金币，购买进入背包；使用后立即孵化场上已有的蛋，无蛋不消耗。
133伊布在状态页选择水之石→134水伊布／雷之石→135雷伊布／火之石→136火伊布，各200金币，三条互斥且均为最终形态；永久拒绝按个体锁定并赠送唯一纯净之心。
五种神兽仅在集齐八枚徽章后挑战获胜取得指定蛋，从背包放入庭院孵化；普通孵蛋和野生遭遇排除神兽。
存档v17自动迁移v1至v16，新档提交成功后回收旧版本键；步数、个体资料、永久不进化决定和礼物一同保存。
仓库返回键右侧为纯图标排序，点击循环原序／等级从高到低／当前形态属性分组；场景伙伴始终按原槽位排在最前最多五只。排序仅改变显示，不移动实际个体或历练、旅行、陪钓对象。
每只宠物每日最多20次点击互动，额度内每次心情+3，前5次经验+1；全家金币每日最多25，额度用完后无点击动画或提示；长按查看信息仍可用，BOOT批量互动共用额度。次数随个体保存，换宠、进化、重启不重置，次日恢复。
场景内清醒健康的宠物每累计活动10分钟，有5%机会找到未拥有藏品赠送给主人；咕噜球弹框显示宠物、藏品与品质，礼物和待确认收据随存档保存。熄屏、离线、战斗不累计，集齐后停止赠送。
野生伙伴每400–700步或前台场景活跃20分钟入队，最多缓存3次；出现后场景可见180秒内点击感叹号打开咕噜球面板，打开后无时限；放走／对战均位于球内。默认使用场景内等级最高的可出战伙伴，不提供切换箭头。获胜领取8金币／2倍逐场经验，再主动使用1颗球捕捉，同家庭可重复成为独立个体；奖励只发一次。命名仅支持最多8个小写英文字母；旧昵称保留读取。钓鱼包含27种独立像素鱼、四档稀有度、捕获次数图鉴与精准收竿概率加成；陪钓从非场景仓库伙伴选择并按个体保存，直到主动更换。当前陪钓伙伴需先更换才能上场或放生。金币两侧仅显示伙伴与鱼图鉴图标；池塘树冠轻摆、湖面涟漪、鱼影渐显渐隐、落叶漂浮、浮漂及宠物动作持续播放，图鉴和选宠暂停本局计时。
首次进入从001/004/007选择伙伴；三张新画稿用于选择卡片。
001–024、026–{form_count:03}各自参考已确认原画生成六动作×8帧；025保留已确认原24位移槽位。
137–151全套动作和44个技能已接入；138／140在40级进化，147在30级进化148，再在55级进化149。
技能威力为本游戏自定义数值，实际伤害由威力／形态／等级／属性相克决定；Web按所选形态展示威力。
历练每场胜利基础经验2×(1+floor(0.3×有效敌级))，按等级差／饱腹／心情调整；有效敌级最高自身等级+3。失败只有击倒敌宠才有少量经验。旅行三小时80金币、出发等级经验条20%。医院免费仅恢复健康至60，20金币单宠健康至100，其余状态由照料和休息恢复。
1→60合计35400XP，满级后经验归零。
设置「历练自动补给」默认关闭并独立保存。开启后饱腹／清洁／心情低于40、健康低于30、精力低于20时，优先用背包对应普通道具，不足再用神树精华；睡眠需补至精力60唤醒。只耗真实库存，不购买；返回、熄屏及重启补算有效，缺少必要状态仍结束。出发条件不变。
历练仅选择当前场景内1–3只伙伴；每轮最多一小时，每完成一场精力-1，每10场饱腹-1，每20场清洁-1，低于精力20／饱腹20／健康30自动返程。返回继续后台历练，“退出”立即结算已完成战斗；每5分钟检查发现，50%获得5–8金币、10%获得基础照料物品、四类照料事件各10%为队伍中最需要的一只恢复2点精力／饱腹／清洁／心情。六景各24–28种、共146种普通形态，连续两场不重复对手；每分钟环境事件不发经验。XP小数、消耗余数随个体保存；换仓、重启不刷新。旧v16正在运行的历练保留已得成果并结束，避免仓库参战与新规则冲突。
{chain_count}条进化链已接入，{runtime["egg"]["ordinaryFamilies"]}个家庭参与普通孵化，另5个神兽家庭需挑战指定蛋；019/021在20级、023在22级进化，025在状态页花200金币使用雷之石进化026。
027在22级进化028；029/032在16级进化，再使用200金币月之石；035也使用月之石进化。
077在40级、079在37级、081在30级、084在31级进化；086在34级进化087；088在38级、090用水之石、092在25级后用联系之绳进化，096在26级、098在28级、100在30级、104在28级进化；102用叶之石200金币进化103，106/107/108/113/114/115为本世代独立形态；109在35级进化110、111在40级进化112；116在32级进化117、118在33级进化119、120使用水之石200金币进化121；122–128本世代独立；129在20级进化130暴鲤龙。
进化按当前形态的等级/道具条件判断，进化石及联系之绳均200金币，在状态页直接购买使用。
全部{form_count}形态与左右动画、{runtime_move_count}运行技能内置，无需SD卡；技能目录共{move_count}配置，74个普通家庭与5个挑战专属神兽家庭均可上场。进化前可选择永久保留当前形态，赠送收藏品纯净之心。
583件旅行收藏均有名称和独立像素图，品质数量普通350/精良146/稀有58/史诗23/传说6，特殊纯净之心1件。
旅行从未拥有藏品中等概率抽取，583件集齐后仅金币和经验；七个品质页签、分页和收藏详情保留原收藏位图。
实机刷写与启动验证范围见项目 docs/VALIDATION.md；电脑测试不能替代触摸与长期稳定性测量。
''')
archive=root/'dist/pokemon-plus-all-in-one.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
 for file in sorted(dst.rglob('*')):
  if file.is_file():z.write(file,file.relative_to(dst))
print('Packaged:',archive,'merged bytes:',merged.stat().st_size)
