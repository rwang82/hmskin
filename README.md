# hmskin

Win32 桌面应用换肤库（skin library）。不要求重写控件——通过 Hook 已有窗口的消息流程，
配合 JSON 皮肤清单（manifest）与 GDI / GDI+ 自绘，把标准 Win32 窗口"就地"换肤：
背景、按钮、复选框、Tab、滚动条、弹出菜单、非客户区（标题栏/边框/关闭按钮）等。

- 语言/技术：C++ (Win32 API + GDI/GDI+)，无 MFC 依赖
- 平台：**Win32 / x64 / ARM64**（Windows 10/11 SDK）
- 工具集：Visual Studio 2026（v145）
- 依赖：gdiplus + [jsoncpp](https://github.com/open-source-parsers/jsoncpp) 1.9.6（源码已内置，见 `hmskin/dependence/json/`）

## 工作原理

1. `hm_lib_startup()` 启动库：解析 manifest（图片/字体/颜色/布局/控件描述），注册窗口类，安装 Hook
   （`WH_CALLWNDPROC` 窗口过程钩子、鼠标钩子、滚轮引擎；`TrackPopupMenu` 的 inline API hook 为休眠功能）
2. `hm_wndpropid_connect()` 把某个"窗口属性 ID"的皮肤绑定到任意已有 `HWND`
3. Hook 拦截目标窗口消息 → 按消息类型分发给各 **WMP**（Window Message Processor：按钮/背景/Tab/滚动条……）
   → 自绘替代默认渲染；布局由坐标系统 + PosDecoder/LayoutTrigger 驱动
4. 控件交互通过观察者模式回调宿主程序（`hmskin_event_listener_base`）

## 仓库结构

| 目录 | 说明 |
|---|---|
| `hmskin/` | 核心 DLL（换肤引擎）。`inc/` 公开头文件，`src/` 实现 |
| `scrolldecorator/` | 独立的自绘滚动条 DLL（状态机实现，可脱离 hmskin 单独使用） |
| `scrolldecorator_test/` | 滚动条测试宿主程序 |
| `demo/` | 集成示例（`demo.cpp` + `assets/` 皮肤资源） |
| `doc/` | 历史内部文档（字体度量截图、问题清单） |

核心内部模块（`hmskin/src/win/`）：

| 模块 | 职责 |
|---|---|
| `frame/` | UI 根对象、keel obj（UI 骨架对象）、布局对象、坐标系、PosDecoder |
| `hook_system/` | WndProc/鼠标/滚轮 Hook、inline API hook（按架构生成跳转补丁） |
| `wmp/` | 各控件消息处理器：button、button_nca、checkbox、tab、scrollbar、popupmenu、rootmenu、bkgnd(_nca)、dialogview 等 |
| `paint_system/` | 绘制层：GDI 与 GDI+ 双后端 drawtool、canvas |
| `resource_system/` | 图片/颜色/文本/字体资源管理 |
| `loader/` | manifest 解析（JSON 主路径，XML 备选）与对象装配 |

## 构建

要求：

- Visual Studio 2026，工作负载 "Desktop development with C++"
- 组件 **"C++ ATL for v145 build tools"**（代码使用 `AtlConv.h`）
- Windows 10/11 SDK（工程里 `WindowsTargetPlatformVersion=10.0` 自动取最新）
- ARM64 需勾选 "C++ ARM64 生成工具"（在 ARM64 设备本机编译或在 x64 主机交叉编译均可，工程配置与主机无关）

打开 `hmskin.sln`，配置矩阵 **{Win32, x64, ARM64} × {Debug, Release}**，直接生成即可。

产物位置：

- 所有输出：`<仓库根>\{Platform}\{Configuration}\`（`hmskin.dll/.lib`、`demo.exe`、`scrolldecorator.dll` 与其测试程序同目录）
- hmskin 库副本：`hmskin\lib\{Platform}\`（Debug 带 `D` 后缀，便于外部工程取用；旧脚本 `CopyTo3.0.bat` 仍适用于 Win32）
- demo 的皮肤资源（`demo.hm.manifest` + `image/`）由 PostBuild 从 `demo\assets\` 自动拷到输出目录

运行 `demo.exe` 即可看到换肤效果（背景/按钮/复选框/Tab 标题栏等，含中文与微软雅黑字体渲染）。

## 快速集成

```cpp
#include "hmskin.h"
#include "hmskin_event_listener_base.h"

// 1. 实现事件监听器
class my_listener : public hmskin_event_listener_base {
public:
    virtual void onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
        // uEventId 见 hmskin_event_defs.h：
        //   HMSKIN_EVENT_BTN_CLICK / CHECKBOX_CLICK / NCABTN_CLICK / TABBTN_CLICK
        // lParam1 = hm_keel_obj*；TABBTN_CLICK 的 lParam2 = 选中索引
    }
};

