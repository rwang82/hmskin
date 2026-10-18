#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin smoke tests - golden-image compare + event-chain assertion + manifest validation

Usage ( Windows VM, any cmd ):
python tools\\smoke_test.py                # all three phases
python tools\\smoke_test.py --gold         # first-time golden capture ( verify the shot by hand )
python tools\\smoke_test.py --no-screenshot  # skip golden ( manifest + event chain only )
python tools\\smoke_test.py --perf D2D     # A/B perf: backend override + EVT wheel scenario + graceful close -> prints hmskin_perf.log
python tools\\smoke_test.py --perf Gdiplus
python tools\\smoke_test.py --tree           # keelobj tree dump + assertions ( AI-agent infra )

Phases:
1. manifest validation - JSON syntax + structure of both manifests ( K/WMP reference consistency )
2. event-chain assertion - launch demo -> simulate interaction -> expected events in hmskin_trace.log
3. golden compare - screenshot after settle -> tolerance compare against tools/golden/*.png

Exit code: 0 = all pass, non-zero = failures ( CI friendly ).
"""
import os, sys, json, re, time, subprocess, argparse

# ---- paths ----
REPO   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTDIR = os.path.join(REPO, "ARM64", "Debug")
TRACE  = os.path.join(OUTDIR, "hmskin_trace.log")
GOLDEN = os.path.join(REPO, "tools", "golden")
MANIFESTS = [
    os.path.join(REPO, "demo", "assets", "Debug", "demo.hm.manifest"),
    os.path.join(REPO, "demo", "assets", "Release", "demo.hm.manifest"),
]

# ============================================================
# phase 1: manifest validation
# ============================================================
def check_manifest(path):
    """JSON syntax + keelobj/WMP reference integrity. Returns (ok, msg)."""
    tag = os.path.basename(os.path.dirname(path))
    try:
        raw = open(path, encoding="utf-8").read()
        d = json.loads(re.sub(r"^\s*//.*$", "", raw, flags=re.M))
    except Exception as e:
        return False, f"[manifest:{tag}] JSON PARSE FAIL: {e}"

    errors = []

    # structure
    for key in ("RawPic", "Font", "Color", "CoordinateSystem",
                "WndMsgProcessor", "MsgSink", "KeelObj"):
        if key not in d:
            errors.append(f"missing section: {key}")

    if errors:
        return False, f"[manifest:{tag}] " + "; ".join(errors)

    # reference integrity
    wmp_ids    = {w["idWMP"] for w in d["WndMsgProcessor"]}
    keel_ids   = {k["idKeelObj"] for k in d["KeelObj"]}
    sink_ids   = {m["id"] for m in d["MsgSink"]}
    pic_ids    = {f["id"] for f in d["RawPic"]["PicFiles"]}
    loaded     = set()   # keelobjs appear in order; refs must already exist
    dup_k      = [k["idKeelObj"] for k in d["KeelObj"]
                  if [x["idKeelObj"] for x in d["KeelObj"]].count(k["idKeelObj"]) > 1]

    for k in d["KeelObj"]:
        kid = k["idKeelObj"]
        # parent exists
        pid = k.get("ParentKeelObjId")
        if pid is not None and pid not in keel_ids:
            errors.append(f"K{kid}: parent K{pid} not found")
        # ref exists and loaded before ( ref 0 = the window itself, always valid )
        ref = k["LayoutObj"]["LayoutDecoder"].get("RefKeelObjId")
        if ref is not None and ref != 0 and ref not in keel_ids:
            errors.append(f"K{kid}: ref K{ref} not found")
        elif ref is not None and ref != 0 and ref not in loaded and ref != kid:
            errors.append(f"K{kid}: ref K{ref} loads AFTER K{kid}")
        # WMP ids exist
        for w in k.get("WMP", []):
            if w["id"] not in wmp_ids:
                errors.append(f"K{kid}: WMP{w['id']} not found")
        # sink exists
        sink = k.get("WndMsgSource", {}).get("MsgSinkId")  # key is "id" in the MsgSink array
        if sink is not None and sink not in sink_ids:
            errors.append(f"K{kid}: MsgSink{sink} not found")
        loaded.add(kid)

    if dup_k:
        errors.append(f"duplicate idKeelObj: {sorted(set(dup_k))}")

    # WMP districts reference valid RawPic ids
    for w in d["WndMsgProcessor"]:
        for dist in w.get("DistrictObjs", []):
            for so in dist.get("ShowObjs", []):
                pid = so.get("UIRes", {}).get("RawPicId")
                if pid is not None and pid not in pic_ids:
                    errors.append(f"WMP{w['idWMP']} d{dist['id']}: RawPic {pid} not found")

    if errors:
        return False, f"[manifest:{tag}] " + "; ".join(errors[:6])
    return True, f"[manifest:{tag}] ok ({len(keel_ids)} keelobjs, {len(wmp_ids)} WMPs)"

