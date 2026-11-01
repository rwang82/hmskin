# hmskin Integration Guide

[中文](INTEGRATION.zh-CN.md) | **English**

hmskin is a Win32 skinning library: **hooks existing windows + GDI/GDI+
bitmap self-drawing + JSON manifest driven**.
This guide targets C++ integrators: minimal hookup, manifest syntax, the
control (WMP) catalog, the event table and known pitfalls.
A living example: the whole `demo/` application is a runnable integration
sample.

---

## 1. Minimal hookup ( five steps )

```cpp
#include "hmskin_cpp.h"
#include "hmskin_event_listener_base.h"

// (1) implement an event listener - every control interaction surfaces here
class my_listener : public hmskin_event_listener_base {
public:
    virtual void onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
        // see the section-5 event table; lParam1 is usually the keelobj pointer
    }
};
static my_listener g_listener;

// (2) start the engine: host instance + listener + manifest path ( exe-relative )
hm_startup_param param( hInstance, &g_listener );
hm_uiroot* pRoot = NULL;
hm_lib_startup( &param, _T("myapp.hm.manifest"), &pRoot );

// (3) connect the skin after creating the window ( PropId is your pick;
//     the manifest's MsgSink / CoordinateSystem entries reference it )
CreateWindow( ... );                       // your window
hm_wndpropid_connect( pRoot, hWnd, 258 );  // 258 = the value the demo main window uses

// (4) receive events and drive your app; fetch controls by id for
//     runtime control
hm_keel_obj* pKeelObj = NULL;
hm_get_keelobj( pRoot, 47, &pKeelObj );    // an idKeelObj from the manifest
hm_wmp_scrollbar* pBar = dynamic_cast< hm_wmp_scrollbar* >( pKeelObj->getWMP( 70 ) );
pBar->setValRange( 0, 100 );
hm_wmp_set_text( pRoot, 47, 62, _T("READY") );  // text WMP: public API, cache
                                                // invalidates itself

// (5) at exit ( popup-class windows with self-managed lifetimes must
//     disconnect first - see section 3, iron rule 3 )
hm_lib_shutdown( pRoot );
```

You never forward window messages - the engine hooks/subclasses and
dispatches on its own ( `hm_hs_wnd_msg_dispatch` ).
Linking: `hmskin.lib` ( import library ); at runtime `hmskin.dll` + the
manifest + the skin bitmap directory sit next to the exe.

### Runtime front-door quick reference ( hmskin.h, all addressed by manifest id )

| Domain | Functions |
|---|---|
| Session | `hm_lib_startup` / `hm_lib_shutdown` |
| Window binding | `hm_wndpropid_connect` / `hm_wndpropid_disconnect` |
| keelobj control | `hm_get_keelobj` ( advanced-face pointer ) / `hm_keelobj_set_visible` / `hm_keelobj_set_clip_children` / `hm_keelobj_get_layout_rect` / `hm_keelobj_dump_tree` ( JSON tree dump, agent/debug ) |
| Text | `hm_wmp_set_text` / `hm_wmp_set_text_at` ( multi-text targeting ) / `hm_wmp_get_text` ( two-call read ) |
| Rendering | `hm_set_drawtool_all` ( `HM_DRAWTOL_GDI/GDIPLUS/D2D` backend swap for every WMP ) / `hm_layout_scale`, `hm_layout_scale_px` ( 96-baseline conversion ) |
| Menus | `hm_lib_enable_trackpopupmenu_hook` / `hm_lib_popupmenu_show` |

Common operations ( chapter switching, scroll containers, text, invalidation
rects ) **never need an engine pointer** - ids in, results out;
control-level and layout-level configuration goes through the section-3
"advanced face".

---

## 2. Core concepts ( five-minute version )

| Concept | What it is |
| --- | --- |
| **keelobj** | A node of the control tree ( has layout, visibility and parent/child ownership ). `idKeelObj` is its numeric identity in the manifest |
| **WMP** | Window message processor = the control behavior class ( button / checkbox / scrollbar ... ). Attached to a keelobj; one keelobj can carry several |
| **manifest** | The JSON skin description: bitmap table, font table, color table, coordinate systems, the control tree, per-WMP drawing config |
| **MsgSink** | Message-source binding: PropId ( window property mark ) or CtrlId ( child control id ) - decides which window's messages a WMP receives |
| **CoordinateSystem** | A coordinate system: some window's client or window area; layout decoder rects are expressed inside it |
| **district** | A drawing partition inside a WMP: a group of showobjs ( bitmap / text / solid color + optional posdecoder ) |

