#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin MCP server - AI-agent bridge into the demo VM.

Architecture: this script runs on the mac side as a stdio MCP server
( FastMCP, `pip install mcp` ). VM access is two-tier:

  - non-GUI queries ( process liveness, marker files ) go over plain
    `ssh -> powershell`;
  - desktop operations reach the user's interactive session like this:
      * demo is LAUNCHED through `explorer.exe <path>` so its parent is
        the shell ( a plain ssh child would create its window in a
        headless logon session; a scheduled task child gets no window
        station access - both were tried );
      * the demo publishes its window handle in every tree dump and
        serves two self-operations, WM_APP+1 ( refresh live tree ) and
        WM_APP+2 ( PrintWindow itself into _mcp_shot.png ), which this
        side triggers with cross-session PostMessage;
      * a one-shot dump is requested through the exe-dir "_dump_req"
        convention file ( explorer.exe cannot pass command-line args ).

Files produced on the VM land inside the shared folder ( also mounted
on the mac side ), so screenshots / tree dumps are read back by path
translation - no scp needed.

Engine-side hooks ( demo.exe, see hmskin_helper.cpp / demo.cpp ):
  --dump-tree <path>   one-shot: dump the keelobj tree after the first
                       frame settles, then exit ( same via _dump_req )
  WM_APP+1 message     live dump to <exe_dir>\\hmskin_tree.live.json
                       ( reflects chapter switches / visibility; also
                       published once automatically at startup )
  WM_APP+2 message     self-screenshot to <exe_dir>\\_mcp_shot.png

Usage:
  python3 tools/hmskin_mcp.py --selftest --ssh <user@vmhost>
  python3 tools/hmskin_mcp.py --ssh <user@vmhost>            ( MCP serve )
