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
	RECT rcMap2WndDest;
	
	// map m_rect from hWndSrc to hWndDest.
	if ( m_pCoordinateSystem->getHWnd() == hWndDest ) {
		rcMap2WndDest = m_rect;
	} else {
		HWND hWndSrc;
		RECT rcScreenSrc;
		RECT rcScreenDest;
		int x;
		int y;
		
		hWndSrc = m_pCoordinateSystem->getHWnd();
		if ( !::GetWindowRect( hWndSrc, &rcScreenSrc ) )
			return false;
		if ( !::GetWindowRect( hWndDest, &rcScreenDest ) )
			return false;
		//
		x = rcScreenSrc.left - rcScreenDest.left;
		y = rcScreenSrc.top - rcScreenDest.top;
		//
		rcMap2WndDest.left = m_rect.left + x;
		rcMap2WndDest.right = m_rect.right + x;
		rcMap2WndDest.top = m_rect.top + y;
		rcMap2WndDest.bottom = m_rect.bottom + y;
	}
	// CSH_CLIENT_TYPE -> CSH_CLIENT_TYPE || CSH_WINDOW_TYPE -> CSH_WINDOW_TYPE
	if ( m_pCoordinateSystem->getType() == eType ) {
		rcDest = rcMap2WndDest;
	} else if ( m_pCoordinateSystem->getType() == coordinate_system_hwnd::CSH_CLIENT_TYPE ) { // CSH_CLIENT_TYPE -> CSH_WINDOW_TYPE
		if ( !ClientToWnd( hWndDest, &rcMap2WndDest ) ) return false;
		rcDest = rcMap2WndDest;
	} else { // CSH_WINDOW_TYPE -> CSH_CLIENT_TYPE
		if ( !WndToClient( hWndDest, &rcMap2WndDest ) ) return false;
		rcDest = rcMap2WndDest;
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
	if ( !prcClt2Wnd ) return false;
	RECT rcWnd;
	POINT ptCltLT;
	RECT rcDest;
	
	//
	ptCltLT.x = prcClt2Wnd->left;
	ptCltLT.y = prcClt2Wnd->top;
	::ClientToScreen( hWnd, &ptCltLT );
	::GetWindowRect( hWnd, &rcWnd );
	//
	rcDest.left = prcClt2Wnd->left - ( rcWnd.left - ptCltLT.x );
	rcDest.top = prcClt2Wnd->top - ( rcWnd.top - ptCltLT.y );
	rcDest.right = rcDest.left + ( prcClt2Wnd->right - prcClt2Wnd->left );
	rcDest.bottom = rcDest.top + ( prcClt2Wnd->bottom - prcClt2Wnd->top );
	//
	*prcClt2Wnd = rcDest;
	return true;
}
//
bool WndToClient( HWND hWnd, RECT* prcWnd2Clt )
{
	if ( !prcWnd2Clt ) return false;
	RECT rcWnd;
	POINT ptCltLT;
	RECT rcDest;
	
	//
	ptCltLT.x = prcWnd2Clt->left;
	ptCltLT.y = prcWnd2Clt->top;
	::ClientToScreen( hWnd, &ptCltLT );
	::GetWindowRect( hWnd, &rcWnd );
	//
	rcDest.left = prcWnd2Clt->left + ( rcWnd.left - ptCltLT.x );
	rcDest.top = prcWnd2Clt->top + ( rcWnd.top - ptCltLT.y );
	rcDest.right = rcDest.left + ( prcWnd2Clt->right - prcWnd2Clt->left );
	rcDest.bottom = rcDest.top + ( prcWnd2Clt->bottom - prcWnd2Clt->top );
	//
	*prcWnd2Clt = rcDest;
	return true;
}	