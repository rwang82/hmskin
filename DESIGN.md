\---  
\# gstack: design-md-format=spec  
name: hmskin Gallery  
description: 工业仪表风「Win32 皮肤基准台」——碳黑台面、1px 发丝线、一根通电的信号橙；克制、精密、带电  
colors:  
  primary: "#FF5C1F"        # 信号橙：可操作/焦点，唯一饱和交互色  
  on-primary: "#141414"     # 橙面上的文字/图形  
  bg: "#0F1216"             # 窗体底  
  surface: "#161B21"        # 面板/标题栏/菜单  
  raised: "#1C2229"         # 凸面（激活 tab、次级按钮）  
  sunken: "#11151A"         # 凹陷（滚动视图、复选框底、滚动条轨道）  
  line: "#262C34"           # 发丝线  
  line-strong: "#363E48"    # 强边框（复选框描边、弹出菜单外框）  
  text: "#E8ECF1"           # 正文  
  text-muted: "#8B939E"     # 次级文字  
  text-faint: "#5B636E"     # 弱文字/禁用  
  accent: "#FF5C1F"         # 与 primary 同源；hover #FF7A47 / press #CC4817  
  success: "#3DDC97"        # 状态绿：只表示"状态为真"（勾选/激活标记）  
  error: "#E5484D"          # 只用于关闭钮悬停与破坏性菜单项  
  inner-highlight: "#FFFFFF0F"  # alpha 内描边（顶边 1px 高光）  
  outer-edge: "#54626C9C"   # 窗框外描边（沿 Win11 8px 圆弧）  
typography:  
  display:  
    fontFamily: "Microsoft YaHei UI"  
    fontWeight: 600  
    fontSize: 22px  
    letterSpacing: 0.01em  
  display-light:  
    fontFamily: "Microsoft YaHei UI"  
    fontWeight: 300  
    fontSize: 28px  
  body:  
    fontFamily: "Microsoft YaHei UI"  
    fontWeight: 400  
    fontSize: 13px  
    lineHeight: 1.6  
  label:  
    fontFamily: "Microsoft YaHei UI"  
    fontWeight: 400  
    fontSize: 12px  
  mono:  
    fontFamily: "Cascadia Code"  
    fontWeight: 400  
    fontSize: 12px  
    letterSpacing: 0em  
  mono-micro:  
    fontFamily: "Cascadia Code"  
    fontWeight: 400  
    fontSize: 11px  
    letterSpacing: 0.08em  
rounded:  
  sm: 3px  
  md: 4px  
  lg: 8px  
spacing:  
  xs: 4px  
  sm: 8px  
  md: 16px  
  lg: 24px  
  xl: 32px  
  measure: 28px            # 左侧对齐基准线  
components:  
  title-bar:  
    height: 44px  
    backgroundColor: "{colors.surface}"  
    innerHighlight: "{colors.inner-highlight}"  
  caption-close-hover:  
    backgroundColor: "{colors.error}"  
  root-menu-bar:  
    height: 28px  
    backgroundColor: "{colors.surface}"  
  button-primary:  
    backgroundColor: "{colors.primary}"  
    textColor: "{colors.on-primary}"  
    borderColor: "#C97F0C"  
    height: 32px  
    rounded: "{rounded.md}"  
  button-secondary:  
    backgroundColor: "{colors.raised}"  
    borderColor: "#303A42"  
    height: 32px  
  checkbox:  
    size: 16px  
    rounded: "{rounded.md}"  
    borderColor: "{colors.line-strong}"  
    backgroundColor: "{colors.sunken}"  
  checkbox-checked:  
    backgroundColor: "{colors.success}"  
  tab-active:  
    backgroundColor: "{colors.raised}"  
    topMarker: "2px {colors.primary}"  
    height: 34px  
  menu-item:  
    height: 26px  
  menu-item-hover:  
    backgroundColor: "#2E3A43"  
  scrollbar-thumb:  
    width: 6px  
    rounded: "{rounded.sm}"  
    backgroundColor: "#4A5660"  
  status-rail:  
    height: 30px  
    backgroundColor: "{colors.surface}"  
\---

# hmskin Gallery

## Overview

