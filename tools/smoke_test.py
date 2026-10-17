#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin 冒烟测试——金图比对 + 事件链断言 + manifest 校验

用法（Windows VM 上，任意 cmd）:
    python tools\\smoke_test.py                # 全部三阶段
    python tools\\smoke_test.py --gold         # 首次生成金图（人工确认截图无误后）
    python tools\\smoke_test.py --no-screenshot  # 跳过金图（只跑 manifest + 事件链）

阶段:
  1. manifest 校验——三份 manifest JSON 语法 + 结构完整性（ K/WMP 引用一致性 ）
  2. 事件链断言——启动 demo → 模拟交互 → 检查 hmskin_trace.log 中的预期事件序列
  3. 金图比对——demo 稳定后截屏 → 与 tools/golden/*.png 容差比对

退出码: 0=全过, 非 0=有失败（ CI 可判 ）。
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
        return None, f"screenshot failed ( ps rc={r.returncode}: {r.stderr[:200] if r.stderr else 'unknown'} )"
    shot_png = shot
    # run trace check while demo was up
    trace_ok, trace_msg = check_trace_basics()
    proc.terminate()
    try: proc.wait(timeout=5)
    except: proc.kill()
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
# main
# ============================================================
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gold", action="store_true", help="save screenshot as new golden")
    ap.add_argument("--no-screenshot", action="store_true", help="skip golden compare ( manifest + trace only )")
    args = ap.parse_args()

    results = []
    fail = 0

    # phase 1
    for m in MANIFESTS:
        ok, msg = check_manifest(m)
        results.append((ok, msg))
        if not ok: fail += 1

    # phase 2+3 ( Windows only — demo.exe can't run elsewhere )
    if sys.platform != "win32":
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
        # full mode: launch → screenshot → golden compare + trace check
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
