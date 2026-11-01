#include "stdafx.h"
#include "win\frame\hm_layout_obj.h"
//////////////////////////////////////////////////////////
static bool ClientToWnd( HWND hWnd, RECT* prcClt2Wnd );
static bool WndToClient( HWND hWnd, RECT* prcWnd2Clt );
//
hm_layout_obj::hm_layout_obj()
: m_pLayoutDecoder( NULL )
, m_pCoordinateSystem( NULL )
{}
//
hm_layout_obj::~hm_layout_obj()
{
	if ( m_pLayoutDecoder )
		delete m_pLayoutDecoder;
	m_pLayoutDecoder = NULL;
	
	if ( m_pCoordinateSystem )
		m_pCoordinateSystem->UnRegister( this, HM_EVENT_COORDSYS_CHANGE );
	m_pCoordinateSystem = NULL;


}
//
void hm_layout_obj::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue )
{
	switch( idEvent )
	{
	case cmn::ID_EVENT_POSCHANGED:
		{
			//the observed subject's position been changed, so the observer need to recalculate himself's position
			calcPosition();
		}
		break;
	case HM_EVENT_COORDSYS_CHANGE:
		{
			calcPosition();
		}
		break;
	default:
		break;
	}	
}

void hm_layout_obj::calcPosition()
{
	if( !m_pLayoutDecoder )
		return;
	//
	m_pLayoutDecoder->decoderLayout();
	// 
	Notify( cmn::ID_EVENT_POSCHANGED );
}

bool hm_layout_obj::mapPosition( HWND hWndDest, coordinate_system_hwnd::ENUMCSHTYPE eType, RECT& rcDest )
{
	if ( !m_pCoordinateSystem )
		return false;
	RECT rcMap2Dest;

	// map m_rect from hWndSrc into hWndDest's CLIENT area.
	if ( m_pCoordinateSystem->getHWnd() == hWndDest ) {
		rcMap2Dest = m_rect;
		if ( m_pCoordinateSystem->getType() == coordinate_system_hwnd::CSH_WINDOW_TYPE ) {
			if ( !WndToClient( hWndDest, &rcMap2Dest ) ) return false;
		}
	} else if ( m_pCoordinateSystem->getType() == coordinate_system_hwnd::CSH_CLIENT_TYPE ) {
		// client -> client: MapWindowPoints is NC-aware. window-rect
		// deltas ( the old way ) mis-place by the non-client height
		// whenever the two windows frame differently.
		rcMap2Dest = m_rect;
		if ( ::MapWindowPoints( m_pCoordinateSystem->getHWnd(), hWndDest, (POINT*)&rcMap2Dest, 2 ) == 0 )
			return false;
	} else {
		// window -> window offset, then into the client area.
		RECT rcScreenSrc;
		RECT rcScreenDest;
		int x;
		int y;

		if ( !::GetWindowRect( m_pCoordinateSystem->getHWnd(), &rcScreenSrc ) )
			return false;
		if ( !::GetWindowRect( hWndDest, &rcScreenDest ) )
			return false;
		//
		x = rcScreenSrc.left - rcScreenDest.left;
		y = rcScreenSrc.top - rcScreenDest.top;
		//
		rcMap2Dest = m_rect;
		rcMap2Dest.left += x;
		rcMap2Dest.right += x;
		rcMap2Dest.top += y;
		rcMap2Dest.bottom += y;
		if ( !WndToClient( hWndDest, &rcMap2Dest ) ) return false;
	}
	// dest CLIENT -> requested eType
	rcDest = rcMap2Dest;
	if ( eType == coordinate_system_hwnd::CSH_WINDOW_TYPE ) {
		if ( !ClientToWnd( hWndDest, &rcDest ) ) return false;
	}

	return true;
}

bool hm_layout_obj::screen2LayoutObj( POINT& ptS2L ) 
{
	if ( !m_pCoordinateSystem )
		return false;
	if ( !m_pCoordinateSystem->getHWnd() )
		return true; // ptScreen = ptScreen
	
	switch ( m_pCoordinateSystem->getType() ) {
	case coordinate_system_hwnd::CSH_CLIENT_TYPE:
		{
			return (::ScreenToClient( m_pCoordinateSystem->getHWnd(), &ptS2L ) == TRUE);
		}
		break;
	case coordinate_system_hwnd::CSH_WINDOW_TYPE:
		{
			RECT rcWnd;

			if ( !::GetWindowRect( m_pCoordinateSystem->getHWnd(), &rcWnd ) )
				return false;
			ptS2L.x -= rcWnd.left;
			ptS2L.y -= rcWnd.top;
			return true;
		}
		break;
	default:
		return false;
		break;
	}

	return false;
}

void hm_layout_obj::setCoordinateSystem( coordinate_system_ptr_type pCoordinateSystem )
{ 
	//unregister 
	if ( m_pCoordinateSystem ) {
		m_pCoordinateSystem->UnRegister( this, HM_EVENT_COORDSYS_CHANGE );
	}
	
	m_pCoordinateSystem = pCoordinateSystem; 

	//register
	if ( m_pCoordinateSystem ) {
		m_pCoordinateSystem->Register( this, HM_EVENT_COORDSYS_CHANGE );
	}
}


void hm_layoutdecoder_i::setLayoutRefObj( const ref_obj_ptr_type pRefObj ) { 
	if ( m_pRefObj == pRefObj ) return;
	// cancel old reference relationship
	if ( m_pRefObj ) {
		m_pRefObj->UnRegister( m_pOutObj, cmn::ID_EVENT_POSCHANGED );
	}
	m_pRefObj = pRefObj;
	// create new reference relationship
	if ( m_pRefObj ) {
		m_pRefObj->Register( m_pOutObj, cmn::ID_EVENT_POSCHANGED );
	}
}
//
bool ClientToWnd( HWND hWnd, RECT* prcClt2Wnd )
{
	// window = client + ( clientOrigin - windowOrigin ), both origins in
	// screen space: a uniform shift independent of the INPUT rect ( the old
	// body fed the rect topleft into ClientToScreen, which cancelled or
	// doubled the input coordinates ).
	if ( !prcClt2Wnd ) return false;
	POINT ptCltLT = { 0, 0 };
	RECT rcWnd;
	if ( !::GetWindowRect( hWnd, &rcWnd ) )
		return false;
	::ClientToScreen( hWnd, &ptCltLT );
	OffsetRect( prcClt2Wnd, ptCltLT.x - rcWnd.left, ptCltLT.y - rcWnd.top );
	return true;
}
//
bool WndToClient( HWND hWnd, RECT* prcWnd2Clt )
{
	// client = window - ( clientOrigin - windowOrigin )
	if ( !prcWnd2Clt ) return false;
	POINT ptCltLT = { 0, 0 };
	RECT rcWnd;
	if ( !::GetWindowRect( hWnd, &rcWnd ) )
		return false;
	::ClientToScreen( hWnd, &ptCltLT );
	OffsetRect( prcWnd2Clt, rcWnd.left - ptCltLT.x, rcWnd.top - ptCltLT.y );
	return true;
}