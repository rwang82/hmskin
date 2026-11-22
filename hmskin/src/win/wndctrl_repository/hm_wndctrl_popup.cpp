#include "stdafx.h"
#include "tchar.h"
#include "win\wndctrl_repository\hm_wndctrl_popup.h"
#include "win\frame\hm_uiroot.h"
#include "win\hm_wndmsg_defs.h"
#include "hmskin.h"   // hm_wndpropid_connect / hm_lib_popupmenu_show

const TCHAR hm_wndctrl_popup::s_szPopupClsName[] = _T( "hmskinPopupMenuCls" );
ATOM hm_wndctrl_popup::s_atomPopupCls = 0;

hm_wndctrl_popup::hm_wndctrl_popup() {}
hm_wndctrl_popup::~hm_wndctrl_popup() {}

bool hm_wndctrl_popup::ensureCls() {
	if ( s_atomPopupCls )
		return true;
	s_atomPopupCls = _regWndCls( (HINSTANCE)::GetModuleHandle( NULL ),
		s_szPopupClsName, CS_SAVEBITS, NULL );
	return s_atomPopupCls != 0;
}

bool hm_wndctrl_popup::create( int x, int y ) {
	if ( !ensureCls() )
		return false;
	return CreateWnd( WS_EX_TOOLWINDOW | WS_EX_TOPMOST, NULL, WS_POPUP,
		x, y, 160, 100, NULL, NULL, (HINSTANCE)::GetModuleHandle( NULL ) );
}

// engine-driven popup menus: hm_lib_popupmenu_show creates a borderless
// popup window bound to the manifest's PopupSkin slot, binds the HMENU and
// pumps messages until it closes ( native TrackPopupMenu semantics ).
// the transparent apihook handoff ( hmskin.cpp ) lands here too, so host apps
// keep calling the native API with zero skin code.

// slot lifecycle: one PropId <-> one HWND in the skin ( single-uiroot ).
static HWND s_hWndLastPopup = NULL;

bool hm_lib_popupmenu_show( hm_uiroot* pRoot, HMENU hMenu, int x, int y, HWND hWndOwner ) {
	if ( !pRoot || !hMenu )
		return false;
	if ( pRoot->m_cfg.m_uPopupPropId == 0 ) {
		// manifest has no "PopupSkin" declaration - no engine popup.
		return false;
	}

	// the popup slot is exclusive ( one PropId <-> one HWND in the skin ):
	// release it for BOTH death paths of the previous popup before binding.
	if ( s_hWndLastPopup ) {
		hm_wndpropid_disconnect( pRoot, s_hWndLastPopup, pRoot->m_cfg.m_uPopupPropId );
		if ( ::IsWindow( s_hWndLastPopup ) )
			::DestroyWindow( s_hWndLastPopup );
		s_hWndLastPopup = NULL;
	}
	// the wndctrl object owns the window for the whole modal pump below.
	hm_wndctrl_popup wndPopup;
	if ( !wndPopup.create( x, y ) )
		return false;
	HWND hWndPopup = wndPopup.m_hWnd;
	s_hWndLastPopup = hWndPopup;
	hm_wndpropid_connect( pRoot, hWndPopup, pRoot->m_cfg.m_uPopupPropId );
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
	hm_wndpropid_disconnect( pRoot, hWndPopup, pRoot->m_cfg.m_uPopupPropId );
	if ( s_hWndLastPopup == hWndPopup )
		s_hWndLastPopup = NULL;
	return true;
}

// hm_lib_shutdown tail guard: a popup that outlived its modal pump
// ( host tore down mid-menu ) is destroyed so the slot binding does not
// outlive the engine. returns true when a live popup was found.
bool hm_tpm_destroy_live_popup( hm_uiroot* pRoot ) {
	BOOL bFoundLive = FALSE;
	if ( s_hWndLastPopup && ::IsWindow( s_hWndLastPopup ) ) {
		if ( pRoot )
			hm_wndpropid_disconnect( pRoot, s_hWndLastPopup, pRoot->m_cfg.m_uPopupPropId );
		::DestroyWindow( s_hWndLastPopup );
		bFoundLive = TRUE;
	}
	s_hWndLastPopup = NULL;
	return bFoundLive != FALSE;
}
