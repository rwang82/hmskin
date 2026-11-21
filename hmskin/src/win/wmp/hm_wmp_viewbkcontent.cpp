#include "stdafx.h"
#include "win\wmp\hm_wmp_viewbkcontent.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hook_system\hm_hs_whellengine.h"
#include "win\hm_win_globals.h"
#include "win\hm_wndmsg_defs.h"
#include "win\hm_event_defs.h"

hm_wmp_viewbkcontent::hm_wmp_viewbkcontent( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_hWndCanvas( NULL )
, m_uFlag( 0 )
{
	_createDistrictObjs();
}
//
hm_wmp_viewbkcontent::~hm_wmp_viewbkcontent() {
	
}
//
MSGBIND_BEGIN( hm_wmp_viewbkcontent )
	MSGBIND( WM_MOUSEWHEEL, _OnMouseWhell )
	MSGBIND( HMWM_MOUSEWHEEL, _OnHMMouseWhell )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_viewbkcontent )
	ORDERWNDMSG( WM_MOUSEWHEEL )
	ORDERWNDMSG( HMWM_MOUSEWHEEL )
ORDERWNDMSG_END()
//
void hm_wmp_viewbkcontent::_createDistrictObjs() {
	
}
//
void hm_wmp_viewbkcontent::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent ) {
	case HM_EVENT_LAYOUT_POSCHANGED:
		{
			int a = 0;					  
		}
		break;
	case HM_EVENT_DRAWSELF:
		{
			HEDSPARAM* pParam = NULL;
			RECT rcCanvas;
			
			pParam = (HEDSPARAM*)pEventParam;
			if ( !pParam->m_pLayoutObjCanvas->mapPosition( pParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
				return;
			// save m_hWndCanvas
			m_hWndCanvas = pParam->m_hWndCanvas;
			
			// draw 
			_doDrawSkinRaw( pParam->m_hDCCanvas, rcCanvas );

		}
		break;
	case HM_EVENT_MOVE_VIEWCONTENT:
		{
			// pure notification - nothing to do.
		}
		break;
	default:
		break;
	}
}

void hm_wmp_viewbkcontent::_doDrawSkinRaw( HDC hDC, const RECT& rcCanvas ) {
	COLORREF clrBK = RGB( 120,20,20 );

	HBRUSH hBrush = ::CreateSolidBrush( clrBK );
	::FillRect( hDC, &rcCanvas, hBrush );
	::DeleteObject( hBrush );


}

void hm_wmp_viewbkcontent::_mapRectContentVisible2Canvas( const RECT& rcContentVisible, const RECT& rcCanvas, RECT& rcContentV2Canvas ) {
	if ( RECT_WIDTH( rcContentVisible ) >= RECT_WIDTH( rcCanvas ) ) {
		rcContentV2Canvas.left = rcCanvas.left;
		rcContentV2Canvas.right = rcCanvas.right;
	} else {
		rcContentV2Canvas.left = rcCanvas.left + ( RECT_WIDTH( rcCanvas ) - RECT_WIDTH( rcContentVisible ) ) / 2;
		rcContentV2Canvas.right = rcContentV2Canvas.left + RECT_WIDTH( rcContentVisible );
	}
	//
	if ( RECT_HEIGHT( rcContentVisible ) >= RECT_HEIGHT( rcCanvas ) ) {
		rcContentV2Canvas.top = rcCanvas.top;
		rcContentV2Canvas.bottom = rcCanvas.bottom;
	} else {
		rcContentV2Canvas.top = rcCanvas.top + ( RECT_HEIGHT( rcCanvas ) - RECT_HEIGHT( rcContentVisible ) ) / 2;
		rcContentV2Canvas.bottom = rcContentV2Canvas.top + RECT_HEIGHT( rcContentVisible );
	}

}

void hm_wmp_viewbkcontent::_OnMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	int zDelta = GET_WHEEL_DELTA_WPARAM( pWP->wParam );
}

void hm_wmp_viewbkcontent::_OnHMMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	unsigned int uNoTriggerTagCount;
	wtd_container_type* pWTDContainer = NULL;
	int nDistance;
	
	uNoTriggerTagCount = (unsigned int)pWP->wParam;
	pWTDContainer = (wtd_container_type*)pWP->lParam;
	nDistance = WHELLHELPER_INSTANCE->getDistancePixel( uNoTriggerTagCount, pWTDContainer, 2000 );
	if ( nDistance == 0 ) {
		if ( this->m_uFlag & HMVBKC_FLAG_WHELLSTOPED )
			return;
	} else {
		// remove HMVBKC_FLAG_WHELLSTOPED flag
		m_uFlag &= ~HMVBKC_FLAG_WHELLSTOPED;
	}
	
	this->Notify( HM_EVENT_MOUSEWHEEL, (LPARAM)nDistance );
	
	if ( nDistance == 0 ) {
		this->m_uFlag |= HMVBKC_FLAG_WHELLSTOPED;
		WHELLENGINE_INSTANCE->pauseTriggerThread();
	}
}