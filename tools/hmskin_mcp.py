#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
hmskin MCP server - AI-agent bridge into the demo VM.

Architecture: this script runs on the mac side as a stdio MCP server
( FastMCP, `pip install mcp` ). VM access is two-tier:

  - non-GUI queries ( marker files ) go over plain `ssh -> powershell`;
  - desktop operations talk to the RESIDENT demo through its in-process
    agent service ( demo.exe listens on 127.0.0.1:9321, JSON line
    protocol ), reached via an `ssh -L` tunnel - no firewall/port
    changes, and every logon-session / window-station wall disappears.
    The demo itself is STARTED MANUALLY by the user ( double-click on
    the VM desktop ): remote launches end up in headless logon sessions
    and scheduled-task children lack window-station access ( both were
    tried ), so instead of auto-launching, every tool that needs the
    demo fails fast with a reminder to start it by hand.

Files produced on the VM land inside the shared folder ( also mounted
on the mac side ), so screenshots / tree dumps are read back by path
translation - no scp needed.

Engine-side hooks ( demo.exe, see demo.cpp ):
  TCP 127.0.0.1:9321   agent ops: ping / quit / live / shot /
                       click x,y / dump <path>
  WM_APP+1/2/3         live tree dump / self-screenshot ( PrintWindow
                       into _mcp_shot.png ) / on-demand deep dump
  --dump-tree <path>   one-shot cold dump used by smoke_test.py --tree
                       ( not used by this bridge )

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
VM_TRACE = VM_OUTDIR + r"\\hmskin_trace.log"
VM_MARKER = VM_OUTDIR + r"\\hmskin.trace"

# the demo is started manually ( double-click on the VM desktop );
# remote launches proved unreliable, so tools fail fast with this
# reminder instead of trying to launch it themselves.
DEMO_NOT_RUNNING = ("demo is not running - start it by double-clicking "
                    "demo.exe ( ARM64\\Debug ) on the VM desktop, then retry"
                    " ( 请在 VM 桌面双击启动 demo 后重试 )")


def vm2mac(vm_path):
    """translate a VM-side repo path to the mac-side mount."""
    return vm_path.replace(VM_REPO, MAC_REPO).replace("\\", "/")


class HmskinBridge:
    """all VM interactions, serialized by an RLock ( tool methods nest );
    GUI ops through the demo's TCP agent service ( ssh -L tunnel ),
    the rest over ssh."""

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

    def _raw_probe(self, timeout=2):
        """full ping round trip through the local forward ( TCP connect alone
        succeeds even when the remote side is dead )."""
        import socket
        with socket.create_connection(("127.0.0.1", self._PORT), timeout=timeout) as sock:
            sock.sendall(b"ping\n")
            buf = b""
            while b"\n" not in buf:
                chunk = sock.recv(256)
                if not chunk:
                    break
                buf += chunk
        if b"pong" not in buf:
            raise RuntimeError("tunnel probe: no pong")

    def _tunnel(self):
        """make sure ONE working ssh -L forward is up; returns the local port.
        orphans from earlier bridge instances fight over the local port, so
        clear them first and probe before trusting a spawned forward."""
        with self.lock:
            if self._tunnel_proc and self._tunnel_proc.poll() is None:
                try:
                    self._raw_probe()
                    return self._PORT
                except Exception:
                    self._tunnel_proc.kill()   # alive but broken: recycle
                    self._tunnel_proc = None
            subprocess.run(["pkill", "-f",
                            "ssh.*-L.*%d:127.0.0.1:%d" % (self._PORT, self._PORT)],
                           capture_output=True)
            time.sleep(0.3)
            self._tunnel_proc = subprocess.Popen(
                ["ssh", "-o", "BatchMode=yes", "-o", "ExitOnForwardFailure=yes",
                 "-N", "-L", f"{self._PORT}:127.0.0.1:{self._PORT}", self.ssh],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            deadline = time.time() + 8
            while time.time() < deadline:
                time.sleep(0.4)
                if self._tunnel_proc.poll() is not None:
                    break   # exited ( bind conflict / ssh failure )
                try:
                    self._raw_probe()
                    return self._PORT
                except Exception:
                    continue
            raise RuntimeError("agent tunnel would not come up "
                               "( ssh forward failed? demo not running? )")

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
    def ensure_demo(self):
        with self.lock:
            try:
                self._agent("ping", timeout=4)
            except RuntimeError:
                raise RuntimeError(DEMO_NOT_RUNNING) from None

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
        """on-demand deep dump from the resident demo ( WM_APP+3 ),
        polled from the shared folder and read it."""
        with self.lock:
            self.ensure_demo()
            mac_dump = vm2mac(VM_DUMP_TREE)
            if os.path.exists(mac_dump):
                os.remove(mac_dump)
            self._agent(f"dump {VM_DUMP_TREE}")
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
    step("dump_tree", lambda: f"{len(bridge.dump_tree())} nodes ( agent dump )")

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
        """keelobj tree on-demand dump from the resident demo ( demo must be running )."""
        return bridge.dump_tree()

    @mcp.tool()
    def live_tree() -> list:
        """CURRENT keelobj tree of the resident demo ( visibility, rects; demo must be running )."""
        return bridge.live_tree()

    @mcp.tool()
    def screenshot() -> str:
        """PrintWindow the resident demo ( demo must be running ); returns the mac-side png path."""
        return bridge.screenshot()

    @mcp.tool()
    def click_keelobj(keelobj_id: int) -> dict:
        """click the center of a keelobj ( client-area coords, synthetic down/up; demo must be running )."""
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
