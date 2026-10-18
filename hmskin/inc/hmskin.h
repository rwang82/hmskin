#ifndef __LIBHM_H__
#define __LIBHM_H__
#include "tchar.h"
#include "win\hm_event_defs.h"
#include "win\hm_export_defs.h"

//
EXTERN_C_START

//////////////////////////////////////////////////////////////////////////
//uuid define
#define UUID_TSTR_WNDPROP_ID _T("170AEF81-E698-4313-B2CA-FD13FFDDF6B0")
//////////////////////////////////////////////////////////////////////////
class hmskin_event_listener_base;
struct hm_startup_param {
	// uDpi: the DPI the skin should be laid out for ( manifests are
	// authored on a 96dpi baseline, the loader scales geometry up ).
	// default 96 = no scaling.
	hm_startup_param( HINSTANCE hInst, hmskin_event_listener_base* pHMSkinEventListener, unsigned int uDpi = 96 )
		: m_hInst( hInst )
		, m_pHMSkinEventListener( pHMSkinEventListener )
		, m_uDpi( uDpi ) {}
	~hm_startup_param(){}

	HINSTANCE m_hInst;	
	hmskin_event_listener_base* m_pHMSkinEventListener;
	unsigned int m_uDpi;
};
class hm_uiroot;
HMLIB_API bool hm_lib_startup( hm_startup_param* pParam, const TCHAR* szResFilePath, hm_uiroot** ppHMUIRoot );
//////////////////////////////////////////////////////////////////////////
HMLIB_API void hm_lib_shutdown( hm_uiroot* pHMUIRoot );
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_connect( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
//////////////////////////////////////////////////////////////////////////
// transparent TrackPopupMenu skinning ( opt-in ): patches user32!TrackPopupMenu;
// native calls are redirected to HMSKIN_EVENT_TRACKPOPUPMENU so the app
// listener can open its skinned popup. the listener MUST handle the event
// ( there is no forwarding trampoline ). disable before shutdown is not
// required - hm_lib_shutdown unhooks itself.
// transparent mode: native calls are handled ENGINE-side ( hm_lib_popupmenu_show
// with the manifest "PopupSkin" slot ); selection is delivered as native
// WM_COMMAND - apps need NO listener code for hooked menus.
HMLIB_API bool hm_lib_enable_trackpopupmenu_hook( hm_uiroot* pHMUIRoot, BOOL bEnable );
// engine popup: show a skinned popup menu for hMenu at SCREEN x/y; the
// selected command is delivered to hWndOwner as native WM_COMMAND ( plus
// HMSKIN_EVENT_POPUPMENU_CLICK ). requires the manifest "PopupSkin" slot.
HMLIB_API bool hm_lib_popupmenu_show( hm_uiroot* pHMUIRoot, HMENU hMenu, int x, int y, HWND hWndOwner );
//////////////////////////////////////////////////////////////////////////
HMLIB_API bool hm_wndpropid_disconnect( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
//////////////////////////////////////////////////////////////////////////
class hm_keel_obj;
HMLIB_API bool hm_get_keelobj( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, hm_keel_obj** ppKeelObj );
//////////////////////////////////////////////////////////////////////////
// id-based keelobj runtime controls ( chapter switching, scroll-view
// container assembly ): hosts speak manifest ids, no keelobj pointers.
HMLIB_API bool hm_keelobj_set_visible( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, BOOL bVisible );
HMLIB_API bool hm_keelobj_set_clip_children( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, BOOL bClip );
//////////////////////////////////////////////////////////////////////////
// host text API: change a WMP's text by the ids the host wrote into its
// manifest - no engine internals. set_text targets the FIRST text uires in
// declaration order ( single-text WMPs ); set_text_at targets the text
// showobj at ( manifest district id, declaration index ) for multi-text
// WMPs. both invalidate the WMP's pixel cache automatically.
HMLIB_API bool hm_wmp_set_text( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, unsigned int uWMPId, const TCHAR* szText );
HMLIB_API bool hm_wmp_set_text_at( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, unsigned int uWMPId, int nDistrictId, unsigned int uShowObjIndex, const TCHAR* szText );
//////////////////////////////////////////////////////////////////////////


EXTERN_C_END
#endif //__LIBHM_H__