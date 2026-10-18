#include "stdafx.h"
#include "win\wmp\hm_wmp_drawtrigger_nca.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_debug_utils.h"
#include <assert.h>

hm_wmp_drawtrigger_nca::hm_wmp_drawtrigger_nca( hmskin_event_listener_base* pEventListener )
: hm_wndmsgprocessor_i( pEventListener )
{
}

hm_wmp_drawtrigger_nca::~hm_wmp_drawtrigger_nca()
{}

MSGBIND_BEGIN( hm_wmp_drawtrigger_nca )
	MSGBIND( WM_NCPAINT, _OnNCPaint )
	MSGBIND( WM_NCACTIVATE, _OnNCActivate )
	MSGBIND( WM_SIZE, _OnSize )
	MSGBIND( WM_WINDOWPOSCHANGED, _OnWindowPosChanged )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_drawtrigger_nca )
	ORDERWNDMSG( WM_NCPAINT )
	ORDERWNDMSG( WM_NCACTIVATE )
	ORDERWNDMSG( WM_SIZE )
	ORDERWNDMSG( WM_WINDOWPOSCHANGED )
ORDERWNDMSG_END()
//
void hm_wmp_drawtrigger_nca::_OnWindowPosChanged( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	HWND hWndCanvas;
	RECT rcCanvas;
	hm_layout_obj* pLayoutObj = NULL;
	SIZE newSize;
	HWND hWnd;

	//
	hWnd = pWP->hwnd;
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;

	hWndCanvas = pWP->hwnd;
	if ( !pLayoutObj->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	if ( (rcCanvas.left >= rcCanvas.right) || (rcCanvas.top >= rcCanvas.bottom) )
		return; 

	//
	newSize.cx = rcCanvas.right - rcCanvas.left;
	newSize.cy = rcCanvas.bottom - rcCanvas.top;
	m_hmCanvas.updateMemBitmapSize( newSize );

}
//
void hm_wmp_drawtrigger_nca::_OnSize( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	HWND hWndCanvas;
	RECT rcCanvas;
	hm_layout_obj* pLayoutObj = NULL;
	SIZE newSize;
	HWND hWnd;

	//
	hWnd = pWP->hwnd;
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;

	hWndCanvas = pWP->hwnd;
	if ( !pLayoutObj->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_WINDOW_TYPE, rcCanvas ) )
		return;
	if ( (rcCanvas.left >= rcCanvas.right) || (rcCanvas.top >= rcCanvas.bottom) )
		return; 

	//
	newSize.cx = rcCanvas.right - rcCanvas.left;
	newSize.cy = rcCanvas.bottom - rcCanvas.top;
	m_hmCanvas.updateMemBitmapSize( newSize );


}
//
void hm_wmp_drawtrigger_nca::_OnNCPaint( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	HDC hDC;
	
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 1;
	//
	hDC = ::GetWindowDC( pWP->hwnd );
	
	_doTrigger( pKeelObj, hDC, pWP->hwnd );

    ReleaseDC( pWP->hwnd, hDC );
}

void hm_wmp_drawtrigger_nca::_OnNCActivate( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	HDC hDC;
	
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC; // | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 1;
	//
	hDC = ::GetWindowDC( pWP->hwnd );

	DWORD dwTimeLast;
	DWORD dwTimeCur;
	TCHAR szMsg[100];

	dwTimeLast = ::GetTickCount();

	_doTrigger( pKeelObj, hDC, pWP->hwnd );

	dwTimeCur = ::GetTickCount();
	_stprintf_s( szMsg, _T("_doTrigger time:%d\n"), dwTimeCur - dwTimeLast );
	::OutputDebugString( szMsg );


    ReleaseDC( pWP->hwnd, hDC );
}


void hm_wmp_drawtrigger_nca::_doTrigger( hm_keel_obj* pKeelObj, HDC hDC, HWND hWnd ) {
	RECT rcWnd;
	RECT rcClient;
	RECT rcC2W;
	SIZE sizeWnd;

	if ( !::GetWindowRect( hWnd, &rcWnd ) )
		return;
	sizeWnd.cx = rcWnd.right - rcWnd.left;
	sizeWnd.cy = rcWnd.bottom - rcWnd.top;

	//
	hm_util_invlog( _T("nca-full"), NULL );   // NC path walks without a dirty rect
	pKeelObj->doDrawAll( m_hmCanvas.GetMemDC(), hWnd );

	//
	::GetClientRect( hWnd, &rcClient );
	rcC2W = rcClient;
	::ClientToScreen( hWnd, (LPPOINT)(&rcC2W) );
	::ClientToScreen( hWnd, ((LPPOINT)(&rcC2W)) + 1 );
	rcC2W.left -= rcWnd.left;
	rcC2W.top -= rcWnd.top;
	rcC2W.right -= rcWnd.left;
	rcC2W.bottom -= rcWnd.top;
	ExcludeClipRect( hDC, rcC2W.left, rcC2W.top, rcC2W.right, rcC2W.bottom );
	::BitBlt( hDC, 0, 0, sizeWnd.cx, sizeWnd.cy, m_hmCanvas.GetMemDC(), 0, 0, SRCCOPY );


}
