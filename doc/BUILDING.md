# hmskin 构建指南

## 前置（一次性）

VS Installer 确认已装：
- **Desktop development with C++** 工作负载
- **C++ ATL for v145 build tools**（6 个文件 include AtlConv.h，缺了编译失败）
- 最新 Win10/11 SDK

## 命令行构建（推荐给 CI / 远程协作）

打开 **Developer Command Prompt for VS 2026**，cd 到仓库根：

```
build.cmd                rem 全 6 配置（Win32/x64/ARM64 × Debug/Release）
build.cmd Debug ARM64    rem 单配置（日常开发推荐）
build.cmd Release        rem 三平台单配置
```

- 全量输出重定向到 **build.log**（仓库根，已 .gitignore）——Mac/协作者直接读文件
- 任一配置失败 exit code 非零（CI 可判）
- msbuild 不在 PATH 时自动用 vswhere 探测

## Visual Studio 构建

打开 `hmskin.sln`，Configuration Manager 选配置，常规 F7。

## 产物位置

```
{Win32|x64|ARM64}\{Debug|Release}\    ← exe/dll/manifest/image 全家
hmskin\lib\{Platform}\                ← hmskin 导入库（hmskinD.dll 调试用）
```

demo 运行需 `demo.exe + hmskin.dll + demo.hm.manifest + image\` 同目录
（PostBuild 自动复制，命令行/VS 构建均生效）。

## 常见失败

| 症状 | 原因 |
|---|---|
| AtlConv.h 找不到 | 缺 ATL 组件（见前置） |
| .pch 找不到 | PCH 条件需逐平台写在 stdafx.cpp 条目（历史踩坑），Rebuild 一次 |
| ARM64 平台不存在 | 旧 sln；确认拉了最新 master |
