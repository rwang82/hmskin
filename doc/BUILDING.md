# hmskin Build Guide

[中文](BUILDING.zh-CN.md) | **English**

## Prerequisites ( one-time )

Confirm in the VS Installer:
- the **Desktop development with C++** workload
- **C++ ATL for v145 build tools** ( 6 files include AtlConv.h; missing it
  breaks the build )
- the latest Win10/11 SDK

## Command-line build ( recommended for CI / remote collaboration )

Open a **Developer Command Prompt for VS 2026**, cd to the repo root:

```
build.cmd                rem all 6 configs ( Win32/x64/ARM64 x Debug/Release )
build.cmd Debug ARM64    rem single config ( recommended for daily work )
build.cmd Release        rem single config across three platforms
```

- All output is redirected to **build.log** ( repo root, gitignored ) -
  Mac-side / collaborators just read the file
- Any failing config yields a non-zero exit code ( CI-friendly )
- msbuild is auto-detected via vswhere when not on PATH

## Building in Visual Studio

Open `hmskin.sln`, pick a configuration in the Configuration Manager, F7 as
usual.

## Artifact locations

```
{Win32|x64|ARM64}\{Debug|Release}\    <- exe/dll/manifest/image family
hmskin\lib\{Platform}\                <- hmskin import library ( hmskinD.dll for debug )
```

Running the demo needs `demo.exe + hmskin.dll + demo.hm.manifest + image\`
in one directory ( PostBuild copies automatically; works for command-line
and VS builds alike ).

## Common failures

| Symptom | Cause |
|---|---|
| AtlConv.h not found | missing the ATL component ( see prerequisites ) |
| .pch not found | the PCH conditions must be written per-platform on the stdafx.cpp entry ( historical pitfall ); Rebuild once |
| ARM64 platform missing | an old sln; make sure you pulled the latest master |

## Headless regression gate

`run_regression.cmd` ( repo root ) is a second, session-independent check:
it builds only hmskin + the HMSkinRegression console project and runs 22
headless engine-logic assertions ( lifecycle, DPI math, contracts ). Exit
code = number of failed cases; output lands in `regression.log`. Unlike
`tools/smoke_test.py` / `e2e_test.py` ( which drive the GUI demo and need
an interactive desktop session ), this gate runs anywhere - even over ssh.
