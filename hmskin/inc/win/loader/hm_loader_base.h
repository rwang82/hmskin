#ifndef __HM_LOADER_BASE_H__
#define __HM_LOADER_BASE_H__
#include "windows.h"
#include "tchar.h"
#include <assert.h>
#include "win\loader\hm_manifest_json.h"
#include "win\loader\hm_manifest_xml.h"

// manifest parse failure with a Release-visible breadcrumb: the file:line
// pinpoints the exact parse site ( a bare assert left Release builds with
// nothing but a failed hm_lib_startup ).
#define HM_LOADER_FAIL( ret ) \
	do { \
		assert( false ); \
		TCHAR szFail[ 128 ]; \
		_stprintf_s( szFail, 128, _T("hmskin: manifest parse failed at %hs:%d\n"), __FILE__, __LINE__ ); \
		::OutputDebugString( szFail ); \
		return ret; \
	} while ( 0 )

// same breadcrumb for TOLERATED failures ( the item is skipped and the
// load continues - e.g. hm_loader_wmp's per-entry continue ): silent
// absence in Release is worse than a failed startup.
#define HM_LOADER_SKIP() \
	do { \
		assert( false ); \
		TCHAR szSkip[ 128 ]; \
		_stprintf_s( szSkip, 128, _T("hmskin: manifest item SKIPPED at %hs:%d\n"), __FILE__, __LINE__ ); \
		::OutputDebugString( szSkip ); \
	} while ( 0 )

//
class hm_uiroot;
//
class hm_loader_base {
public:
	hm_loader_base(){};
	virtual ~hm_loader_base(){};

	// interface of hm_loader_base
public:
	virtual bool load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot ) = 0;
	virtual bool load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot ) = 0;

};




#endif //__HM_LOADER_BASE_H__