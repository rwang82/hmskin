#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin smoke tests - manifest validation + event-chain trace + e2e interaction suite

Usage ( Windows VM, any cmd ):
python tools\\smoke_test.py                # manifest + trace + e2e ( one command, full GUI verify )
python tools\\smoke_test.py --no-e2e      # manifest + trace only
python tools\\smoke_test.py --perf D2D     # A/B perf: backend override + EVT wheel scenario + graceful close -> prints hmskin_perf.log
python tools\\smoke_test.py --perf Gdiplus
python tools\\smoke_test.py --tree           # keelobj tree dump + assertions ( AI-agent infra )

Phases:
1. manifest validation - JSON syntax + structure of both manifests ( K/WMP reference consistency )
2. event-chain trace - launch demo -> dispatcher flow assertions in hmskin_trace.log
3. e2e interaction suite ( e2e_test.run_with_fresh_demo, a private demo
   instance - no hand-started resident needed ):
     structure   live_tree keelobj visibility per chapter
     appearance  the FOUR chapter goldens - the only screenshot tier, the
                 pixel-truth guard for cache/draw regressions logs can't see
     behavior    EVT log lines + click-target pixel diffs

Exit code: 0 = all pass, non-zero = failures ( CI friendly ).
"""
import os, sys, json, re, time, subprocess, argparse

# ---- paths ----
REPO   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTDIR = os.path.join(REPO, "ARM64", "Debug")
TRACE  = os.path.join(OUTDIR, "hmskin_trace.log")
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
# perf A/B variant ( backend override + wheel scenario )
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
    # cross-window mapping wrinkle is documented in hmskin.h )
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
    ap.add_argument("--no-e2e", action="store_true", help="skip the merged e2e interaction phase")
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

    # phase 2+3 ( Windows only - demo.exe can't run elsewhere ): ONE demo
    # instance carries both the trace check and the e2e suite ( no double
    # launch, no window handover ). skipped in perf/tree mode ( those runs
    # ARE the demo launch ).
    if ( args.perf or args.tree ) and sys.platform == "win32":
        pass
    elif sys.platform != "win32":
        results.append((True, "[smoke] non-Windows: manifest phase only"))
    elif not os.path.exists(os.path.join(OUTDIR, "demo.exe")):
        results.append((False, "[trace] demo.exe not found ( build first )"))
        fail += 1
    else:
        # pre-flight: a leftover demo holding port 9321 would hijack every
        # e2e agent op ( the suite's own instance binds nothing ) - refuse
        # loudly instead of testing the WRONG process.
        import socket
        s = socket.socket()
        s.settimeout(0.5)
        try:
            s.connect(("127.0.0.1", 9321))
            busy = True
        except OSError:
            busy = False
        finally:
            s.close()
        if busy:
            results.append((False, "[setup] a demo is already running on port "
                            "9321 - close it first ( the suite launches its "
                            "own instance ) / 已有 demo 在运行，请先关闭再跑套件"))
            fail += 1
        else:
            if os.path.exists(TRACE):
                os.remove(TRACE)
            marker = os.path.join(OUTDIR, "hmskin.trace")
            open(marker, "w").write("1")
            proc = subprocess.Popen([os.path.join(OUTDIR, "demo.exe")], cwd=OUTDIR)
            try:
                time.sleep(4)  # paint + settle
                ok, msg = check_trace_basics()
                results.append((ok, msg))
                if not ok: fail += 1
                if not args.no_e2e:
                    import e2e_test
                    print("\n-- phase 3: e2e interaction suite --")
                    fail += e2e_test.run_suite()
            finally:
                proc.terminate()
                try: proc.wait(timeout=5)
                except: proc.kill()
                if os.path.exists(marker):
                    os.remove(marker)   # leave the output dir as we found it

    print()
    print("=" * 60)
    for ok, msg in results:
        print(("  PASS  " if ok else "  FAIL  ") + msg)
    print("=" * 60)
    print(f"{'ALL PASS' if fail == 0 else f'{fail} FAILED'}")
    sys.exit(0 if fail == 0 else 1)

if __name__ == "__main__":
    main()
