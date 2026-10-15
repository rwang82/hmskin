#include "stdafx.h"
#include "win\frame\hm_layout_decoders.h"
#include "win\frame\hm_coordinate_system_hwnd.h"

hm_layoutdecoder_clientarea::hm_layoutdecoder_clientarea( out_obj_ptr_type pOutObj )
: hm_layoutdecoder_i( pOutObj )
{
		
}

hm_layoutdecoder_clientarea::~hm_layoutdecoder_clientarea()
{
	
}

void hm_layoutdecoder_clientarea::decoderLayout()
{
	const coordinate_system_hwnd* pCoordSysHwnd = NULL;
	HWND hWndCoordSys;
	RECT rcClient;
	
	pCoordSysHwnd = ( const coordinate_system_hwnd* )m_pOutObj->getCoordinateSystem();
	hWndCoordSys = pCoordSysHwnd->getHWnd();
	if ( !hWndCoordSys ){
		::OutputDebugString( _T("err 1.\n") );
		return;
	}
	if ( !::GetClientRect( hWndCoordSys, &rcClient ) ) {

		::OutputDebugString( _T("err 2.\n") );
		return;
	}
	//
	setOutObjRect( rcClient );

	//TCHAR szDebug[ 100 ];
	//_stprintf_s( szDebug, _T("l:%d,r:%d,t:%d,b:%d\n"), m_pOutObj->m_rect.left, m_pOutObj->m_rect.right, m_pOutObj->m_rect.top, m_pOutObj->m_rect.bottom );
	//::OutputDebugString( szDebug );
}
///////////////////////////////////////////////////////
hm_layoutdecoder_wndarea::hm_layoutdecoder_wndarea( out_obj_ptr_type pOutObj )
: hm_layoutdecoder_i( pOutObj )
{}

hm_layoutdecoder_wndarea::~hm_layoutdecoder_wndarea()
{}

