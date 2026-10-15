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

#endif //__HMSKIN_EVENT_DEFS_H__