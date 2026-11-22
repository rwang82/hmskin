#include "stdafx.h"
#include "win\wmp\hm_wmp_popupmenu.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_event_defs.h"
#include "win\hm_wndmsg_defs.h"
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

//
static bool _IsMenuSeparator( HMENU hMenu, unsigned int uIndex ) {
	MENUITEMINFO miinfo;
	memset( &miinfo, 0, sizeof( miinfo ) );
	miinfo.cbSize = sizeof( miinfo );
	miinfo.fMask = MIIM_FTYPE;
	return ::GetMenuItemInfo( hMenu, uIndex, TRUE, &miinfo )
	       && ( miinfo.fType & MFT_SEPARATOR );
}

// separators occupy a 1px slot only - the surrounding item spacing forms
// their margins ( native menus show a hairline, not a full item row ).
static unsigned int _ItemSlotHeight( HMENU hMenu, unsigned int uIndex, unsigned int uMIHeight ) {
	return _IsMenuSeparator( hMenu, uIndex ) ? 1 : uMIHeight;
}

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

	unsigned int nItemsH = 0;
	for ( int i = 0; i < nMICount; ++i )
		nItemsH += _ItemSlotHeight( hMenu, (unsigned int)i, m_uMIHeight );
	pMenuWndSize->cx = m_uSpace + m_uMIWidth + m_uSpace;
	pMenuWndSize->cy = m_uSpace + nItemsH + m_uSpace*(nMICount-1) + m_uSpace;
	return true;
}
//
bool dimension_menuwnd_default::getMIRect( HMENU hMenu, unsigned int uMIIndex, RECT* pMIRect ) {
	if ( !hMenu || !pMIRect ) return false;
	int nMICount;

	nMICount = GetMenuItemCount( hMenu );
	if ( (nMICount == -1) || (nMICount <= (int)uMIIndex) ) return false;

	unsigned int nTop = m_uSpace;
	for ( unsigned int i = 0; i < uMIIndex; ++i )
		nTop += _ItemSlotHeight( hMenu, i, m_uMIHeight ) + m_uSpace;
	pMIRect->left = m_uSpace;
	pMIRect->top = nTop;
	pMIRect->right = pMIRect->left + m_uMIWidth;
	pMIRect->bottom = nTop + _ItemSlotHeight( hMenu, uMIIndex, m_uMIHeight );

	return true;
}
//
hm_wmp_popupmenu::hm_wmp_popupmenu( dimension_menuwnd_i* pCalcMenuWndRectObj, hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_hMenu( NULL )
, m_pDimensionMenuWndObj( pCalcMenuWndRectObj )
, m_nHoverMIIndex( -1 ) {
	_createDistrictObjs();
}
//
hm_wmp_popupmenu::~hm_wmp_popupmenu() {
	delete m_pDimensionMenuWndObj;
}
//

MSGBIND_BEGIN( hm_wmp_popupmenu )
	MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
	MSGBIND( WM_KEYDOWN, _OnKeyDown )
	MSGBIND( WM_NCHITTEST, _OnNCHitTest )
	MSGBIND( WM_RBUTTONDOWN, _OnRButtonDown )
	MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
	MSGBIND( WM_DESTROY, _OnDestroy )
	MSGBIND( HMWM_BINDMENU, _OnHMWMBindMenu )
	MSGBIND( WM_KILLFOCUS, _OnKillFocus )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_popupmenu )
	ORDERWNDMSG( WM_LBUTTONDOWN )
	ORDERWNDMSG( WM_KEYDOWN )
	ORDERWNDMSG( WM_NCHITTEST )
	ORDERWNDMSG( WM_RBUTTONDOWN )
	ORDERWNDMSG( WM_MOUSEMOVE )
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
		case HM_EVENT_DRAWSELF:
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

		// resolve the clicked item and notify the app before closing.
		// clicks on separators and disabled items are absorbed - no command,
		// the menu stays open ( native, verified on Win11 ).
		if ( m_pDimensionMenuWndObj && m_hMenu ) {
			int nMICount = ::GetMenuItemCount( m_hMenu );
			for ( int nMIIndex = 0; nMIIndex < nMICount; ++nMIIndex ) {
				RECT rcMI;
				if ( !m_pDimensionMenuWndObj->getMIRect( m_hMenu, (unsigned int)nMIIndex, &rcMI ) )
					continue;
				if ( ::PtInRect( &rcMI, ptClient ) ) {
					if ( !_isSelectableItem( nMIIndex ) )
						return;   // separator/disabled: no-op, stays open
					_activateItem( nMIIndex, pKeelObj );
					break;
				}
			}
		}
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
	// ( PostQuitMessage here was a leftover from a standalone test program -
	//   closing a popup menu must never quit the host application. )

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
	// owner window ( engine-driven popups ): selected commands are forwarded
	// as native WM_COMMAND, so host apps keep their existing menu handlers.
	m_hWndOwner = (HWND)pWP->lParam;
	if ( m_pDimensionMenuWndObj && m_pDimensionMenuWndObj->getWndSize( m_hMenu, &sizeMenuWnd ) ) {
		::GetWindowRect( pWP->hwnd, &rcCurWnd );
		int x = rcCurWnd.left;
		int y = rcCurWnd.top;
		// native TrackPopupMenu parity: flip the menu back into the monitor
		// work area - the anchor point becomes the menu's right/bottom edge.
		MONITORINFO mi;
		mi.cbSize = sizeof( mi );
		HMONITOR hMon = ::MonitorFromWindow( pWP->hwnd, MONITOR_DEFAULTTONEAREST );
		if ( hMon && ::GetMonitorInfo( hMon, &mi ) ) {
			if ( x + sizeMenuWnd.cx > mi.rcWork.right )
				x -= sizeMenuWnd.cx;
			if ( y + sizeMenuWnd.cy > mi.rcWork.bottom )
				y -= sizeMenuWnd.cy;
			if ( x < mi.rcWork.left ) x = mi.rcWork.left;
			if ( y < mi.rcWork.top ) y = mi.rcWork.top;
		}
		::MoveWindow( pWP->hwnd, x, y, sizeMenuWnd.cx, sizeMenuWnd.cy, TRUE );
	}
}

