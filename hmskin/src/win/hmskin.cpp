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
static bool ReadResFromFile( const TCHAR* szResFilePath );
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

	//// read resource data from file
	//if ( !ReadResFromFile( szResFilePath ) )
	//	return false;
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

bool ReadResFromFile( const TCHAR* szResFilePath )
{
	return true;

//	if ( !szResFilePath )
//		return false;
//	if ( _taccess( szResFilePath, 0 ) != 0 )
//		return false;
//	if ( MSGSINKFACTORY_INSTANCE == NULL )
//		return false;
//
//	//it a dummy now. need more code here to read szResFilePath file. :D
//	TCHAR szFileDir[ MAX_PATH ];
//	msg_sink_factory_type::obj_id_type idMsgSink;
//	hm_rs_wndmark wndMark;
//	msg_sink_i* pMsgSink = NULL;
//	hm_keel_obj* pKeelObj = NULL;
//	hm_keel_obj_factory::obj_id_type objId;
//	hm_layout_obj* pLayoutObj = NULL;
//	hm_layoutdecoder_i* pLayoutDecoder = NULL;
//	coordinate_system_hwnd* pCoordSys = NULL;
//	hm_coordsys_factory_type::obj_id_type idCoordSys;
//	hm_layoutdecoder_clientarea* p_layout_decoder_client_area = NULL;
//	hm_layoutdecoder_wndarea* p_layout_decoder_wnd_area = NULL;
//	hm_ps_paint_layer_pic* pLayerPic = NULL;
//	hm_layout_obj* pLayoutMainWndClient = NULL;
//	hm_layout_obj* pLayoutMainWndWindow = NULL;
//	hm_wndmsgprocessor_i* pWMPTmp = NULL;
//	hm_wmp_id_type wmpId;
//	hm_districtobj* pDistrictObj = NULL;
//	hm_showobj* pShowObj = NULL;
//	hm_rs_uires_i* pUIRes = NULL;
//	hm_rs_rawres_pic* pRawResPicBtnNor = NULL;
//	hm_rs_rawres_pic* pRawResPicBtnHov = NULL;
//	hm_rs_rawres_pic* pRawResPicBtnPre = NULL;
//	hm_layoutdecoder_dock* ptr_layout_decoder_dock = NULL;
//	hm_rs_font_param fontParam;
//	hm_rawres_font_factory::obj_id_type idResFontTitle;
//	hm_rawres_color_factory::obj_id_type idResColorTitle;
//	hm_rawres_font_factory::obj_id_type idResFont;
//	hm_rawres_color_factory::obj_id_type idResColor;
//	hm_rawres_pic_factory::obj_id_type RAWID_RESBMP_BTN_Nor;
//	hm_rawres_pic_factory::obj_id_type RAWID_RESBMP_BTN_Hov;
//	hm_rawres_pic_factory::obj_id_type RAWID_RESBMP_BTN_Pre;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpCaption;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpRight;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpLeft;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpBottom;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpRootMenuBK;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpRootMINor;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpRootMIHov;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpRootMIPre;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpRootMIDis;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpPopMenuBK;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpScrollbarNor;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpScrollbarHov;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpScrollbarPre;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpScrollbarNorHor;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpScrollbarHovHor;
//	hm_rawres_pic_factory::obj_id_type idRawResBmpScrollbarPreHor;
//	hm_rs_uires_pic* pUIResPic = NULL;
//	std::basic_string< TCHAR > strImagePath;
//	
//	//bind coordinate system id to windows property id
//	idCoordSys = 1;
//	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
//	wndMark.m_uWndPropID = 258;
//	WNDMARK2CSID_TABLE->bindObjId2WndMark( idCoordSys, wndMark );
//	//
//	idCoordSys = 2;
//	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_CTRLID;
//	wndMark.m_uCtrlID = 20;
//	WNDMARK2CSID_TABLE->bindObjId2WndMark( idCoordSys, wndMark );
//	//
//	idCoordSys = 3;
//	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
//	wndMark.m_uWndPropID = 258;
//	WNDMARK2CSID_TABLE->bindObjId2WndMark( idCoordSys, wndMark );
//	//
//	idCoordSys = 4; //popup menu.
//	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
//	wndMark.m_uWndPropID = 21;
//	WNDMARK2CSID_TABLE->bindObjId2WndMark( idCoordSys, wndMark );
//
//	//
//	::GetCurrentDirectory( MAX_PATH, szFileDir );
//	// raw bmp
//	RAWID_RESBMP_BTN_Nor = 1;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\btn_nor.bmp");
//	pRawResPicBtnNor = RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( RAWID_RESBMP_BTN_Nor, strImagePath.c_str() );
//	RAWID_RESBMP_BTN_Hov = 2;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\btn_hov.bmp");
//	pRawResPicBtnHov = RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( RAWID_RESBMP_BTN_Hov, strImagePath.c_str() );
//	RAWID_RESBMP_BTN_Pre = 3;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\btn_pre.bmp");
//	pRawResPicBtnPre = RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( RAWID_RESBMP_BTN_Pre, strImagePath.c_str() );
//	idRawResBmpCaption = 4;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\nc_bk_caption.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpCaption, strImagePath.c_str() );
//	idRawResBmpRight = 5;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\nc_bk_right.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpRight, strImagePath.c_str());
//	idRawResBmpLeft = 6;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\nc_bk_left.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpLeft, strImagePath.c_str() );
//	idRawResBmpBottom = 7;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\nc_bk_bottom.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpBottom, strImagePath.c_str() );
//	idRawResBmpRootMenuBK = 8;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\root_menu_bk.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpRootMenuBK, strImagePath.c_str() );
//	idRawResBmpRootMINor = 9;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\root_mi_nor.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpRootMINor, strImagePath.c_str() );
//	idRawResBmpRootMIHov = 10;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\root_mi_hov.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpRootMIHov, strImagePath.c_str() );
//	idRawResBmpRootMIPre = 11;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\root_mi_pre.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpRootMIPre, strImagePath.c_str() );
//	idRawResBmpRootMIDis = 12;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\root_mi_nor.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpRootMIDis, strImagePath.c_str() );
//	idRawResBmpPopMenuBK = 13;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\pop_menu_bk.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpPopMenuBK, strImagePath.c_str() );	
//	idRawResBmpScrollbarNor = 20;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\scrollbar_nor.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpScrollbarNor, strImagePath.c_str() );	
//	idRawResBmpScrollbarHov = 21;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\scrollbar_hov.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpScrollbarHov, strImagePath.c_str() );	
//	idRawResBmpScrollbarPre = 22;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\scrollbar_pre.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpScrollbarPre, strImagePath.c_str() );	
//	idRawResBmpScrollbarNorHor = 23;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\scrollbar_nor_hor.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpScrollbarNorHor, strImagePath.c_str() );	
//	idRawResBmpScrollbarHovHor = 24;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\scrollbar_hov_hor.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpScrollbarHovHor, strImagePath.c_str() );	
//	idRawResBmpScrollbarPreHor = 25;
//	strImagePath = szFileDir;
//	strImagePath +=	_T("\\image\\scrollbar_pre_hor.bmp");
//	RAWPICFACTORY_INSTANCE->createObj< hm_rs_rawres_pic >( idRawResBmpScrollbarPreHor, strImagePath.c_str() );	
//
//
//	// popup menu.
//	idMsgSink = 18;
//	pMsgSink = MSGSINKFACTORY_INSTANCE->createObj< hm_msg_sink >( idMsgSink );
//	wndMark.m_eflag = hm_rs_wndmark::WNDMARK_WNDPROPID;
//	wndMark.m_uCtrlID = 21;
//	WNDMARK2MSID_TABLE->bindObjId2WndMark( idMsgSink, wndMark );
//	//
//	objId = 7;
//	pKeelObj = KEELOBJ_FACTORY->createObj< hm_keel_obj >( objId );
//	pKeelObj->switchWndMsgSource( pMsgSink );
//	//
//	idCoordSys = 4;
//	pCoordSys = COORDSYSOBJFACTORY_INSTANCE->createObj< coordinate_system_hwnd, coordinate_system_hwnd::ENUMCSHTYPE >( idCoordSys, coordinate_system_hwnd::CSH_CLIENT_TYPE );
//	//
//	pLayoutObj = new hm_layout_obj();
//	pLayoutObj->setCoordinateSystem( pCoordSys );
//	p_layout_decoder_client_area = new hm_layoutdecoder_clientarea( pLayoutObj );
//	pLayoutObj->setLayoutDecoder( p_layout_decoder_client_area );
//	pKeelObj->setLayoutObj( pLayoutObj );
//	//
//	wmpId = 71;
//	pWMPTmp = WMPFACTORY_INSTANCE->createObj< hm_wmp_layouttrigger >( wmpId );
//	pKeelObj->pushbackWMP( wmpId );
//	//
//	wmpId = 72;
//	pWMPTmp = WMPFACTORY_INSTANCE->createObj< hm_wmp_drawtrigger >( wmpId );
//	pKeelObj->pushbackWMP( wmpId );
//	//
//	wmpId = 73; 
//	dimension_menuwnd_i* pCalcMenuWndRectObj = new dimension_menuwnd_default( 25, 150, 5 );
//	hm_wmp_popupmenu* pWMPPopupMenu = WMPFACTORY_INSTANCE->createObj< hm_wmp_popupmenu, dimension_menuwnd_i* >( wmpId, pCalcMenuWndRectObj );
//	pKeelObj->Register( pWMPPopupMenu, HMKEELOBJEVENT_DRAWSELF );
//	pKeelObj->pushbackWMP( wmpId );
//	//
//	pWMPPopupMenu->setDrawTool( hm_ps_drawtool_gdiplus::getInstance() );
//	// root menu background
//	pUIResPic = new hm_rs_uires_pic( idRawResBmpPopMenuBK );
//	pUIResPic->m_ufixL = 35;
//	pUIResPic->m_ufixT = 5;
//	pUIResPic->m_ufixR = 5;
//	pUIResPic->m_ufixB = 5;
//	pShowObj = new hm_showobj( pUIResPic );
//	pWMPPopupMenu->getDistrictObj( hm_wmp_popupmenu::EDISTRICTID_BKGND, &pDistrictObj );
//	pDistrictObj->pushBackShowObj( pShowObj );
//
//    // hm_keelctrl_scrollview
//	objId = 20; 
//	hm_keelctrl_scrollview::INITIDSET initSet;
//	initSet.idScrollBarV = 110;
//	initSet.idScrollBarH = 111;
//	initSet.idViewContent = 112;
//	initSet.wmpIdScrollBarV = 91;
//	initSet.wmpIdScrollBarH = 92;
//	initSet.wmpIdViewContent = 93;
//	hm_keelctrl_scrollview* pKeelObjScrollView = KEELOBJ_FACTORY->createObj< hm_keelctrl_scrollview, hm_keelctrl_scrollview::INITIDSET >( objId, initSet );
//	pKeelObjScrollView->switchWndMsgSource( pKeelObjBKGND );
//	pKeelObjScrollView->setParent( pKeelObjBKGND );
//
//	//
//	idCoordSys = 1;
//	pCoordSys = COORDSYSOBJFACTORY_INSTANCE->getObj( idCoordSys );
//	//
//	pLayoutObj = new hm_layout_obj();
//	pLayoutObj->setCoordinateSystem( pCoordSys );
//	hm_layoutdecoder_dock* ptr_layout_decoder_dock_scrollview = new hm_layoutdecoder_dock( pLayoutObj );
//	ptr_layout_decoder_dock_scrollview->m_space1 = 5;
//	ptr_layout_decoder_dock_scrollview->m_space2 = 5;
//	ptr_layout_decoder_dock_scrollview->m_space3 = 5;
//	ptr_layout_decoder_dock_scrollview->m_thickness = 250;
//	ptr_layout_decoder_dock_scrollview->m_eDockType = hm_layoutdecoder_dock::DOCK_BOTTOM; 
//	ptr_layout_decoder_dock_scrollview->setLayoutRefObj( pLayoutMainWndClient );
//	pLayoutObj->setLayoutDecoder( ptr_layout_decoder_dock_scrollview );
//	pKeelObjScrollView->setLayoutObj( pLayoutObj );
//	hm_layout_obj* pLayoutObjScrollView = pLayoutObj;
//
//	// fill scroll bar vertical
//	hm_wmp_scrollbar* pWMPScrollBar = pKeelObjScrollView->getWMPScrollBarV();
//	//
//	//pDrawTool = new hm_ps_drawtool_gdiplus();
//	pWMPScrollBar->setDrawTool( hm_ps_drawtool_gdi::getInstance() ); 
//	// scrollbar background
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_BKGND, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNor );
//		pUIResPic->m_rcSelect.top = 40;
//		pUIResPic->m_rcSelect.bottom = 120;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	// scrollbar start thumb
//	pWMPScrollBar->getStartThumbPosDecoder()->m_space1 = 0;
//	pWMPScrollBar->getStartThumbPosDecoder()->m_space2 = 0;
//	pWMPScrollBar->getStartThumbPosDecoder()->m_space3 = 0;
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_NOR, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_HOV, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarHov );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_PRE, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarPre );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	// scrollbar end thumb
//	pWMPScrollBar->getEndThumbPosDecoder()->m_space1 = 0;
//	pWMPScrollBar->getEndThumbPosDecoder()->m_space2 = 0;
//	pWMPScrollBar->getEndThumbPosDecoder()->m_space3 = 0;
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_NOR, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNor );
//		pUIResPic->m_rcSelect.top = 121;
//		pUIResPic->m_rcSelect.bottom = 139;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_HOV, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarHov );
//		pUIResPic->m_rcSelect.top = 121;
//		pUIResPic->m_rcSelect.bottom = 139;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_PRE, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarPre );
//		pUIResPic->m_rcSelect.top = 121;
//		pUIResPic->m_rcSelect.bottom = 139;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	// scrollbar track thumb/.
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_space1 = 0;
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_space2 = 0;
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_distance = 0;//185;
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_nThickness = 20; // 190; //200;
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_NOR, &pDistrictObj ) ) {
//		// normal
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNor );
//		pUIResPic->m_rcSelect.top = 19;
//		pUIResPic->m_rcSelect.bottom = 34;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 2;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 2;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_HOV, &pDistrictObj ) ) {
//		// hover
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarHov );
//		pUIResPic->m_rcSelect.top = 19;
//		pUIResPic->m_rcSelect.bottom = 34;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 2;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 2;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_PRE, &pDistrictObj ) ) {
//		// press
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarPre );
//		pUIResPic->m_rcSelect.top = 19;
//		pUIResPic->m_rcSelect.bottom = 34;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 2;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 2;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//
//	// fill scroll bar horizontal
//	pWMPScrollBar = pKeelObjScrollView->getWMPScrollBarH();
//	//
//	//pDrawTool = new hm_ps_drawtool_gdiplus();
//	pWMPScrollBar->setDrawTool( hm_ps_drawtool_gdi::getInstance() );
//	// scrollbar background
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_BKGND, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNorHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 40;
//		pUIResPic->m_rcSelect.right = 120;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	// scrollbar start thumb
//	pWMPScrollBar->getStartThumbPosDecoder()->m_space1 = 0;
//	pWMPScrollBar->getStartThumbPosDecoder()->m_space2 = 0;
//	pWMPScrollBar->getStartThumbPosDecoder()->m_space3 = 0;
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_NOR, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNorHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_HOV, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarHovHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_PRE, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarPreHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 0;
//		pUIResPic->m_rcSelect.right = 18;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	// scrollbar end thumb
//	pWMPScrollBar->getEndThumbPosDecoder()->m_space1 = 0;
//	pWMPScrollBar->getEndThumbPosDecoder()->m_space2 = 0;
//	pWMPScrollBar->getEndThumbPosDecoder()->m_space3 = 0;
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_NOR, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNorHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 121;
//		pUIResPic->m_rcSelect.right = 139;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_HOV, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarHovHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 121;
//		pUIResPic->m_rcSelect.right = 139;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_PRE, &pDistrictObj ) ) {
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarPreHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 121;
//		pUIResPic->m_rcSelect.right = 139;
//		pUIResPic->m_ufixL = 0;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 0;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	// scrollbar track thumb/.
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_space1 = 0;
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_space2 = 0;
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_distance = 0;//185;
//	pWMPScrollBar->getTrackThumbPosDecoder()->m_nThickness = 20;
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_NOR, &pDistrictObj ) ) {
//		// normal
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarNorHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 19;
//		pUIResPic->m_rcSelect.right = 34;
//		pUIResPic->m_ufixL = 5;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 5;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_HOV, &pDistrictObj ) ) {
//		// hover
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarHovHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 19;
//		pUIResPic->m_rcSelect.right = 34;
//		pUIResPic->m_ufixL = 5;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 5;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//	if ( pWMPScrollBar->getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_PRE, &pDistrictObj ) ) {
//		// press
//		pUIResPic = new hm_rs_uires_pic( idRawResBmpScrollbarPreHor );
//		pUIResPic->m_rcSelect.top = 0;
//		pUIResPic->m_rcSelect.bottom = 18;
//		pUIResPic->m_rcSelect.left = 19;
//		pUIResPic->m_rcSelect.right = 34;
//		pUIResPic->m_ufixL = 5;
//		pUIResPic->m_ufixT = 0;
//		pUIResPic->m_ufixR = 5;
//		pUIResPic->m_ufixB = 0;
//		pShowObj = new hm_showobj( pUIResPic );
//		pDistrictObj->pushBackShowObj( pShowObj );
//	}
//
//	// add hm_wmp_dialogview.
//	pKeelObj = pKeelObjScrollView->getKeelObjViewContent();
//	wmpId = 121;
//	hm_wmp_dialogview* pWMPDialogView = WMPFACTORY_INSTANCE->createObj< hm_wmp_dialogview, hm_keelctrl_scrollview* >( wmpId, pKeelObjScrollView );
//	pKeelObj->Register( pWMPDialogView, HMKEELOBJEVENT_DRAWSELF );
//	pKeelObjScrollView->Register( pWMPDialogView, HM_EVENT_MOVE_VIEWCONTENT );
//	pKeelObj->pushbackWMP( wmpId );	
//	pWMPDialogView->setDrawTool( hm_ps_drawtool_gdi::getInstance() );
//	// add font.
//	fontParam.m_strFaceName = _T("Tahoma");
//	fontParam.m_uFlagStyle = FONTSTYLE_REGULAR;
//	fontParam.m_uFlagSizeUnit = FONTUNIT_POINT;
//	fontParam.m_uSize = 12;
//	idResFont = RAWFONTFACTORY_INSTANCE->createObj< hm_rs_rawres_font, hm_rs_font_param >( fontParam );
//	fontParam.m_strFaceName = _T("Tahoma");
//	fontParam.m_uFlagStyle = FONTSTYLE_BOLD;
//	fontParam.m_uFlagSizeUnit = FONTUNIT_POINT;
//	fontParam.m_uSize = 16;		
//	idResFontTitle = RAWFONTFACTORY_INSTANCE->createObj< hm_rs_rawres_font, hm_rs_font_param >( fontParam );
//	// add color
//	idResColor = RAWCOLORFACTORY_INSTANCE->createObj< hm_rs_rawres_color, COLORREF >( RGB( 0, 0, 0 ) );
//	idResColorTitle = RAWCOLORFACTORY_INSTANCE->createObj< hm_rs_rawres_color, COLORREF >( RGB( 12, 94, 156 ) );
//	// add test data.
//	for ( int i = 0; i<3; ++i ) {
//		hm_dlgmsg_item_param* pItemParam = NULL;
//		pItemParam = new hm_dlgmsg_item_param();
//		pItemParam->m_strMsgFrom = _T("rom.wang:");
//		pItemParam->m_idFontMsgFrom = idResFontTitle;
//		pItemParam->m_idColorMsgFrom = idResColorTitle;
//		pItemParam->m_strMsgContent = _T("hello, world!");
//		pItemParam->m_idFontMsgContent = idResFont;
//		pItemParam->m_idColorMsgContent = idResColor;
//		pWMPDialogView->pushbackItem( pItemParam );
//		//
//		pItemParam = new hm_dlgmsg_item_param();
//		pItemParam->m_strMsgFrom = _T("quilt.qu:");
//		pItemParam->m_idFontMsgFrom = idResFontTitle;
//		pItemParam->m_idColorMsgFrom = idResColorTitle;
//		pItemParam->m_strMsgContent = _T("һ��������һŮ�����á�һ����ϰ���ϣ��������ڹ�������д�����������Ǹ�Ů��������д�ţ�����ʵ��ע��\
//										 ��ܾ��ˡ�����һ����������ִ�����������д�ţ���������������ʦ���ұ�֤�Ժ���Ҳ���ϿοĹ����ˣ���\
//										 һ��������һŮ�����á�һ����ϰ���ϣ��������ڹ�������д�����������Ǹ�Ů��������д�ţ�����ʵ��ע��\
//										 ��ܾ��ˡ�����һ����������ִ�����������д�ţ���������������ʦ���ұ�֤�Ժ���Ҳ���ϿοĹ����ˣ���\
//										 һ��������һŮ�����á�һ����ϰ���ϣ��������ڹ�������д�����������Ǹ�Ů��������д�ţ�����ʵ��ע��\
//										 ��ܾ��ˡ�����һ����������ִ�����������д�ţ���������������ʦ���ұ�֤�Ժ���Ҳ���ϿοĹ����ˣ���\
//										 һ��������һŮ�����á�һ����ϰ���ϣ��������ڹ�������д�����������Ǹ�Ů��������д�ţ�����ʵ��ע��\
//										 ��ܾ��ˡ�����һ����������ִ�����������д�ţ���������������ʦ���ұ�֤�Ժ���Ҳ���ϿοĹ����ˣ���");
//		pItemParam->m_idFontMsgContent = idResFont;
//		pItemParam->m_idColorMsgContent = idResColor;
//		pWMPDialogView->pushbackItem( pItemParam );
//	}
//
//	return true;
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
