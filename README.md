# hmskin

[中文](README.zh-CN.md) | **English**

**Replace the entire UI of an existing Win32 application with a DirectUI
self-drawing engine** — no UI code rewrite, no reliance on native control
rendering.

hmskin combines two traditionally separate branches of Windows UI history:

| | Classic skin libraries | Mainstream DirectUI frameworks | **hmskin** |
|---|---|---|---|
| Takes over existing apps | ✓ hook + subclassing, zero host changes | ✗ must rewrite UI on its framework | ✓ **hooks existing windows, zero host changes** |
| Control rendering | bitmap skin templates, adapted per standard control type | fully self-drawn (handle-less logical controls) | ✓ **fully self-drawn**: keelobj draw tree + control state machines |
| Skin format | binary skin files, appearance read-only | XML layout + code-driven | ✓ **JSON manifest**: appearance/layout/behavior fully declarative |
| Control behavior | reuses system control logic | self-drawn control classes | ✓ WMP behavior classes + 8 event callbacks |
| Agent introspection | ✗ skin file read-only, no runtime view | partial ( debug inspectors in some frameworks ) | ✓ **built-in agent service**: live control tree / screenshots / synthetic input as MCP tools |

> In one sentence: **the integration cost of a skin library, the rendering
> and behavioral freedom of DirectUI** — a combination neither traditional
> approach offers. The engine turns the host window's client area into a
> DirectUI canvas: native controls are replaced by a self-drawn control tree
> (buttons / checkboxes / toggles / radios / tabs / scrollbars / menus /
> progress bars…), taking over the title-bar buttons, the root menu bar and
> even `TrackPopupMenu` popups; the rendering backend is a one-string choice
> (GDI / GDI+ / Direct2D+DirectWrite).

**Zero host cooperation**: your code keeps calling `CreateWindow`,
`TrackPopupMenu` and handling `WM_COMMAND` as usual — hmskin transparently
takes over rendering and interaction inside the engine.

![hmskin Gallery — skin benchmark bench](doc/img/demo_gallery.png)

> The screenshot is the `demo/` application (written as plain native code
> with zero skin concepts): title bar, menu bar, popup menus, button matrix,
> checkboxes/toggles/radios, tabs, scrollbars — all drawn by the engine.

## Highlights

### Transparent Takeover

```cpp
// host code — 100% native style, zero skin concepts
case WM_RBUTTONDOWN:
    hMenu = ::CreatePopupMenu();
    ::InsertMenuItem( hMenu, IDM_ABOUT, FALSE, &mi );
    ::TrackPopupMenu( hMenu, TPM_LEFTALIGN, x, y, 0, hWnd, NULL );  // ← intercepted by hmskin
    break;

case WM_COMMAND:
    switch ( LOWORD(wParam) ) {
    case IDM_ABOUT:   // ← selection delivered back with native semantics; existing handlers just work
        ...
    }
```

After `hm_lib_enable_trackpopupmenu_hook(root, TRUE)`, native `TrackPopupMenu`
calls are taken over and skinned by the engine; the selection is posted back
to the owner window via `PostMessage(WM_COMMAND, id)`.

### Declarative Skins (JSON Manifest)

Everything a skin defines — bitmaps, layout, fonts, palette, behavior config —
lives in JSON. Artists change skins without touching C++ or rebuilding the engine.

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

### Composable Controls (one state machine, several controls)

The checkbox's eight-state state machine + bitmap families + inheritance
hooks = three controls: checkbox, toggle, radio. Reskinning is just swapping
bitmaps; the behavioral engine stays untouched.

### Agent-Ready Introspection ( AI-agent infrastructure )

The demo embeds a tiny agent service ( TCP 127.0.0.1:9321, JSON line
protocol ) exposing the live control tree, screenshots and synthetic
input. `tools/hmskin_mcp.py` serves it as six MCP tools —
`load_manifest` / `dump_tree` / `live_tree` / `screenshot` /
`click_keelobj` / `stop_demo` — reachable over an ssh tunnel, so any
AI agent can query structures and drive the UI directly instead of
reverse-engineering pixels:

```
live_tree node -> { "id": 3, "parentId": 1, "visible": true,
                    "rect": { "l": 46, "t": 160, "r": 256, "b": 202 } }
click_keelobj(3) -> WM_LBUTTONDOWN/UP verified in the message trace
```

The engine itself stays network-free — it only provides the
introspection interfaces; the host application chooses how to serve
them ( see `tools/hmskin_mcp.py --selftest` ).

## Supported Controls

