@echo off
rem ==========================================================================
rem hmskin build.cmd - developer command-line build with log redirection.
rem
rem usage:
rem   build.cmd                     build ALL 6 configs ( Win32/x64/ARM64 x
rem                                  Debug/Release ), log ^> build.log
rem   build.cmd Debug ARM64         build one config, log ^> build.log
rem   build.cmd Release             build all platforms for one config
rem
rem output: build.log in repo root ( compile + link + warnings, full msbuild
rem verbosity:minimal ). exit code non-zero on any failure. run from a
rem "Developer Command Prompt for VS 2026" ( or any shell with msbuild on
rem PATH ).
rem ==========================================================================
setlocal enabledelayedexpansion

set SLN=hmskin.sln
set LOG=build.log

rem ---- resolve msbuild ( if not on PATH, try vswhere ) ----
where msbuild >nul 2>nul
if %errorlevel% neq 0 (
    echo [build] msbuild not on PATH, probing via vswhere...
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" (
        echo [build] ERROR: msbuild not found. open a "Developer Command Prompt for VS 2026" first.
        exit /b 1
    )
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set MSBUILD=%%i
    if not defined MSBUILD (
        echo [build] ERROR: vswhere found no MSBuild. install "Desktop development with C++".
        exit /b 1
    )
) else (
    set MSBUILD=msbuild
)

rem ---- config matrix ----
set CFGS=Debug Release
set PLATS=Win32 x64 ARM64
if not "%~1"=="" set CFGS=%~1
if not "%~2"=="" set PLATS=%~2

echo [build] solution: %SLN%
echo [build] configs:  %CFGS%
echo [build] platforms: %PLATS%
echo [build] log: %LOG%

set FAIL=0
echo ==== hmskin build %date% %time% ==== > %LOG%

for %%c in (%CFGS%) do (
    for %%p in (%PLATS%) do (
        echo.
        echo [build] ==== %%c ^| %%p ====
        echo ==== %%c ^| %%p ====

"%MSBUILD%" %SLN% /m /v:m /p:Configuration=%%c /p:Platform=%%p >> %LOG% 2>&1
        if !errorlevel! neq 0 (
            echo [build] FAILED: %%c ^| %%p ( see %LOG% )
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