void hm_wmp_popupmenu::_doDrawSkin( HDC hDC, const RECT& rcCanvas ) {
	if ( !m_pDimensionMenuWndObj || !m_hMenu ) return;
	int nMICount;
	RECT rcMI;
	MENUITEMINFO miinfo;
	TCHAR* pMITextBuf = NULL;
	string_type strMIText;
	hm_districtobj* pDistrictObj = NULL;

	// draw background.
	if ( _getDistrictObj( EDISTRICTID_BKGND, &pDistrictObj ) )
		pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );

	// draw menu items.
	nMICount = ::GetMenuItemCount( this->m_hMenu );
	if ( nMICount == -1 ) return;
	for ( int nMIIndex = 0; nMIIndex<nMICount; ++nMIIndex ) {
		if ( !m_pDimensionMenuWndObj->getMIRect( m_hMenu, (unsigned int)nMIIndex, &rcMI ) )
			continue;
		// item background: disabled items render DIS, hover gets HOV, others NOR.
		bool bDisabled = !_isSelectableItem( nMIIndex );
		if ( _getDistrictObj( bDisabled ? EDISTRICTID_MENUITEM_DIS
				: ( ( nMIIndex == m_nHoverMIIndex ) ? EDISTRICTID_MENUITEM_HOV : EDISTRICTID_MENUITEM_NOR ),
				&pDistrictObj ) )
			pDistrictObj->show( hDC, rcMI, m_pDrawTool );
		// draw menu item text here.
		// get menu item text length.
		memset( &miinfo, 0, sizeof(MENUITEMINFO) );
		miinfo.cbSize = sizeof( MENUITEMINFO );
		miinfo.fMask = MIIM_TYPE;
		miinfo.fType = MFT_STRING;
		miinfo.dwTypeData = NULL;
		if ( !::GetMenuItemInfo( m_hMenu, nMIIndex, TRUE, &miinfo ) )
			continue;
		if ( miinfo.fType & MFT_SEPARATOR ) {
			// hairline #262C34 ( DESIGN.md ), insets match the label margins.
			RECT rcLine = { rcMI.left + 12, ( rcMI.top + rcMI.bottom ) / 2,
			                rcMI.right - 8, ( rcMI.top + rcMI.bottom ) / 2 + 1 };
			HBRUSH hbrLine = ::CreateSolidBrush( RGB( 0x26, 0x2C, 0x34 ) );
			::FillRect( hDC, &rcLine, hbrLine );
			::DeleteObject( hbrLine );
			continue;
		}
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
		RECT rcText = rcMI;
		rcText.left += 12;
		rcText.right -= 8;
		int nBKModeOld = ::SetBkMode( hDC, TRANSPARENT );
		// disabled text #5B636E ( DESIGN.md ), normal #E8ECF1.
		COLORREF clrTextOld = ::SetTextColor( hDC,
			bDisabled ? RGB( 91, 99, 110 ) : RGB( 232, 236, 241 ) );
		// explicit CJK font ( same as the root menu bar ): the DC default
		// bitmap font has no Chinese glyphs.
		HFONT hFontOld = (HFONT)::SelectObject( hDC, _GetMenuLabelFont() );
		::DrawText( hDC, strMIText.c_str(), -1, &rcText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS );
		::SelectObject( hDC, hFontOld );
		::SetTextColor( hDC, clrTextOld );
		::SetBkMode( hDC, nBKModeOld );

	}
}

