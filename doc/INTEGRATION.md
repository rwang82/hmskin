# hmskin 集成指南

hmskin 是 Win32 换肤库：**hook 已有窗口 + GDI/GDI+ 位图自绘 + JSON manifest 驱动**。
本文面向 C++ 集成者，覆盖最小接入、manifest 语法、控件（WMP）清单、事件表与已知坑。
活例子：`demo/` 整个演示程序就是一份可运行的集成样例。

---

## 1. 最小接入（五步）

```cpp
#include "hmskin_cpp.h"
#include "hmskin_event_listener_base.h"

// ① 实现事件监听器（所有控件交互都从这里出）
class my_listener : public hmskin_event_listener_base {
public:
    virtual void onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
        // 见 §5 事件表；lParam1 通常是 keelobj 指针
    }
};
static my_listener g_listener;

// ② 启动引擎：传入宿主实例 + 监听器 + manifest 路径（exe 相对路径）
hm_startup_param param( hInstance, &g_listener );
hm_uiroot* pRoot = NULL;
hm_lib_startup( &param, _T("myapp.hm.manifest"), &pRoot );

// ③ 创建窗口后连接皮肤（PropId 任选，manifest 的 MsgSink/CoordinateSystem 引用它）
CreateWindow( ... );                       // 你的窗口
hm_wndpropid_connect( pRoot, hWnd, 258 );  // 258 = demo 主窗口用的值

// ④ 接收事件、驱动业务；可按 id 取控件实例做运行时控制
hm_keel_obj* pKeelObj = NULL;
hm_get_keelobj( pRoot, 47, &pKeelObj );    // manifest 里的 idKeelObj
hm_wmp_scrollbar* pBar = dynamic_cast< hm_wmp_scrollbar* >( pKeelObj->getWMP( 70 ) );
pBar->setValRange( 0, 100 );

// ⑤ 退出时（自管生命周期的弹窗类窗口需先 disconnect，见 §3 铁律 3）
hm_lib_shutdown( pRoot );
```

窗口消息不需要你转发——引擎自行 hook/子类化分发（`hm_hs_wnd_msg_dispatch`）。
链接：`hmskin.lib`（导入库），运行时需要 `hmskin.dll` + manifest + 皮肤位图目录在 exe 旁。

---

## 2. 核心概念（五分钟版）

| 概念 | 是什么 |
| --- | --- |
| **keelobj** | 控件树节点（有布局、有可见性、有父子里含关系）。`idKeelObj` 是 manifest 里的数字身份 |
| **WMP** | 窗口消息处理器 = 控件行为类（按钮/复选框/滚动条……）。挂在 keelobj 上，一个 keelobj 可挂多个 |
| **manifest** | JSON 皮肤描述：位图表、字体表、色表、坐标系、控件树、每个 WMP 的绘制配置 |
| **MsgSink** | 消息源绑定：PropId（窗口属性标记）或 CtrlId（子控件 id）→ 决定 WMP 收哪个窗口的消息 |
| **CoordinateSystem** | 坐标系：某个窗口的客户区/窗口区，布局解码器的 rect 在它里面表达 |
| **district** | WMP 内的绘制分区：一组 showobj（位图/文字/纯色 + 可选 posdecoder） |

## 3. manifest 章节速查

```
RawPic           { DirPath, PicFiles:[{id, name}] }   位图表（PNG，alpha 可用）
Font             [{id, FaceName, Style, SizeUnit, Size}]   Style: REGULAR|BOLD|ITALIC
Color            [{id, R, G, B}]
CoordinateSystem [{CSId, WndMarkFlag:PropId|CtrlId, Type:Client|Window}]
MsgSink          [{idMsgSink, WndMark:PropId|CtrlId}]
WndMsgProcessor  [{idWMP, WMPClsName, DrawTool, DistrictObjs, ...各类 Cfg}]
KeelObj          [{idKeelObj, WndMsgSource, ParentKeelObjId, LayoutObj{CSId, LayoutDecoder}, WMP:[{id, RegEvts}]}]
```

**三条铁律**：
1. `ParentKeelObjId` 决定**绘制树归属**（子不在父子树里就永远不画）；
   `RefKeelObjId`（在 LayoutDecoder 里）只管**定位参照**。两者独立，别混。
2. KeelObj 数组顺序 = 加载顺序；`RefKeelObjId` 引用的对象必须已存在。
3. **一个 PropId ↔ 一个 HWND**（`hm_rs_hwnddepository` 会断言）；弹窗类自管生命周期的窗口，
   两条死亡路径都要先 `hm_wndpropid_disconnect`。

### 布局解码器速查