"""
import argparse
import json
import os
import re
import subprocess
import sys
import threading
import time

MAC_REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VM_REPO = os.environ.get("HMSKIN_VM_REPO", r"\\Mac\data_2T_990pro_samsung\win_dev\vm_data\hmskin")
VM_OUTDIR = VM_REPO + r"\ARM64\Debug"
VM_LIVE_TREE = VM_OUTDIR + r"\\hmskin_tree.live.json"
VM_DUMP_TREE = VM_REPO + r"\\tools\\_mcp_tree.json"
VM_SHOT = VM_OUTDIR + r"\\_mcp_shot.png"
VM_DUMP_REQ = VM_OUTDIR + r"\\_dump_req"
VM_DEMO = VM_OUTDIR + r"\\demo.exe"
VM_TRACE = VM_OUTDIR + r"\\hmskin_trace.log"
VM_MARKER = VM_OUTDIR + r"\\hmskin.trace"


def vm2mac(vm_path):
    """translate a VM-side repo path to the mac-side mount."""
    return vm_path.replace(VM_REPO, MAC_REPO).replace("\\", "/")


class HmskinBridge:
    """all VM interactions, serialized by an RLock ( tool methods nest );
    GUI ops through the desktop-session task bridge, the rest over ssh."""

    def __init__(self, ssh):
        self.ssh = ssh
        self.lock = threading.RLock()
        self._seq = 0
        self._tunnel_proc = None

    # ---- ssh plumbing ( non-GUI only ) ----
    def _ps(self, script, timeout=60):
        r = subprocess.run(
            ["ssh", self.ssh, "powershell", "-NoProfile", "-Command",
             "[Console]::OutputEncoding=[Text.Encoding]::UTF8; " + script],
            capture_output=True, text=True, encoding="utf-8", errors="replace",
            timeout=timeout)
        if r.returncode != 0:
            raise RuntimeError(f"vm powershell rc={r.returncode}: {r.stderr.strip()[:200]}")
        return r.stdout.strip()

    # ---- agent channel ( v4 ): ssh -L tunnel -> demo's TCP service ----
    # the demo serves 127.0.0.1:9321 ( JSON line protocol, see demo.cpp );
    # we forward it over an ssh tunnel so no firewall/port changes are
    # needed, and every logon-session / window-station wall disappears.
    _PORT = 9321

    def _tunnel(self):
        """make sure the ssh -L forward is up; returns the local port."""
        with self.lock:
            if self._tunnel_proc and self._tunnel_proc.poll() is None:
                return self._PORT
            self._tunnel_proc = subprocess.Popen(
                ["ssh", "-o", "BatchMode=yes", "-N", "-L",
                 f"{self._PORT}:127.0.0.1:{self._PORT}", self.ssh],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            time.sleep(1.0)
            return self._PORT

    def _agent(self, op, timeout=20):
        """one request/response line to the demo's agent service."""
        import socket
        self._tunnel()
        deadline = time.time() + timeout
        while time.time() < deadline:
            try:
                with socket.create_connection(("127.0.0.1", self._PORT), timeout=3) as sock:
                    sock.sendall((op + "\n").encode())
                    buf = b""
                    while b"\n" not in buf:
                        chunk = sock.recv(4096)
                        if not chunk:
                            break
                        buf += chunk
                    line = buf.decode(errors="replace").strip()
                    if line:
                        res = json.loads(line)
                        if not res.get("ok"):
                            raise RuntimeError(f"agent {op}: {res.get('data')}")
                        return res.get("data")
            except RuntimeError:
                raise
            except Exception:
                time.sleep(0.4)
        raise RuntimeError(f"agent {op}: unreachable ( demo not running? )")

    # ---- demo process management ----
    def _demo_alive(self):
        out = self._ps("(Get-Process demo -ErrorAction SilentlyContinue) -ne $null; exit 0")
        return out.endswith("True")

    def ensure_demo(self):
        with self.lock:
            try:
                self._agent("ping", timeout=4)
                return
            except RuntimeError:
                pass
            # not serving: try to launch through the shell ( a plain ssh
            # child would land in a headless logon session )
            self._ps(f"explorer.exe {VM_DEMO}; exit 0", timeout=30)
            for _ in range(20):
                time.sleep(0.5)
                try:
                    self._agent("ping", timeout=2)
                    return
                except RuntimeError:
                    continue
            raise RuntimeError("demo agent unreachable after launch attempt "
                               "( start demo.exe by hand once? )")

    def stop_demo(self):
        with self.lock:
            try:
                self._agent("quit")
            except RuntimeError:
                self._ps("Stop-Process -Name demo -Force; exit 0")

    # ---- tree / manifest ----
    def load_manifest(self):
        """keelobj <-> WMP join straight from the shared manifest ( no ssh )."""
        raw = open(os.path.join(MAC_REPO, "demo", "assets", "Debug", "demo.hm.manifest"),
                   encoding="utf-8").read()
        d = json.loads(re.sub(r"^\s*//.*$", "", raw, flags=re.M))
        wmp_cls = {w["idWMP"]: w["WMPClsName"] for w in d["WndMsgProcessor"]}
        keelobjs = [{
            "id": k["idKeelObj"],
            "parent": k.get("ParentKeelObjId", -1),
            "wmps": [{"id": w["id"], "class": wmp_cls.get(w["id"], "?")}
                     for w in k.get("WMP", [])],
        } for k in d["KeelObj"]]
        return {"keelobjs": keelobjs, "count": len(keelobjs)}

    @staticmethod
    def _read_tree(mac_path):
        return json.load(open(mac_path, encoding="utf-8-sig"))["hmskin_tree"]

    @staticmethod
    def _read_hwnd(mac_path):
        return json.load(open(mac_path, encoding="utf-8-sig")).get("hwnd")

    def dump_tree(self):
        """one-shot: launch demo --dump-tree on the desktop ( exits itself ),
        poll for the JSON file, read it."""
        with self.lock:
            mac_dump = vm2mac(VM_DUMP_TREE)
            if os.path.exists(mac_dump):
                os.remove(mac_dump)
            # prefer the resident agent ( WM_APP+3, same async pattern as
            # live/shot ); the cold _dump_req+explorer path proved flaky (
            # explorer swallows second launches of an already-running exe )
            try:
                self._agent(f"dump {VM_DUMP_TREE}")
            except RuntimeError:
                open(vm2mac(VM_DUMP_REQ), "w").write(VM_DUMP_TREE + "\n")
                self._ps(f"explorer.exe {VM_DEMO}; exit 0", timeout=30)
            deadline = time.time() + 30
            while time.time() < deadline:
                time.sleep(0.5)
                if os.path.exists(mac_dump):
                    time.sleep(0.2)   # let the write finish
                    tree = self._read_tree(mac_dump)
                    if tree:
                        return tree
            raise RuntimeError("dump produced no file")

    def live_tree(self):
        """dump the CURRENT tree state of the resident demo ( WM_APP+1 )."""
        with self.lock:
            self.ensure_demo()
            mac_live = vm2mac(VM_LIVE_TREE)
            before = os.path.getmtime(mac_live) if os.path.exists(mac_live) else 0
            self._agent("live")
            for _ in range(20):   # shared-folder propagation + dump write
                time.sleep(0.3)
                if os.path.exists(mac_live) and os.path.getmtime(mac_live) != before:
                    return self._read_tree(mac_live)
            raise RuntimeError("live tree file did not refresh ( WM_APP+1 )")

    # ---- interaction ----
    def click_keelobj(self, keelobj_id):
        """click the center of a keelobj's client-area rect. the synthetic
        down/up pair reaches the WMPs through the subclassed wndproc ( same
        path as real input ); synthetic mousemove does NOT raise
        WM_MOUSELEAVE, so hover states may stick - a known limitation."""
        nodes = self.live_tree()
        node = next((n for n in nodes if n["id"] == keelobj_id), None)
        if not node:
            raise KeyError(f"keelobj {keelobj_id} not in tree")
        if not node["visible"]:
            raise RuntimeError(f"keelobj {keelobj_id} is not visible ( wrong chapter? )")
        r = node.get("rect") or {}
        x = (r["l"] + r["r"]) // 2
        y = (r["t"] + r["b"]) // 2
        self._agent(f"click {x},{y}")
        return {"clicked": keelobj_id, "client_xy": [x, y]}

    def screenshot(self):
        """PrintWindow the resident demo into the shared folder; returns the
        mac-side png path."""
        with self.lock:
            self.ensure_demo()
            mac_shot = vm2mac(VM_SHOT)
            if os.path.exists(mac_shot):
                os.remove(mac_shot)
            self._agent("shot")
            deadline = time.time() + 15
            while time.time() < deadline:
                time.sleep(0.3)
                if os.path.exists(mac_shot) and os.path.getsize(mac_shot) > 0:
                    return mac_shot
            raise RuntimeError("screenshot file missing after PrintWindow")


