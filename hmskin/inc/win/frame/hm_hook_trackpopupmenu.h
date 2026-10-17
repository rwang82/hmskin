#ifndef __HM_HOOK_TRACKPOPUPMENU_H__
#define __HM_HOOK_TRACKPOPUPMENU_H__

typedef BOOL ( WINAPI *fnTrackPopupMenuType)(          HMENU hMenu,
    UINT uFlags,
    int x,
    int y,
    int nReserved,
    HWND hWnd,
    CONST RECT * prcRect
);

BOOL WINAPI hmTrackPopupMenu(          HMENU hMenu,
    UINT uFlags,
    int x,
    int y,
    int nReserved,
    HWND hWnd,
    CONST RECT * prcRect
);

// engine-internal ( hmskin.cpp ): root handoff for the transparent hook.
class hm_uiroot;
void hm_tpm_set_root( hm_uiroot* pRoot );

#endif //__HM_HOOK_TRACKPOPUPMENU_H__
