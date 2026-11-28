# hmskin 核心框架架构

> Win32 换肤库：hook 已有窗口 + GDI/GDI+ 自绘 + JSON manifest 声明式皮肤。  
> 本文面向扩展开发者：各系统如何协作、每个核心基类的虚函数契约（派生类必须  
> 实现什么、参数语义）。签名照抄头文件，行号锚点可回溯源码。

## 1. 分层总览

```
┌──────────────────────────── 宿主进程 ────────────────────────────────────┐
│  hm_lib_startup( manifest 路径, DPI )            hmskin.cpp              │
│    └─ hm_uiroot（会话聚合根: 7 工厂/表 + hm_uicfg 会话配置               │
│       + 宿主事件出口 m_pEventListener,单会话守卫）    hm_uiroot.h        │
│        └─ hm_manifest_loader（10 个 section loader 链,顺序敏感）          │
│                                                                           │
│  ┌─ 窗口接管层 ──────────────────────────────────────────────────────┐   │
│  │ hm_rs_hwnddepository（PropId ↔ HWND 窗口登记簿）                  │   │
│  │ msg_sink_i / MsgDispatcher（消息汇 + 派发器,观察者序派发）         │   │
│  │ hm_wndctrl_popup: 引擎弹窗 HWND（hm_wndctrl_base 派生:注册类+创建）│   │
│  │ hm_hs_apihook: 通用 API 补丁工具（现用于 user32!TrackPopupMenu）   │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  ┌─ 绘制树层 ────────────────────────────────────────────────────────┐   │
│  │ hm_keel_obj（树节点: doDrawAll 递归/dirty 剔除/clipChildren）      │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  ┌─ 布局层 ──────────────────────────────────────────────────────────┐   │
│  │ hm_layout_obj（位置容器,subject+observer 双角色）                  │   │
│  │ hm_layoutdecoder_i + 5 派生（clientarea/wndarea/corner/dock/offset）│  │
│  │ coordinate_system_hwnd（client/window 双型坐标系）                 │   │
│  │ hm_pos_decoder_i + dock/slider/offset（WMP 内部微布局）            │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  ┌─ WMP 层（消息处理器 = 控件行为）───────────────────────────────────┐  │
│  │ hm_wndmsgprocessor_i（基类 + MSGBIND 宏族）                        │   │
│  │ hm_wmp_display（显示基类: district/showobj/三档缓冲/leave 跟踪）   │   │
│  │ 具体控件: bkgnd/button/checkbox/radio/scrollbar/scrollview(编排)/  │   │
│  │           msgout(自持滚动,内嵌 hm_scroll_core)/                  │   │
│  │           tab/rootmenu/popupmenu/movewnd/drawtrigger/            │   │
│  │           layouttrigger/viewbkcontent                            │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  ┌─ 资源层 ──────────────────────────────────────────────────────────┐   │
│  │ hm_rs_uires_i（抽象资源,继承 subject,广播 CONTENTCHANGED）         │   │
│  │   └─ text / pic / color                                           │   │
│  │ hm_rs_rawres_pic/font/color（引用计数原资源,hm_cmn_refcnt）        │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  ┌─ 绘制后端层（Visitor）─────────────────────────────────────────────┐  │
│  │ hm_ps_drawtool_i → gdi / gdiplus / d2d(混合后端) 三实现            │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  ┌─ 事件层 ──────────────────────────────────────────────────────────┐   │
│  │ cmn::hm_cmn_subject / hm_cmn_observer_i（Register 非幂等!）        │   │
│  │ hm_event_defs.h（17 个事件,全部 observer 总线 id,p1/p2 逐条注释）                        │   │
│  │ hmskin_event_listener_base（宿主事件监听面）                       │   │
│  └────────────────────────────────────────────────────────────────────┘   │
│  公共 API: hmskin.h 21 函数正门（宿主只说 manifest id,不碰引擎内部）      │
└───────────────────────────────────────────────────────────────────────────┘
```

## 2. 启动装配线

