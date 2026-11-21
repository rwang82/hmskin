#include "stdafx.h"
#include "win\frame\hm_hook_trackpopupmenu.h"
#include "win\frame\hm_uiroot.h"
#include "hmskin.h"   // hm_wndpropid_connect / hm_lib_popupmenu_show
#include "win\hm_wndmsg_defs.h"
#include "win\wndctrl_repository\hm_wndctrl_popup.h"

// engine-driven popup menus: hm_lib_popupmenu_show creates a borderless
// popup window bound to the manifest's PopupSkin slot, binds the HMENU and
// pumps messages until it closes ( native TrackPopupMenu semantics ).
// the transparent apihook ( hm_hs_apihook ) lands here too, so host apps
// keep calling the native API with zero skin code.

// the session root this menu subsystem serves ( single-uiroot contract ):
// written by BOTH entry paths - the transparent hook handoff and every
// popupmenu_show - so direct calls work without the hook being enabled.
static hm_uiroot* s_pMenuRoot = NULL;
static HWND s_hWndLastPopup = NULL;   // slot lifecycle: one PropId <-> one HWND

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
	s_pMenuRoot = pRoot;
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

void hm_tpm_set_root( hm_uiroot* pRoot ) {
	s_pMenuRoot = pRoot;
}

// hm_lib_shutdown tail guard: a popup that outlived its modal pump
// ( host tore down mid-menu ) is destroyed so the slot binding does not
// outlive the engine. returns true when a live popup was found.
bool hm_tpm_destroy_live_popup() {
	BOOL bFoundLive = FALSE;
	if ( s_hWndLastPopup && ::IsWindow( s_hWndLastPopup ) ) {
		if ( s_pMenuRoot )
			hm_wndpropid_disconnect( s_pMenuRoot, s_hWndLastPopup, s_pMenuRoot->m_cfg.m_uPopupPropId );
		::DestroyWindow( s_hWndLastPopup );
		bFoundLive = TRUE;
	}
	s_hWndLastPopup = NULL;
	return bFoundLive != FALSE;
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
	if ( s_pMenuRoot && hm_lib_popupmenu_show( s_pMenuRoot, hMenu, x, y, hWnd ) )
		return TRUE;
	return FALSE;
}
