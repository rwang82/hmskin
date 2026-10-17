# hmskin

Win32 桌面应用换肤库（skin library）。不要求重写控件——通过 Hook 已有窗口的消息流程，
配合 JSON 皮肤清单（manifest）与 GDI / GDI+ 自绘，把标准 Win32 窗口"就地"换肤。

**宿主程序零配合**：你的代码照常调 `CreateWindow`、`TrackPopupMenu`、响应 `WM_COMMAND`，
hmskin 在引擎内部透明接管渲染与交互。

## 核心特色

### 透明接管（Transparent Takeover）

```cpp
// 宿主代码 —— 完全原生写法，零皮肤概念
case WM_RBUTTONDOWN:
    hMenu = ::CreatePopupMenu();
    ::InsertMenuItem( hMenu, IDM_ABOUT, FALSE, &mi );
    ::TrackPopupMenu( hMenu, TPM_LEFTALIGN, x, y, 0, hWnd, NULL );  // ← 被 hmskin 劫持
    break;

case WM_COMMAND:
    switch ( LOWORD(wParam) ) {
    case IDM_ABOUT:   // ← 选单按原生语义投递回来，现有 handler 无感工作
        ...
    }
```

调用 `hm_lib_enable_trackpopupmenu_hook(root, TRUE)` 后，原生 `TrackPopupMenu` 调用
被引擎接管并皮肤化，选单按 `PostMessage(WM_COMMAND, id)` 送回属主窗口。

### 声明式皮肤（JSON Manifest）

皮肤的全部定义——位图、布局、字体、色板、行为配置——在 JSON 里。
美术改皮肤不碰 C++、不改引擎重编译。

```json
"KeelObj": [
  {
    "idKeelObj": 3,
    "ParentKeelObjId": 12,
    "LayoutObj": {
      "LayoutDecoder": {
        "DecoderType": "hm_layoutdecoder_dock",
        "m_eDockType": "DOCK_TOP",
        "m_thickness": 34
      }
    },
    "WMP": [{ "id": 4, "RegEvts": [6] }]
  }
]
```

### 组合式控件（一份状态机，多种控件）

checkbox 的八态状态机 + 位图族 + 继承钩子 = 复选框 / 开关(Toggle) / 单选(Radio) 三种控件。
换肤只需换位图，行为引擎不变。

## 支持的控件

| 控件 | WMP 类 | 要点 |
|---|---|---|
| 按钮 | `hm_wmp_button` | 三态位图（normal/hover/press）+ 禁用态 |
| 复选框 | `hm_wmp_checkbox` | `setCheck()` 状态 API；八态状态机 |
| 开关(Toggle) | `hm_wmp_checkbox` 换肤 | 药丸位图族即开关，零引擎改动 |
| 单选(Radio) | `hm_wmp_radio` | 继承 checkbox；已选中点击不翻转；组语义由应用层驱动 |
| Tab 标签条 | `hm_wmp_tabheader` | 雪碧图三态 + 选中态；键盘 ←/→ 循环切换 |
| 滚动条 | `hm_wmp_scrollbar` | 值域 API；拖动/端钮/滚轮三路输入；容器内自动步进 |
| 根菜单栏 | `hm_wmp_rootmenu` | 标签直接读 HMENU（`GetMenuItemInfo`） |
| 弹出菜单 | `hm_wmp_popupmenu` | 引擎自建弹窗、自适尺寸、自绘条目；透明接管或主动调用 |
| 静态面/文本 | `hm_wmp_bkgnd` | 文字走 `setWMPDisplayText()` 实时刷新 |
| 非客户区 | `button_nca` / `bkgnd_nca` / `drawtrigger_nca` | 皮肤标题栏、边框、最小化/最大化/关闭三钮 |
| 进度条 | 纯 manifest 拼装 | 轨道 + 填充 dock 厚度即百分比，零引擎改动 |

## 可扩展架构

### 绘制引擎可插拔（DrawTool）

```
hm_ps_drawtool_i (抽象基类)
├── hm_ps_drawtool_gdi      ← GDI 后端
└── hm_ps_drawtool_gdiplus  ← GDI+ 后端（当前默认）
```

manifest 里每个 WMP 声明 `"DrawTool": "Gdiplus"`，换后端只改一个字符串。
将来加 Direct2D / Skia 后端 = 新增一个 `hm_ps_drawtool_i` 实现。

### 皮肤加载器可插拔（Loader）

```
hm_wmp_loader_base (抽象基类)
├── hm_wmp_button_loader
├── hm_wmp_checkbox_loader
├── hm_wmp_radio_loader
├── hm_wmp_scrollbar_loader
├── hm_wmp_popupmenu_loader
├── ... ( 16 个控件加载器 )
└── hm_wmp_loader_depot     ← 按类名分发（"hm_wmp_button" → button_loader）
```

新增控件 = 新增一个 WMP 类 + 一个 loader + depot 注册一行。
`DECLARE_FACTORY` 宏自动生成工厂注册，manifest 按类名实例化。

### 布局系统（LayoutDecoder）

| 解码器 | 语义 | resize 时 |
|---|---|---|
| `corner` | 某角 + 定宽高 | 只跟角走，尺寸不变 |
| `dock` | 贴某边 + 定厚 | 厚度不变，其余贴满 |
| `offset` | 四边内缩 | 宽高随参考伸缩 |
| `clientarea` | = 宿主客户区 | 跟随（被接管的原生控件） |