# ============================================================
# phase 2: event-chain assertion
# ============================================================
# expected sequences: ( interaction description, [event substrings in order] )
# the demo auto-logs to EVT-08; hmskin_trace.log gets [disp] flow.
# for now we assert on the trace after a plain launch: window created,
# paint happened, no MAPFAIL.

def check_trace_basics():
    """After a demo run: trace must exist, have paint flow, no MAPFAIL."""
    if not os.path.exists(TRACE):
        return False, "[trace] hmskin_trace.log not found (demo ran without trace marker?)"
    s = open(TRACE, encoding="utf-8", errors="replace").read()
    errors = []
    if "[disp] >>" not in s:
        errors.append("no [disp] flow ( dispatcher dead? )")
    if "msg=0x000F" not in s:
        errors.append("no WM_PAINT ( never painted? )")
    if "MAPFAIL" in s:
        errors.append("MAPFAIL present ( layout mapping broken? )")
    if errors:
        return False, "[trace] " + "; ".join(errors)
    n_paint = s.count("msg=0x000F")
    return True, f"[trace] ok ({n_paint} paints, dispatcher alive, no MAPFAIL)"

# ============================================================
# phase 3: golden-image compare
# ============================================================
def grab_screenshot():
    """Launch demo, wait for stability, capture window via PowerShell PrintWindow."""
    exe = os.path.join(OUTDIR, "demo.exe")
    if not os.path.exists(exe):
        return None, "demo.exe not found ( build first )"
    # clean old trace so the run is fresh
    if os.path.exists(TRACE):
        os.remove(TRACE)
    # the engine gates the message trace on a marker file next to the exe
    # ( '1' = level 1: everything but the mousemove flood ). smoke owns
    # this precondition itself - depending on a marker left behind by some
    # earlier debug session made the phase pass or fail on leftover state.
    marker = os.path.join(OUTDIR, "hmskin.trace")
    open(marker, "w").write("1")
    # launch
    proc = subprocess.Popen([exe], cwd=OUTDIR)
    time.sleep(4)  # paint + settle
    # capture via PowerShell ( PrintWindow into BMP, convert handled by PIL )
    shot = os.path.join(REPO, "tools", "_smoke_shot.png")
    ps_script = r'''
Add-Type -AssemblyName System.Windows.Forms,System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32Cap {
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
"@
$p = Get-Process demo -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $p) { exit 2 }
$h = $p.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { exit 3 }
$r = New-Object Win32Cap+RECT
[Win32Cap]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.R - $r.L; $ht = $r.B - $r.T
$bmp = New-Object System.Drawing.Bitmap($w, $ht)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[Win32Cap]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc)
$g.Dispose()
$bmp.Save("SHOT_PATH", [System.Drawing.Imaging.ImageFormat]::Png)
exit 0
'''.replace("SHOT_PATH", shot)
    r = subprocess.run(["powershell", "-NoProfile", "-Command", ps_script],
                       capture_output=True, text=True, timeout=30)
    if r.returncode != 0 or not os.path.exists(shot):
        proc.terminate()
        os.remove(marker)
        return None, f"screenshot failed ( ps rc={r.returncode}: {r.stderr[:200] if r.stderr else 'unknown'} )"
    shot_png = shot
    # run trace check while demo was up
    trace_ok, trace_msg = check_trace_basics()
    proc.terminate()
    try: proc.wait(timeout=5)
    except: proc.kill()
    os.remove(marker)   # leave the output dir as we found it
    return (shot_png, trace_msg) if trace_ok else (None, trace_msg)

def compare_golden(shot_path, tolerance=0.03):
    """Compare screenshot against golden image. tolerance = fraction of pixels allowed to differ."""
    from PIL import Image
    gold_path = os.path.join(GOLDEN, "demo_default.png")
    if not os.path.exists(gold_path):
        return False, f"golden not found: {gold_path} ( run with --gold first )"
    a = Image.open(gold_path).convert("RGB")
    b = Image.open(shot_path).convert("RGB")
    if a.size != b.size:
        return False, f"size mismatch: gold {a.size} vs shot {b.size}"
    pa, pb = a.load(), b.load()
    W, H = a.size
    diff = 0
    total = W * H
    step = 3  # sample every 3rd px ( speed )
    sampled = 0
    for y in range(0, H, step):
        for x in range(0, W, step):
            ca, cb = pa[x, y], pb[x, y]
            if abs(ca[0]-cb[0]) > 30 or abs(ca[1]-cb[1]) > 30 or abs(ca[2]-cb[2]) > 30:
                diff += 1
            sampled += 1
    frac = diff / max(sampled, 1)
    if frac > tolerance:
        return False, f"golden MISMATCH: {frac:.1%} pixels differ ( tolerance {tolerance:.0%} )"
    return True, f"golden ok ({frac:.1%} diff, tolerance {tolerance:.0%})"

