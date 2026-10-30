# 日常场景动态渲染视频

基于 2026-10-27 的 v0.33.28，竖屏 720×1280，24 fps，15 秒。

- 0–5 秒：晨间书桌，神树镜湖、宠物巡游、孵蛋点阵光芒。
- 5–10 秒：午后咖啡桌，常青道馆真实出招、技能特效、伤害和血条变化。
- 10–15 秒：入夜桌面，宠物仓库翻页、打开个体信息、返回。

屏幕是当前共享 LVGL 预览程序连续输出的真实运行画面，使用独立演示存档，不访问设备存档。仓库交互通过预览程序的真实指针输入执行，未拼造界面。每段 120 帧；采样间隔 40 ms，成片以 24 fps 播放。

设备外观参照 `assets/reference/courtyard-watch-20261022.png` 建模，包括香槟银圆框、黑色内圈、深色侧壁和小侧键。外形属于视觉近似，无精确结构尺寸；本片是三维渲染演示，并非设备实拍。镜头进行空间移动，叶片和咖啡蒸汽有动画。桌面木纹由 MobiIoT 图片服务生成，其余模型和动画由 Blender 脚本创建。

`pokemon-plus-life-9x16.mp4` 为成片；`poster.jpg` 为封面；`storyboard.jpg` 为三镜头预览；`verification.json` 为 ffprobe 检查结果；`motion-check.json` 记录帧数、动态差异与成片 SHA256。配乐由本地脚本合成，无外部音乐采样。

## 重建

1. 编译 `preview/build/pokemon_plus`，设置 `SDL_VIDEODRIVER=dummy`。
2. 使用 `--activity egg --scene 12`、`--activity viridian-battle`、`--activity warehouse --warehouse-film`，分别加上 `--intro-seen --demo --seed 42 --at-ms 1 --frame-count 120 --frame-step 40 --frames-dir ...`，录入 `build/lifestyle-v03328/ui-{scene,battle,warehouse}`。
3. 将 PPM 原帧逐张无损转为同名 PNG。设置 `--clock` 只改变预览显示，不写设备时钟。
4. Blender 4.5.9 后台运行 `tools/render_lifestyle_video.py`，分别传入 `-- --shot 0`、`1`、`2`。`--preview` 可先导出第 60 帧。
5. 运行 `python3 tools/finish_lifestyle_video.py` 添加标题、原创合成配乐并编码 H.264/AAC。需 Pillow、NumPy、ffmpeg 及脚本所列 macOS 字体。

视频生成 API 预检返回不可用，因此本片采用本地 Blender EEVEE 渲染。未修改固件逻辑、图片资源包或发布版本，未刷设备。
