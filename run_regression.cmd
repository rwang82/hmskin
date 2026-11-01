@echo off
rem Regression entry point: build the HEADLESS engine gate, then run it.
rem Exit code = number of failed cases ( 0 = all green ). This gate needs
rem NO interactive session ( unlike tools/smoke_test.py + e2e_test.py,
rem which drive the GUI demo and must run on a desktop session ).
rem Run after every engine change; the GUI suites stay the pre-push check.
setlocal
rem optional platform argument: Win32, x64 or ARM64 (default)
set "PLAT=%~1"
if not defined PLAT set "PLAT=ARM64"
cd /d "%~dp0"

set "MSBUILD="
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe 2^>nul`) do set "MSBUILD=%%i"
if not defined MSBUILD (
    echo hmskin: REGRESS harness FAIL - MSBuild not found via vswhere
    exit /b 100
)

rem serial + no node reuse: parallel/lingering cl.exe and msbuild nodes
rem exhausted PCH virtual memory in the VM. Building the regression
rem project pulls hmskin.vcxproj via the project reference - the demo
rem and other GUI layers are NOT part of this gate. SolutionDir is
rem passed explicitly: without a .sln context it degenerates to each
rem project's own dir and the shared output layout breaks.
"%MSBUILD%" HMSkinRegression\HMSkinRegression.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=%PLAT% /p:SolutionDir=%~dp0 /nr:false /v:m > regression.log 2>&1
if errorlevel 1 (
    echo hmskin: REGRESS harness FAIL - build error, see regression.log
    exit /b 100
)

echo hmskin: REGRESS run begin (platform %PLAT%, headless)
set "OUTDIR=%PLAT%\Release"
if "%PLAT%"=="Win32" set "OUTDIR=Release"
rem run from the output dir: the manifest + image\ are post-build copied
rem next to the exe ( same layout the demo runs from ).
pushd "%OUTDIR%"
HMSkinRegression.exe > ..\..\regression_run.tmp 2>&1
set "RETCODE=%errorlevel%"
popd
type regression_run.tmp
type regression_run.tmp >> regression.log
del regression_run.tmp
rem goto-style: a bare ")" inside an if-block's echo text closes the block
rem early and the parser chokes on the rest ( "was unexpected at this
rem time" ) - keep echo text paren-free.
if "%RETCODE%"=="0" goto regress_green
echo hmskin: REGRESS FAILED: %RETCODE% failed case count, see regression.log
exit /b %RETCODE%
:regress_green
echo hmskin: REGRESS ALL GREEN - exit code 0
exit /b 0
