#include "stdafx.h"
#include "win\wmp\hm_wmp_checkbox.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_layoutobj_defs.h"
#include "win\hm_event_defs.h"
#include "hmskin_event_defs.h"
#include <assert.h>

hm_wmp_checkbox::hm_wmp_checkbox( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_uFlag( HM_CHECK_FLAG_NONE )
, m_eCheckStatus( ENOCHECK_STATUS_NORMAL ) {
	// click-rate control: fresh buffer bitmap per interaction draw.
	m_eBufStrategy = BUF_ALLOC;
	_createDistrictObjs();
}

hm_wmp_checkbox::~hm_wmp_checkbox() {

}

MSGBIND_BEGIN( hm_wmp_checkbox )
	MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
	MSGBIND( WM_LBUTTONUP, _OnLButtonUp )
	MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
	MSGBIND( WM_MOUSELEAVE, _OnMouseLeave )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_checkbox )
	ORDERWNDMSG( WM_LBUTTONDOWN )
	ORDERWNDMSG( WM_LBUTTONUP )
	ORDERWNDMSG( WM_MOUSEMOVE )
	ORDERWNDMSG( WM_MOUSELEAVE )
ORDERWNDMSG_END()

void hm_wmp_checkbox::_createDistrictObjs() {
	hm_districtobj* pDistrictObj = NULL;

	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_CHECK_NOR, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_CHECK_HOV, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_CHECK_PRE, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_CHECK_DIS, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_NOCHECK_NOR, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_NOCHECK_HOV, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_NOCHECK_PRE, pDistrictObj );
	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK_NOCHECK_DIS, pDistrictObj );
}

void hm_wmp_checkbox::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue )
{
	switch ( idEvent ) {
	case HMKEELOBJEVENT_DRAWSELF:
	{
		HEDSPARAM* pHEDSParam = NULL;
		RECT rcCanvas;

		pHEDSParam = ( HEDSPARAM* )pEventParam;
		if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
			return;

		// paint path: the base funnel detects the memory DC and draws direct.
		_drawWithStrategy( pHEDSParam->m_hDCCanvas, rcCanvas );
	}
		break;
	default:
		break;
	}
}

void hm_wmp_checkbox::_render( HDC hDC, const RECT& rcCanvas ) {
	hm_districtobj* pDistrictObj = NULL;
	EDISTRICTID eDistricId;

	eDistricId = _getCurDistrictId();
	if ( !_getDistrictObj( eDistricId, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
}

void hm_wmp_checkbox::_OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	RECT rcKeelObj;
	POINT ptClient;
	hm_layout_obj* pLayoutObj = NULL;
	
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );

	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj )
		return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	if ( !::PtInRect( &rcKeelObj, ptClient ) )
		return;

	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC|EATMSGFLAG_DISABLE_OTHER_MSGSINK|EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;

	//
	if ( m_eCheckStatus == ECHECK_STATUS_DISABLE )
		return;
	if ( m_eCheckStatus == ENOCHECK_STATUS_DISABLE )
		return;
	m_uFlag |= HM_CHECK_FLAG_LBTDOWN;
	if ( m_eCheckStatus == ECHECK_STATUS_HOVER ){
		m_eCheckStatus = ECHECK_STATUS_PRESS;
	} else if ( m_eCheckStatus == ENOCHECK_STATUS_HOVER ) {
		m_eCheckStatus = ENOCHECK_STATUS_PRESS;
	} else if ( m_eCheckStatus == ECHECK_STATUS_NORMAL ) {
		m_eCheckStatus = ECHECK_STATUS_PRESS;
	} else if ( m_eCheckStatus == ENOCHECK_STATUS_NORMAL ) {
		m_eCheckStatus = ENOCHECK_STATUS_PRESS;
	} else {
		assert( false );
	}
	//
	if ( ::GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	//
	drawSelf( pKeelObj, pWP->hwnd );
}

