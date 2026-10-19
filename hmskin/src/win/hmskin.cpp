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
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_layout_decoders.h"
#include "win\frame\hm_uiroot.h"
#include "win\wmp\hm_wmp_display.h"
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
#include "win\paint_system\hm_ps_drawtool_d2d.h"
#include "win\keelctrl_repository\hm_keelctrl_scrollview.h"
#include "win\loader\hm_manifest_loader.h"
#include "win\loader\hm_loader_wndstyle.h"
#include <dwmapi.h>
//#include "hmlog.h"
#include "common\FuncPack.h"
#include "common\MemFuncPack.h"
//
#include "io.h"
#include <string>

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

// single-uiroot contract counter ( see hm_lib_startup )
static void __cdecl hm_count_uiroot( hm_uiroot* /*pRoot*/, void* pParam ) {
	++*( unsigned int* )pParam;
}

bool hm_lib_startup( hm_startup_param* pParam, const TCHAR* szResFilePath, hm_uiroot** ppHMUIRoot )
{
	// single-uiroot contract ( see hmskin.h ): refuse a second session.
	unsigned int uLiveUIRoots = 0;
	HMUIROOTFACTORY_INSTANCE->enumObjs( hm_count_uiroot, &uLiveUIRoots );
	if ( uLiveUIRoots > 0 ) {
		::OutputDebugString( _T("hmskin: second uiroot refused ( single-uiroot contract ).\n") );
		return false;
	}

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
//////////////////////////////////////////////////////////////////////////
// id-based keelobj runtime controls ( see hmskin.h )
HMLIB_API bool hm_keelobj_set_visible( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, BOOL bVisible ) {
	hm_keel_obj* pKeelObj = NULL;
	if ( !hm_get_keelobj( pHMUIRoot, uKeelObjId, &pKeelObj ) || !pKeelObj )
		return false;
	pKeelObj->setVisible( bVisible );
	return true;
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_keelobj_set_clip_children( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, BOOL bClip ) {
	hm_keel_obj* pKeelObj = NULL;
	if ( !hm_get_keelobj( pHMUIRoot, uKeelObjId, &pKeelObj ) || !pKeelObj )
		return false;
	pKeelObj->setClipChildren( bClip );
	return true;
}
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_keelobj_get_layout_rect( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, RECT* prc ) {
	hm_keel_obj* pKeelObj = NULL;
	if ( !prc || !hm_get_keelobj( pHMUIRoot, uKeelObjId, &pKeelObj ) || !pKeelObj )
		return false;
	hm_layout_obj* pLayout = pKeelObj->getLayoutObj();
	if ( !pLayout )
		return false;
	*prc = pLayout->m_rect;
	return true;
}
//////////////////////////////////////////////////////////////////////////
// agent/debug aid: keelobj tree JSON dump ( see hmskin.h ). hand-rolled
// JSON - the payload is numbers/bools only, no escaping is needed.
struct hm_dump_ctx {
	HWND hWnd;
	std::basic_string< TCHAR > strJson;
	unsigned int nCount;
};

static void __cdecl hm_dump_one( hm_keel_obj* pKeelObj, void* pParam ) {
	hm_dump_ctx* p = ( hm_dump_ctx* )pParam;
	hm_keel_obj* pParent = pKeelObj->getParent();
	hm_layout_obj* pLayout = pKeelObj->getLayoutObj();
	RECT rc;
	BOOL bMapped = FALSE;
	if ( pLayout && pLayout->hasCoordinateSystem() )
		bMapped = pLayout->mapPosition( p->hWnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rc );
	if ( p->nCount )
		p->strJson += _T(",\n");
	TCHAR szNode[ 256 ];
	if ( bMapped ) {
		_stprintf_s( szNode, 256,
			_T("    { \"id\": %u, \"parentId\": %d, \"visible\": %s, \"rect\": { \"l\": %ld, \"t\": %ld, \"r\": %ld, \"b\": %ld } }"),
			pKeelObj->getId(),
			pParent ? (int)pParent->getId() : -1,
			pKeelObj->isVisible() ? _T("true") : _T("false"),
			rc.left, rc.top, rc.right, rc.bottom );
	} else {
		RECT rcEmpty = { 0, 0, 0, 0 };
		const RECT& rcRaw = pLayout ? pLayout->m_rect : rcEmpty;
		_stprintf_s( szNode, 256,
			_T("    { \"id\": %u, \"parentId\": %d, \"visible\": %s, \"rect\": null, \"rectRaw\": { \"l\": %ld, \"t\": %ld, \"r\": %ld, \"b\": %ld } }"),
			pKeelObj->getId(),
			pParent ? (int)pParent->getId() : -1,
			pKeelObj->isVisible() ? _T("true") : _T("false"),
			rcRaw.left, rcRaw.top, rcRaw.right, rcRaw.bottom );
	}
	p->strJson += szNode;
	p->nCount++;
}

HMLIB_API unsigned int hm_keelobj_dump_tree( hm_uiroot* pHMUIRoot, HWND hWndMain, const TCHAR* szJsonPath ) {
	if ( !pHMUIRoot || !pHMUIRoot->m_pKeelObjFactory || !szJsonPath || !szJsonPath[ 0 ] || !hWndMain )
		return 0;
	hm_dump_ctx ctx;
	ctx.hWnd = hWndMain;
	ctx.nCount = 0;
	TCHAR szHead[ 64 ];
	_stprintf_s( szHead, 64, _T("{\n  \"hwnd\": %lu,\n  \"hmskin_tree\": [\n"), (unsigned long)(ULONG_PTR)hWndMain );
	ctx.strJson = szHead;
	pHMUIRoot->m_pKeelObjFactory->enumObjs( hm_dump_one, &ctx );
	ctx.strJson += _T("\n  ]\n}\n");
	FILE* fp = _tfopen( szJsonPath, _T("w") );
	if ( !fp )
		return 0;
	_fputts( ctx.strJson.c_str(), fp );
	fclose( fp );
	return ctx.nCount;
}
//////////////////////////////////////////////////////////////////////////
struct hm_settool_ctx {
	hm_ps_drawtool_i* pTool;
};

static void __cdecl hm_settool_one( hm_wndmsgprocessor_i* pWMP, void* pParam ) {
	hm_wmp_display* pDisp = dynamic_cast< hm_wmp_display* >( pWMP );
	if ( pDisp )
		pDisp->setDrawTool( static_cast< hm_settool_ctx* >( pParam )->pTool );
}

// swap the draw tool of every display WMP ( see hmskin.h )
HMLIB_API bool hm_set_drawtool_all( hm_uiroot* pHMUIRoot, int eTool ) {
	if ( !pHMUIRoot || !pHMUIRoot->m_pWMPFactory )
		return false;
	hm_ps_drawtool_i* pTool = NULL;
	switch ( eTool ) {
	case HM_DRAWTOL_D2D:
		pTool = hm_ps_drawtool_d2d::getInstance();
		break;
	case HM_DRAWTOL_GDIPLUS:
		pTool = hm_ps_drawtool_gdiplus::getInstance();
		break;
	case HM_DRAWTOL_GDI:
		pTool = hm_ps_drawtool_gdi::getInstance();
		break;
	default:
		return false;
	}
	hm_settool_ctx ctx;
	ctx.pTool = pTool;
	pHMUIRoot->m_pWMPFactory->enumObjs( hm_settool_one, &ctx );
	return true;
}
//////////////////////////////////////////////////////////////////////////
// host text API ( see hmskin.h ): locate the WMP by manifest ids, then the
// text uires - first-in-declaration-order, or exact ( district, index )
static hm_rs_uires_text* _find_wmp_text_first( hm_wmp_display* pWMPDisplay ) {
	hm_wmp_display::district_id_container_type containerIds;
	hm_wmp_display::district_id_container_type::iterator itId, iendId;

	pWMPDisplay->getAllDistrictId( containerIds );
	iendId = containerIds.end();
	for ( itId = containerIds.begin(); itId != iendId; ++itId ) {
		hm_districtobj* pDistrict = NULL;
		if ( !pWMPDisplay->getDistrictObj( *itId, &pDistrict ) || !pDistrict )
			continue;
		for ( unsigned int i = 0; i < pDistrict->getShowObjCount(); ++i ) {
			hm_rs_uires_text* pText =
			    dynamic_cast< hm_rs_uires_text* >( pDistrict->getShowObj( i )->m_pUIRes );
			if ( pText )
				return pText;
		}
	}
	return NULL;
}

HMLIB_API bool hm_wmp_set_text( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, unsigned int uWMPId, const TCHAR* szText ) {
	if ( !pHMUIRoot || !szText )
		return false;
	hm_keel_obj* pKeelObj = NULL;
	if ( !hm_get_keelobj( pHMUIRoot, uKeelObjId, &pKeelObj ) || !pKeelObj )
		return false;
	hm_wmp_display* pWMPDisplay = dynamic_cast< hm_wmp_display* >( pKeelObj->getWMP( uWMPId ) );
	if ( !pWMPDisplay )
		return false;
	hm_rs_uires_text* pText = _find_wmp_text_first( pWMPDisplay );
	if ( !pText )
		return false;
	pText->setText( szText );   // broadcast invalidates the pixel cache
	return true;
}

HMLIB_API bool hm_wmp_set_text_at( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, unsigned int uWMPId,
                                   int nDistrictId, unsigned int uShowObjIndex, const TCHAR* szText ) {
	if ( !pHMUIRoot || !szText )
		return false;
	hm_keel_obj* pKeelObj = NULL;
	if ( !hm_get_keelobj( pHMUIRoot, uKeelObjId, &pKeelObj ) || !pKeelObj )
		return false;
	hm_wmp_display* pWMPDisplay = dynamic_cast< hm_wmp_display* >( pKeelObj->getWMP( uWMPId ) );
	if ( !pWMPDisplay )
		return false;
	hm_districtobj* pDistrict = NULL;
	if ( !pWMPDisplay->getDistrictObj( nDistrictId, &pDistrict ) || !pDistrict )
		return false;
	if ( uShowObjIndex >= pDistrict->getShowObjCount() )
		return false;
	hm_rs_uires_text* pText =
	    dynamic_cast< hm_rs_uires_text* >( pDistrict->getShowObj( uShowObjIndex )->m_pUIRes );
	if ( !pText )
		return false;
	pText->setText( szText );
	return true;
}

// read back the first text uires ( see hmskin.h for the two-call contract )
HMLIB_API unsigned int hm_wmp_get_text( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, unsigned int uWMPId,
                                        LPTSTR lpszBuf, int nMaxTextCount ) {
	hm_keel_obj* pKeelObj = NULL;
	if ( !pHMUIRoot || !hm_get_keelobj( pHMUIRoot, uKeelObjId, &pKeelObj ) || !pKeelObj )
		return 0;
	hm_wmp_display* pWMPDisplay = dynamic_cast< hm_wmp_display* >( pKeelObj->getWMP( uWMPId ) );
	if ( !pWMPDisplay )
		return 0;
	hm_rs_uires_text* pText = _find_wmp_text_first( pWMPDisplay );
	if ( !pText )
		return 0;
	const hm_uires_text_data* pTextData = pText->getData();
	if ( !pTextData )
		return 0;
	unsigned int uLenText = pTextData->m_tstrData.length();
	if ( !lpszBuf )
		return uLenText + 1;   // chars the caller needs to allocate
	if ( nMaxTextCount == 0 )
		return 0;
	unsigned int uLenCopy = ( unsigned int )min( ( unsigned int )( nMaxTextCount - 1 ), uLenText );
	_tcsncpy_s( lpszBuf, nMaxTextCount, pTextData->m_tstrData.c_str(), uLenCopy );
	return uLenCopy;
}
