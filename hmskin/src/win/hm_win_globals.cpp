#include "stdafx.h"
#include "win\hm_win_globals.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\hook_system\hm_hs_apihook.h"
#include "win\frame\hm_keel_obj.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_uiroot.h"
#include "common\FuncPack.h"
#include "win\hm_export_defs.h"
//for gdi+ begin
#include <unknwn.h>
#include <Rpc.h>
#include <gdiplus.h>
//for gdi+ end 

////////////////////////////////////////////////////////////////
using namespace Gdiplus;
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR           gdiplusToken;
CFuncPack g_fpGdiplusDown( GdiplusShutdown, gdiplusToken );
hm_hs_apihook g_instanceAPIHook;
HMLIB_API hm_uiroot_factory_type g_factoryUIRoot;