//
void hm_wmp_popupmenu::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	if ( !m_pDimensionMenuWndObj || !m_hMenu )
		return;
	POINT ptClient;
	int nMICount;
	int nHoverNew = -1;

	//
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	nMICount = ::GetMenuItemCount( m_hMenu );
	for ( int nMIIndex = 0; nMIIndex < nMICount; ++nMIIndex ) {
		RECT rcMI;
		if ( !m_pDimensionMenuWndObj->getMIRect( m_hMenu, (unsigned int)nMIIndex, &rcMI ) )
			continue;
		if ( ::PtInRect( &rcMI, ptClient ) ) {
			// separators/disabled items never take the hover highlight.
			if ( _isSelectableItem( nMIIndex ) )
				nHoverNew = nMIIndex;
			break;
		}
	}
	if ( nHoverNew == m_nHoverMIIndex )
		return; // hover item unchanged.
	_setHoverIndex( nHoverNew, pWP->hwnd );
}

// native TrackPopupMenu keyboard parity: arrows move the selection ( wrapping,
// separators and disabled items skipped; first press selects the first/last
// item ), RETURN/SPACE activate, ESC dismisses.
void hm_wmp_popupmenu::_OnKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	if ( !m_pDimensionMenuWndObj || !m_hMenu )
		return;

	switch ( (UINT)pWP->wParam ) {
	case VK_UP:
	case VK_DOWN: {
		int nMICount = ::GetMenuItemCount( m_hMenu );
		if ( nMICount <= 0 )
			return;
		int nDir = ( pWP->wParam == VK_DOWN ) ? 1 : -1;
		// first press from unselected: DOWN starts before item 0,
		// UP starts after the last item.
		int nNew = ( m_nHoverMIIndex < 0 ) ? ( ( nDir > 0 ) ? -1 : nMICount ) : m_nHoverMIIndex;
		for ( int i = 0; i < nMICount; ++i ) {
			nNew += nDir;
			if ( nNew < 0 ) nNew = nMICount - 1;   // wrap around
			if ( nNew >= nMICount ) nNew = 0;
			if ( _isSelectableItem( nNew ) ) {
				_setHoverIndex( nNew, pWP->hwnd );
				return;
			}
		}
		break;   // nothing selectable - keep the current state
	}
	case VK_RETURN:
	case VK_SPACE:
		if ( m_nHoverMIIndex >= 0 && _isSelectableItem( m_nHoverMIIndex ) )
			_activateItem( m_nHoverMIIndex, pKeelObj );
		::DestroyWindow( pWP->hwnd );
		break;
	case VK_ESCAPE:
		::DestroyWindow( pWP->hwnd );
		break;
	}
}

void hm_wmp_popupmenu::_setHoverIndex( int nMIIndex, HWND hWnd ) {
	if ( nMIIndex == m_nHoverMIIndex )
		return;
	m_nHoverMIIndex = nMIIndex;
	// full redraw through the buffered WM_PAINT cycle ( drawtrigger WMP ):
	// a direct GetDC repaint showed intermediate states on screen -
	// background 9-patch flashing while sweeping across items.
	::InvalidateRect( hWnd, NULL, FALSE );
}

void hm_wmp_popupmenu::_activateItem( int nMIIndex, hm_keel_obj* pKeelObj ) {
	if ( !_isSelectableItem( nMIIndex ) )
		return;
	MENUITEMINFO miinfo;
	memset( &miinfo, 0, sizeof( miinfo ) );
	miinfo.cbSize = sizeof( miinfo );
	miinfo.fMask = MIIM_ID;
	if ( !::GetMenuItemInfo( m_hMenu, nMIIndex, TRUE, &miinfo ) || !miinfo.wID )
		return;
	// native semantics: WM_COMMAND to the owner ( engine-driven popups ) -
	// host menu handlers work unchanged. the listener event stays as well.
	if ( m_hWndOwner )
		::PostMessage( m_hWndOwner, WM_COMMAND, MAKEWPARAM( miinfo.wID, 0 ), 0 );
	if ( m_pEventListener )
		m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_POPUPMENU_CLICK,
			( void* )pKeelObj, ( void* )( UINT_PTR )miinfo.wID );
}

bool hm_wmp_popupmenu::_isSelectableItem( int nMIIndex ) const {
	MENUITEMINFO miinfo;
	memset( &miinfo, 0, sizeof( miinfo ) );
	miinfo.cbSize = sizeof( miinfo );
	miinfo.fMask = MIIM_FTYPE | MIIM_STATE;
	if ( !::GetMenuItemInfo( m_hMenu, nMIIndex, TRUE, &miinfo ) )
		return false;
	if ( miinfo.fType & MFT_SEPARATOR )
		return false;
	if ( miinfo.fState & ( MF_DISABLED | MF_GRAYED ) )
		return false;
	return true;
}

void hm_wmp_popupmenu::_OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	// a popup menu window is all client: claim the hit test so mouse input is
	// generated in client flavor ( WM_LBUTTONDOWN etc. ). without this, some
	// other non-client handler answered HTCAPTION for this window and the
	// WMP never saw the clicks ( trace 2026-10-15: 6x WM_NCLBUTTONDOWN w=2 ).
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_OTHER_WNDMSGPROCESSOR | EATMSGFLAG_DISABLE_SOURCE;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = HTCLIENT;
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