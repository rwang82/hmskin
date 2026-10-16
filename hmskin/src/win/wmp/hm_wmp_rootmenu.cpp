#include "stdafx.h"
#include "win\wmp\hm_wmp_rootmenu.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_layout_obj.h"
#include "win\paint_system\hm_ps_canvas.h"
#include "win\hm_event_defs.h"
#include "hmskin_event_defs.h"

// shared menu-label font ( YaHei UI 12px, GB2312 charset ): the menu WMPs
// draw labels with raw GDI DrawText and the DC default font ( bitmap System
// font ) has no CJK glyphs - localized labels need an explicit font.
static HFONT _GetMenuLabelFont() {
	static HFONT hFontMenuLabel = NULL;
	if ( !hFontMenuLabel ) {
		LOGFONT logFont;
		memset( &logFont, 0, sizeof( LOGFONT ) );
		logFont.lfHeight = -12;
		logFont.lfWeight = FW_NORMAL;
		logFont.lfCharSet = GB2312_CHARSET;
		_tcscpy_s( logFont.lfFaceName, _T( "Microsoft YaHei UI" ) );
		hFontMenuLabel = ::CreateFontIndirect( &logFont );
	}
	return hFontMenuLabel;
}

hm_wmp_rootmenu::hm_wmp_rootmenu( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_uRootMenuFlags( 0 )
, m_uSpaceH( 0 )
, m_uSpaceV( 0 )
, m_uLenMenuItem( 60 )
, m_nCurMIIndex( -1 )
, m_pMemCanvas( new hm_ps_canvas() )
{
	_createDistrictObjs();
}
//
hm_wmp_rootmenu::~hm_wmp_rootmenu(){
	if ( m_pMemCanvas ) {
		delete m_pMemCanvas;
		m_pMemCanvas = NULL;
	}

}
//
MSGBIND_BEGIN( hm_wmp_rootmenu )
	MSGBIND( WM_NCLBUTTONDOWN, _OnNCLButtonDown )
	MSGBIND( WM_NCLBUTTONDBLCLK, _OnNCLButtonDblClk )
	MSGBIND( WM_NCLBUTTONUP, _OnNCLButtonUp )
	MSGBIND( WM_NCMOUSEMOVE, _OnNCMouseMove )
	MSGBIND( WM_NCMOUSELEAVE, _OnNCMouseLeave )
	MSGBIND( WM_NCHITTEST, _OnNCHitTest )
	MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_rootmenu )
	ORDERWNDMSG( WM_NCLBUTTONDOWN )
	ORDERWNDMSG( WM_NCLBUTTONDBLCLK )
	ORDERWNDMSG( WM_NCLBUTTONUP )
	ORDERWNDMSG( WM_NCMOUSEMOVE )
	ORDERWNDMSG( WM_NCMOUSELEAVE )
	ORDERWNDMSG( WM_NCHITTEST )
	ORDERWNDMSG( WM_MOUSEMOVE )