| Decoder | 语义 | 随窗口 resize 时 |
| --- | --- | --- |
| `hm_layoutdecoder_corner` | 参考矩形某角 + **定宽高** | 只跟着角走，尺寸不变 |
| `hm_layoutdecoder_dock` | 贴参考某边 + **定厚**（thickness），其余方向贴满 | 厚度方向不变，其余跟随 |
| `hm_layoutdecoder_offset` | 四边**内缩**（右/下是 inset，正值向内） | 宽高都随参考伸缩 |
| `hm_layoutdecoder_clientarea` | = 宿主窗口客户区（用于被接管的原生控件皮肤） | 跟随 |

经验：贴边随动控件用 dock（滚动条=DOCK_RIGHT 定宽贴满高）；行式文本用 DOCK_TOP（横流纵高钉死）；
参考比内缩小会塌缩成空矩形（干净隐藏，2016 年的老版本会反转出负宽矩形——已修）。

### WMP 类清单

| WMPClsName | 用途 | 关键 API / 备注 |
| --- | --- | --- |
| `hm_wmp_bkgnd` | 静态面/文本/位图 | 文字走 `setWMPDisplayText`（hmskin_helper） |
| `hm_wmp_button` | 按钮 | 三态位图 |
| `hm_wmp_checkbox` | 复选框/开关 | `setCheck(BOOL)`（保留交互子态跨勾选族）；`setRadioStyle` 已被 radio 类取代 |
| `hm_wmp_radio` | 单选（继承 checkbox） | 已选中点击不翻转（无中间态闪屏）；点击发独立 RADIO_CLICK；组语义（选一灭兄弟）由应用层驱动 |
| `hm_wmp_tabheader` | Tab 条 | `←/→` 键循环切换（wrap）；雪碧图三态+SEL |
| `hm_wmp_scrollbar` | 滚动条 | `setValRange/setCurValue/setTrackThumbLen/setStartThumbLen/setEndThumbLen`；滚轮悬停父容器内自动步进 |
| `hm_wmp_rootmenu` | 皮肤菜单栏 | 标签直接读 HMENU（GetMenuItemInfo） |
| `hm_wmp_popupmenu` | 皮肤弹出菜单 | `HMWM_BINDMENU` 绑 HMENU，自适尺寸、自绘条目 |
| `hm_wmp_button_nca`/`bkgnd_nca`/`drawtrigger_nca` | 非客户区三件套 | 标题栏按钮/边框；NC 跟踪用 `TME_LEAVE\|TME_NONCLIENT` |
| `hm_wmp_drawtrigger` | WM_PAINT 承接 | 整树离屏合成一次 blit；配 `layouttrigger` 处理 WM_SIZE 重排 |
| `hm_wmp_movewnd` | **布局驱动原生窗口位置** | 见 §6 坑 5 |
| `hm_wmp_viewbkcontent` | 2016 滚动视图骨架（休眠） | 已被「setClipChildren + 滚动条」组合方案取代 |

### keelobj 运行时 API（节选）

```cpp
pKeelObj->setVisible( bVisible );          // 章节/显隐
pKeelObj->setClipChildren( TRUE );         // 滚动容器：子树绘制裁剪到自身矩形
pKeelObj->getLayoutObj()->m_rect;          // 当前布局矩形（坐标系内坐标）
pLayoutObj->calcPosition();                // 手动重算（运行时改 decoder 参数后）
```

---

## 4. 绘制模型（为什么不会闪）

三层缓冲规则（`hm_wmp_display` 基类统一执行，控件只实现 `_render`）：

| 路径 | 缓冲 |
| --- | --- |
| 全窗 WM_PAINT（拉伸/章节切换） | 根画布离屏合成，单次 blit；控件级自动跳过嵌套缓冲 |
| 交互直绘（点击/悬停） | 控件声明策略：BUF_ALLOC（点击级现建）/ BUF_CACHE_SIZE（拖动级按尺寸缓存复用） |

写新控件：继承 `hm_wmp_display`，实现 `_render(hDC, rcCanvas)`（局部坐标系，
对"最终画到屏幕还是离屏"无感），交互处理器里一行 `drawSelf(pKeelObj, hwnd)`，
构造函数声明 `m_eBufStrategy`。悬停离开跟踪用基类 `armMouseLeaveTracking/onLeaveArrived`。

---

## 5. 事件表（`hmskin_event_defs.h`）

