\---
\# gstack: design-md-format=spec  
name: hmskin Gallery  
description: industrial-instrument "Win32 skin benchmark bench" - carbon-black deck, 1px hairlines, one live signal orange; restrained, precise, electric  
colors:  
  primary: "#FF5C1F"        # signal orange: actionable / focus, the only saturated interactive color  
  on-primary: "#141414"     # text / glyphs on the orange  
  bg: "#0F1216"             # window base  
  surface: "#161B21"        # panels / title bar / menus  
  raised: "#1C2229"         # raised ( active tab, secondary buttons )  
  sunken: "#11151A"         # recessed ( scroll views, checkbox wells, scrollbar tracks )  
  line: "#262C34"           # hairlines  
  line-strong: "#363E48"    # strong borders ( checkbox outline, popup menu frame )  
  text: "#E8ECF1"           # body text  
  text-muted: "#8B939E"     # secondary text  
  text-faint: "#5B636E"     # weak text / disabled  
  accent: "#FF5C1F"         # same source as primary; hover #FF7A47 / press #CC4817  
  success: "#3DDC97"        # status green: only "state is true" ( checks, active marks )  
  error: "#E5484D"          # close-button hover and destructive items only  
  inner-highlight: "#FFFFFF0F"  # alpha inner stroke ( top 1px highlight )  
  outer-edge: "#54626C9C"   # window outer stroke ( follows the Win11 8px rounded corners )  
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
  measure: 28px            # left alignment baseline  
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

[中文](DESIGN.zh-CN.md) | **English**

## Overview

**Creative North Star:** a measuring instrument that makes you want to flip its switches - not a marketing page; every control is a specimen on the calibration bench.  
**Product context:** the demo application of hmskin ( a C++ Win32 skinning library ); the audience is C++ developers integrating the library; the peer category is the major systems' control-gallery samples, but the temperament is an "inspection record sheet", not a product catalog.  
**Mode per surface:** all Operate surfaces ( interactive control displays ), no marketing pages.  
**Reference sites:** system-level control galleries ( information-architecture reference ), mainstream design systems ( sizing / corner-radius conventions ).  
**Key characteristics:** one signal orange on a carbon-black deck; every control carries a mono specimen code ( BTN-01 ) and a 1px leader line; the skin title bar / close button is exhibit #1; the bottom status rail reads out engine capabilities ( 9-PATCH . ALPHA . D2D ).

## Colors

**Strategy:** restrained - one saturated interactive color + neutral graphite surfaces. Color scarcity is what gives it meaning.  
**Light or dark:** dark. The users are developers - long stares, mostly in dim rooms.  
Naming rules: orange = actionable / focus ( buttons, pressed-state thumb, focus stroke, active markers ); green = "state is true" ( checks, menu selection marks ) and never decoration; red = close-button hover and destructive items, banned elsewhere. Neutral surfaces step up bg->surface->raised->sunken; depth comes only from 1px hairlines and the alpha inner stroke.

## Typography

Type comes from the instrument's world: the terminals developers read all day. Chinese uses Microsoft YaHei UI ( Win11 built-in, mature ClearType rendering, Regular only below 12px, Light allowed only above 20px ); Latin / codes / measurements use Cascadia Code ( the Windows Terminal font - monospace, the "machine" voice outside the CJK feel ). The display voice is YaHei UI 600/300 size contrast; no extra fonts ( deploying private GDI+ fonts costs more than the demo gains - an explicit trade-off ). Neighboring levels of the scale differ by at least one size step or two weight steps.

## Layout

A single window split 64/36 asymmetric. Left "benchmark bench" ( 64% ): the 28px left alignment baseline carries the display title, the mono subtitle, then the interactive control groups; right "specimen index" ( 36% ): the event log and token readouts. On top a 44px skin title bar + a 28px root menu bar ( both are exhibits ); a 30px status rail at the bottom. One structural hairline between the two columns - no second kind of divider exists.

## Gallery Information Architecture