ORDERWNDMSG_END()
//
void hm_wmp_rootmenu::setConfigure( const RootMenuCfg& rmCfg ) {
	m_uSpaceH = rmCfg.m_uSpaceH;
	m_uSpaceV = rmCfg.m_uSpaceV;
	m_uLenMenuItem = rmCfg.m_uLenMenuItem;
}
//
void hm_wmp_rootmenu::_createDistrictObjs() {
	_setDistrictObj( EDISTRICTID_BKGND, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_NOR, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_HOV, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_PRE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_DIS, new hm_districtobj() );
	
}
//
void hm_wmp_rootmenu::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent ) {
		case HMKEELOBJEVENT_DRAWSELF:
			{
				HEDSPARAM* pHEDSParam = NULL;
				RECT rcCanvas;
				POINT ptScreen;
				POINT ptInCanvas;
				HMENU hRootMenu;
				RECT rcWnd;

				pHEDSParam = (HEDSPARAM*)pEventParam;
				if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
					return;
				//
				if ( !::GetCursorPos( &ptScreen ) )
					return;
				//
				hRootMenu = ::GetMenu( pHEDSParam->m_hWndCanvas );
				if ( !hRootMenu )
					return;
				if ( !::GetWindowRect( pHEDSParam->m_hWndCanvas, &rcWnd ) )
					return;
				ptInCanvas.x = ptScreen.x - rcWnd.left;
				ptInCanvas.y = ptScreen.y - rcWnd.top;
				_doDrawSkinRaw( pHEDSParam->m_hDCCanvas, rcCanvas, ptInCanvas, hRootMenu );
				
			}
			break;
		default:
			break;
	}	
}
//
void hm_wmp_rootmenu::_OnNCLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptWnd;
	POINT ptScreen;
	RECT rcCanvas;
	RECT rcWnd;
	hm_layout_obj* pLayOutObj = NULL;
	HDC hWndDC;
	
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	//
	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = LOWORD( pWP->lParam ) - rcWnd.left;
	ptWnd.y = HIWORD( pWP->lParam ) - rcWnd.top;
	//
	if ( ::PtInRect( &rcCanvas, ptWnd ) ) {
		//
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;//HTNOWHERE;
		
		//
		ptScreen.x = LOWORD( pWP->lParam );
		ptScreen.y = HIWORD( pWP->lParam );
		hWndDC = ::GetWindowDC( pWP->hwnd );

		_doDrawSkin( hWndDC, rcCanvas, pWP->hwnd, ptScreen );

		::ReleaseDC( pWP->hwnd, hWndDC );	
		//
		m_uRootMenuFlags |= RMFLAG_BTNDOWN;
	}

}
//
void hm_wmp_rootmenu::_OnNCLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptWnd;
	POINT ptScreen;
	RECT rcCanvas;
	RECT rcWnd;
	hm_layout_obj* pLayOutObj = NULL;
	HDC hWndDC;
	
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	//
	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = LOWORD( pWP->lParam ) - rcWnd.left;
	ptWnd.y = HIWORD( pWP->lParam ) - rcWnd.top;
	//
	if ( ::PtInRect( &rcCanvas, ptWnd ) ) {
		//
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;//HTNOWHERE;
		
		//
		ptScreen.x = LOWORD( pWP->lParam );
		ptScreen.y = HIWORD( pWP->lParam );
		hWndDC = ::GetWindowDC( pWP->hwnd );
		_doDrawSkin( hWndDC, rcCanvas, pWP->hwnd, ptScreen );
		::ReleaseDC( pWP->hwnd, hWndDC );	
		//
		m_uRootMenuFlags |= RMFLAG_BTNDOWN;
	}
}
//
void hm_wmp_rootmenu::_OnNCLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptWnd;
	POINT ptScreen;
	RECT rcCanvas;
	RECT rcWnd;
	hm_layout_obj* pLayOutObj = NULL;
	HDC hWndDC;
	
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	//
	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = LOWORD( pWP->lParam ) - rcWnd.left;
	ptWnd.y = HIWORD( pWP->lParam ) - rcWnd.top;
	//
	if ( ::PtInRect( &rcCanvas, ptWnd ) ) {
		//
		BOOL bBtnDown = ( m_uRootMenuFlags & RMFLAG_BTNDOWN );
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;//HTNOWHERE;

		//
		ptScreen.x = LOWORD( pWP->lParam );
		ptScreen.y = HIWORD( pWP->lParam );
		hWndDC = ::GetWindowDC( pWP->hwnd );
		_doDrawSkin( hWndDC, rcCanvas, pWP->hwnd, ptScreen );
		::ReleaseDC( pWP->hwnd, hWndDC );
		// press+release on a menu-bar item: notify the app ( it opens the
		// skinned popup menu for this item ).
		if ( bBtnDown && m_pEventListener ) {
			int nMIIndex = -1;
			if ( _getCurMenuItemIndex( rcCanvas, pWP->hwnd, ptScreen, nMIIndex ) && nMIIndex >= 0 )
				m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_ROOTMENU_CLICK, ( void* )pKeelObj, ( void* )( UINT_PTR )nMIIndex );
		}
	} else if ( m_uRootMenuFlags & RMFLAG_BTNDOWN ) {
		//
		ptScreen.x = LOWORD( pWP->lParam );
		ptScreen.y = HIWORD( pWP->lParam );
		hWndDC = ::GetWindowDC( pWP->hwnd );
		_doDrawSkin( hWndDC, rcCanvas, pWP->hwnd, ptScreen );
		::ReleaseDC( pWP->hwnd, hWndDC );
	}
	// remove RMFLAG_BTNDOWN flag.
	m_uRootMenuFlags &= ~RMFLAG_BTNDOWN;
}
//
void hm_wmp_rootmenu::_OnNCMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptScreen;
	POINT ptWnd;
	RECT rcWnd;
	RECT rcCanvas;
	hm_layout_obj* pLayOutObj = NULL;
	int miindex = -1;
	
	ptScreen.x = ((POINTS*)(&pWP->lParam))->x;
	ptScreen.y = ((POINTS*)(&pWP->lParam))->y;
	//
	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = ((POINTS*)(&pWP->lParam))->x - rcWnd.left;
	ptWnd.y = ((POINTS*)(&pWP->lParam))->y - rcWnd.top;
	//
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	//eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;// | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	if ( ::PtInRect( &rcCanvas, ptWnd ) ) {
		//
		if ( !_getCurMenuItemIndex( rcCanvas, pWP->hwnd, ptScreen, miindex ) ) 
			return; // get miindex failed.
		if ( miindex == m_nCurMIIndex )
			return; // cursor don't move to different menu item.
		m_nCurMIIndex = miindex;
		//
		HDC hDC = ::GetWindowDC( pWP->hwnd );
		_doDrawSkin( hDC, rcCanvas, pWP->hwnd, ptScreen );
		::ReleaseDC( pWP->hwnd, hDC );

	} else {
		if ( m_nCurMIIndex == -1 )
			return;
		//
		if ( !_getCurMenuItemIndex( rcCanvas, pWP->hwnd, ptScreen, miindex ) ) 
			return; // get miindex failed.
		if ( miindex == m_nCurMIIndex )
			return; // cursor don't move to different menu item.
		m_nCurMIIndex = miindex;
		//
		HDC hDC = ::GetWindowDC( pWP->hwnd );
		_doDrawSkin( hDC, rcCanvas, pWP->hwnd, ptScreen );
		::ReleaseDC( pWP->hwnd, hDC );
	}


}
//
void hm_wmp_rootmenu::_OnNCMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	
}
//
void hm_wmp_rootmenu::_OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptWnd;
	RECT rcWnd;
	RECT rcCanvas;
	hm_layout_obj* pLayOutObj = NULL;
	
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	//
	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = LOWORD( pWP->lParam ) - rcWnd.left;
	ptWnd.y = HIWORD( pWP->lParam ) - rcWnd.top;
	//
	if ( ::PtInRect( &rcCanvas, ptWnd ) ) {
		//
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = HTCAPTION;//HTNOWHERE;	
	}
}
//
void hm_wmp_rootmenu::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( m_nCurMIIndex != -1 ) {
		if ( !pKeelObj || !pWP || !pParamWndProc )
			return;
		POINT ptScreen;
		RECT rcCanvas;
		hm_layout_obj* pLayOutObj = NULL;
		
		::GetCursorPos( &ptScreen );
		pLayOutObj = pKeelObj->getLayoutObj();
		if ( !pLayOutObj ) return;
		if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
			return;
		//
		HDC hDC = ::GetWindowDC( pWP->hwnd );
		_doDrawSkin( hDC, rcCanvas, pWP->hwnd, ptScreen );
		::ReleaseDC( pWP->hwnd, hDC );
		::OutputDebugString( _T("draw back to normal.\n") );
		//
		m_nCurMIIndex = -1;
	}
}

