#include "stdafx.h"
#include "win\wmp\hm_wmp_button_nca.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_event_defs.h"
#include "hmskin_event_defs.h"

hm_wmp_button_nca::hm_wmp_button_nca( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_uBtnStateFlags( 0 )
, m_bActive( false )
{
	_createDistrictObjs();
}
hm_wmp_button_nca::~hm_wmp_button_nca(){}
//
MSGBIND_BEGIN( hm_wmp_button_nca )
	MSGBIND( WM_NCLBUTTONDOWN, _OnNCLButtonDown )
	MSGBIND( WM_NCLBUTTONDBLCLK, _OnNCLButtonDblClk )
	MSGBIND( WM_NCLBUTTONUP, _OnNCLButtonUp )
	MSGBIND( WM_LBUTTONUP, _OnLButtonUp )
	MSGBIND( WM_NCMOUSEMOVE, _OnNCMouseMove )
	MSGBIND( WM_NCMOUSELEAVE, _OnNCMouseLeave )
	MSGBIND( WM_NCACTIVATE, _OnNCActivate )
	MSGBIND( WM_NCHITTEST, _OnNCHitTest )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_button_nca )
	ORDERWNDMSG( WM_NCLBUTTONDOWN )
	ORDERWNDMSG( WM_NCLBUTTONDBLCLK )
	ORDERWNDMSG( WM_NCLBUTTONUP )
	ORDERWNDMSG( WM_LBUTTONUP )
	ORDERWNDMSG( WM_NCMOUSEMOVE )
	ORDERWNDMSG( WM_NCMOUSELEAVE )
	ORDERWNDMSG( WM_NCACTIVATE )
	ORDERWNDMSG( WM_NCHITTEST )
