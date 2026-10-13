#include "stdafx.h"
#include "win\wmp\hm_wmp_button.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_layoutobj_defs.h"
#include "win\hm_event_defs.h"
#include "hmskin_event_defs.h"
#include <assert.h>
//
hm_wmp_button::hm_wmp_button( hmskin_event_listener_base* pEventListener ) 
: hm_wmp_display( pEventListener )
, m_uBtnStateFlags( 0 ){
	_createDistrictObjs();
}
//
hm_wmp_button::~hm_wmp_button(){}
//
MSGBIND_BEGIN( hm_wmp_button )
	MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
	MSGBIND( WM_LBUTTONDBLCLK, _OnLButtonDblClk )
	MSGBIND( WM_LBUTTONUP, _OnLButtonUp )
	MSGBIND( WM_MOUSELEAVE, _OnMouseLeave )
	MSGBIND( WM_SETTEXT, _OnSetText )
	MSGBIND( WM_RBUTTONDOWN, _OnRButtonDown )
	MSGBIND( WM_RBUTTONUP, _OnRButtonUp )
	MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
	MSGBIND( WM_GETTEXT, _OnGetText )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_button )
	ORDERWNDMSG( WM_LBUTTONDOWN )
	ORDERWNDMSG( WM_LBUTTONDBLCLK )
	ORDERWNDMSG( WM_LBUTTONUP )
	ORDERWNDMSG( WM_RBUTTONDOWN )
	ORDERWNDMSG( WM_RBUTTONUP )
	ORDERWNDMSG( WM_MOUSEMOVE )
	ORDERWNDMSG( WM_MOUSELEAVE )
	ORDERWNDMSG( WM_SETTEXT )
	ORDERWNDMSG( WM_GETTEXT )
