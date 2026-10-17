@echo off
rem ==========================================================================
rem hmskin build.cmd - developer command-line build with log redirection.
rem
rem usage:
rem   build.cmd                     build ALL 6 configs, log ^> build.log
rem   build.cmd Debug ARM64         build one config
rem   build.cmd Release             all platforms, one config
rem
rem works from ANY cmd ( auto-locates msbuild via vswhere ) or from a
rem "Developer Command Prompt for VS 2026" ( msbuild already on PATH ).
rem ==========================================================================
setlocal enabledelayedexpansion

set SLN=hmskin.sln
set LOG=build.log
set MSBUILD=

rem ---- 1) msbuild already on PATH? ----
where msbuild >nul 2>nul
if %errorlevel% equ 0 (
    set MSBUILD=msbuild
    goto :found
)

rem ---- 2) locate vswhere ( outside if-blocks: ProgramFiles^(x86^) parens ) ----
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
if not exist "%VSWHERE%" set VSWHERE=%ProgramW6432%\Microsoft Visual Studio\Installer\vswhere.exe
if not exist "%VSWHERE%" (
    echo [build] ERROR: msbuild not on PATH and vswhere.exe not found.
    echo [build]        open "Developer Command Prompt for VS 2026" and retry,
    echo [build]        or install VS 2026 with the C++ workload.
    exit /b 1
)

echo [build] msbuild not on PATH, probing via vswhere...
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set MSBUILD=%%i

if not defined MSBUILD (
    echo [build] ERROR: vswhere found no MSBuild. install "Desktop development with C++".
    exit /b 1
)
echo [build] using: !MSBUILD!

:found
rem ---- config matrix ----
set CFGS=Debug Release
set PLATS=Win32 x64 ARM64
if not "%~1"=="" set CFGS=%~1
if not "%~2"=="" set PLATS=%~2

echo [build] solution:  %SLN%
echo [build] configs:   %CFGS%
echo [build] platforms: %PLATS%
echo [build] log:       %LOG%

set FAIL=0
echo ==== hmskin build %date% %time% ==== > %LOG%

for %%c in (%CFGS%) do (
    for %%p in (%PLATS%) do (
        echo.
        echo [build] ==== %%c ^| %%p ====
        echo ==== %%c ^| %%p ====

"%MSBUILD%" %SLN% /m /v:m /p:Configuration=%%c /p:Platform=%%p >> %LOG% 2>&1
        if !errorlevel! neq 0 (
            echo [build] FAILED: %%c ^| %%p ^( see %LOG% ^)
            echo FAILED >> %LOG%
            set FAIL=1
        ) else (
            echo [build] ok: %%c ^| %%p
            echo ok >> %LOG%
        )
    )
)

echo.
if %FAIL% neq 0 (
    echo [build] DONE with FAILURES - full log: %LOG%
) else (
    echo [build] DONE all ok - log: %LOG%
)
exit /b %FAIL%