```
宿主: hm_lib_startup( &param(实例/监听器/DPI), "demo.hm.manifest", &pRoot )
  ├─ 单会话守卫（第二次 startup 拒绝）
  ├─ new hm_uiroot → 工厂/表 + hm_uicfg 就位（m_cfg.m_dblLayoutScale 按 DPI 定标；
  │   m_pEventListener 仅供 WMP 装载期注入,uiroot 自身不调用）  hm_uiroot.cpp:12
  └─ hm_manifest_loader::load → 10 个 loader 顺序执行（顺序敏感!）
       src/win/loader/hm_manifest_loader.cpp:45-52
       wndstyle → popupskin → coordinatesystem → rawpic → font → color
       → WMP（必须在 keelobj 之前）→ msgsink → keelobj
         ├─ WMP loader: WMPClsName 匹配 → 工厂 createObj → _fillWMP
         │   （DistrictObjs/DrawTool/BufStrategy 填充;showobj 经
         │     pushShowObj 代理入区 + 挂 CONTENTCHANGED 订阅）
         └─ KeelObj loader: 逐条目 createObj → WndMsgSource 绑定
             → ParentKeelObjId 挂树 → LayoutObj(CSId+decoder,DPI 咽喉点
             乘 scale) → WMP 数组挂载 + RegEvts
  ├─ hm_wndpropid_connect( pRoot, hWnd, PropId )     窗口登记
  │    → depository 记 PropId↔HWND → 窗口级消息 sink 激活
  └─ hm_lib_enable_trackpopupmenu_hook 可选开弹窗接管

销毁: hm_lib_shutdown —— keelobj 工厂先死(析构解 WMP 订阅) → WMP 工厂后死
```

**装载契约**（hm_manifest_loader）：

- **顺序依赖**：rawpic/font/color → WMP——WMP 的 DistrictObjs 装载期即解析 RawPicId，
  `FontId`/颜色键要查字体/颜色表；WMP + MsgSink → keelobj——keelobj 装载按 id 从工厂
  `getObj` 取 WMP 并绑 WndMsgSource
- **section 必选语义**：除 `PopupSkin`（可选，缺失 = 引擎弹窗禁用）外全部必须声明；
  数组允许为空，节缺失即整装载失败
- **失败行为**：混合体制——资源节（rawpic/font/color 等）逐项 fail-fast；WMP 节逐项
  容错（坏条目跳过、装载继续）。所有失败点经 `HM_LOADER_FAIL`/`HM_LOADER_SKIP` 输出
  file:line 面包屑（Release 亦留痕，OutputDebugString）


## 3. 运行时核心流程

### 3.1 消息流

```
HWND 消息 → skin hook → MsgDispatcher（按 orderWndMsg 注册序遍历观察者）
  → hm_keel_obj::update(HM_EVENT_WND_MSG)              hm_keel_obj.cpp:51
     ├─ 可见链校验（父链上任意隐藏即整树忽略消息）
     ├─ notifyWndMsg → 本节点 WMP 链 doWndMsg（MSGBIND switch 分发）
     ├─ _doWndMsg: 逐 WMP doWndMsg( pKeelObj, CWPSTRUCT*, WNDPROCPARAM* )
     └─ pParamWndProc->m_flagEatMsg & DISABLE_OTHER_MSGOBSRV → *pbContinue=false
        截断后续观察者（如滚动条拖动吃掉鼠标消息）
```

`CWPSTRUCT*` = 原生消息五元组包；`WNDPROCPARAM*` = 处理回写通道  
（`m_flagEatMsg`/`m_lRes`）。派发栈内禁拆除（sink 迭代器悬垂——教训 #18）。

### 3.2 绘制流（WM_PAINT → 像素）