# ============================================================
# phase 4: perf A/B ( backend override + wheel scenario )
# ============================================================
def run_perf(tool):
    """A/B perf run: launch demo with --drawtool override, CLICK the sec.4
    tab ( real input ) then wheel-scroll the real scrollbar ( K47 ), close
    gracefully ( WM_CLOSE so hm_lib_shutdown dumps the text-draw stats into
    hmskin_perf.log ). the sample ring in the log shows what actually drew:
    the old EVT-panel wheel hit no scrollbar and measured two full repaints."""
    exe = os.path.join(OUTDIR, "demo.exe")
    if not os.path.exists(exe):
        return False, "[perf] demo.exe not found ( build first )"
    perflog = os.path.join(OUTDIR, "hmskin_perf.log")
    if os.path.exists(perflog):
        os.remove(perflog)
    proc = subprocess.Popen([exe, "--drawtool", tool], cwd=OUTDIR)
    time.sleep(2.5)
    ps_script = r'''
Add-Type -AssemblyName System.Windows.Forms
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class WCap {
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, int d, UIntPtr e);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out R r);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref P p);
    [StructLayout(LayoutKind.Sequential)] public struct R { public int L, T, Rt, B; }
    [StructLayout(LayoutKind.Sequential)] public struct P { public int X, Y; }
}
"@
$p = Get-Process demo -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $p) { exit 2 }
$h = $p.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { exit 3 }
[WCap]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 300
function ClientPt([int]$cx, [int]$cy) {
    $pt = New-Object WCap+P
    $pt.X = $cx; $pt.Y = $cy
    [WCap]::ClientToScreen($h, [ref]$pt) | Out-Null
    return $pt
}
# 1) click the 4th tab of the main tab strip ( keelobj K4 spans the client
#    top, four sprite tabs; 7/8 across lands mid-tab-4 )
$tab = ClientPt 939 23
[WCap]::SetCursorPos($tab.X, $tab.Y) | Out-Null
Start-Sleep -Milliseconds 250
[WCap]::mouse_event(2, 0, 0, 0, [UIntPtr]::Zero)   # LEFTDOWN
[WCap]::mouse_event(4, 0, 0, 0, [UIntPtr]::Zero)   # LEFTUP
Start-Sleep -Milliseconds 900                       # chapter switch repaint
# 2) wheel over the sec.4 scrollbar ( K47 client ~612..630 x 318..600 )
$sb = ClientPt 621 459
[WCap]::SetCursorPos($sb.X, $sb.Y) | Out-Null
Start-Sleep -Milliseconds 400
for ($i = 0; $i -lt 60; $i++) {
    [WCap]::mouse_event(0x0800, 0, 0, -120, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 25
}
Start-Sleep -Milliseconds 600
$p.CloseMainWindow() | Out-Null
exit 0
'''
    r = subprocess.run(["powershell", "-NoProfile", "-Command", ps_script],
                       capture_output=True, text=True, timeout=120)
    try:
        proc.wait(timeout=10)
    except Exception:
        proc.kill()
    if not os.path.exists(perflog):
        return False, f"[perf:{tool}] log not written ( ps rc={r.returncode}: {r.stderr[:150] if r.stderr else 'unknown'} )"
    # the log carries CJK sample text written by the CRT in the system
    # codepage - never let a decode mismatch kill the run after the data
    # is already on disk
    return True, f"[perf:{tool}]\n  " + open(perflog, errors="replace").read().strip().replace("\n", "\n  ")

