#include "stdafx.h"
#include "win\wmp\hm_wmp_popupmenu.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_event_defs.h"
#include "win\hm_wndmsg_defs.h"

//
dimension_menuwnd_default::dimension_menuwnd_default( unsigned int uMIHeight, unsigned int uMIWidth, unsigned int uSpace )
: dimension_menuwnd_i()
, m_uMIHeight( uMIHeight )
, m_uMIWidth( uMIWidth )
, m_uSpace( uSpace )
{}
//
dimension_menuwnd_default::~dimension_menuwnd_default() {}
//
bool dimension_menuwnd_default::getWndSize( HMENU hMenu, SIZE* pMenuWndSize ){
	if ( !hMenu || !pMenuWndSize ) return false;
	int nMICount;

	nMICount = GetMenuItemCount( hMenu );
	if ( nMICount == -1 )
		return false;
	
	pMenuWndSize->cx = m_uSpace + m_uMIWidth + m_uSpace;
	pMenuWndSize->cy = m_uSpace + m_uMIHeight*nMICount + m_uSpace*(nMICount-1) + m_uSpace;
	return true;
}
//
bool dimension_menuwnd_default::getMIRect( HMENU hMenu, unsigned int uMIIndex, RECT* pMIRect ) {
	if ( !hMenu || !pMIRect ) return false;
	int nMICount;

	nMICount = GetMenuItemCount( hMenu );
	if ( (nMICount == -1) || (nMICount <= (int)uMIIndex) ) return false;
	
	pMIRect->left = m_uSpace;
	pMIRect->top = m_uSpace + uMIIndex*(m_uMIHeight + m_uSpace);
	pMIRect->right = pMIRect->left + m_uMIWidth;
	pMIRect->bottom = pMIRect->top + m_uMIHeight;
	
	return true;
}
//
hm_wmp_popupmenu::hm_wmp_popupmenu( dimension_menuwnd_i* pCalcMenuWndRectObj, hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_hMenu( NULL )
, m_pDimensionMenuWndObj( pCalcMenuWndRectObj ) {
	_createDistrictObjs();
}
//
hm_wmp_popupmenu::~hm_wmp_popupmenu() {
	delete m_pDimensionMenuWndObj;
}
//

MSGBIND_BEGIN( hm_wmp_popupmenu )
	MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
	MSGBIND( WM_RBUTTONDOWN, _OnRButtonDown )
	MSGBIND( WM_DESTROY, _OnDestroy )
	MSGBIND( HMWM_BINDMENU, _OnHMWMBindMenu )
	MSGBIND( WM_KILLFOCUS, _OnKillFocus )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_popupmenu )
	ORDERWNDMSG( WM_LBUTTONDOWN )
	ORDERWNDMSG( WM_RBUTTONDOWN )
	ORDERWNDMSG( WM_DESTROY )
	ORDERWNDMSG( HMWM_BINDMENU )
	ORDERWNDMSG( WM_KILLFOCUS )
ORDERWNDMSG_END()
//
void hm_wmp_popupmenu::_createDistrictObjs() {
	_setDistrictObj( EDISTRICTID_BKGND, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_NOR, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_HOV, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_PRE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_MENUITEM_DIS, new hm_districtobj() );
		
}
//
void hm_wmp_popupmenu::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent ) {
		case HMKEELOBJEVENT_DRAWSELF:
			{
				HEDSPARAM* pHEDSParam = NULL;
				RECT rcCanvas;

				pHEDSParam = (HEDSPARAM*)pEventParam;
				if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
					return;
				
				_doDrawSkin( pHEDSParam->m_hDCCanvas, rcCanvas );
			}
			break;
		default:
			break;
	}
}

void hm_wmp_popupmenu::_OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	::OutputDebugString( _T("enter hm_wmp_popupmenu::_OnLButtonDown.\n") );
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	hm_layout_obj* pLayOutObj = NULL;
	RECT rcCanvas;
	POINT ptClient;

	//
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );

	//
	if ( ::PtInRect( &rcCanvas, ptClient ) ) {
		//
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;//HTNOWHERE;

		::DestroyWindow( pWP->hwnd );
		return;
	}
	
}