```
WM_PAINT → hm_wmp_drawtrigger（根 WMP,自绘接管）
  → 根 keelobj doDrawAll( hDC, hWnd, prcDirty )        hm_keel_obj.cpp:512
     递归每个节点:
       ├─ 不相交 prcDirty → 整子树剔除（性能: 工作量随损伤面积而非控件数）
       ├─ clipChildren 置位 → SaveDC + IntersectClipRect(自身 rect)
       ├─ _doDrawSelf → Notify(HM_EVENT_DRAWSELF, HEDSPARAM*)
       │    └─ 挂在该 keelobj 的显示 WMP 收到 → _drawWithStrategy
       │         BUF_NONE     直画目标 DC
       │         BUF_ALLOC    交互绘制: 临时位图合成后单次 blit
       │         BUF_CACHE_SIZE 真像素缓存: 干净=单次 blt,脏=重渲
       │         hm_wmp_display.cpp _drawWithStrategy
       │    → _render( hDC, rcCanvas )（控件唯一必须实现的渲染方法）
       │    → district->show → showobj->show → uires->show
       │       → drawtool->draw( hDC, rc, 资源 )（Visitor 后端分发）
       └─ _doDrawChildKeelObjs → Notify(DRAWSUBOBJS) → 子节点递归
```

### 3.3 失效/缓存流（文字变更为例）

```
宿主 hm_wmp_set_text(root, keelobjId, wmpId, text)      hmskin.cpp
  → 定位 WMP 首个 text uires → setText
  → hm_rs_uires_text 广播 HM_EVENT_UIRES_CONTENTCHANGED（基类级契约）
  → hm_wmp_display::_uires_observer::update              hm_wmp_display.cpp:17
  → invalidateBuffer()（m_bBufDirty=TRUE）
  → 下一次绘制走重渲而非缓存 blt
订阅建立时机: pushShowObj 代理压 showobj 时（loader 路径全覆盖）
```

### 3.4 交互流（checkbox 点击为例）

```
WM_LBUTTONDOWN →（消息流 3.1）→ checkbox WMP MSGBIND → _OnLButtonDown
  → 翻 check 态 → Notify(HM_EVENT_WMP_STATECHANGED)
  → 宿主 keelobj 收 → InvalidateRect(自身 rect)     hm_keel_obj.cpp:109
  → paint 周期重绘
  → 同路径再 Notify(HMSKIN_EVENT_CHECKBOX_CLICK) → 宿主 event_listener
```

### 3.5 布局流

```
窗口尺寸变化（layouttrigger WMP 收 WM_SIZE）
  → 根 layout calcPosition()
  → decoderLayout()（本节点 decoder 求 rect）
  → Notify(LAYOUT_POSCHANGED) → 子 layout 收（观察 ref）→ 级联 calcPosition
  → 宿主 keelobj 桥接为 KEELOBJ_POSCHANGED，WMP 经 RegEvts 感知
坐标系: coordinate_system_hwnd CSH_CLIENT/CSH_WINDOW 双型
  mapPosition( 目标 HWND, 目标型, &rc ) 跨窗映射（MapWindowPoints NC 感知）
```

### 3.6 弹窗接管（可选）

```
宿主 TrackPopupMenu( ... ) → hm_hs_apihook（通用补丁工具）拦截
  → hm_lib_popupmenu_show
     ├─ hm_wndctrl_popup 建无框弹窗（wndctrl_base 派生:注册类+创建,
     │  hook 层只管 API 语义）
     ├─ HMWM_BINDMENU 绑 HMENU+属主 → popupmenu WMP 自适尺寸自绘条目
     │  （越出监视器工作区时锚点翻转为右/下对齐,原生行为）
     └─ 模态泵至弹窗关闭 → 选中项以原生 WM_COMMAND 回投宿主 → 宿主零弹窗代码
```

## 4. 核心基类虚函数契约（扩展点）

### 4.1 hm_wndmsgprocessor_i —— WMP 基类（inc/win/wmp/hm_wndmsgprocessor_i.h:16）

继承 subject+observer 双角色。两个纯虚（**必须实现**，但实务用宏生成）：

| 签名                                                                                                             | 参数语义                                                                                 | 派生类义务                                       |
| -------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ | ------------------------------------------- |
| `virtual void doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) = 0;` | pKeelObj=宿主节点（WMP 挂在它身上,几何/可见性从它查）；pWP=原生消息；pParamWndProc=回写通道（置 `m_flagEatMsg` 吃消息） | 按 pWP->message 分发到处理函数                      |
| `virtual void getWndMsgs( msg_container_type& containerWndMsg ) = 0;`                                          | 出参=本 WMP 需要订阅的消息 id 列表                                                               | 声明关心的消息（keelobj::pushbackWMP 时据此向消息源 order） |

