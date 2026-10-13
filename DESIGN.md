---
# gstack: design-md-format=spec
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
---

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

单窗口 64/36 非对称分栏。左侧"基准台"（64%）：28px 左对齐基准线上依次是大标题、mono 副标、可交互控件组；右侧"标本索引"（36%）：tab、弹出菜单、滚动条、token 读出。顶部 44px 皮肤标题栏 + 28px 根菜单栏（都是展品），底部 30px 状态栏。分栏之间一条结构性发丝线，不再有第二种分隔物。

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

## Decisions Log
| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-10-13 | 初始设计系统「基准台」 | /design-consultation 三方综合（Codex Graphite Bench + Claude 子代理 Calibration Bench + 主提案）；用户定稿信号橙 #FF5C1F |
| 2026-10-13 | 字体用 Win11 内置雅黑 UI + Cascadia Code | GDI+ CreateFont 零部署；捆绑字体收益不抵 demo 摩擦 |
| 2026-10-13 | 禁用阴影/模糊为立场而非约束 | 发丝线+面层级的仪器感；与引擎能力一致 |
