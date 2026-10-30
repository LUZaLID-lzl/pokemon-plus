# 127–136 · 全套已接入 v0.21.1

001–136已接入固件和Web，共68家庭/272技能；本范围十只均有六动作×八姿态。133伊布三条互斥进化已实现，各200金币。永久拒绝/纯净之心、存档v8/40级与两道馆保持。137–138全套画稿已验收，尚未接入；从139身份继续。

135动作v2虽首次生成成功，但睡眠行部分睁眼而被拒绝；v3第6次生成成功，8帧全部低卧闭眼，最终通过。136身份及动作均首次成功；历史失败和拒绝源均保留。

验证：build/pets136-tests.log、pets136-validate-assets.log、pets136-evolution-smoke.log、pets136-preview-smoke.log、pets136-image-bank.log、pets136-firmware.log。正式包dist/pokemon-plus-all-in-one.zip；未刷机。

## 历史记录（以下状态已被上方进度替代）

# 127–136 · 已发布127–132，133–134全套画稿完成，135动作失败

宠物运行仍001–132，127–132已接入。133–134完整48姿态模板已验收但未接入，135身份已验收；135动作v1六次失败。135动作及136–151待续，当前道馆/固件更新另见lookme.md。本轮未刷机。

| 编号 | 宠物 | 身份图版本 / 实际请求次数 | 动作版本 / 实际请求次数 | 状态 |
| --- | --- | --- | --- | --- |
| 127 | 凯罗斯 | v1 / 4次 | v1 / 1次 | 已接入 |
| 128 | 肯泰罗 | v1 / 1次 | v1 / 1次 | 已接入 |
| 129 | 鲤鱼王 | v1 / 1次 | v1 / 3次 | 已接入 |
| 130 | 暴鲤龙 | v2 / 6次 | v3 / 1次 | 已接入 |
| 131 | 拉普拉斯 | v1 / 6次 | v1 / 1次 | 已接入 |
| 132 | 百变怪 | v1 / 1次 | v1 / 1次 | 已接入 |
| 133 | 伊布 | v6 / 3次成功，v1–v5失败历史保留 | v1 / 1次成功 | 已验收，未接入 |
| 134 | 水伊布 | v8 / 6次成功，旧失败历史保留 | v1 / 2次成功 | 已验收，未接入 |
| 135 | 雷伊布 | v1 / 5次成功 | v1 / 6次失败 | 身份已验收，动作待续 |
| 136–151 | 后续宠物 | 未请求 | 未请求 | 待续 |

六只各六动作×八张独立完整姿态、精确双向，共288基础帧/双向576帧。原生48px、主体≤40px、二值透明、每只共享≤15不透明色；原稿、身份48px和全部动作联系表已目视查看。凯罗斯角爪、肯泰罗四足三尾、鱼类相连鳍尾触须、拉普拉斯四鳍甲壳和百变怪整体软体均检查；所有睡眠为低姿闭眼。130动作v1因缩小后细触须脱落拒收、v2因两张睡姿睁眼拒收，v3通过，拒收原稿与原因保留，不把图像成功误报为服务失败。

130身份v1六次失败属于上一轮；随后v2第六次成功。131身份前3次HTTP500、随后2次HTTP400，第6次成功。133身份v1至v5各六次HTTP400/content_policy且无图，历史均保留；再次继续后v6第3次成功，动作v1首次成功。134身份v1/v2/v3/v4/v5/v6/v7均六次HTTP400/content_policy无输出，最新停在v7，按[MobiIoT ImageGen技能](/Users/lzl/.codex/skills/mobiiot-imagegen/SKILL.md)“连续失败达到上限后停止该批次并报告”停止本轮，未请求135–151。来源/尝试日志/提示词/测量框/SHA256详见identity manifest及docs/gen1-127-136/visual-review.json。