# ============================================================
# selftest ( stdlib only - no MCP dependency )
# ============================================================
def selftest(bridge):
    results = []

    def step(name, fn):
        try:
            out = fn()
            results.append((True, f"[{name}] {out}"))
        except Exception as e:
            results.append((False, f"[{name}] FAIL: {e}"))

    step("manifest", lambda: f"{bridge.load_manifest()['count']} keelobjs joined with WMP classes")
    step("dump_tree", lambda: f"{len(bridge.dump_tree())} nodes ( one-shot demo run )")

    def live():
        tree = bridge.live_tree()
        sec1 = [n for n in tree if n["id"] in (3, 10)]
        if not all(n["visible"] for n in sec1):
            raise RuntimeError("unexpected initial visibility")
        return f"{len(tree)} live nodes, sec.1 visible"
    step("live_tree", live)

    def click():
        # strong anchor: enable the message trace, click K3 ( button ),
        # assert WM_LBUTTONDOWN/UP crossed the subclassed wndproc chain.
        trace_mac = vm2mac(VM_TRACE)
        bridge._ps(f"Set-Content -Path '{VM_MARKER}' -Value '1';"
                   f"if (Test-Path '{VM_TRACE}') {{ Remove-Item '{VM_TRACE}' -Force }}; exit 0")
        bridge.ensure_demo()
        time.sleep(1.5)   # marker picked up on next message
        info = bridge.click_keelobj(3)
        for _ in range(10):
            time.sleep(0.5)
            if os.path.exists(trace_mac):
                body = open(trace_mac, encoding="utf-8", errors="replace").read()
                if "msg=0x0201" in body and "msg=0x0202" in body:
                    bridge._ps(f"Remove-Item '{VM_MARKER}' -ErrorAction SilentlyContinue; exit 0")
                    return f"{info} verified: 0x0201/0x0202 in trace"
        bridge._ps(f"Remove-Item '{VM_MARKER}' -ErrorAction SilentlyContinue; exit 0")
        raise RuntimeError("click did not surface in hmskin_trace.log")
    step("click_keelobj", click)

    def shot():
        path = bridge.screenshot()
        size = os.path.getsize(path)
        if size < 10000:
            raise RuntimeError(f"suspiciously small png ({size} bytes)")
        return f"{path} ({size} bytes)"
    step("screenshot", shot)

    ok = all(r[0] for r in results)
    for good, msg in results:
        print(("  PASS  " if good else "  FAIL  ") + msg)
    print("SELFTEST " + ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


# ============================================================
# MCP layer ( optional import: serve mode only )
# ============================================================
def serve(bridge):
    try:
        from mcp.server.fastmcp import FastMCP
    except ImportError:
        print("serve mode needs the mcp package: pip3 install mcp", file=sys.stderr)
        return 2
    mcp = FastMCP("hmskin")

    @mcp.tool()
    def load_manifest() -> dict:
        """keelobj <-> WMP <-> class-name table from the demo manifest."""
        return bridge.load_manifest()

    @mcp.tool()
    def dump_tree() -> list:
        """one-shot keelobj tree ( launches demo --dump-tree, demo exits )."""
        return bridge.dump_tree()

    @mcp.tool()
    def live_tree() -> list:
        """CURRENT keelobj tree of the resident demo ( visibility, rects )."""
        return bridge.live_tree()

    @mcp.tool()
    def screenshot() -> str:
        """PrintWindow the resident demo; returns the mac-side png path."""
        return bridge.screenshot()

    @mcp.tool()
    def click_keelobj(keelobj_id: int) -> dict:
        """click the center of a keelobj ( client-area coords, synthetic down/up )."""
        return bridge.click_keelobj(keelobj_id)

    @mcp.tool()
    def stop_demo() -> dict:
        """kill the resident demo process."""
        bridge.stop_demo()
        return {"stopped": True}

    mcp.run()
    return 0


def main():
    ap = argparse.ArgumentParser(description="hmskin AI-agent bridge ( MCP / selftest )")
    ap.add_argument("--ssh", required=True, metavar="USER@HOST", help="VM ssh target")
    ap.add_argument("--selftest", action="store_true", help="run the 5-step selftest and exit")
    args = ap.parse_args()
    bridge = HmskinBridge(args.ssh)
    if args.selftest:
        sys.exit(selftest(bridge))
    sys.exit(serve(bridge))


if __name__ == "__main__":
    main()