**宏族**（声明用 `DECLARE_MSGBIND()`，实现用 `MSGBIND_BEGIN(cls) MSGBIND(WM_X,_OnX) ... MSGBIND_END()` +  
`ORDERWNDMSG_BEGIN(cls) ORDERWNDMSG(WM_X) ... ORDERWNDMSG_END()`）。  
基类 `update()` 带默认空体：显示类控件覆写它收 DRAWSELF 等引擎事件。

### 4.2 hm_wmp_display —— 显示控件基类（inc/win/wmp/hm_wmp_display.h:71）

继承 WMP 基类，加绘制缓冲框架。两个纯虚：

| 签名                                                                                         | 语义                                                                                                            |
| ------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------- |
| `virtual void _createDistrictObjs() = 0;`（protected）                                       | 构造期建 district 分区（new hm_districtobj + \_setDistrictObj(id,...)）。district = 绘制分区（背景/三态区等），manifest 可再填 showobj |
| `virtual void _getAllDistrictId( district_id_container_type& containerIds ) = 0;`（private） | 报告全部 district id（资源 load/unload 遍历用）                                                                          |

可选覆写：

- `virtual void _render( HDC hDC, const RECT& rcCanvas ) {}` —— **自绘控件的唯一渲染方法**  
  （默认空； district/showobj 驱动的控件可不覆写）
- `virtual void update( ... )` —— 覆写收 `HM_EVENT_DRAWSELF`（HEDSPARAM* 带 DC/画布/layout），  
  漏斗进 `_drawWithStrategy`
- 架子方法：`drawSelf(pKeelObj,hWnd)`（交互重绘入口）、`setBufStrategy/invalidateBuffer`、  
  `armMouseLeaveTracking/onLeaveArrived`（悬停离开跟踪,派生 \_OnMouseLeave 首行必调 onLeaveArrived）、  
  `pushShowObj(idDistrict, pShowObj)`（框架 push = 存储+uires 事件订阅）

### 4.3 hm_keel_obj —— 绘制树节点（inc/win/frame/hm_keel_obj.h:13）

**全部虚函数带默认体（可选覆写）**——树机制由基类承担：

- `update()`：事件中枢（WND_MSG 路由/DRAWSUBOBJS 递归驱动/LAYOUTOBJ_CHANGED 重绑/  
  WMP_STATECHANGED 失效）
- `pushbackWndMsgNeed()`：消息需求收集（含子树+自有 WMP）
- 构造 protected + friend 工厂（只能经 KEELOBJ 工厂创建）

### 4.4 滚动:hm_scroll_core + hm_wmp_scrollview + hm_wmp_msgout

纵向滚动的状态机抽成纯类 **hm_scroll_core**(inc/win/wmp/hm_scroll_core.h):viewport/extent/top、
钳位、`isAtBottom`(+2px 容差)、`onExtentGrown(nExtentH)`(对**增长前**的 extent 判贴底 → setExtent →
贴底则 scrollTo(maxTop),否则保位 = 免打扰)、`scrollTo()->moved`、`syncBar(bar, barKeelObj)`
(范围 = 溢出像素、值 = top、滑块定长或视口/内容比例下限 MinThumb)。宿主 WMP 自己订阅输入并
转调 `scrollTo`;bar 的 TRACKTHUMB_MOVED 按值转发(HETMPARAM 堆分配即删,不得缓存)。

**hm_wmp_scrollview**——静态子树滚动(整棵 manifest 子树平移),非 keelobj 子类:

```
viewport keelobj  ( ClipChildren:true, 挂 hm_wmp_scrollview, RegEvts:[11]=KEELOBJ_POSCHANGED )
  ├─ content keelobj   ( LayoutScroll:true → 滚动 = layout 滚动偏移平移整棵子树 )
  └─ scrollbar keelobj ( hm_wmp_scrollbar 通常 dock 右;bar 条内滚轮走它自己的细步 )
```

- 接线全声明式: `ScrollViewCfg{ScrollBarWMPId, ContentKeelObjId, BarKeelObjId?,  
  ContentWMPId?, ContentExtent{W,H}?, ArrowLen, MinThumbLen, FixedThumbLen?,  
  LayoutScroll, WheelStep}`( loader 已 DPI 缩放;WMP 节先于 KeelObj 节解析 → id 懒解析)
