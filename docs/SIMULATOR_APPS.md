# 模拟器应用

当前包基于根目录版本v0.33.19及当前资源，存档v13。电脑和Android共用core、game_ui及`preview/simulator_window.c`，游戏逻辑分辨率保持360×360。宿主把原始RGB565画面转换成带圆形alpha的纹理，再等比缩放；不修改固件根screen或用矩形裁切隐藏圆边问题。圆屏外不响应游戏点击，缩放后的触控映射回原始坐标。

圆屏下方四个按钮是`+100步`、`摇一摇`、`+1小时`、`回庭院`。`- / +`调整到可用画面的50%–100%，百分比按钮恢复100%；桌面窗口可拖动边缘调整大小，鼠标滚轮也可缩放。快捷键K／S／空格／Esc保持可用。游戏状态沿用预览的内存生命周期；Android退后台且进程仍在时继续保留，结束进程后重置。

## Windows

输出`dist/pokemon-plus-windows-x64.zip`。完整解压后双击`PokemonPlus.exe`，保留`SDL2.dll`及`resources`。原生命令`sh tools/preview.sh`也使用新的圆屏宿主；离屏导图及原有smoke仍使用原验证路径。

```sh
python3 tools/build_windows.py
```

构建依赖CMake、Ninja、MinGW-w64，SDL2开发包固定2.32.10并校验下载SHA256。打包器校验全部15987项BIN、PE架构／GUI子系统、非系统DLL及ZIP CRC／逐项哈希。当前Mac只做了Windows交叉编译和静态包检查，没有Windows实际运行记录。

## Android

输出`dist/pokemon-plus-android14.apk`，minSdk34、targetSdk36，包含arm64-v8a及x86_64。全部native ELF和APK中native库按16KB页对齐。不需要网络、外部存储或传感器权限；摇晃和步数由显式按钮模拟。首次启动校验并解压内置BIN到应用私有目录，资源版本不变时复用，更新时原子替换。

```sh
python3 tools/build_android.py
```

本次使用Android SDK platform36／build-tools36.0.0、完整NDK25.1.8937393和Android Studio内置JDK21。脚本优先使用已完整安装的最高NDK，也支持`ANDROID_SDK_ROOT`／`ANDROID_NDK_HOME`／`JAVA_HOME`。原生层只用现有C API，NDK25以API33编译，APK安装下限仍严格为Android14。SDL2源码2.32.10从官网下载并校验SHA256；无需Gradle或外部Java依赖。

签名私钥在`android/.signing/pokemon-plus.jks`，已由该目录的`.gitignore`排除。应单独安全备份，保持后续APK升级签名一致；不要加入APK或公开包。旧`build/android-signing`密钥可被脚本迁移复用。普通清理build不应删除正式签名私钥。

GameActivity使用singleTask，入口恢复已有实例，避免重复启动SDL_main。宿主处理前后台、绘制设备重置与窗口尺寸变化；快速触控的按下／释放均立即送入LVGL，避免短触被轮询漏掉。

## 宿主初版验证（v0.33.10）

- 全量`tools/test.sh`通过。`--shell-smoke`覆盖300×420、440×600、800×960、1000×600、圆外输入保护、缩放菜单点击、四个真实快捷接口与缩放还原。
- 10张实际输出帧检查圆形长宽一致、圆外无图像像素。Android14 ARM64模拟设备实测1080×2340和720×1280；50%时菜单点击仍准确，100%显示完整。
- 实际APK触控：计步器100步；蛋3h↔在摇晃后去掉箭头；推进一小时后2h。返回庭院按钮可从菜单／计步页面使用。
- 实际HOME退后台再由Launcher进入：同PID、仅一个SDL_main、缩放及工具栏保留、画面恢复。最初重入创建第二个实例造成黑屏，已修复并复验；旧日志保留，不将首轮当作通过。
- APK签名、min/targetSdk、两ABI、四份ELF的16KB页对齐、zipalign和ZIP校验通过。Android实测为模拟设备，未声称物理手机验收。

证据：`build/simulator-checks/final-checks.json`、`circle-pixel-checks.json`、`android-runtime.log`，以及`build/android/package-checks.json`、`build/windows-x64/package-checks.json`。图片见`docs/screenshots/simulator-desktop.png`、`simulator-android.png`及`simulator-android-flow.png`。本轮不构建或刷入设备固件。

## 新存档随机性（v0.33.12）

Windows、Android与普通电脑模拟器每次新开进程均从操作系统获取新随机种子（Windows BCrypt、macOS／Linux／Android /dev/urandom）。如果读取失败，使用高精度时钟、进程编号与计数混合回退，不回到固定42。正常游玩不在孵化时重新播种；同一局仍保留连续的随机状态。设备固件原本就使用esp_random，逻辑不变。

普通蛋和孵化药剂仍优先抽取未持有的非神兽基础家庭，不保证两局绝不碰巧相同。截图、动作导出、--demo和各smoke保留固定42；--seed N可显式复现指定随机序列。独立进程回归：先构建原生预览，再运行python3 tests/test_simulator_randomness.py，分别检查32次自然孵化与32次药剂孵化，另验固定种子的重放、未拥有优先和神兽排除。

## 成就藏品（v0.33.13）

增加八徽之冠（八馆全部首次通关）与万象图鉴（151形态全点亮），共586件收藏。与固件共享同一条件和界面，旧进度自动满足，不更改v12存档。Windows PE／资源／ZIP检查及Android签名／双ABI／16KB对齐／资源校验通过；本轮未做Windows或Android设备运行。详见`docs/COLLECTION.md`与`build/achievements-release-checks.json`。


## 地区与步行遭遇（v0.33.16）

地图1–12使用各自地区池，普通蛋按孵化完成时所在场景抽取，优先当地未持有家庭。每1500–2500个实际步数出现一只野生伙伴，先显示感叹号，60秒未点击即消失；点击后显示无时限确认面板，不再自动逃走。地图13不产生普通遭遇，普通蛋保留全局池。桌面K与宿主「+100步」使用同一真实计步接口，Windows／Android均共享核心规则。存档仍v12；本轮包格式／资源验证通过，未做Windows／Android实际运行或刷机。地区表与持久化细节见[地区规则](PET_HABITATS.md)。


## 场景伙伴互动第一期（v0.33.18）

新增摸摸回应、空地呼唤、好奇探索、开心小跳、打招呼、结伴散步、并排休息和欢迎回来。Windows／Android与固件共用C行为和LVGL渲染，动作结束从当前位置恢复巡游，睡眠／旅行／喂食时退出相关行为。每日20次摸摸保持，自动行为不发奖励、不新增存档字段。详见[互动规则](HOME_INTERACTIONS.md)。

当前两个安装包均为0.33.18，已完成交叉编译与PE／ZIP／资源或APK签名／双ABI／16KB对齐检查；本版本未做Windows／Android实际运行。真实指针、C/Web行为一致性及共享LVGL局部刷新／圆边检查通过，汇总build/home-interactions-release-checks.json。


## 点击与喂食后移动修复（v0.33.19）

修复旧巡游在互动结束、喂食或唤醒后以当前位置为新起点，却把越界路径截在草坪边缘的问题。现在路径在边缘折返，走路时保持实际移动；互动移动朝向跟随路径方向，像素取整不再导致反复转身。低精力巡游休息从25.6秒缩至9.6秒，精力跨过30时从当前位置开始新周期，不跳到另一段路线。三个发布包均更新，未刷机或进行本版Windows／Android实际运行。
