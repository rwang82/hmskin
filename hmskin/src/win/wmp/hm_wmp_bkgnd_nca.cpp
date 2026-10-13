#include "stdafx.h"
#include "win\wmp\hm_wmp_bkgnd_nca.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "Winuser.h"
#include "win\hm_event_defs.h"
//
#define HMBKGNDNCA_THICK_TOP_DEFAULT (30)
#define HMBKGNDNCA_THICK_BOTTOM_DEFAULT (5)
#define HMBKGNDNCA_THICK_LEFT_DEFAULT (5)
#define HMBKGNDNCA_THICK_RIGHT_DEFAULT (5)
//
hm_wmp_bkgnd_nca::hm_wmp_bkgnd_nca( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_bActive( false )
, m_uThickL( HMBKGNDNCA_THICK_LEFT_DEFAULT )
, m_uThickR( HMBKGNDNCA_THICK_RIGHT_DEFAULT )
, m_uThickT( HMBKGNDNCA_THICK_TOP_DEFAULT )
, m_uThickB( HMBKGNDNCA_THICK_BOTTOM_DEFAULT )
{
	_createDistrictObjs();
}
//
hm_wmp_bkgnd_nca::~hm_wmp_bkgnd_nca(){}
//
MSGBIND_BEGIN( hm_wmp_bkgnd_nca )
	MSGBIND( WM_NCACTIVATE, _OnNCActivate )
	MSGBIND( WM_NCCALCSIZE, _OnNCCalcSize ) 
	MSGBIND( WM_NCHITTEST, _OnNCHitTest )
	MSGBIND( WM_SYSKEYDOWN, _OnSysKeyDown )
	MSGBIND( WM_SYSKEYUP, _OnSysKeyUp )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_bkgnd_nca )
	ORDERWNDMSG( WM_NCACTIVATE )
	ORDERWNDMSG( WM_NCCALCSIZE )
	ORDERWNDMSG( WM_NCHITTEST )
	ORDERWNDMSG( WM_SYSKEYDOWN )
	ORDERWNDMSG( WM_SYSKEYUP )
