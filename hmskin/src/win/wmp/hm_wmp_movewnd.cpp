#include "stdafx.h"
#include "win\wmp\hm_wmp_movewnd.h"
#include "common\hm_cmn_envcfg.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_uiroot.h"
#include "win\resource_system\hm_rs_wndmark.h"
#include "win\hm_event_defs.h"
#include <assert.h>

hm_wmp_movewnd::hm_wmp_movewnd( hmskin_event_listener_base* pEventListener )
: hm_wndmsgprocessor_i( pEventListener ) {

}

hm_wmp_movewnd::~hm_wmp_movewnd() {

}

void hm_wmp_movewnd::doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {

}

void hm_wmp_movewnd::getWndMsgs( msg_container_type& containerWndMsg ) {

}

void hm_wmp_movewnd::_OnHMKEELOBJEVENT_POSCHANGED( void* pSubject, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	if ( !m_pHMUIRoot )
		return;
	HWND hWndDest = NULL;
	hm_keel_obj* pKeelObj = NULL;
	hm_layout_obj* pLayoutObj = NULL;
	HWND hWndDeskTop;
	HWND hWndDestParent = NULL;
	RECT rcDest;

	hWndDest = m_pHMUIRoot->m_pHWNDDespository->wndMark2HWND( m_wndMark );
	if ( !hWndDest ) {
		assert( FALSE );
		return;
	}
	pKeelObj = (hm_keel_obj*)pEventParam;
	pLayoutObj = pKeelObj->getLayoutObj();
	hWndDeskTop = ::GetDesktopWindow();
	if ( hWndDest == hWndDeskTop ) {
		assert( FALSE );
		return;
	}
	hWndDestParent = ::GetParent( hWndDest );
	if ( !hWndDestParent ) {
		if ( !hWndDeskTop ) {
			assert( FALSE );
			return;
		}	
		hWndDestParent = hWndDeskTop;
	}
	if ( !pLayoutObj->mapPosition( hWndDestParent, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcDest ) ) {
		assert( FALSE );
		return;
	}
	//
	::MoveWindow( hWndDest, rcDest.left, rcDest.top, ( rcDest.right - rcDest.left ), ( rcDest.bottom - rcDest.top ), TRUE );
}

void hm_wmp_movewnd::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent )
	{
	case HMKEELOBJEVENT_POSCHANGED:
		{
			_OnHMKEELOBJEVENT_POSCHANGED( pSubject, pEventParam, pEventParam2, pbContinue );
		}
		break;
	default:
		break;
	}
}

void hm_wmp_movewnd::setUIRoot( hm_uiroot* pHMUIRoot ) {
	m_pHMUIRoot = pHMUIRoot;
}

void hm_wmp_movewnd::setWndMark( const hm_rs_wndmark& wndMark ) {
	m_wndMark = wndMark;
}