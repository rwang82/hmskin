#include "stdafx.h"
#include "hmskin_event_listener_base.h"
#include "win\frame\hm_hook_trackpopupmenu.h"
#include "hm_layout_scale.h"
#include <intrin.h>
#include "hmskin.h"
#include "tchar.h"
//#include <stdio.h>
//for gdi+ begin
#include <unknwn.h>
#include <Rpc.h>
#include <gdiplus.h>
//for gdi+ end 
#include "win\hook_system\hm_hs_wnd_proc_hook.h"
#include "win\hook_system\hm_hs_mouse_proc_hook.h"
#include "win\frame\hm_msg_sink.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_layout_decoders.h"
#include "win\frame\hm_uiroot.h"
#include "win\frame\hm_perf_stats.h"
#include "win\hm_layoutobj_defs.h"
#include "win\paint_system\hm_ps_paint_layer_pic.h"
#include "win\wmp\hm_wmp_button.h"
#include "win\wmp\hm_wmp_layouttrigger.h"
#include "win\wmp\hm_wmp_drawtrigger.h"
#include "win\wmp\hm_wmp_bkgnd.h"
#include "win\wmp\hm_wmp_drawtrigger_nca.h"
#include "win\wmp\hm_wmp_bkgnd_nca.h"
#include "win\wmp\hm_wmp_button_nca.h"
#include "win\wmp\hm_wmp_rootmenu.h"
#include "win\wmp\hm_wmp_popupmenu.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\wmp\hm_wmp_dialogview.h"
#include "win\hook_system\hm_hs_apihook.h"
#include "win\wndctrl_repository\hm_wndctrl_base.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\resource_system\hm_rs_uires_pic.h"
#include "win\resource_system\hm_rs_wndmark.h"
#include "win\paint_system\hm_ps_drawtool_gdi.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "win\keelctrl_repository\hm_keelctrl_scrollview.h"
#include "win\loader\hm_manifest_loader.h"
#include "win\loader\hm_loader_wndstyle.h"
#include <dwmapi.h>
//#include "hmlog.h"
#include "common\FuncPack.h"
#include "common\MemFuncPack.h"
//
#include "io.h"

///////////////////////////////////////////////////////////////////////
using namespace Gdiplus;
extern GdiplusStartupInput gdiplusStartupInput;
extern ULONG_PTR           gdiplusToken;
///////////////////////////////////////////////////////////////////////
extern HMODULE g_hModule;
///////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_connect_msgsink( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
HMLIB_API bool hm_wndpropid_disconnect_msgsink( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
HMLIB_API bool hm_wndpropid_connect_coordsys( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
HMLIB_API bool hm_wndpropid_disconnect_coordsys( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
///////////////////////////////////////////////////////////////////////
static bool ParseUIManifest( const TCHAR* szUIManifestFilePath, hm_uiroot* pHMUIRoot );
static bool hm_wndcls_reg( HINSTANCE hInst );
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// DPI layout scale ( see hm_layout_scale.h )
static double g_dblLayoutScale = 1.0;
double hm_layout_scale() {
	return g_dblLayoutScale;
}
int hm_layout_scale_px( int nValue96 ) {
	return (int)( nValue96 * g_dblLayoutScale + 0.5 );
}

bool hm_lib_startup( hm_startup_param* pParam, const TCHAR* szResFilePath, hm_uiroot** ppHMUIRoot )
{
	if ( !g_hModule ) {
		::OutputDebugString( _T("error: g_hModule == NULL.\n") );
		return false;
	} else	{
		::OutputDebugString( _T("ok: g_hModule != NULL.\n") );
	}
	
	// show which module calling hm library
	{
	HMODULE hCallerModule = NULL;   
	TCHAR szModuleName[MAX_PATH] = _T("");   
	void *callerAddress = _ReturnAddress();   
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)callerAddress, &hCallerModule))   
	{   
		GetModuleFileName(hCallerModule, szModuleName, MAX_PATH );   
		_tprintf( _T("%s is calling hm lib"), szModuleName );
	}

	}
	// Initialize GDI+
	_tprintf( _T("call GdiplusStartup().\n") );
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// register window class of hm lib
	if ( !hm_wndcls_reg( pParam->m_hInst ) )
		return false;
	hm_uiroot* pHMUIRootNew = NULL;
	hm_uiroot_factory_type::obj_id_type idHMUIRoot;

	idHMUIRoot = HMUIROOTFACTORY_INSTANCE->createObj< hm_uiroot >( pParam->m_pHMSkinEventListener );
	pHMUIRootNew = HMUIROOTFACTORY_INSTANCE->getObj( idHMUIRoot );
	if ( !pHMUIRootNew )
		return false;
	CMemFuncPack mfpkDestroyHMUIRoot( HMUIROOTFACTORY_INSTANCE, &hm_uiroot_factory_type::destroyObj, idHMUIRoot );

	// DPI: apply the layout scale BEFORE any manifest number is parsed
	// ( hm_startup_param::m_uDpi, 96 baseline ). authors write 96dpi
	// manifests; the runtime works in physical pixels. the session value
	// lives on the uiroot ( loaders read it ); the global is only the
	// app-facing mirror ( hm_layout_scale/hm_layout_scale_px ).
	pHMUIRootNew->m_dblLayoutScale = ( pParam->m_uDpi > 0 ? pParam->m_uDpi : 96 ) / 96.0;
	g_dblLayoutScale = pHMUIRootNew->m_dblLayoutScale;

	if ( !ParseUIManifest( szResFilePath, pHMUIRootNew ) )
		return false;

	//
	hs_wnd_proc_hook_start( g_hModule );
	//
	//hs_mouse_ll_proc_hook_start( g_hModule );
	//
	hs_mouse_proc_hook_start( g_hModule );
	//

	mfpkDestroyHMUIRoot.Cancel();
	*ppHMUIRoot = pHMUIRootNew;
	return true;
}

