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
	case cmn::ID_EVENT_POSCHANGED:
		{
			int a = 0;					  
		}
		break;
	case HMKEELOBJEVENT_DRAWSELF:
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
			HEVMPARAM* pParam = NULL;
			RECT rcCanvas;

			pParam = (HEVMPARAM*)pEventParam;
			rcCanvas.left = 0;
			rcCanvas.top = 0;
			rcCanvas.right = pParam->m_sizeCanvas.cx;
			rcCanvas.bottom = pParam->m_sizeCanvas.cy;
			_doDrawSkinRaw( pParam->m_hDCMem, rcCanvas );
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


#if 0
	COLORREF clrBK = RGB( 120,20,20 );
	TCHAR szMsg[ 200 ];
	RECT rcDrawText;
	RECT rcCV2C;
	HDC hMemDC;
	HBITMAP hBitmap, hBitmapOld;
	HBRUSH hBrush;

	rcDrawText = rcCanvas;
	hm_util_draw_rect( hDC, rcCanvas, clrBK );
	_stprintf_s( szMsg, _T("m_uContentHeight: %d, m_uContentWidth: %d\nm_rcContentVisible  left:%d, right:%d, top:%d, bottom:%d\n"), 
		m_uContentHeight, m_uContentWidth, m_rcContentVisible.left, m_rcContentVisible.right, m_rcContentVisible.top, m_rcContentVisible.bottom );
	::DrawText( hDC, szMsg, -1, &rcDrawText, DT_CENTER );

	_mapRectContentVisible2Canvas( m_rcContentVisible, rcCanvas, rcCV2C );
	
	//
	hMemDC = ::CreateCompatibleDC( hDC );
	hBitmap = ::CreateCompatibleBitmap( hDC, (int)m_uContentWidth, (int)m_uContentHeight );
	hBitmapOld = ( HBITMAP )::SelectObject( hMemDC, hBitmap );

	hBrush = ::CreateSolidBrush( RGB( 120, 130, 140 ) );
	::FillRect( hMemDC, &m_rcContentVisible, hBrush );
	::DeleteObject( hBrush );
	::MoveToEx( hMemDC, 0, 0, NULL );
	::LineTo( hMemDC, m_uContentWidth, m_uContentHeight );
	::MoveToEx( hMemDC, m_uContentWidth, 0, NULL );
	::LineTo( hMemDC, 0, m_uContentHeight );
	::BitBlt( hDC, rcCV2C.left, rcCV2C.top, RECT_WIDTH( m_rcContentVisible ), RECT_HEIGHT( m_rcContentVisible )
		, hMemDC, m_rcContentVisible.left, m_rcContentVisible.top, SRCCOPY );

	::SelectObject( hMemDC, hBitmapOld );
	::DeleteObject( hBitmap );
	::DeleteDC( hMemDC );

	//
	::MoveToEx( hDC, rcCV2C.left, rcCV2C.top, NULL );
	::LineTo( hDC, rcCV2C.left, rcCV2C.bottom - 1 );
	::LineTo( hDC, rcCV2C.right - 1, rcCV2C.bottom - 1 );
	::LineTo( hDC, rcCV2C.right - 1, rcCV2C.top );
	::LineTo( hDC, rcCV2C.left, rcCV2C.top );
#endif //0
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
	
	uNoTriggerTagCount = pWP->wParam;
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