void hm_wmp_popupmenu::_OnDestroy( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	
	//
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;//HTNOWHERE;
	//
	::PostQuitMessage( 0 ); 
	
}

void hm_wmp_popupmenu::_OnHMWMBindMenu( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	RECT rcCurWnd;
	SIZE sizeMenuWnd;
	
	//
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	
	m_hMenu = (HMENU)pWP->wParam;
	if ( m_pDimensionMenuWndObj && m_pDimensionMenuWndObj->getWndSize( m_hMenu, &sizeMenuWnd ) ) {
		::GetWindowRect( pWP->hwnd, &rcCurWnd );
		::MoveWindow( pWP->hwnd, rcCurWnd.left, rcCurWnd.top, sizeMenuWnd.cx, sizeMenuWnd.cy, TRUE );
	}
}

void hm_wmp_popupmenu::_doDrawSkin( HDC hDC, const RECT& rcCanvas ) {
	if ( !m_pDimensionMenuWndObj || !m_hMenu ) return;
	//COLORREF clrBK;
	COLORREF clrMI;
	int nMICount;
	RECT rcMI;
	MENUITEMINFO miinfo;
	TCHAR* pMITextBuf = NULL;
	string_type strMIText;
	hm_districtobj* pDistrictObj = NULL;
	hm_wmp_display::district_id_type idDistrict;
	
	// draw bk
	idDistrict = EDISTRICTID_BKGND;
	if ( !_getDistrictObj(  idDistrict, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
	// draw meun item.
	clrMI = RGB( 60, 30, 60 );
	nMICount = ::GetMenuItemCount( this->m_hMenu );
	if ( nMICount == -1 ) return;
	for ( int nMIIndex = 0; nMIIndex<nMICount; ++nMIIndex ) {
		if ( !m_pDimensionMenuWndObj->getMIRect( m_hMenu, (unsigned int)nMIIndex, &rcMI ) )
			continue;
		hm_util_draw_rect( hDC, rcMI, clrMI );

		// draw menu item text here.
		// get menu item text length.
		memset( &miinfo, 0, sizeof(MENUITEMINFO) );
		miinfo.cbSize = sizeof( MENUITEMINFO );
		miinfo.fMask = MIIM_TYPE;
		miinfo.fType = MFT_STRING;
		miinfo.dwTypeData = NULL;
		if ( !::GetMenuItemInfo( m_hMenu, nMIIndex, TRUE, &miinfo ) )
			continue;
		// get menu item text
		pMITextBuf = new TCHAR[ miinfo.cch + 1 ];
		miinfo.dwTypeData = pMITextBuf;
		miinfo.cch = miinfo.cch + 1;
		if ( !::GetMenuItemInfo( m_hMenu, nMIIndex, TRUE, &miinfo ) ) {
			delete[] pMITextBuf;
			continue;		
		}
		strMIText = pMITextBuf;
		delete[] pMITextBuf;
		// draw menu item text.
		::DrawText( hDC, strMIText.c_str(), -1, &rcMI, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		
	}
}

void hm_wmp_popupmenu::_OnRButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	hm_layout_obj* pLayOutObj = NULL;
	RECT rcCanvas;
	POINT ptClient;

	//
	pLayOutObj = pKeelObj->getLayoutObj();
	if ( !pLayOutObj ) return;
	if ( !pLayOutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	if ( ::PtInRect( &rcCanvas, ptClient ) ) {
		//
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;//HTNOWHERE;

		//::MessageBox( pWP->hwnd, _T("RLButtonDown"), _T(""), MB_OK );
		//::InvalidateRect( pWP->hwnd, NULL, TRUE );
		return;
	} 
}
//
void hm_wmp_popupmenu::_OnKillFocus( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	::DestroyWindow( pWP->hwnd );
}

void hm_wmp_popupmenu::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_MENUITEM_DIS );
	containerIds.push_back( EDISTRICTID_MENUITEM_PRE );
	containerIds.push_back( EDISTRICTID_MENUITEM_HOV );
	containerIds.push_back( EDISTRICTID_MENUITEM_NOR );
	containerIds.push_back( EDISTRICTID_BKGND );
}