## 3. Manifest section quick reference

```
RawPic           { DirPath, PicFiles:[{id, name}] }   bitmap table ( PNG, alpha ok )
Font             [{id, FaceName, Style, SizeUnit, Size}]   Style: REGULAR|BOLD|ITALIC
Color            [{id, R, G, B}]
CoordinateSystem [{CSId, WndMarkFlag:PropId|CtrlId, Type:Client|Window}]
MsgSink          [{idMsgSink, WndMark:PropId|CtrlId}]
WndMsgProcessor  [{idWMP, WMPClsName, DrawTool, DistrictObjs, ...various Cfg}]
KeelObj          [{idKeelObj, WndMsgSource, ParentKeelObjId, LayoutObj{CSId, LayoutDecoder}, WMP:[{id, RegEvts}]}]
```

**Three iron rules**:
1. `ParentKeelObjId` decides **draw-tree ownership** ( a child outside its
   parent's subtree never draws ); `RefKeelObjId` ( inside the
   LayoutDecoder ) only sets the **positioning reference**. They are
   independent - do not conflate.
2. The KeelObj array order = load order; an object referenced by
   `RefKeelObjId` must already exist.
3. **One PropId <-> one HWND** ( `hm_rs_hwnddepository` asserts it );
   popup-class windows with self-managed lifetimes must
   `hm_wndpropid_disconnect` on both death paths.

### Layout decoder quick reference

| Decoder | Semantics | On window resize |
| --- | --- | --- |
| `hm_layoutdecoder_corner` | anchored to a corner of the reference rect + **fixed size** | follows the corner, size unchanged |
| `hm_layoutdecoder_dock` | docked to an edge + **fixed thickness**, fills the rest | thickness axis unchanged, rest follows |
| `hm_layoutdecoder_offset` | **inset** on four sides ( right/bottom are insets, positive = inward ) | width and height stretch with the reference |
| `hm_layoutdecoder_clientarea` | = the host window's client area ( for adopted native control skins ) | follows |

Rules of thumb: edge-following controls use dock ( scrollbar = DOCK_RIGHT,
fixed width, full height ); row-style text uses DOCK_TOP ( horizontal flow,
height pinned ). A reference smaller than the insets collapses to an empty
rect ( cleanly hidden - the 2016 vintage used to invert into negative-width
rects; fixed ).

### WMP class catalog

| WMPClsName | Purpose | Key API / notes |
| --- | --- | --- |
| `hm_wmp_bkgnd` | static surface / text / bitmap | text via the public API `hm_wmp_set_text`/`_at` ( pixel cache invalidates itself ) |
| `hm_wmp_button` | button | tri-state bitmaps |
| `hm_wmp_checkbox` | checkbox / toggle | `setCheck(BOOL)` ( interactive substates kept across the check family ); `setRadioStyle` superseded by the radio class |
| `hm_wmp_radio` | radio ( inherits checkbox ) | a checked radio does not flip on re-click ( no mid-state flicker ); fires its own RADIO_CLICK; group semantics ( select one, uncheck siblings ) is app-driven |
| `hm_wmp_tabheader` | tab strip | `left/right` arrow cycling ( wrap ); sprite tri-state + SEL |
| `hm_wmp_scrollbar` | scrollbar | `setValRange/setCurValue/setTrackThumbLen/setStartThumbLen/setEndThumbLen`; wheel auto-steps while hovering the parent container |
| `hm_wmp_rootmenu` | skin menu bar | labels read straight from the HMENU ( GetMenuItemInfo ) |
| `hm_wmp_popupmenu` | skin popup menu | `HMWM_BINDMENU` binds an HMENU; self-sizing, self-drawn items |
| `hm_wmp_button_nca`/`bkgnd_nca`/`drawtrigger_nca` | the non-client trio | title-bar buttons / borders; NC tracking uses `TME_LEAVE\|TME_NONCLIENT` |
| `hm_wmp_drawtrigger` | WM_PAINT taker | composites the tree offscreen, one blit; pair with `layouttrigger` for WM_SIZE relayout |
| `hm_wmp_movewnd` | **layout drives a native window's position** | see section 6, pit 5 |
| `hm_wmp_viewbkcontent` | 2016 scroll-view skeleton ( dormant ) | superseded by the setClipChildren + scrollbar combo |

### The advanced face ( pointer flows ), standard posture

