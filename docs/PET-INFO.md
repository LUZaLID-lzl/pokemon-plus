# 长按宠物信息（2026-10-24）

长按庭院宠物约 1 秒，先进入该个体的「宠物信息」，展示名字、等级、经验、当前形态的属性、饱腹、清洁、健康、心情、精力和进化状态。点击双向箭头图标按钮才打开候补列表，点击候补才交换，成功后立即返回场景；「添伙伴」和「收回休息」保持原有规则。

长按打开页面时等待触摸释放，手指按住后滑到换宠按钮再松手不会误触。查看信息、打开候补列表和返回都不改变宠物，也不结算摸头奖励。

Web 同步 1 秒定时长按与独立换宠图标。浏览器使用预设等级／经验和照料数值演示相同排版，并显示进化条件或永久不进化决定；浏览器试玩状态与设备存档独立。中文字库补充「信」，缓存追加 `-petinfo3`（v0.22.8）。

验证记录：

- `build/pet-info-touch.log`：真实队列指针长按、滑动松手、主动换宠入口和返回 checksum 不变通过；原快速连点购买、导航和唤醒回归通过。
- `build/pet-info-smoke.log`：完整共享 UI 回归通过，包括实际候补交换、272 技能和 136 形态各 45 秒驻留。
- `build/pet-info-assets.log`：资源、字库、Web/SD manifest 校验通过；vendor 26 项检查通过，硬件参数未改。
- `build/pet-info-web-checks.json`：Tabbit 验证 1440×1000 / 390×844，长按信息、松手不误触、主动换宠与关闭返回通过；手机无横向溢出，浏览器存储不变，无脚本或资源错误。
- 共享截图 `screenshots/page-4.png`、`screenshots/contact-sheet.png` 和 `screenshots/pet-info-web-*.png` 同步，目视无重叠，操作按钮四角满足 r168。
- `build/pet-info-firmware.log`、`build/pet-info-package.log`：ESP-IDF 5.5.3 构建、完整包更新、资源回读、分区、NVS 布局、SHA256 与 ZIP 校验通过。

未刷机，以上为电脑验证。


v0.22.6：换宠入口为36×36px双向像素箭头图标，C与Web使用相同几何与点击区域。信息页仍展示当前长按个体；更换／添伙伴成功后直接返回场景，取消候补选择返回信息页。长按滑动松手不会触发图标，也不会额外摸头。保留原候补交换、旅行／战斗限制和个体进化决定。

本轮证据：`build/pet-swap-touch.log`、`pet-swap-smoke.log`、`pet-swap-edges.log`、`pet-swap-web-checks.json`、`pet-swap-firmware.log`及`pet-swap-package-validation.log`。共享`page-4.png`／`contact-sheet.png`已更新，新页面`screenshots/pet-swap-info.png`及Web桌面／手机截图。未刷机。


v0.22.8美化：232×68身份卡片包含52px底座上的原生48px宠物、18px名字、等级徽签、经验数和细进度条。五项照料改为24px既有图标、12px短标签、14px数值和独立色彩短条；设备数值低于25时切为红色。进化入口位于y230，底部换宠／返回／添伙伴均36px、y286同一基线。使用现有图片与字体，无新增运行图片或帧内缩放；保留长按先看信息、取消不换宠、选择后回庭院。Web新增的添伙伴入口仅选择已有试玩个体，不授予新宠物。

美化截图：`screenshots/pet-profile-info.png`，Web桌面／手机`pet-profile-web-*.png`。验收记录`build/pet-profile-touch.log`／`pet-profile-smoke.log`／`pet-profile-edges.log`／`pet-profile-web-checks.json`／`pet-profile-package-validation.log`；未刷入设备。


## v0.34.12 · 当前形态属性

庭院长按打开的「宠物信息」在经验条下方显示16px属性图标和12px中文名称，双属性并排。仓库点击宠物打开的「个体信息」、以及信息卡进入的同一详情页，也在名字／等级下方显示相同属性。

属性读取与战斗相克一致的当前形态表：庭院按 `pp_art_id`，仓库按 `pp_stored_art`，Web按所选个体的 `art`；不按技能属性或进化家庭的初始形态推断。例如小火龙为火、喷火龙为火／飞行。所有入口重建时更新，不留上一只宠物的第二属性。

复用战斗常量图标和既有12px字库。中文属性名由 `tools/battle_type_icons.py` 与 Web 同源生成；固件只共用同一份图标描述符，不新增图片文件、定时器或逐帧读图。信息卡增加属性行，照料指标与仓库个体值适度调整间距；原操作、数值及v17存档保持。

验证：151形态双向属性／图标专项、原生真实长按／换宠返回和仓库真实指针专项、资产／字体校验；浏览器桌面及390px减少动态模式检查实际Canvas文字、双→单属性切换与返回。共享截图 `screenshots/pet-info-types.png`，发布证据 `../build/pet-info-types-release-checks.json`。固件已构建打包，本轮未刷设备，未重建Windows／Android。


## v0.34.13 · 两页共用信息卡

以「宠物信息」为准，固件 `pet_identity_card` 和Web `drawPetIdentityCard` 同时用于庭院信息页与仓库／个体详情。卡片均位于(64,76)，232×90，18px圆角、相同深色底与边框，52px头像底座、48px原图、18px姓名、等级徽签、实际经验数值和进度条、双属性图标／中文名的位置与配色保持一致。仓库显示所选个体的等级和经验；仅信息页保留可点击进入详情的右箭头，详情不显示没有下一层的箭头。

个体值三行使用27px间距，给统一卡片和底部操作留白。原长按、换宠、命名、上场／收回／替换和放生行为保持，v17存档不变。无新增图片、字体、定时器或逐帧读图。

三组原生（妙蛙种子／小火龙／喷火龙）232×90卡片逐像素对照，差异仅为右箭头49像素；完整真实触控／仓库专项、资产字体及固件包校验通过。浏览器1454／390px稳定绘制后对照卡片内容，仓库与同一个体的详情一致，页面无横溢出或脚本／HTTP错误。截图 `screenshots/pet-card-unified.png`，证据 `../build/pet-card-release-checks.json`。已打包，未刷设备，未重建Windows／Android。