static bool s_bTPMHookEnabled = false;
bool hm_lib_enable_trackpopupmenu_hook( hm_uiroot* pHMUIRoot, BOOL bEnable )
{
	if ( !pHMUIRoot )
		return false;
	if ( bEnable ) {
		if ( s_bTPMHookEnabled )
			return true;
		hm_tpm_set_root( pHMUIRoot );
		if ( !APIHOOK_INSTANCE->hookOnTrackPopupMenu( _T("TrackPopupMenu"), _T("user32.dll") ) )
			return false;
		s_bTPMHookEnabled = true;
	} else {
		if ( !s_bTPMHookEnabled )
			return true;
		APIHOOK_INSTANCE->hookOff( _T("TrackPopupMenu"), _T("user32.dll") );
		hm_tpm_set_root( NULL );
		s_bTPMHookEnabled = false;
	}
	return true;
}

void hm_lib_shutdown( hm_uiroot* pHMUIRoot )
{
	// A/B perf counters outlast everything else ( plain statics, no engine
	// state ) - dump first thing so even a crashy teardown never eats them.
	hm_perf_dump();

	// restore patched user32 bytes BEFORE anything tears down - a patched
	// API pointing into a dying DLL would crash the host at next menu call.
	if ( s_bTPMHookEnabled ) {
		APIHOOK_INSTANCE->hookOff( _T("TrackPopupMenu"), _T("user32.dll") );
		hm_tpm_set_root( NULL );
		s_bTPMHookEnabled = false;
	}
	//
	//g_instanceAPIHook.hookOff( _T("TrackPopupMenu"), _T("user32.dll") );
	//
	hs_mouse_proc_hook_stop();
	//
	hs_mouse_ll_proc_hook_stop();
	//
	hs_wnd_proc_hook_stop();
}

bool ParseUIManifest( const TCHAR* szUIManifestFilePath, hm_uiroot* pHMUIRoot ) {
	hm_manifest_loader* pManifestLoader = NULL;

	pManifestLoader = new hm_manifest_loader( szUIManifestFilePath );
	
	if ( !pManifestLoader->load( pHMUIRoot ) ) {
		delete pManifestLoader;
		return false;
	}

	delete pManifestLoader;
	return true;
}

