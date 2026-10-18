#include "stdafx.h"
#include "win\frame\hm_hook_trackpopupmenu.h"
#include "win\frame\hm_uiroot.h"
#include "hmskin.h"   // hm_wndpropid_connect / hm_lib_popupmenu_show
#include "win\hm_wndmsg_defs.h"

// engine-driven popup menus: hm_lib_popupmenu_show creates a borderless
// popup window bound to the manifest's PopupSkin slot, binds the HMENU and
// pumps messages until it closes ( native TrackPopupMenu semantics ).
// the transparent apihook ( hm_hs_apihook ) lands here too, so host apps
// keep calling the native API with zero skin code.

static hm_uiroot* g_pTPMRoot = NULL;
static ATOM s_atomPopupCls = 0;
static HWND s_hWndLastPopup = NULL;   // slot lifecycle: one PropId <-> one HWND

static LRESULT CALLBACK HMPopupWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
	return ::DefWindowProc( hWnd, message, wParam, lParam );
}

static bool _EnsurePopupCls() {
	if ( s_atomPopupCls )
		return true;
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_SAVEBITS;
	wcex.lpfnWndProc = HMPopupWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = (HINSTANCE)::GetModuleHandle( NULL );
	wcex.hIcon = NULL;
	wcex.hCursor = ::LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T( "hmskinPopupMenuCls" );
	wcex.hIconSm = NULL;
	s_atomPopupCls = ::RegisterClassEx( &wcex );
	return s_atomPopupCls != 0;
}

bool hm_lib_popupmenu_show( hm_uiroot* pRoot, HMENU hMenu, int x, int y, HWND hWndOwner ) {
	if ( !pRoot || !hMenu )
		return false;
	if ( !_EnsurePopupCls() )
		return false;
	if ( pRoot->m_uPopupPropId == 0 ) {
		// manifest has no "PopupSkin" declaration - no engine popup.
		return false;
	}

	// the popup slot is exclusive ( one PropId <-> one HWND in the skin ):
	// release it for BOTH death paths of the previous popup before binding.
	if ( s_hWndLastPopup ) {
		hm_wndpropid_disconnect( pRoot, s_hWndLastPopup, pRoot->m_uPopupPropId );
		if ( ::IsWindow( s_hWndLastPopup ) )
			::DestroyWindow( s_hWndLastPopup );
		s_hWndLastPopup = NULL;
	}
	//
	HWND hWndPopup = ::CreateWindowEx( WS_EX_TOOLWINDOW | WS_EX_TOPMOST, _T( "hmskinPopupMenuCls" ),
		NULL, WS_POPUP, x, y, 160, 100, NULL, NULL, (HINSTANCE)::GetModuleHandle( NULL ), NULL );
	if ( !hWndPopup )
		return false;
	s_hWndLastPopup = hWndPopup;
	hm_wndpropid_connect( pRoot, hWndPopup, pRoot->m_uPopupPropId );
	// bind the HMENU + the WM_COMMAND forward target; the popup WMP resizes
	// the window to fit and draws the items.
	::SendMessage( hWndPopup, HMWM_BINDMENU, (WPARAM)hMenu, (LPARAM)hWndOwner );
	::ShowWindow( hWndPopup, SW_SHOW );

	// native TrackPopupMenu is modal: pump until the popup closes. the popup
	// WMP destroys the window on selection and on outside clicks.
	MSG msg;
	HMODULE hm = NULL; UNREFERENCED_PARAMETER( hm );
	BOOL bQuitSeen = FALSE;
	while ( ::IsWindow( hWndPopup ) ) {
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
			if ( msg.message == WM_QUIT ) {
				// WM_QUIT belongs to the OUTER loop: re-post and leave.
				// DispatchMessage( WM_QUIT ) is a no-op - swallowing it
				// here left the app hanging in GetMessage after teardown.
				bQuitSeen = TRUE;
				break;
			}
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		} else {
			::WaitMessage();
		}
	}
	if ( bQuitSeen ) {
		if ( ::IsWindow( hWndPopup ) )
			::DestroyWindow( hWndPopup );   // owner may not be dying too
		::PostQuitMessage( (int)msg.wParam );
	}
	// window dead: drop the binding so the next popup can claim the slot.
	hm_wndpropid_disconnect( pRoot, hWndPopup, pRoot->m_uPopupPropId );
	if ( s_hWndLastPopup == hWndPopup )
		s_hWndLastPopup = NULL;
	return true;
}

void hm_tpm_set_root( hm_uiroot* pRoot ) {
	g_pTPMRoot = pRoot;
}

BOOL WINAPI hmTrackPopupMenu( HMENU hMenu,
    UINT uFlags,
    int x,
    int y,
    int nReserved,
    HWND hWnd,
    CONST RECT * prcRect
	) {
	UNREFERENCED_PARAMETER( uFlags );
	UNREFERENCED_PARAMETER( nReserved );
	UNREFERENCED_PARAMETER( prcRect );

	// intercepted native call: show the ENGINE popup ( transparent skinning,
	// selection is delivered to the owner as native WM_COMMAND ).
	if ( g_pTPMRoot && hm_lib_popupmenu_show( g_pTPMRoot, hMenu, x, y, hWnd ) )
		return TRUE;
	return FALSE;
}
