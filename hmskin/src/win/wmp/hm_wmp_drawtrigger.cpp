#include "stdafx.h"
#include "win\wmp\hm_wmp_drawtrigger.h"
#include "common\FuncPack.h"
#include "common\MemFuncPack.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include <assert.h>
//
hm_wmp_drawtrigger::hm_wmp_drawtrigger( hmskin_event_listener_base* pEventListener )
: hm_wndmsgprocessor_i( pEventListener )
{
}

hm_wmp_drawtrigger::~hm_wmp_drawtrigger()
{
}

MSGBIND_BEGIN( hm_wmp_drawtrigger )
	MSGBIND( WM_PAINT, _OnPaint )
	MSGBIND( WM_ERASEBKGND, _OnEraseBkGnd )
	MSGBIND( WM_SIZE, _OnSize )
	MSGBIND( WM_WINDOWPOSCHANGED, _OnWindowPosChanged )
MSGBIND_END()


ORDERWNDMSG_BEGIN( hm_wmp_drawtrigger )
	ORDERWNDMSG( WM_PAINT )
	ORDERWNDMSG( WM_ERASEBKGND )
	ORDERWNDMSG( WM_SIZE )
	ORDERWNDMSG( WM_WINDOWPOSCHANGED )
ORDERWNDMSG_END()
//
void hm_wmp_drawtrigger::_OnWindowPosChanged( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	HWND hWndCanvas;
	RECT rcCanvas;
	hm_layout_obj* pLayoutObj = NULL;
	SIZE newSize;

	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;

	hWndCanvas = pWP->hwnd;
	if ( !pLayoutObj->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;
	if ( (rcCanvas.left >= rcCanvas.right) || (rcCanvas.top >= rcCanvas.bottom) )
		return; 

	//
	newSize.cx = rcCanvas.right - rcCanvas.left;
	newSize.cy = rcCanvas.bottom - rcCanvas.top;
	m_hmCanvas.updateMemBitmapSize( newSize );
}
//
void hm_wmp_drawtrigger::_OnSize( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	HWND hWndCanvas;
	RECT rcCanvas;
	hm_layout_obj* pLayoutObj = NULL;
	SIZE newSize;

	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;

	hWndCanvas = pWP->hwnd;
	if ( !pLayoutObj->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;
	if ( (rcCanvas.left >= rcCanvas.right) || (rcCanvas.top >= rcCanvas.bottom) )
		return; 

	//
	newSize.cx = rcCanvas.right - rcCanvas.left;
	newSize.cy = rcCanvas.bottom - rcCanvas.top;
	m_hmCanvas.updateMemBitmapSize( newSize );
}
//
void hm_wmp_drawtrigger::_OnPaint( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	HDC hPaintDC;
	PAINTSTRUCT ps;
	hm_layout_obj* pLayoutObj = NULL;
	HWND hWndCanvas;
	RECT rcCanvas;

	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	//
	hWndCanvas = pWP->hwnd;
	if ( !pLayoutObj->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;
	if ( (rcCanvas.left >= rcCanvas.right) || (rcCanvas.top >= rcCanvas.bottom) )
		return;

	hPaintDC = ::BeginPaint( pWP->hwnd, &ps );
	//
	pKeelObj->doDrawAll( m_hmCanvas.GetMemDC(), pWP->hwnd );
	::BitBlt( hPaintDC, rcCanvas.left, rcCanvas.top, (rcCanvas.right - rcCanvas.left), (rcCanvas.bottom - rcCanvas.top), m_hmCanvas.GetMemDC(), 0, 0, SRCCOPY );

	::EndPaint( pWP->hwnd, &ps );
}
//
void hm_wmp_drawtrigger::_OnEraseBkGnd( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;

	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 1;
}