ORDERWNDMSG_END()
//
void hm_wmp_bkgnd_nca::_createDistrictObjs() {
	_setDistrictObj( EDISTRICTID_CAPTION_ACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_BOTTOM_ACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_LEFT_ACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_RIGHT_ACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_CAPTION_INACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_BOTTOM_INACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_LEFT_INACTIVE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_RIGHT_INACTIVE, new hm_districtobj() );
		
}
//
void hm_wmp_bkgnd_nca::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {

	switch ( idEvent ) {
	case HMKEELOBJEVENT_DRAWSELF:
		{
			HEDSPARAM* pParam = NULL;
			RECT rcCanvas;

			pParam = (HEDSPARAM*)pEventParam;
			if ( !pParam->m_pLayoutObjCanvas->mapPosition( pParam->m_hWndCanvas, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
				return;
			
			_doDrawCaption( pParam->m_hDCCanvas, pParam->m_hWndCanvas );
			_doDrawBottom( pParam->m_hDCCanvas, pParam->m_hWndCanvas );
			_doDrawLeft( pParam->m_hDCCanvas, pParam->m_hWndCanvas );
			_doDrawRight( pParam->m_hDCCanvas, pParam->m_hWndCanvas );
			_doDrawSysBtn( pParam->m_hDCCanvas, pParam->m_hWndCanvas );
		}
		break;
	default:
		break;
	}
}
//
void hm_wmp_bkgnd_nca::_doDrawCaption( HDC hDC, HWND hWndDest ) {
	RECT rcWnd;
	RECT rcC2W;
	RECT rcCanvas;
	//TITLEBARINFO tbInfo;
	int heightCaption;
	hm_districtobj* pDistrictObj = NULL;

	//get heightCaption
	::GetWindowRect( hWndDest, &rcWnd );
	::GetClientRect( hWndDest, &rcC2W );
	::ClientToScreen( hWndDest, LPPOINT( &rcC2W ) );
	::ClientToScreen( hWndDest, ( LPPOINT( &rcC2W ) + 1) );
	heightCaption = ( rcC2W.top - rcWnd.top );
	//::GetWindowRect( hWndDest, &rcWnd );
	//tbInfo.cbSize = sizeof( TITLEBARINFO );
	//if ( !::GetTitleBarInfo( hWndDest, &tbInfo ) )
	//	return;
	
	rcCanvas.left = 0;
	rcCanvas.top = 0;
	rcCanvas.right = ( rcWnd.right - rcWnd.left );
	rcCanvas.bottom = heightCaption;
	if ( !_getDistrictObj( m_bActive ? EDISTRICTID_CAPTION_ACTIVE : EDISTRICTID_CAPTION_INACTIVE, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
}
//
void hm_wmp_bkgnd_nca::_doDrawBottom( HDC hDC, HWND hWndDest ) {
	int heightBottom;
	RECT rcWnd;
	RECT rcC2W;
	RECT rcCanvas;
	//COLORREF clrBK;
	hm_districtobj* pDistrictObj = NULL;
	
	::GetWindowRect( hWndDest, &rcWnd );
	::GetClientRect( hWndDest, &rcC2W );
	::ClientToScreen( hWndDest, LPPOINT( &rcC2W ) );
	::ClientToScreen( hWndDest, ( LPPOINT( &rcC2W ) + 1) );
	heightBottom = ( rcWnd.bottom - rcC2W.bottom );// ::GetSystemMetrics( SM_CYBORDER );

	rcCanvas.left = 0;
	rcCanvas.top = ( rcWnd.bottom - rcWnd.top ) - heightBottom;
	rcCanvas.right = ( rcWnd.right - rcWnd.left );
	rcCanvas.bottom = ( rcWnd.bottom - rcWnd.top );
	//
	//clrBK = RGB( 200, 200, 100 );
	//hm_util_draw_rect( hDC, rcCanvas, clrBK );
	if ( !_getDistrictObj( m_bActive ? EDISTRICTID_BOTTOM_ACTIVE : EDISTRICTID_BOTTOM_INACTIVE, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );

}
//
void hm_wmp_bkgnd_nca::_doDrawLeft( HDC hDC, HWND hWndDest ) {
	int heightCaption;
	RECT rcWnd;
	RECT rcC2W;
	RECT rcCanvas;
	//COLORREF clrBK;
	hm_districtobj* pDistrictObj = NULL;
	
	::GetWindowRect( hWndDest, &rcWnd );
	::GetClientRect( hWndDest, &rcC2W );
	::ClientToScreen( hWndDest, LPPOINT( &rcC2W ) );
	::ClientToScreen( hWndDest, ( LPPOINT( &rcC2W ) + 1) );
	//heightCaption = ::GetSystemMetrics( SM_CYCAPTION );
	heightCaption = rcC2W.top - rcWnd.top;
	
	rcCanvas.left = 0;
	rcCanvas.top = heightCaption;
	rcCanvas.right = ( rcC2W.left - rcWnd.left );
	rcCanvas.bottom = ( rcWnd.bottom - rcWnd.top ) - ( rcWnd.bottom - rcC2W.bottom );
	//
	//clrBK = RGB( 100, 100, 200 );
	//hm_util_draw_rect( hDC, rcCanvas, clrBK );
	if ( !_getDistrictObj( m_bActive ? EDISTRICTID_LEFT_ACTIVE : EDISTRICTID_LEFT_INACTIVE, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
}
//
void hm_wmp_bkgnd_nca::_doDrawRight( HDC hDC, HWND hWndDest ) {
	int heightCaption;
	RECT rcWnd;
	RECT rcC2W;
	RECT rcCanvas;
	//COLORREF clrBK;
	hm_districtobj* pDistrictObj = NULL;
	
	::GetWindowRect( hWndDest, &rcWnd );
	::GetClientRect( hWndDest, &rcC2W );
	::ClientToScreen( hWndDest, LPPOINT( &rcC2W ) );
	::ClientToScreen( hWndDest, ( LPPOINT( &rcC2W ) + 1) );
	//heightCaption = ::GetSystemMetrics( SM_CYCAPTION );
	heightCaption = rcC2W.top - rcWnd.top;
	
	rcCanvas.left = ( rcWnd.right - rcWnd.left ) - ( rcWnd.right - rcC2W.right );
	rcCanvas.top = heightCaption;
	rcCanvas.right = ( rcWnd.right - rcWnd.left );
	rcCanvas.bottom = ( rcWnd.bottom - rcWnd.top ) - ( rcWnd.bottom - rcC2W.bottom );
	//
	//clrBK = RGB( 100, 100, 200 );
	//hm_util_draw_rect( hDC, rcCanvas, clrBK );
	if ( !_getDistrictObj( m_bActive ? EDISTRICTID_RIGHT_ACTIVE : EDISTRICTID_RIGHT_INACTIVE, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );

}
//
void hm_wmp_bkgnd_nca::_doDrawSysBtn( HDC hDC, HWND hWndDest ) {
	
}
//
void hm_wmp_bkgnd_nca::_OnNCActivate( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 1;
	//
	m_bActive = ( pWP->wParam != 0 );
}
//
void hm_wmp_bkgnd_nca::_OnNCCalcSize( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	if ( pWP->wParam != 0 ) {
		NCCALCSIZE_PARAMS* pCalcSizeParam = (NCCALCSIZE_PARAMS*)pWP->lParam;
		RECT rcWndNew;
		RECT rcWndOld;
		RECT rcCltOld;
		
		//
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = WVR_VALIDRECTS|WVR_REDRAW|WVR_ALIGNTOP|WVR_ALIGNLEFT;
		//
		rcWndNew = pCalcSizeParam->rgrc[0];
		rcWndOld = pCalcSizeParam->rgrc[1];
		rcCltOld = pCalcSizeParam->rgrc[2];
		//
		pCalcSizeParam->lppos->x = rcWndNew.left;
		pCalcSizeParam->lppos->y = rcWndNew.left;
		pCalcSizeParam->lppos->cx = ( rcWndNew.right - rcWndNew.left );
		pCalcSizeParam->lppos->cy = ( rcWndNew.bottom - rcWndNew.top );
		pCalcSizeParam->lppos->hwnd = pWP->hwnd;
		pCalcSizeParam->lppos->hwndInsertAfter = NULL;
		pCalcSizeParam->lppos->flags = SWP_NOACTIVATE|SWP_NOZORDER;
		pCalcSizeParam->rgrc->left = rcWndNew.left + m_uThickL;
		pCalcSizeParam->rgrc->right = rcWndNew.right - m_uThickR;
		pCalcSizeParam->rgrc->top = rcWndNew.top + m_uThickT;
		pCalcSizeParam->rgrc->bottom = rcWndNew.bottom - m_uThickB;
		
	} else {
		RECT* pRect = (RECT*)pWP->lParam;

		pRect->left += m_uThickL;
		pRect->right -= m_uThickR;
		pRect->top += m_uThickT;
		pRect->bottom -= m_uThickB;
		
	}
}

void hm_wmp_bkgnd_nca::_OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	POINT ptWnd;

	//
	ptWnd.x = LOWORD( pWP->lParam );
	ptWnd.y = HIWORD( pWP->lParam );
	//
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = _Pt2HitTestCode( pWP->hwnd, ptWnd );
	//
}
//
int hm_wmp_bkgnd_nca::_Pt2HitTestCode( HWND hWnd, const POINT& ptWnd ) {
	int heightCaption;
	int widthLeft;
	int widthRight;
	int heightBottom;
	RECT rcWnd;
	RECT rcC2W;
	RECT rcCaption;
	RECT rcTop;
	RECT rcLeft;
	RECT rcRight;
	RECT rcBottom;
	RECT rcLT;
	RECT rcLB;
	RECT rcRT;
	RECT rcRB;
	int lenCorner;
	
	lenCorner = 10;
	//
	::GetWindowRect( hWnd, &rcWnd );
	::GetClientRect( hWnd, &rcC2W );
	::ClientToScreen( hWnd, LPPOINT( &rcC2W ) );
	::ClientToScreen( hWnd, ( LPPOINT( &rcC2W ) + 1) );
	//heightCaption = ::GetSystemMetrics( SM_CYCAPTION );
	heightCaption = rcC2W.top - rcWnd.top;
	widthLeft = rcC2W.left - rcWnd.left;
	widthRight = rcWnd.right - rcC2W.right;
	heightBottom = rcWnd.bottom - rcC2W.bottom;
	
	//
	rcCaption.top = rcWnd.top;
	rcCaption.left = rcWnd.left;
	rcCaption.right = rcWnd.right;
	rcCaption.bottom = rcWnd.top + heightCaption;
	//
	rcTop.top = rcWnd.top;
	rcTop.left = rcWnd.left;
	rcTop.right = rcWnd.right;
	rcTop.bottom = rcWnd.top + max( 0, min( lenCorner, heightCaption ) );
	//
	rcLeft.top = rcWnd.top + min( lenCorner, heightCaption );
	rcLeft.left = rcWnd.left;
	rcLeft.right = rcWnd.left + min( lenCorner, widthLeft );
	rcLeft.bottom = rcWnd.bottom - min( lenCorner, heightBottom );
	//
	rcRight.top = rcWnd.top + min( lenCorner, heightCaption );
	rcRight.left = rcWnd.right - min( lenCorner, widthRight );
	rcRight.right = rcWnd.right;
	rcRight.bottom = rcWnd.bottom - min( lenCorner, heightBottom );
	//
	rcBottom.top = rcWnd.bottom - min( lenCorner, heightBottom );
	rcBottom.left = rcWnd.left;
	rcBottom.right = rcWnd.right;
	rcBottom.bottom = rcWnd.bottom;
	//
	rcLT.top = rcWnd.top;
	rcLT.left = rcWnd.left;
	rcLT.right = rcWnd.left + lenCorner;
	rcLT.bottom = rcWnd.top + lenCorner;
	//
	rcLB.top = rcWnd.bottom - lenCorner;
	rcLB.left = rcWnd.left;
	rcLB.right = rcWnd.left + lenCorner;
	rcLB.bottom = rcWnd.bottom;
	//
	rcRT.top = rcWnd.top;
	rcRT.left = rcWnd.right - lenCorner;
	rcRT.right = rcWnd.right;
	rcRT.bottom = rcWnd.top + lenCorner;
	//
	rcRB.top = rcWnd.bottom - lenCorner;
	rcRB.left = rcWnd.right - lenCorner;
	rcRB.right = rcWnd.right;
	rcRB.bottom = rcWnd.bottom;
	
	//
	if ( ::PtInRect( &rcLT, ptWnd ) )
		return HTTOPLEFT;
	else if ( ::PtInRect( &rcLB, ptWnd ) )
		return HTBOTTOMLEFT;
	else if ( ::PtInRect( &rcRB, ptWnd ) )
		return HTBOTTOMRIGHT;
	else if ( ::PtInRect( &rcRT, ptWnd ) )
		return HTTOPRIGHT;
	else if ( ::PtInRect( &rcTop, ptWnd ) )
		return HTTOP;
	else if ( ::PtInRect( &rcLeft, ptWnd ) )
		return HTLEFT;
	else if ( ::PtInRect( &rcRight, ptWnd ) )
		return HTRIGHT;
	else if ( ::PtInRect( &rcBottom, ptWnd ) )
		return HTBOTTOM;
	else if ( ::PtInRect( &rcCaption, ptWnd ) )
		return HTCAPTION;
	else if ( ::PtInRect( &rcC2W, ptWnd ) )
		return HTCLIENT;
	
	return HTNOWHERE;
}
//
void hm_wmp_bkgnd_nca::_OnSysKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	//disable raw menu popup
	if ( pWP->wParam == VK_MENU ) {
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;
	}
}
//
void hm_wmp_bkgnd_nca::_OnSysKeyUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	//disable raw menu popup
	if ( pWP->wParam == VK_MENU ) {
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;
		pParamWndProc->m_bValidRes = true;
		pParamWndProc->m_lResult = 0;
	}
}

void hm_wmp_bkgnd_nca::setThickL( unsigned int uThick ) {
	m_uThickL = uThick;
}

void hm_wmp_bkgnd_nca::setThickR( unsigned int uThick ) {
	m_uThickR = uThick;
}

void hm_wmp_bkgnd_nca::setThickT( unsigned int uThick ) {
	m_uThickT = uThick;
}

void hm_wmp_bkgnd_nca::setThickB( unsigned int uThick ) {
	m_uThickB = uThick;
}

void hm_wmp_bkgnd_nca::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_CAPTION_ACTIVE );
	containerIds.push_back( EDISTRICTID_BOTTOM_ACTIVE );
	containerIds.push_back( EDISTRICTID_LEFT_ACTIVE );
	containerIds.push_back( EDISTRICTID_RIGHT_ACTIVE );
	containerIds.push_back( EDISTRICTID_CAPTION_INACTIVE );
	containerIds.push_back( EDISTRICTID_BOTTOM_INACTIVE );
	containerIds.push_back( EDISTRICTID_LEFT_INACTIVE );
	containerIds.push_back( EDISTRICTID_RIGHT_INACTIVE );
}