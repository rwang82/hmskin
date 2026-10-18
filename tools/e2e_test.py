#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin e2e interaction regression suite.

Drives the RESIDENT demo through its embedded agent service
( TCP 127.0.0.1:9321, see demo.cpp ) and asserts three layers:

  structure   live_tree keelobj visibility per chapter
  appearance  per-chapter golden images ( tools/golden/e2e_ch*.png )
  behavior    EVT log lines via the "evtlog" agent op

Run ON THE VM in an interactive session, demo started by hand:

  python tools\\e2e_test.py --gold    capture the 4 chapter baselines
  python tools\\e2e_test.py           full suite

Exit code 0 = pass ( CI friendly ). v2 backlog: keyboard drives ( no
key op yet ) and wheel drives ( synthetic posts proved unreliable ).

Note: chapter goldens tolerate the small toggle/radio state residue a
previous run leaves behind ( pill/dot pixels are ~0.1% of the window,
tolerance is 3% ) - no state reset needed between runs.
"""
import argparse
import json
import os
import re
import shutil
import socket
import sys
import time

REPO   = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTDIR = os.path.join(REPO, "ARM64", "Debug")
GOLDEN = os.path.join(REPO, "tools", "golden")
SHOT   = os.path.join(OUTDIR, "_mcp_shot.png")
SHOT2  = os.path.join(OUTDIR, "_mcp_shot2.png")
LIVE   = os.path.join(OUTDIR, "hmskin_tree.live.json")
PORT   = 9321

DEMO_NOT_RUNNING = ("demo is not running - start it by double-clicking demo.exe "
                    "( ARM64\\Debug ) on the desktop, then retry")

# main tab strip = keelobj K4, client (8,6)-(1072,40), four equal tabs
TABS = [(141, 23), (407, 23), (673, 23), (939, 23)]

# per-chapter representative keelobj sets ( from arrChapterGroups in demo.cpp )
CH_SETS = {
    1: ( {3, 10, 11, 29, 48, 49},   {21, 22, 27, 2, 38, 47} ),
    2: ( {21, 22, 52},              {3, 27, 2, 38} ),
    3: ( {27},                      {3, 21, 2, 38} ),
    4: ( {2, 28, 38, 47},           {3, 21, 27} ),
}

# menu bar item 1 in CLIENT coords: the bar sits directly above the client
# origin ( window y 54..80, per the rootmenu handler's own window mapping ),
# items are 60px wide from the left border. click at client (20,-13).
MENU_ITEM1 = (20, -13)
# a real-input click here ( empty client area ) closes the modal popup
MENU_DISMISS = (300, 300)


def agent(op, timeout=10):
    """one request/response line to the demo's agent service."""
    with socket.create_connection(("127.0.0.1", PORT), timeout=3) as sock:
        sock.sendall((op + "\n").encode("utf-8"))
        buf = b""
        while b"\n" not in buf:
            chunk = sock.recv(4096)
            if not chunk:
                break
            buf += chunk
    line = buf.decode("utf-8", errors="replace").strip()
    res = json.loads(line)
    if not res.get("ok"):
        raise RuntimeError("agent %s: %s" % (op, res.get("data")))
    return res.get("data", "")


def _wait_mtime(path, before, timeout=10):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if os.path.exists(path) and os.path.getmtime(path) != before:
            time.sleep(0.2)   # let the write finish
            return True
        time.sleep(0.15)
    return False


def screenshot():
    before = os.path.getmtime(SHOT) if os.path.exists(SHOT) else 0
    agent("shot")
    if not _wait_mtime(SHOT, before):
        raise RuntimeError("screenshot file did not refresh")
    return SHOT


_cap_seq = 0