- WMP 只做胶水:事件订阅/keelobj 解析/失效/平移,纵向状态全部委托内嵌 core
- 滚轮: 绑 WM_MOUSEWHEEL,命中用**消息 lParam 坐标**(真实/合成输入一致),bar 条内让位
- 关停契约: **被引用 WMP id( bar/内容 )必须小于自身 idWMP**——WMP 工厂按 id 升序销毁,  
  被引用者先死、其 ~subject::EraseAllRegister 自动清订阅;本类析构不回触任何指针  
  ( loader assert 双保险 )
- 已知时序坑(已修): 装载期 layout calcPosition 的 POSCHANGED 早于同条目 WMP 的 RegEvts  
  注册 → 编排器可能错过首次布局 → 懒接线成功后自补一次 `_recalcFromViewport`  
  ( m_bRangeBound ),宿主零依赖

**hm_wmp_msgout**——聊天消息列表(doc/MsgOutEditor.md),**自持滚动**(组合 core,绕开菱形继承
与 MSGBIND 单链):一个 WMP 挂在视口 keelobj 上,**没有内容 keelobj、没有事件总线往返**。

```
viewport keelobj  ( ClipChildren:true, 挂 hm_wmp_msgout, RegEvts:[6,11] )
  └─ scrollbar keelobj ( hm_wmp_scrollbar;由 msgout 直接驱动 )
```

- **RegEvts 必须含 6(DRAWSELF)**——漏了列表永远空白(Debug 接线时 assert 自检);11 取视口尺寸
- 接线: `MsgOutCfg{BarWMPId(必填,< 自身 id), BarKeelObjId?, FontId, TextColor, 自/对方气泡色,  
  系统色, 自/对方头像色, AvatarLen, MaxBubblePct, MsgGap, PadH, PadV, CornerLen,  
  ArrowLen, MinThumbLen, FixedThumbLen?, WheelStep}`;ctor 默认值即 96 基准数,loader 统一缩放
- 条目模型: `hm_msgout_item_i{ measure(rowW)->H, render(hdc, rcContent, drawtool),  
  getBubbleSize }(0,0=填满行)`;列表负责头像/气泡/行定位,条目只画内容。内置 text(走  
  drawtool 三后端)与 image(GDI+,路径或借用的 HBITMAP,≤240 等比缩不放大);宿主经  
  `hm_msgout_append_item` 接入新类型。文字选择能力是可选接口 `hm_msgout_textsel_i`
- 滚动: KEELOBJ_POSCHANGED 取视口高/内容宽(视口宽 − bar 实宽)并重测;追加 → `onExtentGrown`  
  → `syncBar` → 失效视口;自收 WM_MOUSEWHEEL(命中「视口扣 bar 条」,lParam 屏幕坐标)与  
  HMWM_MOUSEWHEEL(惯性距离直接 scrollTo,归零自泊车);bar 的 TRACKTHUMB_MOVED/端钮/条内  
  滚轮经同一 `scrollTo` 汇点
- 渲染: DRAWSELF 里条目按 `row.top − top` 平移绘制,ClipChildren 裁剪;系统行居中细字
- 选取: LButtonDown 定锚(客户区坐标!共享命中测试说屏幕坐标,先 ClientToScreen——混用会吃掉  
  bar 上的点击)→ 拖拽扩选 → LButtonUp 收;WM_KEYDOWN + GetKeyState(VK_CONTROL) 判 'A'/'C'  
  (WM_CHAR 会把 Ctrl+A 变 0x01);复制 CF_UNICODETEXT,消息间 `\r\n`,Notify HMSKIN_EVENT_MSGOUT_COPY。
  高亮矩形为比例近似模型(行距实测、字符按比例),画在文字之下