// 2. 启动（manifest 路径建议取 exe 同目录）
my_listener listener;
hm_startup_param param( hInstance, &listener );
hm_lib_startup( &param, _T("myapp.hm.manifest"), &g_pHMRoot );

// 3. 把皮肤绑到窗口（258 这类 ID 与 manifest 中 KeelObj 定义的窗口属性对应）
hm_wndpropid_connect( g_pHMRoot, hWnd, MAINWND_WNDPROPID );

// 4. 退出时
hm_wndpropid_disconnect( g_pHMRoot, hWnd, MAINWND_WNDPROPID );
hm_lib_shutdown( g_pHMRoot );
```

链接 `hmskin.lib`（即输出目录里的导入库），运行时 `hmskin.dll` 需在 exe 旁边。完整示例见 `demo/demo.cpp`。

## 皮肤 manifest（JSON）

manifest 必须为 **UTF-8**（jsoncpp 严格校验；中文字体名/标题直接写 UTF-8 即可，任何系统 locale 下都能正确显示）。
顶层结构与职责（对照 `demo/assets/Debug/demo.hm.manifest`）：

| 键 | 内容 |
|---|---|
| `HMEvent` | 内部事件 ID 对照表 |
| `RawPic` | 图片资源：目录 + 文件→ID 映射（供 `UIRes` 引用） |
| `Font` / `Color` | 字体（FaceName/Style/Size）与颜色资源 |
| `CoordinateSystem` | 坐标系定义（相对窗口/父对象等） |
| `WndMsgProcessor` | 各控件处理器（WMP）实例：类名、区域图、触发器、布局参数 |
| `MsgSink` | 消息槽（消息→处理器的订阅关系） |
| `KeelObj` | UI 骨架对象树：绑定的 WMP、UIRes（图片/文本）、PosDecoder（位置解算） |
| `ENUMDISTRICTID` | 控件区域枚举值文档（无代码消费，仅注释性质） |

片段示例：

```json
"RawPic": { "DirPath": "image", "PicFiles": [
    {"id":1, "name":"btn_nor.bmp"}, {"id":2, "name":"btn_hov.bmp"} ] },
"Font": [
    {"id":1, "FaceName":"微软雅黑", "Style":"BOLD", "SizeUnit":"POINT", "Size":16} ]
```

Debug 与 Release 的 demo manifest 内容略有差异（Debug 含更多控件）。

## 注意事项

- **编辑器编码**：部分源文件含 GBK 中文注释，请勿"以其他编码重新保存"，否则会产生大量无意义 diff
- `TreatWarningAsError` 目前为关闭状态（升级工具链后存在历史告警：C4267/C4244 尺寸转换、C4996 `Json::Reader` 弃用提示等，不影响功能）
- `TrackPopupMenu` 的 inline API hook 为**休眠功能**（调用点在 `hmskin/src/win/hmskin.cpp` 中处于注释状态），三架构补丁代码已就绪但未经实机冒烟
- 未做 DPI 感知：绝对像素皮肤引擎，高 DPI 屏幕上由 DWM 位图拉伸兜底；如需原生高 DPI 属后续专项

## 项目历史

| 时间 | 事件 |
|---|---|
| 2016 | 初版：VS2008 工程起步，VS2013 `v120_xp` 工具集，仅 32 位，jsoncpp 0.5（vc71 预编译库） |
| 2026-10 | 现代化：VS2026 v145、Win32/**x64/ARM64** 三平台、jsoncpp 1.9.6 源码内置、manifest 规范化为 UTF-8 合法 JSON、指针宽度安全化（`SetWindowLongPtr` / `LPARAM` 事件参数）并修复 manifest 编码导致的加载失败 |