| Control | WMP class | Notes |
|---|---|---|
| Button | `hm_wmp_button` | tri-state bitmaps (normal/hover/press) + disabled state |
| Checkbox | `hm_wmp_checkbox` | `setCheck()` state API; eight-state machine |
| Toggle | `hm_wmp_checkbox` reskinned | pill bitmap family is a toggle, zero engine changes |
| Radio | `hm_wmp_radio` | inherits checkbox; stays checked on re-click; group semantics app-driven |
| Tab header | `hm_wmp_tabheader` | sprite tri-state + selected state; keyboard ←/→ cycling |
| Scrollbar | `hm_wmp_scrollbar` | value-range API; drag/arrow/wheel input paths; auto-step inside containers |
| Root menu bar | `hm_wmp_rootmenu` | labels read directly from HMENU (`GetMenuItemInfo`) |
| Popup menu | `hm_wmp_popupmenu` | engine-created window, auto-sized, self-drawn items; takeover or direct call |
| Static / text | `hm_wmp_bkgnd` | text refreshes live via `setWMPDisplayText()` |
| Non-client area | `button_nca` / `bkgnd_nca` / `drawtrigger_nca` | skins the title bar, borders, min/max/close buttons |
| Progress bar | pure manifest assembly | track + docked fill thickness = percentage, zero engine changes |

## Extensible Architecture

### Pluggable Renderers (DrawTool)

```
hm_ps_drawtool_i (abstract base)
├── hm_ps_drawtool_gdi      ← GDI backend
├── hm_ps_drawtool_gdiplus  ← GDI+ backend (Release default)
└── hm_ps_drawtool_d2d      ← Direct2D + DirectWrite backend (Debug default)
```

Each WMP declares `"DrawTool": "D2D" / "Gdiplus" / "Gdi"` in the manifest —
switching backends is a one-string edit; the host can also override globally
at runtime (`demo.exe --drawtool D2D`, useful for perf comparisons and
environment adaptation).

The D2D backend is currently a hybrid (M1): **text renders via DirectWrite**
(quality on par with GDI+, more modern glyph antialiasing) while bitmap/
solid/measurement calls forward to GDI+ (layout metrics unchanged). Migrating
the bitmap path to native D2D (M2) is driven by A/B perf data — measured,
each D2D text draw pays a DC render target round trip (1394µs vs 166µs for
GDI+), making round-trip cost the top M2 design constraint. Perf telemetry
is always on (every run automatically produces `hmskin_perf.log`).

### Pluggable Skin Loaders

```
hm_wmp_loader_base (abstract base)
├── hm_wmp_button_loader
├── hm_wmp_checkbox_loader
├── hm_wmp_radio_loader
├── hm_wmp_scrollbar_loader
├── hm_wmp_popupmenu_loader
├── ... ( 16 control loaders )
└── hm_wmp_loader_depot     ← dispatches by class name ("hm_wmp_button" → button_loader)
```

A new control = one WMP class + one loader + one depot registration line.
The `DECLARE_FACTORY` macro generates the factory registration; the manifest
instantiates by class name.

### Layout System (LayoutDecoder)

| Decoder | Semantics | On resize |
|---|---|---|
| `corner` | anchored to a corner, fixed size | follows the corner, size unchanged |
| `dock` | docked to an edge, fixed thickness | thickness unchanged, rest fills |
| `offset` | inset on four sides | width/height stretch with the reference |
| `clientarea` | = host client area | follows (for adopted native controls) |

New decoders inherit `hm_layoutdecoder_i` and register with the loader.

### Drawing Buffer Framework (hm_wmp_display base)

New controls inherit `hm_wmp_display` and implement only
`_render(hDC, rcCanvas)` (local coordinates, unaware of screen vs offscreen);
interaction handlers call `drawSelf(pKeelObj, hwnd)`. Buffering is a
three-level declarative choice: `BUF_NONE` (direct), `BUF_ALLOC`
(allocate per click-rate draw), `BUF_CACHE_SIZE` (cache by size for
drag-rate draws).

### DPI Awareness

The third `hm_startup_param` argument `uDpi` (default 96) — manifests are
authored on a 96dpi baseline and the loader scales to physical pixels at a
single choke point. GDI+ point-based fonts convert automatically through the
Graphics DPI.

## How It Works

```
hm_lib_startup( manifest )          ← parse JSON → assemble keelobj tree + WMPs + resources
    ↓
hm_wndpropid_connect( hWnd, id )    ← bind the skin to an existing HWND
    ↓
Hooks intercept window messages     ← WH_CALLWNDPROC / WH_MOUSE / API hooks
    ↓
Messages routed to WMPs             ← dispatched by message type to control handlers
    ↓
WMPs self-draw over default render  ← GDI/GDI+ bitmaps + text + 9-patch stretching
    ↓
Interaction callbacks ( onHKSkinEvent )  ← observer pattern, 8 event kinds
```

