#include "stdafx.h"
#include "hmskin_helper.h"
#include "win/wmp/hm_wmp_display.h"
#include "win/resource_system/hm_rs_uires_text.h"
#include "win/frame/hm_uiroot.h"
#include "win/frame/hm_keel_obj.h"
#include "win/frame/hm_layout_obj.h"
#include <assert.h>
#include <stdio.h>
#include <string>
//

//
unsigned int getWMPDisplayText( void* pWMPDisplayRaw, LPTSTR lpszStringBuf, int nMaxTextCount, int nDistrictId ) {
	hm_wmp_display* pWMPDisplay = ( hm_wmp_display* )pWMPDisplayRaw;
	hm_districtobj* pDistrictObj = NULL;
	hm_showobj* pShowObj = NULL;
	unsigned int uShowObjCount;
	hm_rs_uires_text* pUIResText = NULL;
	const hm_uires_text_data* pTextData = NULL;
	unsigned int uLenText = 0 ;
	unsigned int uLenTextDoCopy = 0;

	if ( pWMPDisplay->getDistrictObj( nDistrictId, &pDistrictObj ) ) {
		uShowObjCount = pDistrictObj->getShowObjCount();
		for ( unsigned int uShowObjIndex = 0; uShowObjIndex < uShowObjCount; ++uShowObjIndex ) {
			pShowObj = pDistrictObj->getShowObj( uShowObjIndex );
			if ( !pShowObj ) 
				continue;
			pUIResText = dynamic_cast<hm_rs_uires_text*>( pShowObj->m_pUIRes );
			if ( !pUIResText )
				continue;
			pTextData = pUIResText->getData();
			if ( pTextData ) {
				uLenText = pTextData->m_tstrData.length();
				if ( !lpszStringBuf ) {
					return uLenText + 1; // return the charicter count of buffer need.
				} else {
					if ( nMaxTextCount == 0 )
						return 0; // it specify no buffer.
					uLenTextDoCopy = ( unsigned int )min( ( unsigned int )( nMaxTextCount - 1 ), uLenText );
					_tcsncpy_s( lpszStringBuf, nMaxTextCount, pTextData->m_tstrData.c_str(), uLenTextDoCopy );
					return uLenTextDoCopy;
				}
				assert( false );
			}
		}
	}
	
	return 0;
}
//
// ( text SETTING moved to the public hm_wmp_set_text / _at in hmskin.h -
// the engine's uires observer chain invalidates pixel caches itself )
// ---- AI-agent / debugging aid: keelobj tree dump ( hand-rolled JSON: the
// demo does not link jsoncpp, and the payload is numbers/bools only, so no
// escaping is needed ). the factory enumObjs walk follows the --drawtool
// precedent in demo.cpp.
struct hmsh_dump_ctx {
	HWND hWnd;
	std::basic_string< TCHAR > strJson;
	unsigned int nCount;
};

static void __cdecl hmsh_dump_one( hm_keel_obj* pKeelObj, void* pParam ) {
	hmsh_dump_ctx* p = ( hmsh_dump_ctx* )pParam;
	hm_keel_obj* pParent = pKeelObj->getParent();
	hm_layout_obj* pLayout = pKeelObj->getLayoutObj();
	RECT rc;
	BOOL bMapped = FALSE;
	if ( pLayout && pLayout->hasCoordinateSystem() )
		bMapped = pLayout->mapPosition( p->hWnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rc );
	if ( p->nCount )
		p->strJson += _T(",\n");
	TCHAR szNode[ 256 ];
	if ( bMapped ) {
		_stprintf_s( szNode, 256,
			_T("    { \"id\": %u, \"parentId\": %d, \"visible\": %s, \"rect\": { \"l\": %ld, \"t\": %ld, \"r\": %ld, \"b\": %ld } }"),
			pKeelObj->getId(),
			pParent ? (int)pParent->getId() : -1,
			pKeelObj->isVisible() ? _T("true") : _T("false"),
			rc.left, rc.top, rc.right, rc.bottom );
	} else {
		RECT rcEmpty = { 0, 0, 0, 0 };
		const RECT& rcRaw = pLayout ? pLayout->m_rect : rcEmpty;
		_stprintf_s( szNode, 256,
			_T("    { \"id\": %u, \"parentId\": %d, \"visible\": %s, \"rect\": null, \"rectRaw\": { \"l\": %ld, \"t\": %ld, \"r\": %ld, \"b\": %ld } }"),
			pKeelObj->getId(),
			pParent ? (int)pParent->getId() : -1,
			pKeelObj->isVisible() ? _T("true") : _T("false"),
			rcRaw.left, rcRaw.top, rcRaw.right, rcRaw.bottom );
	}
	p->strJson += szNode;
	p->nCount++;
}

unsigned int hmsh_dump_tree( void* pHMUIRootRaw, HWND hWndMain, const TCHAR* szJsonPath ) {
	hm_uiroot* pRoot = ( hm_uiroot* )pHMUIRootRaw;
	if ( !pRoot || !pRoot->m_pKeelObjFactory || !szJsonPath || !szJsonPath[ 0 ] || !hWndMain )
		return 0;
	hmsh_dump_ctx ctx;
	ctx.hWnd = hWndMain;
	ctx.nCount = 0;
	TCHAR szHead[ 64 ];
	_stprintf_s( szHead, 64, _T("{\n  \"hwnd\": %lu,\n  \"hmskin_tree\": [\n"), (unsigned long)(ULONG_PTR)hWndMain );
	ctx.strJson = szHead;
	pRoot->m_pKeelObjFactory->enumObjs( hmsh_dump_one, &ctx );
	ctx.strJson += _T("\n  ]\n}\n");
	FILE* fp = _tfopen( szJsonPath, _T("w") );
	if ( !fp )
		return 0;
	_fputts( ctx.strJson.c_str(), fp );
	fclose( fp );
	return ctx.nCount;
}