# ============================================================
# phase 5: keelobj tree dump ( AI-agent infra M1 )
# ============================================================
def run_tree_dump():
    """launch demo --dump-tree, then assert the JSON tree against the manifest."""
    exe = os.path.join(OUTDIR, "demo.exe")
    if not os.path.exists(exe):
        return False, "[tree] demo.exe not found ( build first )"
    treepath = os.path.join(REPO, "tools", "_tree_dump.json")
    if os.path.exists(treepath):
        os.remove(treepath)
    proc = subprocess.Popen([exe, "--dump-tree", treepath], cwd=OUTDIR)
    try:
        proc.wait(timeout=20)
    except Exception:
        proc.kill()
        return False, "[tree] demo did not exit in 20s"
    if not os.path.exists(treepath):
        return False, "[tree] dump file not written"
    try:
        tree = json.load(open(treepath))["hmskin_tree"]
    except Exception as e:
        os.remove(treepath)
        return False, f"[tree] JSON parse fail: {e}"
    errors = []
    man = json.loads(re.sub(r"^\s*//.*$", "",
                            open(MANIFESTS[0], encoding="utf-8").read(), flags=re.M))
    n_manifest = len(man["KeelObj"])
    if len(tree) != n_manifest:
        errors.append(f"node count {len(tree)} != manifest {n_manifest}")
    byid = {n["id"]: n for n in tree}
    k1 = byid.get(1)   # root: covers the client area
    if not k1 or not k1.get("rect"):
        errors.append("K1 ( root ) has no mapped rect")
    else:
        r = k1["rect"]
        if r["r"] - r["l"] < 800 or r["b"] - r["t"] < 500:
            errors.append(f"K1 rect suspiciously small: {r}")
    # specimen nodes carry non-zero rects ( popup-slot nodes excluded: the
    # cross-window mapping wrinkle is documented in hmskin_helper.h )
    for kid in (3, 18, 38):
        n = byid.get(kid)
        if not n:
            errors.append(f"K{kid} missing from tree")
        elif n.get("rect"):
            r = n["rect"]
            if r["r"] <= r["l"] or r["b"] <= r["t"]:
                errors.append(f"K{kid} zero-area rect: {r}")
    sec1 = {3, 19, 10, 11, 29, 30, 31, 32, 33, 34, 35, 36, 48, 49, 50, 51}
    hidden = sorted(k for k in sec1 if k in byid and not byid[k]["visible"])
    if hidden:
        errors.append(f"sec.1 nodes not visible: {hidden}")
    os.remove(treepath)
    if errors:
        return False, "[tree] " + "; ".join(errors[:5])
    return True, f"[tree] ok ({len(tree)} nodes; root/specimens/chapter-visibility verified)"

# ============================================================
# main
# ============================================================
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gold", action="store_true", help="save screenshot as new golden")
    ap.add_argument("--no-screenshot", action="store_true", help="skip golden compare ( manifest + trace only )")
    ap.add_argument("--perf", nargs="?", const="D2D", default=None,
                    help="A/B perf run ( D2D | Gdiplus ): backend override + EVT wheel scenario + graceful close")
    ap.add_argument("--tree", action="store_true",
                    help="keelobj tree dump + assertions ( AI-agent infra M1 )")
    args = ap.parse_args()

    results = []
    fail = 0

    # phase 1
    for m in MANIFESTS:
        ok, msg = check_manifest(m)
        results.append((ok, msg))
        if not ok: fail += 1

    # tree mode: standalone, mutually exclusive with the demo phases below
    if args.tree and sys.platform == "win32":
        ok, msg = run_tree_dump()
        results.append((ok, msg))
        if not ok: fail += 1

    # perf mode ( mutually exclusive with the golden phases )
    if args.perf and sys.platform == "win32":
        ok, msg = run_perf(args.perf)
        results.append((ok, msg))
        if not ok: fail += 1

    # phase 2+3 ( Windows only - demo.exe can't run elsewhere ).
    # skipped entirely in perf/tree mode ( those runs ARE the demo launch ).
    if ( args.perf or args.tree ) and sys.platform == "win32":
        pass
    elif sys.platform != "win32":
        results.append((True, "[smoke] non-Windows: manifest phase only"))
    elif args.gold:
        # --gold: capture screenshot and save as new golden ( manual verify + commit )
        os.makedirs(GOLDEN, exist_ok=True)
        shot, trace_msg = grab_screenshot()
        if shot:
            from PIL import Image
            img = Image.open(shot)
            img.save(os.path.join(GOLDEN, "demo_default.png"))
            results.append((True, f"[golden] saved {img.size} + trace: {trace_msg}"))
            os.remove(shot)
        else:
            results.append((False, f"[golden] capture failed: {trace_msg}"))
            fail += 1
    elif not args.no_screenshot:
        # full mode: launch -> screenshot -> golden compare + trace check
        shot, trace_msg = grab_screenshot()
        if shot:
            ok, msg = compare_golden(shot)
            results.append((ok, msg + f" | {trace_msg}"))
            if not ok: fail += 1
            os.remove(shot)
        else:
            results.append((False, f"[smoke] {trace_msg}"))
            fail += 1
    else:
        # --no-screenshot: launch + trace check only ( no golden compare )
        exe = os.path.join(OUTDIR, "demo.exe")
        if os.path.exists(exe):
            if os.path.exists(TRACE): os.remove(TRACE)
            p = subprocess.Popen([exe], cwd=OUTDIR)
            time.sleep(3)
            ok, msg = check_trace_basics()
            p.terminate()
            try: p.wait(timeout=5)
            except: p.kill()
            results.append((ok, msg))
            if not ok: fail += 1

    print()
    print("=" * 60)
    for ok, msg in results:
        print(("  PASS  " if ok else "  FAIL  ") + msg)
    print("=" * 60)
    print(f"{'ALL PASS' if fail == 0 else f'{fail} FAILED'}")
    sys.exit(0 if fail == 0 else 1)

if __name__ == "__main__":
    main()
