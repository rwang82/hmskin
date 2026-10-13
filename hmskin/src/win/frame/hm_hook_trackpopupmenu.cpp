#include "stdafx.h"
#include "win\frame\hm_hook_trackpopupmenu.h"
#include "win\wndctrl_repository\hm_wndctrl_popmenu.h"
#include "win\hm_win_globals.h"
#include "win\hm_wndmsg_defs.h"
#include "Winuser.h"
#include "tchar.h"
#include "hmskin.h"

//
struct HMTPMParam{
	bool volatile m_bCopied;
	HWND m_hWndPopupMenu;
	HMENU m_hMenu;
	UINT m_uFlags;
	int m_x;
	int m_y;
	HINSTANCE m_hInst;
	HWND m_hParentWnd;
};
static DWORD WINAPI HMTPMThreadProc(
  LPVOID lpParameter
);
BOOL WINAPI hmTrackPopupMenu( HMENU hMenu,
    UINT uFlags,
    int x,
    int y,
    int nReserved,
    HWND hWnd,
    CONST RECT * prcRect
	) {
	HANDLE hThread;
	HMTPMParam hmtpmParam;
	DWORD dwWait;

	hmtpmParam.m_bCopied = false;
	hmtpmParam.m_hWndPopupMenu = NULL;
	hmtpmParam.m_hParentWnd = hWnd;
	hmtpmParam.m_hInst = (HINSTANCE)(INT_PTR)nReserved;
	hmtpmParam.m_hMenu = hMenu;
	hmtpmParam.m_uFlags = uFlags;
	hmtpmParam.m_x = x;
	hmtpmParam.m_y = y;
		
	hThread = ::CreateThread( NULL, 0, HMTPMThreadProc, &hmtpmParam, 0, NULL );
	while( !hmtpmParam.m_bCopied ) {
		Sleep( 10 );
	}
	//
	if ( hmtpmParam.m_hWndPopupMenu ) {
		//hm_wndpropid_connect( hmtpmParam.m_hWndPopupMenu, dwWndPropId );
		//
		::SendMessage( hmtpmParam.m_hWndPopupMenu, HMWM_BINDMENU, (WPARAM)hMenu, 0 );
		//
		::ShowWindow( hmtpmParam.m_hWndPopupMenu, SW_NORMAL );
		// before call hm_wndpropid_connect, lose a WM_PAINT message to deal, so add one to draw menu.
//		::InvalidateRect( hmtpmParam.m_hWndPopupMenu, NULL, TRUE );
	}
	//
	dwWait = ::WaitForSingleObject( hThread, INFINITE );
	//
	if ( hmtpmParam.m_hWndPopupMenu ) {
		//
		//hm_wndpropid_disconnect( hmtpmParam.m_hWndPopupMenu, dwWndPropId );
	}
	
	// clear mouse message in queue
	MSG msg;
	while ( PeekMessage( &msg, hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );

	return TRUE;
}
//
DWORD WINAPI HMTPMThreadProc( LPVOID lpParameter ) {
	HMTPMParam* pHMTPMParam = NULL;
	hm_wndctrl_popmenu popMenu;
	MSG msg; 
	BOOL bCreatePopMenuOK = false;
	HWND hWndCaptureOld;

	pHMTPMParam = (HMTPMParam*)lpParameter;
	if ( !pHMTPMParam ) return 0;
	//
	bCreatePopMenuOK = popMenu.CreateWnd( pHMTPMParam->m_x, pHMTPMParam->m_y, 50, 50, pHMTPMParam->m_hParentWnd, NULL, pHMTPMParam->m_hInst );
	if ( bCreatePopMenuOK ) {
		pHMTPMParam->m_hWndPopupMenu = popMenu.m_hWnd;
	} else {
		pHMTPMParam->m_hWndPopupMenu = NULL;
	}
	//
	pHMTPMParam->m_bCopied = true;
	if ( !bCreatePopMenuOK )
		return FALSE;

	hWndCaptureOld = ::SetFocus( popMenu.m_hWnd );
	//
	while(GetMessage(&msg,NULL,0,0)) 
	{ 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
	//
	::SetFocus( hWndCaptureOld );

	return TRUE;
}