void hm_wmp_checkbox::_OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	RECT rcKeelObj;
	POINT ptClient;
	unsigned int uFlagOld;
	hm_layout_obj* pLayoutObj = NULL;

	uFlagOld = m_uFlag;
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj )
		return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	if ( !::PtInRect( &rcKeelObj, ptClient ) ) {
		if ( uFlagOld & HM_CHECK_FLAG_LBTDOWN ) {
			m_uFlag &= ~HM_CHECK_FLAG_LBTDOWN;
		}
		return;
	}
	if ( uFlagOld & HM_CHECK_FLAG_LBTDOWN ) {
		::ReleaseCapture();
	}
	m_uFlag &= ~HM_CHECK_FLAG_LBTDOWN;
	if ( ( m_eCheckStatus == ECHECK_STATUS_PRESS ) || ( m_eCheckStatus == ENOCHECK_STATUS_HOVER ) || ( m_eCheckStatus == ENOCHECK_STATUS_NORMAL ) ){
		// radio subclass keeps the item checked ( _keepCheckedOnClick ):
		// unchecking here + re-checking from the app would leave the wrong
		// state on screen for one paint cycle ( visible flash ).
		m_eCheckStatus = _keepCheckedOnClick() ? ECHECK_STATUS_HOVER : ENOCHECK_STATUS_HOVER;
	} else if ( ( m_eCheckStatus == ENOCHECK_STATUS_PRESS ) || ( m_eCheckStatus == ECHECK_STATUS_HOVER ) || ( m_eCheckStatus == ECHECK_STATUS_NORMAL ) ){
		m_eCheckStatus = ECHECK_STATUS_HOVER;
	} else{
		assert( false );
	}
	//
	drawSelf( pKeelObj, pWP->hwnd );
	// eat message
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	if ( ( ( uFlagOld & HM_CHECK_FLAG_LBTDOWN ) == HM_CHECK_FLAG_LBTDOWN ) && ::PtInRect( &rcKeelObj, ptClient ) ) {
		if ( m_pEventListener ) {
			m_pEventListener->onHKSkinEvent( _getClickEventId(), (void*)pKeelObj, (void*)(UINT_PTR)isCheck() );
		}
		////notify parent
		WPARAM wParam = MAKEWPARAM( GetDlgCtrlID( pWP->hwnd ), HM_CHECK_CMDEVT_DOWN );
		LPARAM lParam = ( LPARAM )isCheck();
		::SendMessage( ::GetParent( pWP->hwnd ), WM_COMMAND, wParam, lParam );
	}
}