void hm_wmp_rootmenu::_doDrawSkin( HDC hDC, const RECT& rcCanvas, HWND hWnd, const POINT& ptScreen ) {
	SIZE sizeMemBitmap;
	RECT rcMemCanvas;
	RECT rcWnd;
	POINT ptInMemCanvas;
	POINT ptInScreen;
	HMENU hRootMenu;

	sizeMemBitmap.cx = rcCanvas.right - rcCanvas.left;
	sizeMemBitmap.cy = rcCanvas.bottom - rcCanvas.top;
	m_pMemCanvas->updateMemBitmapSize( sizeMemBitmap );
	//
	rcMemCanvas.left = 0;
	rcMemCanvas.top = 0;
	rcMemCanvas.right = rcCanvas.right - rcCanvas.left;
	rcMemCanvas.bottom = rcCanvas.bottom - rcCanvas.top;
	//
	if ( !::GetWindowRect( hWnd, &rcWnd ) )
		return;
	if ( !::GetCursorPos( &ptInScreen ) )
		return;
	ptInMemCanvas.x = ptInScreen.x - rcWnd.left;
	ptInMemCanvas.y = ptInScreen.y - rcWnd.top;
	ptInMemCanvas.x -= rcCanvas.left;
	ptInMemCanvas.y -= rcCanvas.top;
	//
	hRootMenu = ::GetMenu( hWnd );
	if ( !hRootMenu )
		return;
	
	_doDrawSkinRaw( m_pMemCanvas->GetMemDC(), rcMemCanvas, ptInMemCanvas, hRootMenu );

	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, ( rcCanvas.right - rcCanvas.left ), ( rcCanvas.bottom - rcCanvas.top),
		m_pMemCanvas->GetMemDC(), 0, 0, SRCCOPY);
}