//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_connect( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp )
{
	if ( !pHMUIRoot )
		return false;
	bool bResult = true;

	if ( !pHMUIRoot->m_pHWNDDespository->bindHWND2PropId( hWnd, idWndProp ) )
		return false;
	CMemFuncPack mfpkUnBindHWND( pHMUIRoot->m_pHWNDDespository, &hm_rs_hwnddepository::unBindHWND, hWnd ); 
	if ( !hm_wndpropid_connect_msgsink( pHMUIRoot, hWnd, idWndProp ) )
		return false;
	CFuncPack fpkDisconnectMsgSink( hm_wndpropid_disconnect_msgsink, pHMUIRoot, hWnd, idWndProp );
	if ( !hm_wndpropid_connect_coordsys( pHMUIRoot, hWnd, idWndProp ) )
		return false;
	CFuncPack fpkDisconnectCoordSys( hm_wndpropid_disconnect_coordsys, pHMUIRoot, hWnd, idWndProp );

	// window style adaptation ( windows 11+ corner preference etc. ).
	// on windows 10 and older DwmSetWindowAttribute fails harmlessly for this
	// attribute id, so no version check is needed.
	if ( pHMUIRoot->m_nWndCornerPref != HM_WNDCORNER_NOTSET ) {
		INT nPref = (INT)pHMUIRoot->m_nWndCornerPref;
		::DwmSetWindowAttribute( hWnd, HM_DWMWA_WINDOW_CORNER_PREFERENCE,
		                         &nPref, sizeof( nPref ) );
	}

	mfpkUnBindHWND.Cancel();
	fpkDisconnectMsgSink.Cancel();
	fpkDisconnectCoordSys.Cancel();
	return true;
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_disconnect( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp )
{
	if ( !pHMUIRoot )
		return false;
	bool bResult = true;
	
	if ( !hm_wndpropid_disconnect_msgsink( pHMUIRoot, hWnd, idWndProp ) ) {
		bResult = false;
	}
	if ( !hm_wndpropid_disconnect_coordsys( pHMUIRoot, hWnd, idWndProp ) ) {
		bResult = false;
	}
	pHMUIRoot->m_pHWNDDespository->unBindHWND( hWnd );
	
	return bResult;
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_connect_msgsink( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp )
{
	if ( !pHMUIRoot || ( !hWnd ) || ( !IsWindow( hWnd ) ) )
		return false;
	hm_rs_wndmark wndMark;

	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
	wndMark.m_uWndPropID = idWndProp;
	return ConnectHwnd2MsgSink( pHMUIRoot, hWnd, wndMark );
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_disconnect_msgsink( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp )
{
	if ( !pHMUIRoot || ( !hWnd )  )
		return false;

	hm_rs_wndmark wndMark;

	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
	wndMark.m_uWndPropID = idWndProp;
	//
	return DisconnectHwnd2MsgSink( pHMUIRoot, hWnd, wndMark );
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_connect_coordsys( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp )
{		
	if ( !pHMUIRoot || ( !hWnd ) || ( !IsWindow( hWnd ) ) )
		return false;
	hm_rs_wndmark wndMark;

	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
	wndMark.m_uWndPropID = idWndProp;
	return ConnectHwnd2CoordSys( pHMUIRoot, hWnd, wndMark );
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_disconnect_coordsys( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp )
{
	if ( !pHMUIRoot || !hWnd )
		return false;
	hm_rs_wndmark wndMark;
	
	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
	wndMark.m_uWndPropID = idWndProp;
	//
	return DisconnectHwnd2CoordSys( pHMUIRoot, hWnd, wndMark );
}

bool hm_wndcls_reg( HINSTANCE hInst ) {
	hm_wndctrl_base::RegWndClass( hInst );
	return true;
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_get_keelobj( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, hm_keel_obj** ppKeelObj ) {
	if ( !pHMUIRoot || !ppKeelObj )
		return false;
	hm_keel_obj* pKeelObj = NULL;

	pKeelObj = pHMUIRoot->m_pKeelObjFactory->getObj( uKeelObjId );
	*ppKeelObj = pKeelObj;
	return true;
}