| 事件 | lParam1 | lParam2 |
| --- | --- | --- |
| `HMSKIN_EVENT_BTN_CLICK` (1) | keelobj* | — |
| `HMSKIN_EVENT_CHECKBOX_CLICK` (2) | keelobj* | 勾选态 BOOL |
| `HMSKIN_EVENT_NCABTN_CLICK` (3) | keelobj*（id 区分 min/max/close） | — |
| `HMSKIN_EVENT_TABBTN_CLICK` (4) | keelobj* | 选中索引（0 基） |
| `HMSKIN_EVENT_ROOTMENU_CLICK` (5) | keelobj* | 菜单栏条目索引 |
| `HMSKIN_EVENT_POPUPMENU_CLICK` (6) | keelobj*（弹窗） | 命令 id（菜单项 id） |
| `HMSKIN_EVENT_SCROLLBAR_CHANGED` (7) | keelobj* | 当前值 |
| `HMSKIN_EVENT_RADIO_CLICK` (8) | keelobj* | 恒 TRUE（radio 不翻转） |
| `HMSKIN_EVENT_POPUPMENU_CLICK` (6) | keelobj*（弹窗） | 命令 id（菜单项 id）——引擎弹窗选中时 WM_COMMAND 已同时投递属主 |

外部改控件状态（如灭单选兄弟）调 `setCheck`——它内部广播状态事件，
宿主 keelobj 自动作废矩形重画，**应用层不需要碰任何 InvalidateRect**。

---

## 6. 已知坑（全部实测踩过）

1. **文本矩形高度**：GDI+ 单行模式带 `StringFormatFlagsLineLimit`——行高放不下的整行不画。
   文本 district 高度必须 ≥ 字号行高（12pt ≈ 19px）。"换字体就隐身"多半是矩形太矮。
2. **位图 1:1**：district 位图是整体拉伸绘制的，非原生尺寸即变形发糊。
   `gen_skin.py` 与 manifest 尺寸必须同步（滚动条 thumb/端钮的教训）。
3. **九宫格无擦除**：NOR 态位图必须不透明（透明 NOR 会让上一态残留）。透明角落只允许在圆角处。
4. **TrackMouseEvent 是窗口级共享资源**：武装幂等无害；**取消（TME_CANCEL）会拆全窗口的台**
   ——控件层不要取消，离开恢复走几何路径或基类 leave 框架。
5. **窗口位置可能被 manifest 驱动**：`hm_wmp_movewnd` + `WndMark` 会在宿主 keelobj 布局
   计算时 `MoveWindow` 被标记窗口。动原生窗口位置前先查 manifest 有没有 movewnd 绑定。
6. **源码编码**：老注释是 GBK，绝不整文件重存；新文件含中文要 UTF-8 BOM；
   `.rc` 中文需 `code_page(65001)` + BOM；菜单文字（裸 DrawText）必须显式选 CJK 字体。
7. **拆除顺序**：uiroot 析构 keelobj 工厂先于 WMP 工厂（依赖方向决定死亡顺序），别改回。

---

## 7. 能力边界（诚实清单）

- **不支持**：阴影/模糊（设计立场：深度靠面层级+发丝线）、保留模式合成、alpha 混合窗口。
- **DPI**：Phase 1 已落——`hm_startup_param` 第三参 `uDpi`（96 基准 manifest 在加载咽喉点缩放；
  GDI+ 点制字体由 Graphics 真实 DPI 自动换算，**切勿再手动乘 scale**）；专用 Cfg 几何
  （菜单条目/NC 边框厚度）同步缩放。Phase 2 backlog：Per-Monitor 动态换档 + 多倍率位图集。
- **透明菜单接管**（opt-in）：`hm_lib_enable_trackpopupmenu_hook(root, TRUE)` 后，宿主的原生
  `TrackPopupMenu` 调用被引擎接管并皮肤化，选单按**原生语义** `PostMessage(WM_COMMAND, id)`
  送回属主窗口——宿主零配合、现有 WM_COMMAND 处理器无感工作。manifest 需声明
  `"PopupSkin": {"PropId": N}`（引擎弹窗连接的皮肤槽位）。主动式弹窗走
  `hm_lib_popupmenu_show(root, hMenu, x, y, hWndOwner)`（同一引擎路径）。
- **渲染后端三选一**：WMP 级声明 `"DrawTool": "Gdiplus"（默认）| "Gdi" | "D2D"`。
  D2D 为混合形态（M1）：文字走 DirectWrite，位图/纯色/`calcRect` 转发 GDI+
  （文本测量保持 GDI+ 度量，跨后端切换布局零位移）。宿主亦可在启动参数整体覆盖
  （`--drawtool D2D`，遍历全部 WMP 运行时换后端）。性能观测常开：进程退出自动
  产出 exe 旁 `hmskin_perf.log`（文字绘制的 count/avg/p50/p95/直方图）。
- **未做**：完整焦点模型+焦点环（backlog）；D2D 位图原生路径（M2，往返成本
  已由 A/B 实测标定）。
- 滚动视图的正确姿势：容器 keelobj `setClipChildren(TRUE)` + 内容 keelobj 高于容器 +
  滚动条值域 = 内容高 − 容器高；三路输入（拖/端钮/滚轮）开箱即用。

更多设计细节见仓库根 `DESIGN.md`；行为基线见 `demo/`。