void hm_wmp_rootmenu::_doDrawSkinRaw( HDC hDC, const RECT& rcCanvas, POINT ptInCanvas, HMENU hRootMenu ) {
	BOOL bLBtnDown = FALSE;
	RECT rcDMICanvas;
	int nItemCount;
	int nItemIndex;
	MENUITEMINFO mii;
	rmiteminfo rmiinfo;
	ENUMROOTMENUITEMSTATE eRMIState = ERMIS_NORMAL;
	
	_doDrawBackground( hDC, rcCanvas );
 
	//
	bLBtnDown = GetKeyState(VK_LBUTTON) & 0x8000;
	rcDMICanvas = rcCanvas;
	rcDMICanvas.top += m_uSpaceV;
	rcDMICanvas.bottom -= m_uSpaceV;
	// draw root menu items.
	nItemCount = GetMenuItemCount( hRootMenu );
	for ( nItemIndex = 0; nItemIndex!=nItemCount; ++nItemIndex ) {
		//
		rcDMICanvas.left = rcCanvas.left + m_uSpaceH + nItemIndex*m_uLenMenuItem;
		rcDMICanvas.right = rcDMICanvas.left + m_uLenMenuItem;
		// get string size.
		mii.cbSize = sizeof( MENUITEMINFO );
		mii.fMask = MIIM_STRING|MIIM_STATE;
		mii.dwTypeData = NULL; 
		mii.cch = 0;
		::GetMenuItemInfo( hRootMenu, nItemIndex, true, &mii );
		// get mii.dwTypeData
		mii.dwTypeData = new TCHAR[ mii.cch + 1 ];
		mii.cch = mii.cch + 1;
		::GetMenuItemInfo( hRootMenu, nItemIndex, true, &mii );
		rmiinfo.m_strText = mii.dwTypeData;
		delete [](mii.dwTypeData);
		mii.dwTypeData = NULL;
		//
		if ( mii.fState & MFS_DISABLED ) {
			eRMIState = hm_wmp_rootmenu::ERMIS_DISABLE;
		} else if ( ::PtInRect( &rcDMICanvas, ptInCanvas ) ) {
			if ( bLBtnDown )
				eRMIState =	hm_wmp_rootmenu::ERMIS_PRESS;
			else 
				eRMIState =	hm_wmp_rootmenu::ERMIS_HOVER;
		} else {
			eRMIState =	hm_wmp_rootmenu::ERMIS_NORMAL;
		}
		
		// draw menu item
		rmiinfo.m_eRMIState = eRMIState;
		_doDrawMenuItem( hDC, rcDMICanvas, nItemIndex, rmiinfo );
	}
}