129在20级进化130，19级/忙碌时不能进化，已是最终形态不能再次进化。127、128、131、132在第一代独立。按个体永久不进化、零金币免费拒绝、唯一纯净之心赠礼及重载保持。伊布三条互斥分支仍为计划，尚未生成或实现，不能串联133→134→135→136。全部现132形态/67家庭/50多形态链/65次进化/268技能，131套完整48姿态加025旧24槽位，6312播放帧/12624双向帧。旧126 art、62 species和248技能ID不变，存档v6、五上场十一候补保持。

新增五家庭20技能ID248–267，家庭固定四槽，暴鲤龙沿用鲤鱼王家庭四槽。百变怪“变身”采用现有轻量伤害/图元特效，未实现复制对手数据；跃起等同样沿用轻量规则。没有新增逐形态学习表或正作异常状态。

资源14045 BIN /13,689,964字节，assets分区剩1,448,852字节。app0x10000/0x180000、assets0x190000/0xE70000及NVS/PHY保持。原生I4/I8对称透明边裁切、逻辑48px中心、Flash零拷贝和常驻双向描述符保持，无逐帧分配/解码/读图；46背景只在启动解压。IMAGE_LIMIT14300、桌面30MiB保持。当前132能容纳，不代表最终151容量已解决。

核心ASan/UBSan及Web规则、全部源帧/像素/字库、12624帧48/56/96px实际C采样、ESP bank、132形态/268技能45秒共享smoke、65进化及拒绝双分支、真实RGB565圆屏外沿、ESP-IDF5.5.3构建、固件逐字节回读/分区/NVS/组件与合并SHA256/ZIP已通过。旧047–056随机抽样覆盖测试由固定512次改为随家庭数增长的独立种子，仍断言所有家庭覆盖，不改运行RNG。证据build/pets132-*.log。

Web桌面1440与手机390、六只36动作/288缩略图、129→130同排及搜索整链、四独立家庭、真实按钮进化/免费永久拒绝/个体隔离/重载/唯一赠礼通过，无脚本错误。验收前pokemon-plus.*按原字符串逐项恢复；收据build/pets132-web-*.json。离线页保留六动作各八独立帧；共享场景/睡眠/旅行/战斗与65对进化拒绝、268技能GIF齐全。新链进化20fps/3200ms，旧链保持已验收版本。

交付：[六只完整模板](../dist/127-132-animation-templates.zip)、[131套总模板，排除025旧槽位](../dist/001-132-animation-templates.zip)、[六只单姿态身份图](../dist/127-132-identity-art.zip)、127–132各独立动作ZIP与[固件包](../dist/pokemon-plus-all-in-one.zip)。单姿态包不能代替完整动作。旧127–129未接入预览包归档在dist/archive，保留当时历史状态。

127–136整批的runtimeIntegrated=false仅表示整个十只范围尚未完成，integratedNationalIds明确列出127–132已发布。completedThrough=132/nextNumber=133，失败详情和历史不覆盖。未刷机，电脑测试不代表本版实机帧率、触摸或长期运行已通过。

本次续作仅重试133身份v2，无新增成功画稿、动作或运行形态。v1失败历史保留在previousStoppedTasks；failureDetails明确v2/6次，activeTask=null。审计见build/pets151-133-v2-stop-audit.log，固件仍0.20.0且本轮未刷机。

最新续作：133身份v3六次HTTP400/content_policy后停止，无新增成功形象或动作；v1/v2历史保留，当前failureDetails=v3/6次。发布仍001–132/v0.20.0，132之后未完成，所有运行文件及固件包按本轮前哈希核对保持。审计build/pets151-133-v3-stop-audit.log，未刷机。

最新续作v4：133身份六次HTTP400/content_policy后停止，没有新增图片或动作；failureDetails=v4/6次、activeTask=null，v1–v3历史保留。用户明确要求保持编号顺序，下轮仍从133继续。运行001–132/v0.20.0保持；审计build/pets151-133-v4-stop-audit.log，本轮未刷机。