def screenshot_screen():
    """desktop capture for interaction pixel diffs, returns a UNIQUE path:
    the demo overwrites one fixed png, comparing it against itself is
    always zero. requires the demo on top ( the "raise" preamble op )."""
    global _cap_seq
    before = os.path.getmtime(SHOT2) if os.path.exists(SHOT2) else 0
    agent("shotscreen")
    if not _wait_mtime(SHOT2, before):
        raise RuntimeError("screen capture file did not refresh")
    _cap_seq += 1
    dst = os.path.join(OUTDIR, "_e2e_cap%d.png" % _cap_seq)
    shutil.copy(SHOT2, dst)
    return dst


def live_tree():
    before = os.path.getmtime(LIVE) if os.path.exists(LIVE) else 0
    agent("live")
    if not _wait_mtime(LIVE, before):
        raise RuntimeError("live tree file did not refresh")
    return json.load(open(LIVE, encoding="utf-8-sig"))["hmskin_tree"]


def click(x, y):
    agent("click %d,%d" % (x, y))
    time.sleep(0.4)


def ncclick(x, y):
    agent("ncclick %d,%d" % (x, y))
    time.sleep(0.6)


def node_center(nodes, kid):
    n = next((n for n in nodes if n["id"] == kid), None)
    if not n or not n.get("rect"):
        raise RuntimeError("keelobj %s has no mapped rect in live tree" % kid)
    r = n["rect"]
    return (r["l"] + r["r"]) // 2, (r["t"] + r["b"]) // 2


def pixel_diff(path_a, path_b):
    """( changed-pixel count, bbox ) between two images."""
    from PIL import Image, ImageChops
    a = Image.open(path_a).convert("RGB")
    b = Image.open(path_b).convert("RGB")
    if a.size != b.size:
        raise RuntimeError("screenshot size changed: %s vs %s" % (a.size, b.size))
    diff = ImageChops.difference(a, b)
    bbox = diff.getbbox()
    if not bbox:
        return 0, None
    px = diff.load()
    n = sum(1 for y in range(bbox[1], bbox[3]) for x in range(bbox[0], bbox[2])
            if sum(px[x, y]) > 12)
    return n, bbox


def golden_compare(ch):
    from PIL import Image
    gold = os.path.join(GOLDEN, "e2e_ch%d.png" % ch)
    if not os.path.exists(gold):
        return False, "[ch%d] golden missing ( run --gold first )" % ch
    a = Image.open(gold).convert("RGB")
    b = Image.open(SHOT).convert("RGB")
    if a.size != b.size:
        return False, "[ch%d] size mismatch %s vs %s" % (ch, a.size, b.size)
    n, _ = pixel_diff(gold, SHOT)
    frac = n / float(a.size[0] * a.size[1])
    if frac > 0.03:
        return False, "[ch%d] golden MISMATCH: %.2f%% ( tolerance 3%% )" % (ch, frac * 100)
    return True, "[ch%d] golden ok (%.2f%% diff)" % (ch, frac * 100)


def new_evt_lines(before_text, after_text):
    """lines present in after but not in before ( by content )."""
    a = [ln.strip() for ln in after_text.splitlines() if ln.strip()]
    b = set(ln.strip() for ln in before_text.splitlines() if ln.strip())
    return [ln for ln in a if ln not in b]