## Quick Integration

```cpp
#include "hmskin.h"
#include "hmskin_event_listener_base.h"

// ① implement an event listener
class my_listener : public hmskin_event_listener_base {
    virtual void onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
        // 8 event kinds: BTN/CHK/RADIO/TAB/MENU-POP/MENU-BAR/SCROLLBAR/TRACKPOPUP
    }
};

// ② start the engine
my_listener listener;
hm_startup_param param( hInstance, &listener, 120 );  // 120 = system DPI
hm_lib_startup( &param, _T("myapp.hm.manifest"), &g_pHMRoot );

// ③ connect a window
CreateWindow( ... );
hm_wndpropid_connect( g_pHMRoot, hWnd, 258 );

// ④ (optional) enable transparent menu takeover
hm_lib_enable_trackpopupmenu_hook( g_pHMRoot, TRUE );

// ⑤ shutdown
hm_lib_shutdown( g_pHMRoot );
```

Full guide: **`doc/INTEGRATION.md`** (five-step integration / manifest
reference / WMP catalog / event table / known pitfalls).

## Building

### Command line (recommended for CI / remote collaboration)

```
build.cmd Debug ARM64        ← daily development
build.cmd                    ← all 6 configurations
```

All output is redirected to `build.log`; any failure yields a non-zero exit
code. See `doc/BUILDING.md` (Chinese).

### Visual Studio

Open `hmskin.sln`; the configuration matrix is
**{Win32, x64, ARM64} × {Debug, Release}** — just build.

### Prerequisites

- Visual Studio 2026, workload "Desktop development with C++"
- Component **"C++ ATL for v145 build tools"** (the code uses `AtlConv.h`)
- Windows 10/11 SDK

### Smoke Tests

```
python tools\smoke_test.py --no-screenshot    ← manifest validation + event chain
python tools\smoke_test.py                    ← + golden-image tolerance compare
python tools\smoke_test.py --perf D2D         ← renderer A/B perf (wheel scenario)
python tools\smoke_test.py --perf Gdiplus
python tools\smoke_test.py --tree           # keelobj tree dump + assertions
```

The golden baseline (`tools/golden/`) guards the current D2D appearance;
`--perf` runs the wheel scenario per backend and prints `hmskin_perf.log`
(count/avg/p50/p95/histogram) after a graceful exit.

## Repository Layout

| Path | Description |
|---|---|
| `hmskin/` | core DLL (hook × DirectUI skinning engine) |
| `demo/` | integration sample (the "benchmark bench" gallery: four chapters of control specimens + event log + token table) |
| `scrolldecorator/` | standalone self-drawn scrollbar DLL (usable without hmskin) |
| `tools/` | `gen_skin.py` bitmap generator + `smoke_test.py` smoke tests + `hmskin_mcp.py` agent bridge |
| `doc/` | `INTEGRATION.md` integration guide + `BUILDING.md` build guide + `img/` screenshots |
| `tools/golden/` | golden-image regression baseline (the appearance the smoke test guards) |
| `DESIGN.md` | design system (palette / type scale / layout / component specs, Chinese) |

## Platform & Dependencies

- **Language**: C++ (Win32 API + GDI/GDI+), no MFC
- **Platforms**: Win32 / x64 / **ARM64** (Windows 10/11 SDK)
- **Toolset**: Visual Studio 2026 (v145)
- **Dependencies**: gdiplus + jsoncpp 1.9.6 (source vendored in `hmskin/dependence/json/`)

## Notes

- Source comments are English throughout; a few UI strings and one
  font-name match are still Chinese ( i18n backlog )
- `TreatWarningAsError` is off (legacy warnings C4267/C4244/C4996 are harmless)
- DPI Phase 1 (static scaling) is done; per-monitor dynamic switching is a
  future milestone

## History

| When | What |
|---|---|
| 2016 | first version: VS2008 → VS2013 v120_xp, 32-bit only, jsoncpp 0.5 prebuilt |
| 2026-10 | modernization: VS2026 v145, three platforms, jsoncpp 1.9.6 source, UTF-8 manifests, pointer-width safety, transparent TrackPopupMenu takeover, DPI Phase 1, dirty-rect clipping, drawing-buffer framework, smoke-test infrastructure, integration docs, **Direct2D/DirectWrite text backend (M1) + renderer A/B perf harness**, golden-image baseline, loader-order root-cause fix |

## License

Dual licensing: **free** for personal learning and non-commercial use;
integrating into commercial products for profit requires prior written
consent by email (r4o5m6e8o@163.com). See [LICENSE](LICENSE) (Chinese).

The vendored jsoncpp component is distributed under its own license
(Public Domain / MIT) and is not subject to the above.