ORDERWNDMSG_END()
//
void hm_wmp_button::_createDistrictObjs() {
	hm_districtobj* pDistrictObj = NULL;
	
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_NOR, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_HOV, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_PRE, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_DIS, pDistrictObj );
}
//
void hm_wmp_button::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue )
{
	switch ( idEvent ) {
		case HMKEELOBJEVENT_DRAWSELF:
			{
				HEDSPARAM* pHEDSParam = NULL;
				RECT rcCanvas;
			
				pHEDSParam = (HEDSPARAM*)pEventParam;	
				if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
					return;
				
				_doDrawSkin( pHEDSParam->m_hDCCanvas, rcCanvas, _getCurState() );
			}
			break;
		default:
			break;
	}
}
//
void hm_wmp_button::_OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	if ( !::PtInRect( &rcKeelObj, ptClient ) )
		return;
	//
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	m_uBtnStateFlags |= BTN_FLAG_LBTNDOWN;
	//
	if ( ::GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	//
	hDC = ::GetDC( pWP->hwnd );
	_doDrawSkin( hDC, rcKeelObj, _getCurState() );
	::ReleaseDC( pWP->hwnd, hDC );
}
//
void hm_wmp_button::_OnLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	if ( !::PtInRect( &rcKeelObj, ptClient ) )
		return;
	//
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	m_uBtnStateFlags |= BTN_FLAG_LBTNDOWN;
	//
	if ( ::GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	//
	hDC = ::GetDC( pWP->hwnd );
	_doDrawSkin( hDC, rcKeelObj, _getCurState() );
	::ReleaseDC( pWP->hwnd, hDC );
}
//
void hm_wmp_button::_OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	BOOL bLBtnDownOld;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	bLBtnDownOld = m_uBtnStateFlags & BTN_FLAG_LBTNDOWN;
	if ( bLBtnDownOld ) {
		::ReleaseCapture();
	}
	m_uBtnStateFlags &= ~BTN_FLAG_LBTNDOWN;
	//
	hDC = ::GetDC( pWP->hwnd );
	if ( ::PtInRect( &rcKeelObj, ptClient ) ) {
		//
		_doDrawSkin( hDC, rcKeelObj, BTN_STATE_HOVER );
		// eat message
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;
	} else if ( bLBtnDownOld ) {
		_doDrawSkin( hDC, rcKeelObj, BTN_STATE_NORMAL );
	}
	::ReleaseDC( pWP->hwnd, hDC );

	// send BN_CLICKED message to parent
	if ( bLBtnDownOld && ::PtInRect( &rcKeelObj, ptClient ) ) {
		//
		if ( m_pEventListener ) {
			m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_BTN_CLICK, (void*)pKeelObj, ( void* )0 );
		}
		////notify parent
		WPARAM wParam = MAKEWPARAM( GetDlgCtrlID( pWP->hwnd ), BN_CLICKED );
		LPARAM lParam = (LPARAM)pWP->hwnd;
		::SendMessage( ::GetParent( pWP->hwnd ), WM_COMMAND, wParam, lParam );
	}

}
//
void hm_wmp_button::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	BOOL bMouse1stIn;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );

	if ( !::PtInRect( &rcKeelObj, ptClient ) ) {
		if ( m_uBtnStateFlags & BTN_FLAG_TRACEMOUSELEAVE ) {
			// leave event.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE|TME_CANCEL;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = pWP->hwnd;

			::TrackMouseEvent( &tme );
			
			m_uBtnStateFlags &= ~BTN_FLAG_TRACEMOUSELEAVE;
			//
			hDC = ::GetDC( pWP->hwnd );
			_doDrawSkin( hDC, rcKeelObj, BTN_STATE_NORMAL );
			::ReleaseDC( pWP->hwnd, hDC );
		}

		return;
	}
	//
	bMouse1stIn = !( m_uBtnStateFlags & BTN_FLAG_TRACEMOUSELEAVE );
	if ( bMouse1stIn ) {
		TRACKMOUSEEVENT tme;

		tme.cbSize = sizeof( TRACKMOUSEEVENT );
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = pWP->hwnd;

		::TrackMouseEvent( &tme );

		m_uBtnStateFlags |= BTN_FLAG_TRACEMOUSELEAVE;
	}

	if ( pWP->wParam == MK_LBUTTON ) {
		int a = 0;
	} else {
		if ( bMouse1stIn ) {
			hDC = ::GetDC( pWP->hwnd );
			_doDrawSkin( hDC, rcKeelObj, BTN_STATE_HOVER );
			::ReleaseDC( pWP->hwnd, hDC );
		}
	}
}
//
void hm_wmp_button::_OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcKeelObj;
	HDC hDC;

	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	//cancel track mouse leave message
	if ( !(m_uBtnStateFlags & BTN_FLAG_TRACEMOUSELEAVE) )
		return;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof( TRACKMOUSEEVENT );
	tme.dwFlags = TME_LEAVE|TME_CANCEL;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = pWP->hwnd;

	::TrackMouseEvent( &tme );
	
	m_uBtnStateFlags &= ~BTN_FLAG_TRACEMOUSELEAVE;
	//
	if ( m_uBtnStateFlags & BTN_FLAG_LBTNDOWN ) {
		::ReleaseCapture();
		m_uBtnStateFlags &= ~BTN_FLAG_LBTNDOWN;
	}
	//
	hDC = ::GetDC( pWP->hwnd );
	_doDrawSkin( hDC, rcKeelObj, BTN_STATE_NORMAL );
	::ReleaseDC( pWP->hwnd, hDC );
}
//
void hm_wmp_button::_OnSetText( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{}
//
void hm_wmp_button::_OnGetText( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{}
//
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "win\paint_system\hm_ps_drawtool_gdi.h"
void hm_wmp_button::_doDrawSkin( HDC hDC, const RECT& rcCanvas, ENUMBTNSTATE eBtnState )
{
	switch ( eBtnState ) {
	case BTN_STATE_NORMAL:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_NOR, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		} 
		break;
	case BTN_STATE_HOVER:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_HOV, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case BTN_STATE_PRESS:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_PRE, &pDistrictObj ) )
				return;
			pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		}
		break;
	case BTN_STATE_DISABLE:
		{
			hm_districtobj* pDistrictObj = NULL;

			if ( !_getDistrictObj( EDISTRICTID_BK_DIS, &pDistrictObj ) )
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

	//strText = s_aryState[ eBtnState ];
	//::DrawText( hDC, strText.c_str(), strText.length(), (RECT*)&rcCanvas, DT_CENTER );
}
//
hm_wmp_button::ENUMBTNSTATE hm_wmp_button::_getCurState() {
	if ( m_uBtnStateFlags & BTN_FLAG_DISABLE ) {
		return BTN_STATE_DISABLE;
	} else if ( m_uBtnStateFlags & BTN_FLAG_LBTNDOWN ) {
		return BTN_STATE_PRESS;
	} else if ( m_uBtnStateFlags & BTN_FLAG_TRACEMOUSELEAVE ) {
		return BTN_STATE_HOVER;
	} else {
		return BTN_STATE_NORMAL;
	}
}
//
void hm_wmp_button::_OnRButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	if ( !::PtInRect( &rcKeelObj, ptClient ) )
		return;
	// eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;

}
//
void hm_wmp_button::_OnRButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	if ( !::PtInRect( &rcKeelObj, ptClient ) )
		return;
	// eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
}

void hm_wmp_button::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_BK_NOR );
	containerIds.push_back( EDISTRICTID_BK_HOV );
	containerIds.push_back( EDISTRICTID_BK_PRE );
	containerIds.push_back( EDISTRICTID_BK_DIS );
}