**Creative North Star:** 一台让人手痒想拨开关的测量仪器，不是营销页——每个控件都是检定台上的标本。  
**Product context:** hmskin（C++ Win32 换肤库）的 demo 展示程序；观众是集成该库的 C++ 开发者；对标品类是各主流系统的控件画廊示例，但气质取"检定记录单"而非"产品目录"。  
**Mode per surface:** 全部为 Operate 表面（可交互的控件陈列），无营销页。  
**Reference sites:** 主流系统级控件画廊（信息架构参考）、主流设计系统（尺寸/圆角惯例参考）。  
**Key characteristics:** 碳黑台面上唯一的信号橙；每个控件带 mono 标本代号（BTN-01）与 1px 引出线；皮肤标题栏/关闭钮是第一件展品；底部状态栏读出引擎能力（9-PATCH · ALPHA · D2D）。

## Colors

**Strategy:** Restrained——一个饱和交互色 + 中性石墨面。色彩稀缺才有含义。  
**Light or dark:** 暗色。使用者是开发者，长时间凝视、多在室内低光。  
命名规则：橙=可操作/焦点（按钮、按下态滚动块、焦点描边、激活标记）；绿=状态为真（勾选、菜单选中标记），绝不做装饰；红=关闭钮悬停与破坏性项，别处禁用。中性面从 bg→surface→raised→sunken 四级抬升，深度只靠 1px 发丝线与 alpha 内描边。

## Typography

字体取自仪器的世界：开发者日常阅读的终端。中文用微软雅黑 UI（Win11 内置、ClearType 渲染成熟、12px 以下只用 Regular，20px 以上才允许 Light）；拉丁/代号/度量值用 Cascadia Code（Windows Terminal 内置字体，等宽、CJK 感外的"机器"声音）。显示声部是雅黑 UI 600/300 的大小对比，不引入额外字体（GDI+ 私有字体部署成本大于收益，此为明确取舍）。字阶差距靠 size+weight 双档拉开，相邻层级至少差一档字号或两级字重。

## Layout

单窗口 64/36 非对称分栏。左侧"基准台"（64%）：28px 左对齐基准线上依次是大标题、mono 副标、可交互控件组；右侧"标本索引"（36%）：事件日志、token 读出。顶部 44px 皮肤标题栏 + 28px 根菜单栏（都是展品），底部 30px 状态栏。分栏之间一条结构性发丝线，不再有第二种分隔物。

## Gallery Information Architecture

窗口自上而下五层，标本编号统一前缀：

| 层            | 标本              | 说明                                                             |
| ------------ | --------------- | -------------------------------------------------------------- |
| ① 皮肤标题栏 44px | WND-00          | 常驻展品：关闭/最小化/最大化三件套，关闭悬停=红                                      |
| ② 根菜单栏 28px  | BAR-00          | 常驻展品：文件/视图/帮助可真实打开弹出菜单（菜单项覆盖勾选/悬停/禁用/分隔线/快捷键）                  |
| ③ 左侧基准台 64%  | TAB-01 + 四章节    | 章节 tab 承担真实导航（tab 即标本）：§1 基础交互 / §2 文本与数据 / §3 菜单系统 / §4 滚动与容器 |
| ④ 右侧标本索引 36% | EVT-08 / TOK-09 | 固定不随章节切换：事件日志 + token 读出                                       |
| ⑤ 状态栏 30px   | RAIL            | 引擎能力读出                                                         |

四章节内容：

- **§1 基础交互**：BTN-01 按钮矩阵（32/26/40 三档 × normal/hover/press/disable 四态，静态陈列全部状态 + 可交互实例）；CHK-02 复选框按语义分两组单列排布（渲染引擎 / 交互选项），组内含焦点态与禁用态标本；TGL-10 开关标本（36×20 药丸，OFF=凹陷灰钮 / ON=信号橙钮，复选框引擎换肤而成，四态齐全）；RAD-11 单选组标本（15×15 圆，选中=状态绿圆点，与勾选族同语义；GDI/GDI+/AUTO 三选项——独立引擎类 `hm_wmp_radio` 继承 checkbox 状态机：已选中点击不翻转〔杜绝中间态闪屏〕+ 独立 RADIO_CLICK 事件，选一灭兄弟由事件监听层驱动）。
- **§2 文本与数据**：TXT-03 字阶标本（display/body/label/mono/micro 同屏）；TOK-04 token 表格——文本左对齐、数值右对齐 mono、行悬停高亮（表格模式）；PRG-12 进度条标本（260×8 轨道 + 信号橙填充 64%，纯 manifest 拼装零引擎改动——填充厚度即 dock thickness，改百分比只动 manifest）。
- **§3 菜单系统**：MENU-05 弹出菜单标本（由根菜单栏触发，含勾选项/悬停/禁用项/语义分隔线/mono 快捷键右对齐）。
- **§4 滚动与容器**：VIEW-06 滚动视图（真文档滚动：内容井 setClipChildren 裁剪子树，内容 600px 高出井，拖 thumb/点端钮/滚轮三路输入滚动，值域=内容高−井高）；SB-07 独立滚动条标本（thumb 三态并排）；BG-08 背景三级面（surface/sunken/raised）对照。2016 的 viewbkcontent/keelctrl_scrollview 骨架被此组合方案取代（去留另议）。