ORDERWNDMSG_END()
//
void hm_wmp_button_nca::_createDistrictObjs() {
	_setDistrictObj( EDISTRICTID_BK_NOR_ACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_HOV_ACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_PRE_ACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_DIS_ACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_NOR_INACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_HOV_INACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_PRE_INACTIVE, new hm_districtobj );
	_setDistrictObj( EDISTRICTID_BK_DIS_INACTIVE, new hm_districtobj );

		
}
//
void hm_wmp_button_nca::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ){
	switch ( idEvent ) {
		case HMKEELOBJEVENT_DRAWSELF:
			{
				HEDSPARAM* pHEDSParam = NULL;
				RECT rcCanvas;
			
				pHEDSParam = (HEDSPARAM*)pEventParam;	
				if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
					return;
				
				_doDrawSkin( pHEDSParam->m_hDCCanvas, rcCanvas, _getCurState() );
			}
			break;
		default:
			break;
	}
}
//
void hm_wmp_button_nca::_OnNCActivate( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ){
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 1;
	//
	m_bActive = ( pWP->wParam != 0 );
}
//
void hm_wmp_button_nca::_doDrawSkin( HDC hDC, const RECT& rcCanvas, ENUMBTNSTATE eBtnState ){
	switch ( eBtnState ) {
	case NCBTN_STATE_NORMAL_ACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_NOR_ACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_HOVER_ACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_HOV_ACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_PRESS_ACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_PRE_ACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_DISABLE_ACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_DIS_ACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_NORMAL_INACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_NOR_INACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_HOVER_INACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_HOV_INACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_PRESS_INACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_PRE_INACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case NCBTN_STATE_DISABLE_INACTIVE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_DIS_INACTIVE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	default:
		break;
	}
	//static TCHAR s_aryState[][20] ={ _T("Normal"), _T("Hover"), _T("Press"), _T("Disable") };
	//std::basic_string< TCHAR > strText;

	//hm_util_draw_rect( hDC, rcCanvas );

	//strText = s_aryState[ eBtnState%4 ];
	//::DrawText( hDC, strText.c_str(), strText.length(), (RECT*)&rcCanvas, DT_CENTER );
}
//
hm_wmp_button_nca::ENUMBTNSTATE hm_wmp_button_nca::_getCurState() {
	if ( m_uBtnStateFlags & NCABTN_FLAG_DISABLE ) {
		return m_bActive ? NCBTN_STATE_DISABLE_ACTIVE : NCBTN_STATE_DISABLE_INACTIVE;
	} else if ( m_uBtnStateFlags & NCABTN_FLAG_LBTNDOWN ) {
		return m_bActive ? NCBTN_STATE_PRESS_ACTIVE : NCBTN_STATE_PRESS_INACTIVE;
	} else if ( m_uBtnStateFlags & NCABTN_FLAG_TRACEMOUSELEAVE ) {
		return m_bActive ? NCBTN_STATE_HOVER_ACTIVE : NCBTN_STATE_HOVER_INACTIVE;
	} else {
		return m_bActive ? NCBTN_STATE_NORMAL_ACTIVE : NCBTN_STATE_NORMAL_INACTIVE;
	}
}
//
void hm_wmp_button_nca::_OnNCLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ){
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptWindow;
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcObj;
	HDC hWndDC;
	RECT rcWnd;

	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWindow.x = ( (POINTS*)(&pWP->lParam) )->x - rcWnd.left;
	ptWindow.y = ( (POINTS*)(&pWP->lParam) )->y - rcWnd.top;
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcObj );
	//
	if ( !::PtInRect( &rcObj, ptWindow ) )
		return;
	//eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	{
		TCHAR szDbg[ 64 ];
		_stprintf_s( szDbg, 64, _T("[nca] press id=%u\n"), pKeelObj->getId() );
		DisTraceOut( szDbg );
	}
	m_uBtnStateFlags |= NCABTN_FLAG_LBTNDOWN;
	//
	if ( GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	//
	hWndDC = ::GetWindowDC( pWP->hwnd );
	_doDrawSkin( hWndDC, rcObj, m_bActive ? NCBTN_STATE_PRESS_ACTIVE : NCBTN_STATE_PRESS_INACTIVE );
	::ReleaseDC( pWP->hwnd, hWndDC );
}
//
void hm_wmp_button_nca::_OnNCLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ){
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcWnd;
	POINT ptWindow;
	RECT rcObj;
	HDC hWndDC;

	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWindow.x = ( (POINTS*)(&pWP->lParam) )->x - rcWnd.left;
	ptWindow.y = ( (POINTS*)(&pWP->lParam) )->y - rcWnd.top;
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcObj );
	//
	if ( !::PtInRect( &rcObj, ptWindow ) )
		return;
	//eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	m_uBtnStateFlags |= NCABTN_FLAG_LBTNDOWN;
	//
	if ( GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	//
	hWndDC = ::GetWindowDC( pWP->hwnd );
	_doDrawSkin( hWndDC, rcObj, m_bActive ? NCBTN_STATE_PRESS_ACTIVE : NCBTN_STATE_PRESS_INACTIVE );
	::ReleaseDC( pWP->hwnd, hWndDC );
}
//
void hm_wmp_button_nca::_OnNCLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ){
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	BOOL bLBtnDown;
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcObj;
	HDC hDC; 
	POINT ptWindow;
	RECT rcWnd;
	//
	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWindow.x = ( (POINTS*)(&pWP->lParam) )->x - rcWnd.left;
	ptWindow.y = ( (POINTS*)(&pWP->lParam) )->y - rcWnd.top;
	//
	pLayoutObj = ( hm_layout_obj* )pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcObj );
	//
	bLBtnDown = (m_uBtnStateFlags & NCABTN_FLAG_LBTNDOWN);
	//state
	if ( bLBtnDown ) {
		::ReleaseCapture();
	}
	m_uBtnStateFlags &= ~NCABTN_FLAG_LBTNDOWN ;

	{
		TCHAR szDbg[ 96 ];
		_stprintf_s( szDbg, 96, _T("[nca] up id=%u btndown=%d inrect=%d pt=(%d,%d)\n"),
			pKeelObj ? pKeelObj->getId() : 0, bLBtnDown ? 1 : 0,
			::PtInRect( &rcObj, ptWindow ) ? 1 : 0, ptWindow.x, ptWindow.y );
		DisTraceOut( szDbg );
	}

	//draw
	hDC = GetWindowDC( pWP->hwnd );
	if ( ::PtInRect( &rcObj, ptWindow ) ) {
		//eat message
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;// | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;
		//
		_doDrawSkin( hDC, rcObj, m_bActive ? NCBTN_STATE_HOVER_ACTIVE : NCBTN_STATE_HOVER_INACTIVE );
	}
	else if ( bLBtnDown ) {
		_doDrawSkin( hDC, rcObj, m_bActive ? NCBTN_STATE_NORMAL_ACTIVE : NCBTN_STATE_NORMAL_INACTIVE );
		m_uBtnStateFlags = 0;
	}
	::ReleaseDC( pWP->hwnd, hDC );

	//send BN_CLICKED message to parent
	if ( bLBtnDown && ::PtInRect( &rcObj, ptWindow ) )
	{
		m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_NCABTN_CLICK, ( void* )pKeelObj, ( void* )0 );
		//////notify parent
		//wParam = MAKEWPARAM( GetDlgCtrlID( pWP->hwnd ), BN_CLICKED );
		//lParam = (LPARAM)pWP->hwnd;
		//::SendMessage( ::GetParent( pWP->hwnd ), WM_COMMAND, wParam, lParam );				
	}
}
//
void hm_wmp_button_nca::_OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	// a button press holds capture, so the release arrives as a CLIENT message
	// ( WM_LBUTTONUP, client coords ) - windows never generates WM_NCLBUTTONUP
	// while captured. the WH_MOUSE hook translates it back to a synthetic
	// WM_NCLBUTTONUP, but that is posted asynchronously, and the spurious
	// WM_NCMOUSELEAVE from the capture change won the race and killed the
	// pending click ( trace 2026-10-15: 9 presses, 0 pairings ). complete the
	// release here, synchronously on the real message.
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	if ( !( m_uBtnStateFlags & NCABTN_FLAG_LBTNDOWN ) )
		return; // not a release of our press

	CWPSTRUCT wpRelease = *pWP;
	POINT ptScreen;

	ptScreen.x = ( (POINTS*)(&pWP->lParam) )->x;
	ptScreen.y = ( (POINTS*)(&pWP->lParam) )->y;
	::ClientToScreen( pWP->hwnd, &ptScreen );
	wpRelease.lParam = MAKELPARAM( ptScreen.x, ptScreen.y );
	//
	_OnNCLButtonUp( pKeelObj, &wpRelease, pParamWndProc );
}
//
void hm_wmp_button_nca::_OnNCMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ){
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	BOOL bMouse1stIn;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hWndDC;
	RECT rcObj;
	POINT ptWnd;
	RECT rcWnd;

	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = ((POINTS*)(&pWP->lParam))->x - rcWnd.left;
	ptWnd.y = ((POINTS*)(&pWP->lParam))->y - rcWnd.top;
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcObj );
	//eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	if ( !::PtInRect( &rcObj, ptWnd ) ) {
		if ( m_uBtnStateFlags & NCABTN_FLAG_TRACEMOUSELEAVE ) {
			// leave event
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE|TME_NONCLIENT|TME_CANCEL; // NC area: cancel needs the same tracking type
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = pWP->hwnd;

			::TrackMouseEvent( &tme );
			::OutputDebugString( _T("cancel mouse leave.\n") );

			m_uBtnStateFlags &= ~NCABTN_FLAG_TRACEMOUSELEAVE;
			hWndDC = ::GetWindowDC( pWP->hwnd );
			_doDrawSkin( hWndDC, rcObj, m_bActive ? NCBTN_STATE_NORMAL_ACTIVE : NCBTN_STATE_NORMAL_INACTIVE );
			::ReleaseDC( pWP->hwnd, hWndDC );
		}
		return;
	}
	//
	bMouse1stIn = !( m_uBtnStateFlags & NCABTN_FLAG_TRACEMOUSELEAVE );
	//
	if ( !( m_uBtnStateFlags & NCABTN_FLAG_TRACEMOUSELEAVE ) ) {
		TRACKMOUSEEVENT tme;

		tme.cbSize = sizeof( TRACKMOUSEEVENT );
		tme.dwFlags = TME_LEAVE|TME_NONCLIENT; // track NC-area leave ( WM_NCMOUSELEAVE ); client-only tracking left hover stuck
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = pWP->hwnd;
		
		::TrackMouseEvent( &tme );
		::OutputDebugString(_T("begin TrackMouseLeave.\n"));

		m_uBtnStateFlags |= NCABTN_FLAG_TRACEMOUSELEAVE;
	}
	//
	if ( m_uBtnStateFlags & NCABTN_FLAG_LBTNDOWN ) {
		int a = 0;
	} else {
		if ( bMouse1stIn )
		{ //hover
			hWndDC = ::GetWindowDC( pWP->hwnd );
			_doDrawSkin( hWndDC, rcObj, m_bActive ? NCBTN_STATE_HOVER_ACTIVE : NCBTN_STATE_HOVER_INACTIVE );
			::ReleaseDC( pWP->hwnd, hWndDC );
			::OutputDebugString( _T("bMouse1stIn is ture.\n") );
		}
	}
	
}
//
void hm_wmp_button_nca::_OnNCMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ){
	::OutputDebugString( _T("enter hm_wmp_button_nca::_OnNCMouseLeave\n") );
	hm_layout_obj* pLayoutObj = NULL;
	HDC hWndDC;
	RECT rcObj;

	if ( !(m_uBtnStateFlags & NCABTN_FLAG_TRACEMOUSELEAVE) )
		return;
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcObj );
	//cancel track mouse leave message
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof( TRACKMOUSEEVENT );
	tme.dwFlags = TME_LEAVE|TME_NONCLIENT|TME_CANCEL; // NC area: cancel needs the same tracking type
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = pWP->hwnd;
	
	::TrackMouseEvent( &tme );
	//
	m_uBtnStateFlags &= ~NCABTN_FLAG_TRACEMOUSELEAVE;
	//
	// do NOT clear NCABTN_FLAG_LBTNDOWN / release capture here: while the
	// button is pressed it holds capture, and the capture change at release
	// time fires a spurious WM_NCMOUSELEAVE BEFORE the ( synthetic )
	// WM_NCLBUTTONUP is dispatched - clearing the press flag there killed the
	// pending click every time ( trace 2026-10-15: 9 presses, 0 pairings ).
	// the release itself completes the press via _OnLButtonUp ( synchronous,
	// real client-flavor message ) or the hook's synthetic NC up; a release
	// outside the button cancels in _OnNCLButtonUp's else branch.
	//
	hWndDC = ::GetWindowDC( pWP->hwnd );
	if ( m_uBtnStateFlags & NCABTN_FLAG_LBTNDOWN )
		_doDrawSkin( hWndDC, rcObj, m_bActive ? NCBTN_STATE_PRESS_ACTIVE : NCBTN_STATE_PRESS_INACTIVE );
	else
		_doDrawSkin( hWndDC, rcObj, m_bActive ? NCBTN_STATE_NORMAL_ACTIVE : NCBTN_STATE_NORMAL_INACTIVE );
	::ReleaseDC( pWP->hwnd, hWndDC );
}

void hm_wmp_button_nca::_OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcObj;
	POINT ptWnd;
	RECT rcWnd;

	::GetWindowRect( pWP->hwnd, &rcWnd );
	ptWnd.x = ( ( POINTS* )( &pWP->lParam ) )->x - rcWnd.left;
	ptWnd.y = ( ( POINTS* )( &pWP->lParam ) )->y - rcWnd.top;
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcObj );

	//
	if ( ::PtInRect( &rcObj, ptWnd ) ) {
		//eat message
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV | EATMSGFLAG_DISABLE_SOURCE;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = HTCAPTION;
	}
}

void hm_wmp_button_nca::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_BK_DIS_INACTIVE );
	containerIds.push_back( EDISTRICTID_BK_PRE_INACTIVE );
	containerIds.push_back( EDISTRICTID_BK_HOV_INACTIVE );
	containerIds.push_back( EDISTRICTID_BK_NOR_INACTIVE );
	containerIds.push_back( EDISTRICTID_BK_DIS_ACTIVE );
	containerIds.push_back( EDISTRICTID_BK_PRE_ACTIVE );
	containerIds.push_back( EDISTRICTID_BK_HOV_ACTIVE );
	containerIds.push_back( EDISTRICTID_BK_NOR_ACTIVE );
}