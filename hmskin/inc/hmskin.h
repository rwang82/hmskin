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
HMLIB_API bool hm_wndpropid_disconnect( hm_uiroot* pHMUIRoot, HWND hWnd, unsigned int idWndProp );
//////////////////////////////////////////////////////////////////////////
class hm_keel_obj;
HMLIB_API bool hm_get_keelobj( hm_uiroot* pHMUIRoot, unsigned int uKeelObjId, hm_keel_obj** ppKeelObj );
//////////////////////////////////////////////////////////////////////////


EXTERN_C_END
#endif //__LIBHM_H__