For **control-level / layout-level configuration**, go through
`hm_get_keelobj` and dive in ( the demo is the living textbook ):

```cpp
// (1) control configuration flow: keelobj -> WMP -> concrete control class
hm_keel_obj* pKeelObj = NULL;
hm_get_keelobj( pRoot, 47, &pKeelObj );
hm_wmp_scrollbar* pBar = dynamic_cast< hm_wmp_scrollbar* >( pKeelObj->getWMP( 70 ) );
pBar->setValRange( 0, 100 );

// (2) runtime layout adjustment ( transitional ): decoder parameters are
//     the same knobs the manifest authors; three-step contract =
//     change the parameter -> calcPosition() -> invalidate the target rect
hm_layout_obj* pLayout = pKeelObj->getLayoutObj();
hm_layoutdecoder_corner* pDecoder =
    dynamic_cast< hm_layoutdecoder_corner* >( pLayout->getLayoutDecoder() );
pDecoder->m_space2 = nNewSpace;
pLayout->calcPosition();
RECT rc;
hm_keelobj_get_layout_rect( pRoot, id, &rc );
::InvalidateRect( hWnd, &rc, FALSE );
```

Event callbacks are inherently pointer flows ( `lParam1` = keelobj*,
`getId()` identifies the source ).

---

## 4. Drawing model ( why it never flickers )

The buffering rules ( enforced by the `hm_wmp_display` base; controls only
implement `_render` ):

| Path | Buffering |
| --- | --- |
| Full-window WM_PAINT ( stretch / chapter switch ) | root canvas offscreen composite, single blit; BUF_CACHE_SIZE controls blt their cached bitmap directly |
| Interactive direct draw ( click / hover ) | control-declared strategy: BUF_ALLOC ( build per click-rate draw ) / BUF_CACHE_SIZE ( true pixel cache: render once, blt afterwards ) |

**The invalidation contract ( fully automatic, two channels )**:
(1) control state changes ( `setCheck` etc. ) - the state-aware cache
snapshots and compares, any change re-renders; (2) host text updates
( `hm_wmp_set_text`/`_at` ) - the uires observer chain broadcasts
TEXTCHANGED and the owning WMP invalidates its pixel cache by itself.
Neither hosts nor control authors ever touch InvalidateRect.

Writing a new control: inherit `hm_wmp_display`, implement
`_render(hDC, rcCanvas)` ( local coordinates, oblivious to screen vs
offscreen ), call `drawSelf(pKeelObj, hwnd)` from interaction handlers, and
declare `m_eBufStrategy` in the constructor. Stateful controls follow the
button/tabheader snapshot pattern ( cache the drawn state, call
`invalidateBuffer` on switch ). Hover-leave tracking uses the base-class
`armMouseLeaveTracking/onLeaveArrived`.

---

## 5. Event table ( `hmskin_event_defs.h` )

| Event | lParam1 | lParam2 |
| --- | --- | --- |
| `HMSKIN_EVENT_BTN_CLICK` (1) | keelobj* | - |
| `HMSKIN_EVENT_CHECKBOX_CLICK` (2) | keelobj* | checked BOOL |
| `HMSKIN_EVENT_NCABTN_CLICK` (3) | keelobj* ( id tells min/max/close apart ) | - |
| `HMSKIN_EVENT_TABBTN_CLICK` (4) | keelobj* | selected index ( 0-based ) |
| `HMSKIN_EVENT_ROOTMENU_CLICK` (5) | keelobj* | menu bar item index |
| `HMSKIN_EVENT_POPUPMENU_CLICK` (6) | keelobj* ( the popup ) | command id ( the menu item id ) - the engine popup also posts native WM_COMMAND to the owner on selection |
| `HMSKIN_EVENT_SCROLLBAR_CHANGED` (7) | keelobj* | current value |
| `HMSKIN_EVENT_RADIO_CLICK` (8) | keelobj* | always TRUE ( radios do not flip ) |

Changing control state externally ( e.g. unchecking radio siblings ) goes
through `setCheck` - it broadcasts the state event internally, the owning
keelobj invalidates and repaints, **the app layer never touches
InvalidateRect**. Text likewise: `hm_wmp_set_text`/`_at` ride the uires
observer chain ( section-4 invalidation contract ); multi-text WMPs use
the `_at` variant targeting ( district id, declaration index ).

---

## 6. Known pitfalls ( all stepped on for real )