写新解码器继承 `hm_layoutdecoder_i`，loader 注册即可。

### 绘制缓冲框架（hm_wmp_display 基类）

新控件继承 `hm_wmp_display`，只实现 `_render(hDC, rcCanvas)`（局部坐标系，
对"画到屏幕还是离屏"无感），交互处理器里一行 `drawSelf(pKeelObj, hwnd)`。
缓冲策略三档声明式：`BUF_NONE`（直画）/ `BUF_ALLOC`（点击级现建）/
`BUF_CACHE_SIZE`（拖动级按尺寸缓存）。

### DPI 感知

`hm_startup_param` 第三参 `uDpi`（默认 96）——manifest 按 96dpi 基准编写，
loader 在咽喉点自动缩放到物理像素。GDI+ 点制字体由 Graphics 真实 DPI 自动换算。

## 工作原理

```
hm_lib_startup( manifest )          ← 解析 JSON → 装配 keelobj 树 + WMP + 资源
    ↓
hm_wndpropid_connect( hWnd, id )    ← 把皮肤绑到已有 HWND
    ↓
Hook 拦截窗口消息                    ← WH_CALLWNDPROC / WH_MOUSE / API hook
    ↓
消息分发给各 WMP                     ← 按消息类型路由到控件处理器
    ↓
WMP 自绘替代默认渲染                 ← GDI/GDI+ 位图 + 文字 + 九宫格拉伸
    ↓
交互回调宿主( onHKSkinEvent )        ← 观察者模式，8 种事件
```

## 快速集成

```cpp
#include "hmskin.h"
#include "hmskin_event_listener_base.h"

// ① 实现事件监听器
class my_listener : public hmskin_event_listener_base {
    virtual void onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
        // 8 种事件：BTN/CHK/RADIO/TAB/MENU-POP/MENU-BAR/SCROLLBAR/TRACKPOPUP
    }
};

// ② 启动引擎
my_listener listener;
hm_startup_param param( hInstance, &listener, 120 );  // 120 = 系统 DPI
hm_lib_startup( &param, _T("myapp.hm.manifest"), &g_pHMRoot );

// ③ 连接窗口
CreateWindow( ... );
hm_wndpropid_connect( g_pHMRoot, hWnd, 258 );

// ④（可选）启用透明菜单接管
hm_lib_enable_trackpopupmenu_hook( g_pHMRoot, TRUE );

// ⑤ 退出
hm_lib_shutdown( g_pHMRoot );
```

详细指南见 **`doc/INTEGRATION.md`**（五步接入 / manifest 速查 / WMP 清单 / 事件表 / 已知坑）。

## 构建

### 命令行（推荐 CI / 远程协作）

```
build.cmd Debug ARM64        ← 日常开发
build.cmd                    ← 全 6 配置
```

全量输出重定向到 `build.log`，任一失败 exit code 非零。详见 `doc/BUILDING.md`。

### Visual Studio

打开 `hmskin.sln`，配置矩阵 **{Win32, x64, ARM64} × {Debug, Release}**，直接生成。

### 前置

- Visual Studio 2026，工作负载 "Desktop development with C++"
- 组件 **"C++ ATL for v145 build tools"**（代码使用 `AtlConv.h`）
- Windows 10/11 SDK

### 冒烟测试

```
python tools\smoke_test.py --no-screenshot    ← manifest 校验 + 事件链
python tools\smoke_test.py                    ← + 金图容差比对
```

## 仓库结构

| 目录 | 说明 |
|---|---|
| `hmskin/` | 核心 DLL（换肤引擎） |
| `demo/` | 集成示例（「基准台」Gallery：四章节控件标本 + 事件日志 + token 表） |
| `scrolldecorator/` | 独立自绘滚动条 DLL（可脱离 hmskin 单独使用） |
| `tools/` | `gen_skin.py` 位图生成器 + `smoke_test.py` 冒烟测试 |
| `doc/` | `INTEGRATION.md` 集成指南 + `BUILDING.md` 构建指南 |
| `DESIGN.md` | 设计系统（色板/字阶/布局/组件规范） |

## 平台与依赖

- **语言**：C++ (Win32 API + GDI/GDI+)，无 MFC 依赖
- **平台**：Win32 / x64 / **ARM64**（Windows 10/11 SDK）
- **工具集**：Visual Studio 2026 (v145)
- **依赖**：gdiplus + jsoncpp 1.9.6（源码已内置 `hmskin/dependence/json/`）

## 注意事项

- 部分源文件含 GBK 中文注释，请勿"以其他编码重新保存"
- `TreatWarningAsError` 关闭（历史告警 C4267/C4244/C4996 不影响功能）
- DPI Phase 1 已落（静态缩放）；Per-Monitor 动态换档属后续专项

## 项目历史

| 时间 | 事件 |
|---|---|
| 2016 | 初版：VS2008 起步，VS2013 v120_xp，仅 32 位，jsoncpp 0.5 预编译库 |
| 2026-10 | 现代化：VS2026 v145、三平台、jsoncpp 1.9.6 源码、UTF-8 manifest、指针宽度安全化、透明 TrackPopupMenu 接管、DPI Phase 1、脏区裁剪、绘制缓冲框架、冒烟测试基建、集成文档 |