void hm_layoutdecoder_wndarea::decoderLayout()
{
	const coordinate_system_hwnd* pCoordSysHwnd = NULL;
	HWND hWndCoordSys;
	RECT rcWnd;
	
	pCoordSysHwnd = ( const coordinate_system_hwnd* )m_pOutObj->getCoordinateSystem();
	hWndCoordSys = pCoordSysHwnd->getHWnd();
	if ( !hWndCoordSys )
		return;
	if ( !::GetWindowRect( hWndCoordSys, &rcWnd ) )
		return;
	rcWnd.right -= rcWnd.left;
	rcWnd.bottom -= rcWnd.top;
	rcWnd.left = 0;
	rcWnd.top = 0;
	//
	setOutObjRect( rcWnd );
}
///////////////////////////////////////////////////////
hm_layoutdecoder_corner::hm_layoutdecoder_corner( out_obj_ptr_type pOutObj )
: hm_layoutdecoder_i( pOutObj )
{}
//
hm_layoutdecoder_corner::~hm_layoutdecoder_corner()
{
	
}
//
void hm_layoutdecoder_corner::decoderLayout()
{
	const coordinate_system_hwnd* pCoordSysOutObj = NULL;
	const coordinate_system_hwnd* pCoordSysRefObj = NULL;
	hm_layout_obj::coordinate_system_ptr_type pCoordSysTmp = NULL;
	if ( !m_pOutObj || !m_pRefObj )
		return;
	pCoordSysTmp = m_pOutObj->getCoordinateSystem();
	if ( !pCoordSysTmp ) 
		return;
	pCoordSysOutObj = ( coordinate_system_hwnd* )pCoordSysTmp;
	pCoordSysTmp = m_pRefObj->getCoordinateSystem();
	if ( !pCoordSysTmp ) 
		return;
	pCoordSysRefObj = ( coordinate_system_hwnd* )pCoordSysTmp;
	
	if ( pCoordSysOutObj->isSameSystem( *pCoordSysRefObj ) )
	{
		switch( m_eCornerType ) {
		case CORNER_LT:
			{
				m_pOutObj->m_rect.left = m_pRefObj->m_rect.left + m_space1;
				m_pOutObj->m_rect.right = m_pOutObj->m_rect.left + m_width;
				m_pOutObj->m_rect.top = m_pRefObj->m_rect.top + m_space2;
				m_pOutObj->m_rect.bottom = m_pOutObj->m_rect.top + m_height;
			}
			break;
		case CORNER_LB:
			{
				m_pOutObj->m_rect.left = m_pRefObj->m_rect.left + m_space1;
				m_pOutObj->m_rect.right = m_pOutObj->m_rect.left + m_width;
				m_pOutObj->m_rect.bottom = m_pRefObj->m_rect.bottom - m_space2;
				m_pOutObj->m_rect.top = m_pOutObj->m_rect.bottom - m_height;
			}
			break;
		case CORNER_RB:
			{
				m_pOutObj->m_rect.right = m_pRefObj->m_rect.right - m_space1;
				m_pOutObj->m_rect.left = m_pOutObj->m_rect.right - m_width;
				m_pOutObj->m_rect.bottom = m_pRefObj->m_rect.bottom - m_space2;
				m_pOutObj->m_rect.top = m_pOutObj->m_rect.bottom - m_height;
			}
			break;
		case CORNER_RT:
			{
				m_pOutObj->m_rect.right = m_pRefObj->m_rect.right - m_space2;
				m_pOutObj->m_rect.left = m_pOutObj->m_rect.right - m_width;
				m_pOutObj->m_rect.top = m_pRefObj->m_rect.top + m_space1;
				m_pOutObj->m_rect.bottom = m_pOutObj->m_rect.top + m_height;
			}
			break;
		default:
			break;
		}
	}
	else
	{
		
	}
	
	//	
//	TCHAR szDebug[ 100 ];
//	_stprintf_s( szDebug, _T("l:%d,r:%d,t:%d,b:%d\n"), m_pOutObj->m_rect.left, m_pOutObj->m_rect.right, m_pOutObj->m_rect.top, m_pOutObj->m_rect.bottom );
//	::OutputDebugString( szDebug );
}
///////////////////////////////////////////////////////////////////////////////////
hm_layoutdecoder_dock::hm_layoutdecoder_dock( out_obj_ptr_type pOutObj )
: hm_layoutdecoder_i( pOutObj )
, m_eDockType( DOCK_TOP )
, m_space1( 0 )
, m_space2( 0 )
, m_space3( 0 )
, m_thickness( 0 )
{}
//
hm_layoutdecoder_dock::~hm_layoutdecoder_dock(){}
//
void hm_layoutdecoder_dock::decoderLayout(){
	const coordinate_system_hwnd* pCoordSysOutObj = NULL;
	const coordinate_system_hwnd* pCoordSysRefObj = NULL;
	hm_layout_obj::coordinate_system_ptr_type pCoordSysTmp = NULL;
	if ( !m_pOutObj || !m_pRefObj )
		return;
	pCoordSysTmp = m_pOutObj->getCoordinateSystem();
	if ( !pCoordSysTmp ) 
		return;
	pCoordSysOutObj = ( coordinate_system_hwnd* )pCoordSysTmp;
	pCoordSysTmp = m_pRefObj->getCoordinateSystem();
	if ( !pCoordSysTmp ) 
		return;
	pCoordSysRefObj = ( coordinate_system_hwnd* )pCoordSysTmp;
	//
	if ( pCoordSysOutObj->isSameSystem( *pCoordSysRefObj ) ) {
		switch ( m_eDockType ) {
		case DOCK_TOP:
			{
				m_pOutObj->m_rect.left = m_pRefObj->m_rect.left + m_space2;
				m_pOutObj->m_rect.right = m_pRefObj->m_rect.right - m_space3;
				m_pOutObj->m_rect.top = m_pRefObj->m_rect.top + m_space1;
				m_pOutObj->m_rect.bottom = m_pOutObj->m_rect.top + m_thickness;
			}
			break;
		case DOCK_LEFT:
			{
				m_pOutObj->m_rect.left = m_pRefObj->m_rect.left + m_space1;
				m_pOutObj->m_rect.right = m_pOutObj->m_rect.left + m_thickness;
				m_pOutObj->m_rect.top = m_pRefObj->m_rect.top + m_space2;
				m_pOutObj->m_rect.bottom = m_pRefObj->m_rect.bottom - m_space3;
			}
			break;
		case DOCK_RIGHT:
			{
				m_pOutObj->m_rect.right = m_pRefObj->m_rect.right - m_space1;
				m_pOutObj->m_rect.left = m_pOutObj->m_rect.right - m_thickness;
				m_pOutObj->m_rect.top = m_pRefObj->m_rect.top + m_space2;
				m_pOutObj->m_rect.bottom = m_pRefObj->m_rect.bottom - m_space3;
			}
			break;
		case DOCK_BOTTOM:
			{
				m_pOutObj->m_rect.left = m_pRefObj->m_rect.left + m_space2;
				m_pOutObj->m_rect.right = m_pRefObj->m_rect.right - m_space3;
				m_pOutObj->m_rect.bottom = m_pRefObj->m_rect.bottom - m_space1;
				m_pOutObj->m_rect.top = m_pOutObj->m_rect.bottom - m_thickness;
			}
			break;
		default:
			break;
		}
		
	} else {
		
	}
}
//
hm_layoutdecoder_offset::hm_layoutdecoder_offset( out_obj_ptr_type pOutObj )
: hm_layoutdecoder_i( pOutObj )
, m_nOffsetL( 0 )
, m_nOffsetT( 0 )
, m_nOffsetR( 0 )
, m_nOffsetB( 0 )
{
	
}
//
hm_layoutdecoder_offset::~hm_layoutdecoder_offset() {
	
}
//
void hm_layoutdecoder_offset::decoderLayout() {
	// R/B offsets are insets from the reference edges ( manifest authors write
	// positive values expecting padding semantics ). the old "+" pushed rects
	// past the reference right/bottom - invisible while only CORNER_LT
	// children consumed left/top, exposed when the §4 scrollbar first
	// anchored to a reference right edge ( drew 912px outside the client ).
	m_pOutObj->m_rect.left = m_pRefObj->m_rect.left + m_nOffsetL;
	m_pOutObj->m_rect.right = m_pRefObj->m_rect.right - m_nOffsetR;
	m_pOutObj->m_rect.top = m_pRefObj->m_rect.top + m_nOffsetT;
	m_pOutObj->m_rect.bottom = m_pRefObj->m_rect.bottom - m_nOffsetB;
}
