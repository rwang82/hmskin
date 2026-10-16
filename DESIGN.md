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
**Product context:** hmskin（C++ Win32 换肤库）的 demo 展示程序；观众是集成该库的 C++ 开发者；对标品类是 WinUI Gallery / Qt 示例，但气质取"检定记录单"而非"产品目录"。  
**Mode per surface:** 全部为 Operate 表面（可交互的控件陈列），无营销页。  
**Reference sites:** WinUI Gallery（信息架构参考）、Fluent 2（尺寸/圆角惯例参考）。  
**Key characteristics:** 碳黑台面上唯一的信号橙；每个控件带 mono 标本代号（BTN-01）与 1px 引出线；皮肤标题栏/关闭钮是第一件展品；底部状态栏读出引擎能力（9-PATCH · ALPHA · GDI+）。

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
- **§4 滚动与容器**：VIEW-06 滚动视图（凹陷内容井，含空状态文案"未加载内容——导入 manifest"+ 主按钮，空状态也是展品）；SB-07 独立滚动条标本（thumb 三态并排）；BG-08 背景三级面（surface/sunken/raised）对照。

右侧事件日志（反馈模式的核心应用）：每次控件交互追加一行 mono 记录，如 `BTN-01 CLICK`、`CHK-02 → true`、`TAB-01 → §3`——把库的 `HMSKIN_EVENT_*` 回调 API 变成可见的事件流，这是给集成者最直接的演示。

交互规则：tab 支持键盘方向键循环；所有可交互控件有可见橙色焦点环（键盘可达性）；每章节至少含一个禁用态标本（禁用专用配色，不用透明度）。

## Elevation & Depth

引擎不支持阴影/模糊——这不是约束而是立场：深度全部由面层级（bg→sunken→surface→raised）+ 1px 发丝线 + 6% 白 alpha 内高光构成。凹陷表达"内容井"（滚动视图），凸起表达"选中面"（激活 tab）。绝不出现零偏移光晕。

## Shapes

三级圆角：窗口 8px（Win11 DWM 强制，皮肤只沿圆弧做 1px 内描边）；控件 4px（按钮、复选框、弹出菜单）；微件 3px（滚动块）。嵌套时内圆角=外圆角−间隙。

## Components

每个控件四态齐全：normal / hover / press / disable（禁用=面 #161B21 + 文字 #5B636E，不加透明度糊弄）。按钮三档高度 32/26/40。复选框 16px、绿底深色勾。Tab 激活=凸面+2px 橙顶标，无嵌套卡片。弹出菜单 26px 项高、mono 快捷键右对齐、语义组间 1px 分隔。标题栏关闭钮悬停变红是全局唯一的红色时刻。窗口即展品：标题栏、菜单栏、关闭钮是 demo 的前三件标本。

## Do's and Don'ts

- Do: 每个控件配 mono 标本代号 + 1px 引出线（BTN-01/CHK-02/TAB-03/SB-04）
- Do: 状态栏读出引擎真实能力（9-PATCH · 3-SEG · ALPHA · GDI+ · DPI 96）
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
│     BUF_CACHE_SIZE  拖动级：按尺寸缓存复用
└── _render(hDC, rcCanvas) 纯虚 ← 控件唯一要写的部分
```

**迁移纪律**：不单开重构提交；下一个碰到谁就迁谁（RAD-11 迁 checkbox、下次碰滚动条迁其
drawSelf、再下次 button/bkgnd），一个控件一个提交带实测验证。

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