最新续作v5：133身份六次HTTP400/content_policy后停止，无新增图片或动作；activeTask=null、failureDetails=v5/6次，v1–v4历史保留。按用户要求保持编号顺序，后续仍从133开始。001–132/v0.20.0保持；审计build/pets151-133-v5-stop-audit.log，本轮未刷机。

## 最新续作：133全套完成，134六次失败
133身份v6第3次成功，动作master-v1首次成功，48独立姿态/双向与原稿、原生联系表目视通过；8张睡姿全部低卧闭眼。独立预览docs/animation-133/index.html与dist/133-animation-template.zip保留来源、提示词、日志和此前失败历史，gameIntegrated=false。
134身份v1六次HTTP400/content_policy无输出，按技能停止整批，未创建v2或请求135–151。当前failureDetails=134-v1/6次、activeTask=null；133-v1至v5历史保留。画稿artCompletedThrough=133/nextArtNumber=134；正式运行completedThrough=132/nextNumber=133与v0.20.0保持，伊布分支仍待实现。审计build/pets133-art134-stop-audit.log，本轮未刷机。

最新续作134-v2：用户继续后六次HTTP400/content_policy，无图片。134-v1与133旧失败历史保留；activeTask=null，failureDetails=134-v2/6次。133全套画稿保持、134–151待续，后续仍从134开始；正式运行001–132/v0.20.0不变。审计build/pets133-art134-v2-stop-audit.log，未刷机。

最新续作134-v3：再次继续后六次HTTP400/content_policy，无新图片。134-v1/v2及133旧失败历史保留；activeTask=null、failureDetails=134-v3/6次。133全套画稿保持，134–151待续，后续仍从134开始；正式Web和固件001–132/v0.20.0不变。审计build/pets133-art134-v3-stop-audit.log，未刷机。

最新续作134-v4：用户继续后以133身份图为参考，首次加5次重试均HTTP400/content_policy，无图片。按MobiIoT技能停止整批，未创建v5、未请求134动作或135–151。134-v1/v2/v3和133旧失败历史保留；activeTask=null、failureDetails=134-v4/6次。画稿仍完成至133，下一134；正式Web与固件001–132/v0.20.0保持。提示词assets/prompts/identity-134-v4.txt，日志build/identity-134-generation-v4.log；审计build/pets133-art134-v4-stop-audit.log。未刷机。

2026-10-24续作134-v5：用户继续尝试后六次HTTP400/content_policy，无图片；提示词assets/prompts/identity-134-v5.txt，日志build/identity-134-generation-v5.log。按MobiIoT技能停止整批，未创建v6或请求134动作及135–151。失败历史保留，activeTask=null、failureDetails=134-v5/6次。画稿完成至133/下一134，正式Web与固件001–132/v0.20.0保持。审计build/pets133-art134-v5-stop-audit.log，未刷机。

本轮收尾核对：等待生图期间core/game.c、core/game.h、main/game_ui.c和assets/evolution.json发生变化，非本轮生图操作所写，已保留；差异记录build/pets134-v5-concurrent-file-changes.json。运行源码基线保持检查未通过，图片/分区/正式固件包另行核验。

本轮审计限制：完整运行资源审计在sdcard/pokemon-plus/avatars/100-0.bin缺失处停止，且检测到上述4项源码/配置变化，未尝试恢复或覆盖。专项画稿审计build/identity134-v5-focused-art-audit.log核对实际尝试、进度/历史、133源帧哈希、3个画稿包及正式固件基线；不得把专项通过称作全运行资源通过。