void hm_wmp_checkbox::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	RECT rcKeelObj;
	POINT ptClient;
	bool bMouse1stIn;
	unsigned int uFlagOld;
	hm_layout_obj* pLayoutObj = NULL;

	uFlagOld = m_uFlag;
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj )
		return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	//
	if ( !::PtInRect( &rcKeelObj, ptClient ) ) {
		if ( m_uFlag & HM_CHECK_FLAG_TRACEMOSELEAVE ) {
			if ( m_uFlag & HM_CHECK_FLAG_LBTDOWN ) {
				if ( m_eCheckStatus == ECHECK_STATUS_PRESS ) {
					m_eCheckStatus = ECHECK_STATUS_NORMAL;
				} else if ( m_eCheckStatus == ENOCHECK_STATUS_PRESS ) {
					m_eCheckStatus = ENOCHECK_STATUS_NORMAL;
				} else {
					assert( false );
				}
			} else {
				if ( m_eCheckStatus == ECHECK_STATUS_HOVER ) {
					m_eCheckStatus = ECHECK_STATUS_NORMAL;
				} else if ( m_eCheckStatus == ENOCHECK_STATUS_HOVER ) {
					m_eCheckStatus = ENOCHECK_STATUS_NORMAL;
				} else {
					assert( false );
				}
			}

			//
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE|TME_CANCEL;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = pWP->hwnd;

			::TrackMouseEvent( &tme );

			m_uFlag &= ~HM_CHECK_FLAG_TRACEMOSELEAVE;
			//
			drawSelf( pKeelObj, pWP->hwnd );
		}

		return;
	}
	//
	bMouse1stIn = !( m_uFlag & HM_CHECK_FLAG_TRACEMOSELEAVE );
	if ( bMouse1stIn ) {
		//
		if ( m_eCheckStatus == ECHECK_STATUS_DISABLE )
			return;
		if ( m_eCheckStatus == ENOCHECK_STATUS_DISABLE )
			return;

		switch ( m_eCheckStatus ) {
			case ENOCHECK_STATUS_NORMAL:
			{
				if ( m_uFlag & HM_CHECK_FLAG_LBTDOWN ) {
					m_eCheckStatus = ENOCHECK_STATUS_PRESS;
				} else {
					m_eCheckStatus = ENOCHECK_STATUS_HOVER;
				}
			}
				break;
			case ECHECK_STATUS_NORMAL:
			{
				if ( m_uFlag & HM_CHECK_FLAG_LBTDOWN ) {
					m_eCheckStatus = ECHECK_STATUS_PRESS;
				} else {
					m_eCheckStatus = ECHECK_STATUS_HOVER;
				}
			}
			break;
		default:
			break;
		}
		//
		TRACKMOUSEEVENT tme;

		tme.cbSize = sizeof( TRACKMOUSEEVENT );
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = pWP->hwnd;

		::TrackMouseEvent( &tme );

		m_uFlag |= HM_CHECK_FLAG_TRACEMOSELEAVE;
		//
		drawSelf( pKeelObj, pWP->hwnd );
	}
}

void hm_wmp_checkbox::_OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	RECT rcKeelObj;
	POINT ptClient;
	hm_layout_obj* pLayoutObj = NULL;
	
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj )
		return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	if ( ::PtInRect( &rcKeelObj, ptClient ) )
		return;
	//
	if ( m_eCheckStatus == ECHECK_STATUS_DISABLE )
		return;

	if ( m_eCheckStatus == ENOCHECK_STATUS_DISABLE )
		return;

	if ( ( m_eCheckStatus == ENOCHECK_STATUS_HOVER ) || ( m_eCheckStatus == ENOCHECK_STATUS_PRESS ) ){
		m_eCheckStatus = ENOCHECK_STATUS_NORMAL;
	} else if ( ( m_eCheckStatus == ECHECK_STATUS_HOVER ) || ( m_eCheckStatus == ECHECK_STATUS_PRESS ) ){
		m_eCheckStatus = ECHECK_STATUS_NORMAL;
	} else if ( m_eCheckStatus == ENOCHECK_STATUS_NORMAL ) {
		m_eCheckStatus = ENOCHECK_STATUS_NORMAL;
	} else if ( m_eCheckStatus == ECHECK_STATUS_NORMAL ) {
		m_eCheckStatus = ECHECK_STATUS_NORMAL;
	}
	else{
		assert( false );
	}
	//cancel track mouse leave message
	if ( !( m_uFlag & HM_CHECK_FLAG_TRACEMOSELEAVE ) )
		return;
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof( TRACKMOUSEEVENT );
	tme.dwFlags = TME_LEAVE|TME_CANCEL;
	tme.dwHoverTime = HOVER_DEFAULT;
	tme.hwndTrack = pWP->hwnd;

	::TrackMouseEvent( &tme );

	m_uFlag &= ~HM_CHECK_FLAG_TRACEMOSELEAVE;
	//
	if ( m_uFlag & HM_CHECK_FLAG_LBTDOWN ) {
		::ReleaseCapture();
		m_uFlag &= ~HM_CHECK_FLAG_LBTDOWN;
	}
	m_uFlag &= ~HM_CHECK_FLAG_LBTDOWN;
	//
	drawSelf( pKeelObj, pWP->hwnd );
}