Five layers top to bottom, unified specimen prefixes:

| Layer | Specimens | Notes |
| --- | --- | --- |
| (1) skin title bar 44px | WND-00 | permanent exhibit: close / minimize / maximize trio, close hover = red |
| (2) root menu bar 28px | BAR-00 | permanent exhibit: File / View / Help open real popup menus ( items cover checks / hover / disabled / separators / shortcut keys ) |
| (3) left benchmark bench 64% | TAB-01 + four chapters | chapter tabs do real navigation ( the tab is itself a specimen ): sec.1 basics / sec.2 text & data / sec.3 menus / sec.4 scrolling & containers |
| (4) right specimen index 36% | EVT-08 / TOK-09 | fixed across chapters: event log + token readouts |
| (5) status rail 30px | RAIL | engine capability readout |

The four chapters:

- **sec.1 basics**: BTN-01 button matrix ( 32/26/40 heights x normal/hover/press/disable - all states displayed statically plus live instances ); CHK-02 checkboxes grouped by semantics into two single columns ( render engine / interaction options ) with focus and disabled specimens in-group; TGL-10 toggle specimen ( 36x20 pill, OFF = recessed gray knob / ON = signal-orange knob, reskinned checkbox engine, all four states ); RAD-11 radio group specimen ( 15x15 circles, selected = status-green dot, same semantics as the check family; GDI / GDI+ / AUTO options - a dedicated engine class `hm_wmp_radio` inherits the checkbox state machine: no flip on re-click [ no mid-state flicker ] + an independent RADIO_CLICK event; select-one-uncheck-siblings is listener-driven ).
- **sec.2 text & data**: TXT-03 type-scale specimens ( display/body/label/mono/micro on one screen ); TOK-04 token table - text left-aligned, values right-aligned mono, row hover highlight ( table mode ); PRG-12 progress bar specimen ( 260x8 track + signal-orange 64% fill, pure manifest assembly with zero engine changes - the fill thickness IS the dock thickness, changing the percentage is manifest-only ).
- **sec.3 menus**: MENU-05 popup menu specimen ( triggered from the root menu bar; covers checked items / hover / disabled / semantic separators / right-aligned mono shortcuts ).
- **sec.4 scrolling & containers**: VIEW-06 scroll view ( real document scrolling: the content well clips its subtree via setClipChildren, content 600px taller than the well, drag / arrow buttons / wheel - all three input paths scroll; range = content height - well height ); SB-07 standalone scrollbar specimen ( thumb states side by side ); BG-08 three surface tiers ( surface/sunken/raised ) for contrast. The 2016 viewbkcontent/keelctrl_scrollview skeleton is superseded by this combo ( its fate is a separate discussion ).

Right column: the TOK-04 interactive token table ( the button row gets hover highlight for free; left column semantic labels, right column mono hex values right-aligned ); the status rail reads out measured engine capabilities ( `* LIVE 9-PATCH . 3-SEG . ALPHA . D2D . DPI <live value>` ).

sec.3 addendum: the right-click menu is the transparent-takeover specimen - the demo's native `TrackPopupMenu` call is intercepted and skinned by the engine, selection delivered as native `WM_COMMAND` ( zero host cooperation ).

Right column event log ( the core use of feedback mode ): every interaction appends one mono line, e.g. `BTN-01 CLICK`, `CHK-02 -> true`, `TAB-01 -> sec.3` - turning the library's `HMSKIN_EVENT_*` callback API into a visible event stream; the most direct demonstration for integrators.

Interaction rules: tabs cycle with arrow keys; focus-ring system ( engine focus model, backlog - arrow cycling shipped ); every chapter carries at least one disabled specimen ( dedicated disabled palette, no opacity tricks ).

## Elevation & Depth

The engine has no shadows / blur - a stance, not a constraint: depth is entirely surface tiers ( bg->sunken->surface->raised ) + 1px hairlines + a 6% white alpha inner highlight. Recessed reads as "content well" ( scroll view ); raised reads as "selected surface" ( active tab ). Zero-offset glows never appear.