## 2026-10-24 · 用户继续后134身份v6六次失败
- 以133已验收身份图作真实家族参考，134水伊布identity-v6首次加5次重试均HTTP400/content_policy，无图片。提示词assets/prompts/identity-134-v6.txt，日志build/identity-134-generation-v6.log；134-v1至v5和133旧失败历史保留。按MobiIoT技能“连续失败达到上限后停止该批次并报告”停止本轮，未建v7、未请求134动作或135–151。
- 画稿完成至133／下一134，133完整48姿态模板未接入；宠物运行范围仍001–132，伊布三互斥分支未实现。activeTask=null、failureDetails=134-v6/6次。本轮未刷机或操作设备／NVS／浏览器状态。
- 开始时正式固件为道馆版v0.20.1；保留其他工作。基线build/pets151-before134-v6.json，期间CMakeLists.txt、main/game_ui.c、web/app.js、web/index.html发生非本生图任务所写的变化，记录build/pets134-v6-baseline-differences.json，未覆盖；不宣称所有运行源码保持基线。专项验收build/identity134-v6-focused-art-audit.log与画稿包分别核对。


## 2026-10-24 · 用户继续后134身份v7六次失败
- 以已验收133伊布为参考执行134水伊布identity-v7，首次加5次重试均HTTP400/content_policy，无图片。提示词assets/prompts/identity-134-v7.txt，日志build/identity-134-generation-v7.log；134-v1至v6及133旧失败历史保留。按MobiIoT技能“连续失败达到上限后停止该批次并报告”停止本轮，未创建v8，未请求134动作或135–151。
- 画稿完成至133／下一134；133完整48姿态模板仍未接入，宠物运行仍001–132，伊布三互斥进化尚未实现。activeTask=null、failureDetails=134-v7/6次；本轮未刷机或操作设备/NVS/浏览器状态。保留道馆及其他工作区更新，本任务不修改运行代码。
- 基线build/pets151-before134-v7.json，差异build/pets134-v7-baseline-differences.json；专项验收build/identity134-v7-focused-art-audit.log。133独立模板、133合集和127–133单姿态包同步当前记录。
- 本轮起初按旧对话状态误调用已有v4任务，被本地防重复检查拦截，未发服务请求。v4权威六次日志与提示词未改；被覆盖的辅助wrapper已注明情况，拦截输出保存在build/identity134-local-v4-guard-rejection.log，未将它计为生图尝试。


## 2026-10-24 · 134水伊布全套与135雷伊布身份完成，135动作失败
- 134身份v8第6次成功，以已验收133为真实家族参考；原稿和48px身份目视通过，鳍耳/围领/四足/背脊/鱼尾相连。此前134-v1至v7各六次失败保留。134动作master-v1第2次成功，六动作×八独立完整身体姿态，原稿/原生联系表目视通过；睡眠8帧全部低卧闭眼，四足移动、相连鱼尾摆动、原生≤40px/≤15色/二值透明/精确镜像通过。
- 135雷伊布身份v1第5次成功，原稿/48px形象目视通过；黄色刺毛、长耳、白色颈毛与完整四足结构保留。135动作master-v1六次HTTP400/content_policy，无图片，依MobiIoT技能“连续失败达到上限后停止该批次并报告”停止整批。提示词assets/prompts/animation-135/master-v1.txt，日志assets/prompts/animation-135/generation-v1.log；未建135动作v2、未请求136–151。
- 画稿完成至134／下一135动作；身份已至135。运行宠物仍001–132，133–134全套与135身份均未接入，伊布三互斥分支未实现；不把133→134→135→136串联。activeTask=null、failureDetails=animation-135-v1/6次。用户保持编号顺序，后续从135动作恢复。
- 134独立预览docs/animation-134/index.html及dist/134-animation-template.zip；新增133–134模板合集dist/133-134-animation-templates.zip与127–135身份包dist/127-135-identity-art.zip，均明确未接入。基线build/pets151-before134-v8.json、差异build/pets134-art135-stop-baseline-differences.json、验收build/pets134-art135-stop-audit.log；保留其他工作区更新，本生图任务不修改运行代码，未刷机或操作设备/NVS/浏览器。


2026-10-24续作结果：137多边兽与138菊石兽均完成真实身份和六动作×八姿态原稿／原生验收，尚未接入；139多刺菊石兽身份v1六次HTTP400/content_policy无输出，按MobiIoT技能停止整批，后续仍从139身份继续。未请求139动作或140–151；正式运行001–136／v0.21.1保持。