右侧 TOK-04 交互 token 表（button 行白拿悬停高亮；左列语义说明、右列 mono 色值右对齐）；状态栏读出实测引擎能力（`● LIVE 9-PATCH · 3-SEG · ALPHA · D2D · DPI <动态值>`）。

§3 补充：右键菜单为透明接管标本——demo 调原生 `TrackPopupMenu` 被引擎劫持皮肤化，选单以原生 `WM_COMMAND` 投递（宿主零配合）。

右侧事件日志（反馈模式的核心应用）：每次控件交互追加一行 mono 记录，如 `BTN-01 CLICK`、`CHK-02 → true`、`TAB-01 → §3`——把库的 `HMSKIN_EVENT_*` 回调 API 变成可见的事件流，这是给集成者最直接的演示。

交互规则：tab 支持键盘方向键循环；焦点环体系（引擎焦点模型，backlog——方向键循环已兑现）；每章节至少含一个禁用态标本（禁用专用配色，不用透明度）。

## Elevation & Depth

引擎不支持阴影/模糊——这不是约束而是立场：深度全部由面层级（bg→sunken→surface→raised）+ 1px 发丝线 + 6% 白 alpha 内高光构成。凹陷表达"内容井"（滚动视图），凸起表达"选中面"（激活 tab）。绝不出现零偏移光晕。

## Shapes

三级圆角：窗口 8px（Win11 DWM 强制，皮肤只沿圆弧做 1px 内描边）；控件 4px（按钮、复选框、弹出菜单）；微件 3px（滚动块）。嵌套时内圆角=外圆角−间隙。

## Components

每个控件四态齐全：normal / hover / press / disable（禁用=面 #161B21 + 文字 #5B636E，不加透明度糊弄）。按钮三档高度 32/26/40。复选框 16px、绿底深色勾。Tab 激活=凸面+2px 橙顶标，无嵌套卡片。弹出菜单 26px 项高、mono 快捷键右对齐、语义组间 1px 分隔。标题栏关闭钮悬停变红是全局唯一的红色时刻。窗口即展品：标题栏、菜单栏、关闭钮是 demo 的前三件标本。

## Do's and Don'ts

- Do: 每个控件配 mono 标本代号 + 1px 引出线（BTN-01/CHK-02/TAB-03/SB-04）
- Do: 状态栏读出引擎真实能力（9-PATCH · 3-SEG · ALPHA · D2D · DPI 动态实测值）
- Do: 深度用面层级和发丝线，橙绿红严格按语义
- Don't: 不做紫渐变、三列卡片阵列、居中排版、发光边缘、圆角图标贴片
- Don't: 不给禁用态用透明度替代专色（位图皮肤要为禁用单独出色板）
- Don't: 中文 12px 以下不用 Light/细体

## Motion

- **Approach:** minimal-functional——GDI 立即渲染，只有状态切换（hover/press 的面色变化），无动画。这本身是"仪器"气质的一部分。
- **Easing:** 无（状态即时切换）
- **Duration:** 0ms
- **The one authored moment:** 底部状态栏的 "● LIVE" 绿点常亮——整窗唯一的呼吸感来自它（可静态）。

## 引擎演进：绘制缓冲的框架化（backlog，优先级随控件数量上升）

**问题**："画到屏幕还是离屏"的知识在调用方（WM_PAINT 的 drawtrigger vs 交互消息的 GetDC），
但现在由每个控件自己用 `WindowFromDC` 反推、各自手工实现缓冲——checkbox 事故（2026-10-16
切换状态闪屏）证明每多一个控件就多一处不一致与回归面。控件越多，代价越大，**越早收编越便宜**。

**现状三层规则**（已生效，作为过渡规范）：

