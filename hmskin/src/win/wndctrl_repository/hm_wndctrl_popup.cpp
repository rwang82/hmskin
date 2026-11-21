#include "stdafx.h"
#include "tchar.h"
#include "win\wndctrl_repository\hm_wndctrl_popup.h"

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