## Shapes

Three corner tiers: window 8px ( Win11 DWM enforced; the skin only traces a 1px inner stroke along the arc ); controls 4px ( buttons, checkboxes, popup menus ); widgets 3px ( scrollbar thumb ). Nested: inner radius = outer radius - gap.

## Components

Every control carries all four states: normal / hover / press / disable ( disabled = surface #161B21 + text #5B636E, never opacity fudges ). Button heights 32/26/40. Checkbox 16px, green well with a dark check. Active tab = raised + 2px orange top marker, no nested cards. Popup menu 26px item height, mono shortcuts right-aligned, 1px separators between semantic groups. The title-bar close hover turning red is the single red moment of the whole UI. The window is the exhibit: title bar, menu bar, close button are the demo's first three specimens.

## Do's and Don'ts

- Do: give every control a mono specimen code + 1px leader line ( BTN-01/CHK-02/TAB-03/SB-04 )
- Do: make the status rail read real engine capabilities ( 9-PATCH . 3-SEG . ALPHA . D2D . live DPI value )
- Do: build depth from surface tiers and hairlines; orange/green/red strictly by semantics
- Don't: purple gradients, three-column card grids, centered layouts, glowing edges, rounded icon badges
- Don't: fake disabled states with opacity instead of a dedicated palette ( bitmap skins need their own disabled swatches )
- Don't: use Light/thin weights for Chinese below 12px

## Motion

- **Approach:** minimal-functional - immediate GDI rendering, only state switches ( hover/press surface-color changes ), no animation. That itself is part of the "instrument" temperament.
- **Easing:** none ( instant state switching )
- **Duration:** 0ms
- **The one authored moment:** the "* LIVE" green dot on the status rail, always on - the window's only breath ( may be static ).

## Engine evolution: the drawing-buffer framework ( backlog; priority rises with control count )

**Problem:** the knowledge of "drawing to screen vs offscreen" lived with callers ( the WM_PAINT drawtrigger vs the interactive GetDC ), yet each control reverse-engineered it via `WindowFromDC` and hand-rolled its own buffering - the checkbox incident ( 2026-10-16 state-switch flicker ) proved every new control adds another inconsistency and regression surface. The more controls, the higher the price - **the earlier the consolidation, the cheaper**.

**The transitional three-tier rule** ( in effect ):

| Draw frequency | Buffer strategy | Examples |
| --- | --- | --- |
| Click/hover rate ( a few per second ) | build a small bitmap, composite, one blt | checkbox / button class |
| Drag rate ( every mousemove ) | cache by size and reuse - never rebuild | scrollbar thumb |
| Full-window redraw ( stretch / chapter switch ) | root canvas only; controls skip ( `WindowFromDC == NULL` detects the memory DC ) | drawtrigger path |

**Target shape**: consolidated into the `hm_wmp_display` base; controls implement only "how to draw":

```
hm_wmp_display
|-- drawSelf()                 <- the only thing interaction handlers call;
|                                 GetDC / destination detection / buffering / release all in the frame
|-- buffer strategy ( control declares, framework executes )
|     BUF_NONE        draw into the root canvas ( WM_PAINT path lands here automatically )
|     BUF_ALLOC       click rate: build and destroy per draw
|     BUF_CACHE_SIZE  true pixel cache: render once then blt ( dirty-flag invalidation )
|-- _render(hDC, rcCanvas) pure virtual <- the only part a control writes
```

**Current state** ( 2026-10-18 ): the framework fully landed - the checkbox family / button / tabheader / bkgnd all ride the base-class buffering; BUF_CACHE_SIZE has been upgraded to a **true pixel cache** ( m_bBufDirty dirty flag, render once then blt, the WM_PAINT path hits it too ); stateful controls use the **state-aware snapshot** pattern ( button/tabheader: snapshot the state when drawing, invalidateBuffer on switch ). Measured on the scroll path, text draws dropped 1355->131 ( -90% ). Invalidation is fully automatic on two channels: control state events + the uires observer chain ( text TEXTCHANGED -> WMPs invalidate their caches themselves ); hosts do zero InvalidateRect.

**Addition: dirty-rect clipping** ( 2026-10-17 ): the doDrawAll dirty-rect overload - subtrees without intersection are skipped whole, the drawtrigger clips its traversal with ps.rcPaint, BitBlt copies only the damaged region. Paint cost dropped from control count to damaged area ( 33 lines of change; verified at the 125% tier with no white flashes ).

**Migration discipline**: no dedicated refactor commits; whoever is touched next migrates - one control, one commit, with measured verification.

**Explicitly not doing**: a retained-mode compositor ( controls invalidate, the framework composites dirty regions ) - that is rewriting the interaction model; direct draw exists precisely to give instant feedback without full-window redraws, and the three-tier rule already extracts that architecture's value.

**Sibling cross-cutting concern - hover-leave tracking** ( opened after the 2026-10-16 tabheader stuck-hover incident ): the mechanism now lives in the base ( `hm_wmp_display::armMouseLeaveTracking/onLeaveArrived`; NC controls pass `TME_LEAVE|TME_NONCLIENT` ), controls keep only the "what state to clear on leave" semantics. Same migration discipline as buffering ( tabheader migrated; checkbox/button/scrollbar on next touch ). Ultimate form, backlog: the dispatch layer synthesizes leave events via a WH_MOUSE hook ( the dispatcher sees every mousemove and naturally knows the last-hit-vs-current delta ) - controls would not even call TrackMouseEvent; but it touches the dispatch core for marginal gain, deferred.

## Decisions Log

| Date | Decision | Rationale |
| ---------- | --- | --- |
| 2026-10-13 | initial design system "benchmark bench" | synthesis of three /design-consultation proposals ( Codex Graphite Bench + Claude sub-agent Calibration Bench + the main proposal ); user finalized the signal orange #FF5C1F |
| 2026-10-13 | fonts: Win11 built-in YaHei UI + Cascadia Code | zero-deploy GDI+ CreateFont; bundling fonts costs more than the demo friction |
| 2026-10-13 | banning shadows/blur as a stance, not a constraint | hairline + surface-tier instrument feel; matches engine capability |
| 2026-10-16 | sec.1 adds TGL-10 toggle specimen ( reskinned checkbox ) | 36x20 pill, eight states; proves the checkbox bitmap family = toggle semantics with zero engine changes |
| 2026-10-16 | three-tier buffering rule + framework backlog ( see the evolution section ) | root cause of the checkbox flicker = unbuffered direct draw; rule set, consolidation chartered - migration price grows with control count |
| 2026-10-16 | RAD-11 becomes its own class hm_wmp_radio | user architecture ruling: the concept deserves a name in the type system; inherits the checkbox state machine + template-method hooks, independent RADIO_CLICK event |
| 2026-10-16 | drawing-buffer framework lands ( hm_wmp_display base, drawSelf + three strategies ) | checkbox migrated first, verified; checkbox->radio->button in progress |
| 2026-10-17 | transparent TrackPopupMenu takeover ( M1-4 verdict: on ) | user ruled "do it"; engine owns everything ( PopupSkin manifest slot + modal loop + native WM_COMMAND delivery ); zero host cooperation; ARM64 patcher first-verified |
| 2026-10-17 | DPI Phase 1 ( load choke-point scaling ) | 96-baseline manifests straight to physical pixels; GDI+ point fonts auto-convert ( never hand-multiply ); three dedicated-Cfg geometries scaled in step |
| 2026-10-17 | dirty-rect clipping ( doDrawAll overload + ps.rcPaint ) | BeginPaint's update region had been ignored for 9 years - wiring it up dropped paint cost from control count to damaged area |
| 2026-10-17 | smoke-test infrastructure ( tools/smoke_test.py, three phases ) | manifest validation / event-chain asserts / golden-image tolerance compare; the safety net for dangerous changes |
| 2026-10-17 | D2D-M1 hybrid backend lands ( hm_ps_drawtool_d2d ) | text self-drawn via DirectWrite; bitmap/solid/calcRect forwarded to gdiplus ( measurement stays GDI+ = zero layout shift ); Debug manifest's 53 WMPs switched, goldens rebuilt as the D2D baseline ( 0.3% -> 4.1% full switch -> rebuilt ); glyph quality on par with GDI+ ( no visible difference on small text at 100% ) |
| 2026-10-17 | renderer A/B perf facility, always on | hm_perf_stats zero-overhead aggregation + demo --drawtool runtime override + smoke --perf scenario; measured D2D 1394us vs GDI+ 166us ( 8.4x; the per-draw DCRT BindDC/EndDraw round trip the prime suspect ) - the input for the M2 bitmap-path and TextLayout/batching verdicts |
| 2026-10-17 | load-order root-cause fix ( 9-year latent debt ) | _fillWMP order swap: districts filled first, DrawTool loaded after - under the old order the resource-preload loop never ran, the unload path went 9 years unexecuted, and the first runtime setDrawTrip underflowed refcounts; also fixed a D2D static-destruction crash ( releasing D2D in DllMain context is forbidden ) |
| 2026-10-18 | BUF_CACHE_SIZE becomes a true pixel cache ( Milestone A ) | m_bBufDirty flag + invalidateBuffer; the WM_PAINT path hits it too; manifest `"BufStrategy":"CACHE_SIZE"`; goldens 0.0% verified |
| 2026-10-18 | state-aware pixel cache + scroll path close-out ( Milestone B ) | button/tabheader snapshot their drawn state, invalidate on switch; real-wheel text draws 1355->131 ( -90% ) |
| 2026-10-18 | D2D discrimination experiment closed | TextLayout/Brush caches stay ( ~30% saved ); verdict = the DCRT BindDC/EndDraw round trip dominates ( cache-hit p50 still >=500us vs GDI+ <100us ) - three M2 exits await a ruling |
| 2026-10-18 | invalidation-chain instrument hm_util_invlog | hmskin.trace marker gates hmskin_invalidate.log; the scroll-time "tabheader repaints" claim proven a sample-ring parallax, culling healthy |
| 2026-10-18 | observer invalidation chain + public text API | uires text made observable ( TEXTCHANGED broadcast ), an embedded WMP observer invalidates the pixel cache on arrival; hm_wmp_set_text/_at - hosts hand over manifest ids only |
| 2026-10-18 | E2E interaction regression suite | tools/e2e_test.py, nine asserts ( chapter goldens + interaction EVT + pixel diff ), remotely drivable via the demo's embedded agent service |
| 2026-10-19 | API-surface review: front door 11 -> 18 functions | register of 21 touch points adjudicated one by one: 11 promoted to the id-addressed front door ( visibility/clipping/layout rect/tree dump/text read-write/global backend swap/DPI conversion ); the rest stay transitional advanced face ( root cause = incomplete controls; re-evaluate per control as each is completed ); demo engine penetrations zeroed, hmskin_helper retired |
| 2026-10-19 | group-A hardening: five small fixes | UnRegMsgSinks snapshot iteration; single-uiroot startup guard; shutdown popup guard + one s_pMenuRoot for the menu subsystem; depository release diagnostics; coordsys template-name fix - the ⑤ audit backlog cleared |
| 2026-11-01 | headless regression gate ( HMSkinRegression ) | hmeditor-style console gate, 22 engine cases, exit code = failure count; runnable over ssh - the GUI suites keep needing a desktop session |
| 2026-10-19 | #32768 in-place menu skinning experiment ( rolled back ) | verdict: the skin's own bar takeover suppresses native menu mode ( NC click / SC_MOUSEMENU / injected Alt all rejected ); capturing menu windows needs WH_CBT, not WH_CALLWNDPROC - recorded for the retrofit scenario |
