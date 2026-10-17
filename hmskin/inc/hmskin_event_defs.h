#ifndef __HMSKIN_EVENT_DEFS_H__
#define __HMSKIN_EVENT_DEFS_H__
//////////////////////////////////////////////////////////////////////////
#define HMSKIN_EVENT_BEGIN (0x00000000)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_BTN_CLICK
// lParam1 - specify the pointer of keel object.
// lParam2 - none.
#define HMSKIN_EVENT_BTN_CLICK (0x00000001)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_CHECKBOX_CLICK
// lParam1 - specify the pointer of keel object.
// lParam2 - none.
#define HMSKIN_EVENT_CHECKBOX_CLICK (0x00000002) 
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_NCABTN_CLICK
// lParam1 - specify the pointer of keel object.
// lParam2 - none.
#define HMSKIN_EVENT_NCABTN_CLICK (0x00000003)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_TABBTN_CLICK
// lParam1 - specify the pointer of keel object.
// lParam2 - the uIndex of table button.
#define HMSKIN_EVENT_TABBTN_CLICK (0x00000004)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_ROOTMENU_CLICK
// lParam1 - specify the pointer of keel object.
// lParam2 - the index of the clicked root menu ( menu bar ) item.
#define HMSKIN_EVENT_ROOTMENU_CLICK (0x00000005)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_POPUPMENU_CLICK
// lParam1 - specify the pointer of keel object ( the popup menu window ).
// lParam2 - the command id of the clicked popup menu item.
#define HMSKIN_EVENT_POPUPMENU_CLICK (0x00000006)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_SCROLLBAR_CHANGED
// lParam1 - specify the pointer of keel object ( the scrollbar ).
// lParam2 - the current scrollbar value ( m_nValCur ).
#define HMSKIN_EVENT_SCROLLBAR_CHANGED (0x00000007)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_RADIO_CLICK
// lParam1 - specify the pointer of keel object ( the radio item ).
// lParam2 - always TRUE ( radio style: the clicked item stays checked ).
#define HMSKIN_EVENT_RADIO_CLICK (0x00000008)
//////////////////////////////////////////////////////////////////////////
// HMSKIN_EVENT_TRACKPOPUPMENU
// the app enabled the transparent TrackPopupMenu hook ( opt-in via
// hm_lib_enable_trackpopupmenu_hook ); a native TrackPopupMenu call was
// intercepted. the listener should open its skinned popup at the screen
// position and return - the native menu is NOT shown.
// lParam1 - hm_trackpopupmenu_param* ( hmskin.h ): menu, screen x/y, owner.
// lParam2 - NONE.
#define HMSKIN_EVENT_TRACKPOPUPMENU (0x00000009)

#endif //__HMSKIN_EVENT_DEFS_H__