void hm_wmp_checkbox::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_BK_NOCHECK_DIS );
	containerIds.push_back( EDISTRICTID_BK_NOCHECK_PRE );
	containerIds.push_back( EDISTRICTID_BK_NOCHECK_HOV );
	containerIds.push_back( EDISTRICTID_BK_NOCHECK_NOR );
	containerIds.push_back( EDISTRICTID_BK_CHECK_DIS );
	containerIds.push_back( EDISTRICTID_BK_CHECK_PRE );
	containerIds.push_back( EDISTRICTID_BK_CHECK_HOV );
	containerIds.push_back( EDISTRICTID_BK_CHECK_NOR );
}

hm_wmp_checkbox::ECHECKBOXSTATUS hm_wmp_checkbox::_getCurState() {
	return m_eCheckStatus;
}

hm_wmp_checkbox::EDISTRICTID hm_wmp_checkbox::_getCurDistrictId() {
	switch ( m_eCheckStatus ) {
		case ENOCHECK_STATUS_NORMAL:
			{
				return EDISTRICTID_BK_NOCHECK_NOR;
			}
			break;
		case ENOCHECK_STATUS_HOVER:
			{
				return EDISTRICTID_BK_NOCHECK_HOV;
			}
			break;
		case ENOCHECK_STATUS_PRESS:
			{
				return EDISTRICTID_BK_NOCHECK_PRE;
			}
			break;
		case ENOCHECK_STATUS_DISABLE:
			{
				return EDISTRICTID_BK_NOCHECK_DIS;
			}
			break;
		case ECHECK_STATUS_NORMAL:
			{
				return EDISTRICTID_BK_CHECK_NOR;
			}
			break;
		case ECHECK_STATUS_HOVER:
			{
				return EDISTRICTID_BK_CHECK_HOV;
			}
			break;
		case ECHECK_STATUS_PRESS:
			{
				return EDISTRICTID_BK_CHECK_PRE;
			}
			break;
		case ECHECK_STATUS_DISABLE:
			{
				return EDISTRICTID_BK_CHECK_DIS;
			}
			break;
		default:
			break;
	}
	assert( false );
	return EDISTRICTID_BK_INVALID;
}

bool hm_wmp_checkbox::isCheck(){

	return ( ( m_eCheckStatus == ECHECK_STATUS_DISABLE ) || ( m_eCheckStatus == ECHECK_STATUS_HOVER ) ||
		( m_eCheckStatus == ECHECK_STATUS_NORMAL ) || ( m_eCheckStatus == ECHECK_STATUS_PRESS ) );
}

bool hm_wmp_checkbox::setCheck( BOOL bCheck ) {
	int nSubState;

	// keep the current interaction sub-state ( NOR/HOV/PRE ), only cross
	// between the check/nocheck family. disable states are left untouched.
	switch ( m_eCheckStatus ) {
	case ENOCHECK_STATUS_NORMAL:
	case ECHECK_STATUS_NORMAL:
		nSubState = 0;
		break;
	case ENOCHECK_STATUS_HOVER:
	case ECHECK_STATUS_HOVER:
		nSubState = 1;
		break;
	case ENOCHECK_STATUS_PRESS:
	case ECHECK_STATUS_PRESS:
		nSubState = 2;
		break;
	default:
		return false;
		break;
	}
	m_eCheckStatus = ( ECHECKBOXSTATUS )( ( bCheck ? ECHECK_STATUS_NORMAL : ENOCHECK_STATUS_NORMAL ) + nSubState );

	// broadcast the state change: the owning keelobj subscribed at bind
	// time and invalidates its own rect. this WMP never learns about
	// keelobjs - callers ( e.g. radio groups unchecking siblings ) never
	// touch invalidation.
	Notify( HM_EVENT_WMP_STATECHANGED );

	return true;
}
