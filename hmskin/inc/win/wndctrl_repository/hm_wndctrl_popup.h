#ifndef __HM_WNDCTRL_POPUP_H__
#define __HM_WNDCTRL_POPUP_H__
#include "win\wndctrl_repository\hm_wndctrl_base.h"

// borderless engine popup window ( transparent TrackPopupMenu takeover ).
// WndProc stays the base default ( DefWindowProc pass-through ): the skin
// dispatcher subclass attached via hm_wndpropid_connect does all the work.
class hm_wndctrl_popup : public hm_wndctrl_base {
public:
	hm_wndctrl_popup();
	virtual ~hm_wndctrl_popup();

	// CS_SAVEBITS + null background: moving the popup never repaints what
	// is underneath, and no class brush flashes before the skin paints.
	static bool ensureCls();
	bool create( int x, int y );

protected:
	virtual LPCTSTR _wndClsName() const { return s_szPopupClsName; }

private:
	static const TCHAR s_szPopupClsName[];
	static ATOM s_atomPopupCls;
};

class hm_uiroot;

// engine-internal ( hmskin.cpp shutdown ): destroy a popup that outlived
// its modal pump. returns true = a live popup was found.
bool hm_tpm_destroy_live_popup( hm_uiroot* pRoot );

#endif //__HM_WNDCTRL_POPUP_H__