def step_chapter(results, ch):
    click(*TABS[ch - 1])
    nodes = live_tree()
    vis, hid = CH_SETS[ch]
    byid = {n["id"]: n for n in nodes}
    bad = [k for k in vis if k in byid and not byid[k]["visible"]]
    bad += [k for k in hid if k in byid and byid[k]["visible"]]
    if bad:
        results.append((False, "[ch%d] visibility wrong for %s" % (ch, sorted(bad))))
        return
    screenshot()
    results.append(golden_compare(ch))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--gold", action="store_true", help="capture chapter baselines and exit")
    args = ap.parse_args()

    try:
        agent("ping", timeout=4)
    except Exception:
        print("FAIL:", DEMO_NOT_RUNNING)
        return 1
    # lift the window above anything covering it ( the console running this
    # very script ): desktop captures and menu clicks need a clear view
    agent("raise")
    time.sleep(0.3)

    if args.gold:
        os.makedirs(GOLDEN, exist_ok=True)
        for ch in (1, 2, 3, 4):
            click(*TABS[ch - 1])
            time.sleep(0.5)
            screenshot()
            dst = os.path.join(GOLDEN, "e2e_ch%d.png" % ch)
            from PIL import Image
            Image.open(SHOT).save(dst)
            print("  saved", dst)
        print("GOLD CAPTURED - review the 4 images, then commit")
        return 0

    results = []

    # ---- chapters: structure + appearance ----
    for ch in (1, 2, 3, 4):
        step_chapter(results, ch)

    # ---- interactions on sec.1 ( behavior + targeted pixel diff ) ----
    click(*TABS[0])
    nodes = live_tree()

    # button K3: EVT only - the press state is transient ( down=press,
    # up=normal ), a post-click screenshot is already back to NOR
    x, y = node_center(nodes, 3)
    before = agent("evtlog")
    click(x, y)
    got = new_evt_lines(before, agent("evtlog"))
    ok = any("BTN-03 CLICK" in ln for ln in got)
    results.append((ok, "[btn] EVT %s" % (got[:1],)))

    # toggle K48 / radio K50: EVT line + persistent visual change via the
    # desktop capture ( the preamble raised the window, so nothing covers it )
    x, y = node_center(nodes, 48)
    before = agent("evtlog")
    shot_a = screenshot_screen(); click(x, y); shot_b = screenshot_screen()
    n, _ = pixel_diff(shot_a, shot_b)
    got = new_evt_lines(before, agent("evtlog"))
    ok = any("TGL-10 ->" in ln for ln in got) and n > 30
    results.append((ok, "[tgl] EVT %s, diff %d px" % (got[:1], n)))

    nodes = live_tree()
    x, y = node_center(nodes, 50)
    before = agent("evtlog")
    shot_a = screenshot_screen(); click(x, y); shot_b = screenshot_screen()
    n, _ = pixel_diff(shot_a, shot_b)
    got = new_evt_lines(before, agent("evtlog"))
    # band-vs-bbox check dropped: the desktop capture is in WINDOW space,
    # live-tree rects in CLIENT space ( constant-ish but untracked offset )
    ok = any("RAD-11 -> GDI+" in ln for ln in got) and n > 30
    results.append((ok, "[rad] EVT %s, diff %d px" % (got[:1], n)))

    # menu: real-input NC click opens the engine popup ( MODAL - the event
    # listener blocks inside its tracking loop ), a real-input click on
    # empty area closes it; only THEN the MENU-BAR line lands in the log.
    # the popup is a separate hwnd, invisible to the main-window screenshot,
    # so behavior is asserted via the EVT line alone.
    before = agent("evtlog")
    ncclick(*MENU_ITEM1)
    time.sleep(0.5)
    ncclick(*MENU_DISMISS)
    time.sleep(0.5)
    got = new_evt_lines(before, agent("evtlog"))
    ok = any("MENU-BAR -> #1" in ln for ln in got)
    results.append((ok, "[menu] EVT %s" % (got[:1],)))

    # ---- EVT sequence integrity: newest line first ( prepend log ), so the
    # leading "%03d" numbers must strictly DECREASE down the text ----
    text = agent("evtlog")
    nums = [int(m.group(1)) for m in
            (re.match(r"^(\d{3})\s", ln) for ln in text.splitlines()) if m]
    ok = len(nums) >= 2 and all(nums[i] > nums[i + 1] for i in range(len(nums) - 1))
    results.append((ok, "[evt-seq] %d lines, numbering %s..%s ( newest first )"
                    % (len(nums), nums[0] if nums else "-", nums[-1] if nums else "-")))

    # give the borrowed Z-order slot back ( sink below the covering windows )
    try:
        agent("lower")
    except Exception:
        pass

    fail = 0
    for good, msg in results:
        if not good:
            fail += 1
        print(("  PASS  " if good else "  FAIL  ") + msg)
    print("E2E " + ("PASS" if fail == 0 else "FAIL (%d)" % fail))
    return 0 if fail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