1. **Text rect height**: GDI+ single-line mode with
   `StringFormatFlagsLineLimit` - a line that does not fit the line height
   is not drawn at all. Text districts must be >= the font's line height
   ( 12pt ~ 19px ). "Vanishes when I change the font" is almost always a
   too-short rect.
2. **Bitmaps are 1:1**: district bitmaps draw stretched as a whole;
   non-native sizes deform and blur. `gen_skin.py` and the manifest sizes
   must stay in sync ( the scrollbar thumb/endbutton lesson ).
3. **Nine-patch has no erase**: NOR-state bitmaps must be opaque ( a
   transparent NOR leaves the previous state bleeding through ). Transparent
   corners are only allowed at rounded corners.
4. **TrackMouseEvent is window-global shared state**: arming is idempotent
   and harmless; **cancelling ( TME_CANCEL ) pulls the rug from under the
   whole window** - controls must not cancel; leave-restoration goes
   through geometry or the base-class leave framework.
5. **Window positions may be manifest-driven**: `hm_wmp_movewnd` +
   `WndMark` calls `MoveWindow` on marked windows during host keelobj
   layout calc. Check the manifest for movewnd bindings before moving
   native windows yourself.
6. **Source encodings**: old comments are GBK - never re-save whole files;
   new files with Chinese need UTF-8 BOM; `.rc` Chinese needs
   `code_page(65001)` + BOM; menu labels ( raw DrawText ) need an explicit
   CJK font.
7. **Teardown order**: uiroot destroys the keelobj factory before the WMP
   factory ( the dependency direction decides the death order ) - do not
   change it back.

---

## 7. Capability boundaries ( the honest list )

- **Not supported**: shadows / blur ( by design stance: depth comes from
  surface tiers + hairlines ), retained-mode compositing, alpha-blended
  windows.
- **DPI**: Phase 1 shipped - `hm_startup_param`'s third argument `uDpi`
  ( 96-baseline manifests scale at the loader choke point; GDI+ point-size
  fonts convert automatically through the Graphics DPI - **never multiply
  by the scale yourself** ); dedicated Cfg geometry ( menu item / NC border
  thicknesses ) scales in step. Phase 2 backlog: per-monitor dynamic
  switching + multi-scale bitmap sets.
- **Transparent menu takeover** ( opt-in ): after
  `hm_lib_enable_trackpopupmenu_hook(root, TRUE)`, the host's native
  `TrackPopupMenu` calls are intercepted and skinned by the engine;
  selections return to the owner with **native semantics** via
  `PostMessage(WM_COMMAND, id)` - zero host cooperation, existing
  WM_COMMAND handlers work untouched. The manifest needs
  `"PopupSkin": {"PropId": N}` ( the skin slot the engine popup connects
  to ). Direct popups go through
  `hm_lib_popupmenu_show(root, hMenu, x, y, hWndOwner)` ( same engine
  path ).
- **Three rendering backends**: per-WMP declaration
  `"DrawTool": "Gdiplus" ( default ) | "Gdi" | "D2D"`. D2D is the hybrid
  ( M1 ): text via DirectWrite; bitmaps / solids / `calcRect` forward to
  GDI+ ( text measurement stays GDI+-metric - switching backends moves no
  layout ). Hosts can also override globally ( the demo's `--drawtool D2D`
  swaps every WMP at runtime ). Perf telemetry is always on: process exit
  drops `hmskin_perf.log` next to the exe ( text draw count/avg/p50/p95/
  histogram ).
- **Not done yet**: a full focus model + focus ring ( backlog ); the D2D
  native bitmap path ( M2 - the discrimination experiment concluded: the
  DCRT BindDC/EndDraw round trip dominates; TextLayout/Brush caching only
  saves another ~30% ).
- The correct scroll-view recipe: container keelobj
  `setClipChildren(TRUE)` + content keelobj taller than the container +
  scrollbar range = content height - container height; all three input
  paths ( drag / arrow buttons / wheel ) work out of the box.
- **Diagnostics APIs ( always on, zero cost )**: drop a `hmskin.trace`
  marker file next to the exe and the whole family wakes up -
  `hmskin_trace.log` ( every dispatched message; first char `2` = include
  the mousemove flood ), `hmskin_invalidate.log` ( invalidation-rect
  census; hosts can add custom points via `hm_util_invlog` ),
  `hmskin_perf.log` ( text-drawing perf stats dumped at process exit ).
  Without the marker everything stays silent.

More design detail in the repo-root `DESIGN.md`; behavioral baseline in
`demo/`.