////
//void hm_wmp_rootmenu::_doDrawSkin( HDC hDC, const RECT& rcCanvas, HWND hWnd, const POINT& ptScreen ) {
//	HMENU hRootMenu;
//	int nItemIndex, nItemCount;
//	MENUITEMINFO mii;
//	ENUMROOTMENUITEMSTATE eRMIState = ERMIS_NORMAL;
//	rmiteminfo rmiinfo;
//	RECT rcDMICanvas;
//	bool bLBtnDown = false;
//	POINT ptWnd;
//	RECT rcWnd;
//	HDC hMemDC;
//	HBITMAP hBitmap, hBitmapOld;
//	RECT rcMemCanvas;
//	RECT rcMemDMICanvas;
//
//	//
//	rcMemCanvas.left = 0;
//	rcMemCanvas.top = 0;
//	rcMemCanvas.right = (rcCanvas.right - rcCanvas.left);
//	rcMemCanvas.bottom = (rcCanvas.bottom - rcCanvas.top);
//	hMemDC = ::CreateCompatibleDC( hDC );
//	hBitmap = ::CreateCompatibleBitmap( hDC, (rcCanvas.right - rcCanvas.left), (rcCanvas.bottom - rcCanvas.top) );
//	hBitmapOld = (HBITMAP)::SelectObject( hMemDC, hBitmap );
//	
//	// draw background
//	_doDrawBackground( hMemDC, rcMemCanvas );
//	//
//	::GetWindowRect( hWnd, &rcWnd );
//	ptWnd.x = ptScreen.x - rcWnd.left;
//	ptWnd.y = ptScreen.y - rcWnd.top;
//	//
//	bLBtnDown = GetKeyState(VK_LBUTTON) & 0x8000;
//	//::OutputDebugString( bLBtnDown ? _T("bLBtnDown\n") : _T("no bLBtnDown\n") );
//	rcDMICanvas = rcCanvas;
//	rcDMICanvas.top += m_uSpaceV;
//	rcDMICanvas.bottom -= m_uSpaceV;
//	// draw root menu items.
//	hRootMenu = ::GetMenu( hWnd );
//	if ( !hRootMenu ) goto exit;
//	nItemCount = GetMenuItemCount( hRootMenu );
//	for ( nItemIndex = 0; nItemIndex!=nItemCount; ++nItemIndex ) {
//		//
//		rcDMICanvas.left = rcCanvas.left + m_uSpaceH + nItemIndex*m_uLenMenuItem;
//		rcDMICanvas.right = rcDMICanvas.left + m_uLenMenuItem;
//		// get string size.
//		mii.cbSize = sizeof( MENUITEMINFO );
//		mii.fMask = MIIM_STRING|MIIM_STATE;
//		mii.dwTypeData = NULL; 
//		mii.cch = 0;
//		::GetMenuItemInfo( hRootMenu, nItemIndex, true, &mii );
//		// get mii.dwTypeData
//		mii.dwTypeData = new TCHAR[ mii.cch + 1 ];
//		mii.cch = mii.cch + 1;
//		::GetMenuItemInfo( hRootMenu, nItemIndex, true, &mii );
//		rmiinfo.m_strText = mii.dwTypeData;
//		delete [](mii.dwTypeData);
//		mii.dwTypeData = NULL;
//		//
//		if ( mii.fState & MFS_DISABLED ) {
//			eRMIState = hm_wmp_rootmenu::ERMIS_DISABLE;
//		} else if ( ::PtInRect( &rcDMICanvas, ptWnd ) ) {
//			if ( bLBtnDown )
//				eRMIState =	hm_wmp_rootmenu::ERMIS_PRESS;
//			else 
//				eRMIState =	hm_wmp_rootmenu::ERMIS_HOVER;
//		} else {
//			eRMIState =	hm_wmp_rootmenu::ERMIS_NORMAL;
//		}
//		
//		// draw menu item
//		rmiinfo.m_eRMIState = eRMIState;
//		rcMemDMICanvas.left = rcDMICanvas.left - rcCanvas.left;
//		rcMemDMICanvas.top = rcDMICanvas.top - rcCanvas.top;
//		rcMemDMICanvas.right = rcDMICanvas.right - rcCanvas.left;
//		rcMemDMICanvas.bottom = rcDMICanvas.bottom - rcCanvas.top;
//		_doDrawMenuItem( hMemDC, rcMemDMICanvas, nItemIndex, rmiinfo );
//	}
//
//	//
//	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, ( rcCanvas.right - rcCanvas.left ), ( rcCanvas.bottom - rcCanvas.top ), hMemDC, 0, 0, SRCCOPY );
//	
//exit:
//	::SelectObject( hMemDC, hBitmapOld );
//	::DeleteObject( hBitmap );
//	::DeleteDC( hMemDC );
//
//}
//
void hm_wmp_rootmenu::_doDrawBackground( HDC hDC, const RECT& rcCanvas ) {
	hm_districtobj* pDistrictObj = NULL;

	if ( !_getDistrictObj( EDISTRICTID_BKGND, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
}
//
void hm_wmp_rootmenu::_doDrawMenuItem( HDC hDC, const RECT& rcDMICanvas, unsigned int uIndex, const rmiteminfo& rmiinfo ) {
	hm_districtobj* pDistrictObj = NULL;
	hm_wmp_display::district_id_type idDistrict;
	int nBKModeOld;
	
	switch ( rmiinfo.m_eRMIState ) {
	case ERMIS_NORMAL:{
		idDistrict = EDISTRICTID_MENUITEM_NOR;
		} break;
	case ERMIS_HOVER: {
		idDistrict = EDISTRICTID_MENUITEM_HOV;
		} break;
	case ERMIS_PRESS: {
		idDistrict = EDISTRICTID_MENUITEM_PRE;
		} break;
	case ERMIS_DISABLE: {
		idDistrict = EDISTRICTID_MENUITEM_DIS;
		} break;
	default:
		idDistrict = EDISTRICTID_MENUITEM_NOR;
		break;
	}
	if ( !_getDistrictObj(  idDistrict, &pDistrictObj ) )
		return;
	pDistrictObj->show ( hDC, rcDMICanvas, m_pDrawTool );

	// draw text
	if ( rmiinfo.m_strText.length() ) {
		nBKModeOld = ::SetBkMode( hDC, TRANSPARENT );
		COLORREF clrTextOld = ::SetTextColor( hDC, RGB( 216, 224, 231 ) );   // label #D8E0E7
		// explicit CJK font: the DC default ( bitmap System font ) has no
		// Chinese glyphs - localized labels would render as tofu.
		HFONT hFontOld = (HFONT)::SelectObject( hDC, _GetMenuLabelFont() );
		::DrawText( hDC, rmiinfo.m_strText.c_str(), rmiinfo.m_strText.length(), (RECT*)&rcDMICanvas, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
		::SelectObject( hDC, hFontOld );
		::SetTextColor( hDC, clrTextOld );
		::SetBkMode( hDC, nBKModeOld );
	}

}

bool hm_wmp_rootmenu::_getCurMenuItemIndex( const RECT& rcMDICanvas, HWND hWnd, const POINT& ptScreen, int& curMIIndex ) {
	HMENU hRootMenu;
	int nItemIndex, nItemCount;
	RECT rcTmp;
	POINT ptWnd;
	RECT rcWnd;
	
	if ( !::GetWindowRect( hWnd, &rcWnd ) ) return false;
	ptWnd.x = ptScreen.x - rcWnd.left;
	ptWnd.y = ptScreen.y - rcWnd.top;
	hRootMenu = ::GetMenu( hWnd );
	if ( !hRootMenu ) return false;
	nItemCount = GetMenuItemCount( hRootMenu );
	for ( nItemIndex = 0; nItemIndex<nItemCount; ++nItemIndex ) {
		rcTmp.top = rcMDICanvas.top + m_uSpaceV;
		rcTmp.bottom = rcMDICanvas.bottom - m_uSpaceV;
		rcTmp.left = rcMDICanvas.left + m_uSpaceH + ( nItemIndex*m_uLenMenuItem );
		rcTmp.right = rcTmp.left + m_uLenMenuItem;
		//
		if ( ::PtInRect( &rcTmp, ptWnd ) ) {
			curMIIndex = nItemIndex;
			return true;
		}
	}
	
	curMIIndex = -1;
	return true;
}

void hm_wmp_rootmenu::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_MENUITEM_DIS );
	containerIds.push_back( EDISTRICTID_MENUITEM_PRE );
	containerIds.push_back( EDISTRICTID_MENUITEM_HOV );
	containerIds.push_back( EDISTRICTID_MENUITEM_NOR );
	containerIds.push_back( EDISTRICTID_BKGND );
}