| 绘制频率 | 缓冲策略 | 例子 |
| --- | --- | --- |
| 点击/悬停级（每秒几次） | 现建小位图合成后一次 blt | checkbox / 按钮类 |
| 拖动级（每 mousemove） | 按尺寸缓存位图复用，绝不现建 | 滚动条 thumb |
| 全窗重绘（拉伸/章节切换） | 仅根画布一层；控件级跳过（`WindowFromDC == NULL` 判内存 DC） | drawtrigger 路径 |

**目标形态**：收进 `hm_wmp_display` 基类，控件只实现"怎么画"：

```
hm_wmp_display
├── drawSelf()                 ← 交互处理器只调这个；GetDC/去处判别/缓冲/释放全在框架
├── 缓冲策略（控件声明，框架执行）
│     BUF_NONE        画进根画布（WM_PAINT 路径自动归这档）
│     BUF_ALLOC       点击级：现建现毁
│     BUF_CACHE_SIZE  真像素缓存：首绘一次后 blt（脏标记失效）
└── _render(hDC, rcCanvas) 纯虚 ← 控件唯一要写的部分
```

**当前状态**（2026-10-18）：框架全量落地——checkbox 族/button/tabheader/bkgnd
全部接入基类缓冲；BUF_CACHE_SIZE 已升级**真像素缓存**（m_bBufDirty 脏标记，
首绘一次后 blt，WM_PAINT 路径同样命中）；有状态控件走**状态感知快照**模式
（button/tabheader：绘制时快照状态，切换即 invalidateBuffer）。滚动路径文字
绘制实测 1355→131（-90%）。失效双通道全自动：控件状态事件 + uires 观察者链
（文字 TEXTCHANGED → WMP 自动作废缓存），宿主零 InvalidateRect。

**新增：脏区裁剪**（2026-10-17 已落）：doDrawAll 脏矩形重载——无交集子树整棵跳过 +
drawtrigger 用 ps.rcPaint 驱动裁剪遍历 + BitBlt 只拷损坏区。绘制成本从控件数量降为
损坏面积（33 行改动，125% 档实测无白屏）。

**迁移纪律**：不单开重构提交；下一个碰到谁就迁谁，一个控件一个提交带实测验证。

**明确不做**：保留模式合成器（控件只失效矩形、框架统一合成脏区）——等于重写交互模型；
直绘存在的意义就是免整窗重绘的即时反馈，三层规则已拿到该架构的性价比。

**同族横切关注点——悬停离开跟踪**（2026-10-16 tabheader 卡悬停事故立项）：
机制已收编基类（`hm_wmp_display::armMouseLeaveTracking/onLeaveArrived`，NC 控件传
`TME_LEAVE|TME_NONCLIENT`），控件只剩"离开后清什么状态"的语义。迁移纪律同缓冲：
碰谁迁谁（tabheader 已迁，checkbox/button/scrollbar 下次碰时迁）。终极形态 backlog：
分发层用 WH_MOUSE 钩子合成离开事件（分发器见全部 mousemove，天然知道上次命中与
当前的差异）——控件连 TrackMouseEvent 都不用调，但动分发核心，收益边际，暂不做。

## Decisions Log