- 已知延迟项: D2D 后端下 `calcRect(NULL DC)` 会陷入 GDI+ calcRect 的 while(1)(任务 #21),  
  Debug demo 暂停 D2D dogfood

### 4.5 hm_layoutdecoder_i —— 布局解码器（inc/win/frame/hm_layout_obj.h:53）

```cpp
virtual void decoderLayout() = 0;   // 由 ref rect 计算本节点 rect,经 setOutObjRect 写回
```

构造注入 out obj；`setLayoutRefObj` 建立引用（自动挂 POSCHANGED 观察级联）。  
五个现成派生（manifest "DecoderType"）：

| 派生           | 语义         | 关键参数                                             |
| ------------ | ---------- | ------------------------------------------------ |
| `clientarea` | = 参考对象客户区  | —                                                |
| `wndarea`    | = 参考对象窗口区  | —                                                |
| `corner`     | 钉某角,定宽高    | m_eCornerType/m_space1/m_space2/m_width/m_height |
| `dock`       | 贴边,定厚      | m_eDockType/m_space1-3/m_thickness               |
| `offset`     | 四边内缩,随参考伸缩 | m_nOffsetL/T/R/B                                 |

自定义布局 = 派生此类 + manifest 新 DecoderType + loader 解析点。

### 4.6 hm_pos_decoder_i —— WMP 内部微布局（inc/win/frame/hm_pos_decoders.h:6）

```cpp
virtual bool getPosition( const RECT& rcRefObj, RECT& rcDestObj ) = 0;
// 由 district 的画布矩形求 showobj 的显示矩形
```

派生：dock（贴边定厚）、slider（值域→位置,滚动条滑块）、offset（内缩）。district 的 showobj  
经 `PosDecoder` 字段配置。

### 4.7 hm_rs_uires_i —— 抽象资源（inc/win/resource_system/hm_rs_uires_i.h:15）

继承 subject（内容变更广播 CONTENTCHANGED,框架级失效契约在此层）。三个纯虚：

| 签名                                                                                     | 语义                                               |
| -------------------------------------------------------------------------------------- | ------------------------------------------------ |
| `virtual void show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool ) = 0;` | 经指定后端把自己画进 rcCanvas                              |
| `virtual bool loadRawRes( hm_ps_drawtool_i* ) = 0;` / `unloadRawRes(...)`              | 后端原资源加载/卸载（drawtool 分派到 loadRes(pic/font/color)） |

派生：`hm_rs_uires_text / pic / color`。新增资源类型 = 派生 + loader `_createUIRes` 分支 +  
drawtool 后端补 draw 重载。


### 4.8 hm_cmn_refcnt —— 原资源引用计数（inc/common/hm_cmn_refcnt.h:6）

```cpp
virtual void doFirstRef( refres_id_type rrId ) = 0;    // 引用数 0→1: 加载实体
virtual void doFinalRelase( refres_id_type rrId ) = 0; // 引用数 →0: 释放实体
```

`hm_rs_rawres_pic/font/color` 实现之——drawtool 的 loadRes/unloadRes 增减引用，  
实体生命周期由计数驱动。

### 4.9 hm_ps_drawtool_i —— 绘制后端 Visitor（inc/win/paint_system/hm_ps_drawtool_i.h:13）

全纯虚。三实现：gdi / gdiplus / d2d（混合后端:文字 DWrite,余转发 gdiplus）。

```cpp
virtual bool loadRes/unloadRes( hm_rs_rawres_pic* ) = 0;    // 图片编解码
virtual bool loadRes/unloadRes( hm_rs_rawres_font* ) = 0;   // 字体实体
virtual bool loadRes/unloadRes( hm_rs_rawres_color* ) = 0;
virtual void draw( HDC, rcCanvas, hm_rs_rawres_pic*, rcRawResPart ) = 0;   // 九宫格等
virtual bool draw( HDC, rcCanvas, hm_rs_rawres_color* ) = 0;              // 纯色
virtual bool draw( HDC, rcCanvas, hm_uires_text_data*, font*, color*, bool bMultiLine ) = 0;
virtual bool calcRect( HDC, &rcCanvas, szText, font*, color*, bool bMultiLine ) = 0;  // 文字测量
```

新后端 = 实现全接口（d2d 即此路径接入）。注意点制字体自动换算 DPI,loader 勿手动乘 scale。

### 4.10 msg_sink_i —— 窗口消息汇（inc/win/hook_system/hm_hs_wnd_msg_dispatch.h:12）

```cpp
virtual void WndProc( HWND hWndDest, UINT message, WPARAM& wParam, LPARAM& lParam,
    WNDPROCPARAM* pParamWndProc ) = 0;   // 窗口消息入口,转发给观察者链
virtual void OnStartSink( HWND ) {} / OnStopSink( HWND ) {}   // 可选:挂上/摘除钩子
```

派生 `hm_msg_sink` 把消息广播给 orderWndMsg 登记的 keelobj 们。生命周期自管  
（析构前必须反注册——派发器不持有所有权）。

### 4.11 控件内模板方法钩子（示例: checkbox → radio）

`hm_wmp_checkbox` 留两个钩子供 radio 特化：

- `virtual BOOL _keepCheckedOnClick() { return FALSE; }` —— 点击后是否保持选中（radio 覆写 TRUE）
- `virtual unsigned int _getClickEventId() { return HMSKIN_EVENT_CHECKBOX_CLICK; }` —— 对外事件 id（radio 覆写 RADIO_CLICK）

新增行为近似的控件优先走模板方法钩子而非复制类。

### 4.12 cmn::hm_cmn_observer_i —— 全库事件入口

```cpp
virtual void Update( void* pSubject, UINT idEvent, LPARAM p1, LPARAM p2, BOOL* pbContinue ) = 0;
```

约定：pSubject=广播源；p1/p2 语义逐事件见 hm_event_defs.h；`*pbContinue=FALSE` 截断。  
**Register 非幂等**（重复注册同 observer+event 触发 assert）——订阅一处一责。

## 5. manifest 装配契约（声明式皮肤）

```
{
  "WndStyle"/"PopupSkin"/"CoordinateSystem"/"RawPic"/"Font"/"Color",
  "WndMsgProcessor": [ { idWMP, WMPClsName, [控件专属 Cfg], DrawTool,
                         BufStrategy, DistrictObjs: [ { id, ShowObjs: [
                           { UIRes{UIResName,...}, PosDecoder{...} } ] } ] } ],
  "MsgSink":     [ { id, WndMark/CtrlId } ],
  "KeelObj":     [ { idKeelObj, WndMsgSource{Type,MsgSinkId|KeelObjId},
                      ParentKeelObjId, ClipChildren?, LayoutObj{ CSId, LayoutDecoder{...} },
                      WMP: [ { id, RegEvts:[事件id...] } ] } ]
}
```

- WMP 区先于 KeelObj 区解析（keelobj 条目引用 WMP id 时必须已存在）
- 几何数值一律 96 基准,loader 咽喉点乘 `hm_layout_scale`（D2D/DCRT 保持 96 像素制）
- 位数图 = district 实绘尺寸（引擎整体拉伸,非原生比例会变形发糊）
- RegEvts 数值 = hm_event_defs 的 (HM_EVENT_BEGIN+N) 值（如 6=DRAWSELF, 11=KEELOBJ_POSCHANGED）
- 几何键一律素名（`AvatarLen`/`WheelStep`…）,整份 manifest 都是 96 基准,由 loader 咽喉点缩放

## 6. 公共 API 正门（hmskin.h,24 函数）

```
会话:     hm_lib_startup / hm_lib_shutdown / hm_wndpropid_connect / disconnect
弹窗:     hm_lib_enable_trackpopupmenu_hook / hm_lib_popupmenu_show
查询:     hm_get_keelobj / hm_keelobj_get_layout_rect / hm_keelobj_dump_tree
控件操纵: hm_keelobj_set_visible / set_clip_children
文字:     hm_wmp_set_text / set_text_at / get_text
消息列表: hm_msgout_append_text / append_item(宿主条目类) / append_image(路径|HBITMAP)
          / hm_msgout_clear / hm_msgout_get_scroll_top
渲染:     hm_set_drawtool_all / hm_lib_warm_render(首绘预热) / hm_layout_scale(转发)
```

高级面（过渡形态）: getWMP 指针配置流（setValRange 等）、invlog 诊断——  
按控件完善进度逐步收编引擎化（见 DESIGN.md 登记册决策）。