| Date       | Decision                          | Rationale                                                                                            |
| ---------- | --------------------------------- | ---------------------------------------------------------------------------------------------------- |
| 2026-10-13 | 初始设计系统「基准台」                       | /design-consultation 三方综合（Codex Graphite Bench + Claude 子代理 Calibration Bench + 主提案）；用户定稿信号橙 #FF5C1F |
| 2026-10-13 | 字体用 Win11 内置雅黑 UI + Cascadia Code | GDI+ CreateFont 零部署；捆绑字体收益不抵 demo 摩擦                                                                 |
| 2026-10-13 | 禁用阴影/模糊为立场而非约束                    | 发丝线+面层级的仪器感；与引擎能力一致                                                                                  |
| 2026-10-16 | §1 新增 TGL-10 开关标本（复选框引擎换肤）         | 36×20 药丸八态；证明 checkbox 位图族=开关语义零引擎改动                                                                    |
| 2026-10-16 | 绘制缓冲三层规则 + 框架化 backlog（见「引擎演进」节） | checkbox 切换闪屏根因=直绘无缓冲；规则定型并立项收编基类，控件越多迁移代价越大                                     |
| 2026-10-16 | RAD-11 单选独立成类 hm_wmp_radio | 用户架构裁决：概念应在类型系统有名；继承 checkbox 状态机 + 模板方法钩子，独立 RADIO_CLICK 事件 |
| 2026-10-16 | 绘制缓冲框架化落地（hm_wmp_display 基类 drawSelf + 三档策略） | checkbox 首迁实测通过；checkbox→radio→button 逐步迁移中 |
| 2026-10-17 | TrackPopupMenu 透明接管（M1-4 决断=启用） | 用户拍板"干"；引擎全权处理（PopupSkin manifest 槽位 + 模态环 + WM_COMMAND 原生投递）；宿主零配合；ARM64 补丁器首验 |
| 2026-10-17 | DPI Phase 1（加载咽喉点缩放） | 96 基准 manifest 直达物理像素；GDI+ 点制字体自动换算（勿手动乘）；专用 Cfg 几何三处补缩放 |
| 2026-10-17 | 脏区裁剪（doDrawAll 脏矩形重载 + ps.rcPaint） | BeginPaint 送上门的 update region 被忽视 9 年——接上后绘制成本从控件数量降为损坏面积 |
| 2026-10-17 | 冒烟测试基建（tools/smoke_test.py 三阶段） | manifest 校验 / 事件链断言 / 金图容差比对；后续危险改动的安全网 |
| 2026-10-17 | D2D-M1 混合后端落地（hm_ps_drawtool_d2d） | 文字 DirectWrite 自绘，位图/纯色/calcRect 转发 gdiplus（测量保 GDI+ 度量=布局零位移）；Debug manifest 53 WMP 全切，金图重建为 D2D 基线（0.3%→全切 4.1%→重建）；字形质量与 GDI+ 持平（100% 档小字无肉眼差异） |
| 2026-10-17 | 渲染性能 A/B 设施常开 | hm_perf_stats 零开销聚合 + demo --drawtool 运行时覆盖 + smoke --perf 场景脚本；实测 D2D 1394µs vs GDI+ 166µs（8.4×，DCRT 每次往返 BindDC/EndDraw 主导嫌疑）——M2 位图路径与 TextLayout/批次优化的裁决依据 |
| 2026-10-17 | 装载时序根因修复（9 年潜伏债） | _fillWMP 对调顺序：district 先填充、DrawTool 后装载——旧序下资源预建 load 循环永远空转，unload 路径 9 年未被执行，运行时 setDrawTool 首次触发即 refcount underflow；同时修 D2D 静态析构崩溃（DllMain 语境禁 Release D2D） |
| 2026-10-18 | BUF_CACHE_SIZE 升级真像素缓存（Milestone A） | m_bBufDirty 脏标记 + invalidateBuffer；WM_PAINT 路径同样命中；manifest 声明 `"BufStrategy":"CACHE_SIZE"`；金图 0.0% 实证 |
| 2026-10-18 | 状态感知像素缓存 + 滚动路径收口（Milestone B） | button/tabheader 缓存绘制态快照、切换即失效；真滚轮场景文字绘制 1355→131（-90%） |
| 2026-10-18 | D2D 性能判别实验闭环 | TextLayout/Brush 缓存留库（约省 30%）；判决=DCRT BindDC/EndDraw 往返主导（缓存命中 p50 仍 ≥500µs vs GDI+ <100µs）——M2 出路三选一待裁决 |
| 2026-10-18 | 失效链仪表 hm_util_invlog | hmskin.trace marker 门控写 hmskin_invalidate.log；滚动期「tabheader 被绘制」定谳为采样环视差，脏区剔除健康 |
| 2026-10-18 | 观察者失效链 + 公共文字 API | uires text 可观察化（TEXTCHANGED 广播），WMP 内嵌 observer 收到事件即作废像素缓存；hm_wmp_set_text/_at 宿主只给 manifest id |
| 2026-10-18 | E2E 交互回归套件 | tools/e2e_test.py 九断言（章节金图+交互 EVT+像素 diff），经 demo 内嵌 agent 服务可远程驱动 |
| 2026-10-19 | 对外接口整理专项：正门 11→18 函数 | 登记册 21 触点逐项裁决：11 项升正门（id 寻址：显隐/裁剪/布局矩形/树转储/文字读写/全量换后端/DPI 换算）；其余判过渡高级面（根因=控件不完善，逐控件完善时回收评估）；demo 引擎穿透清零，hmskin_helper 退役 |
