#include "stdafx.h"
#include "win\frame\hm_uiroot.h"
#include "win\loader\hm_loader_wmp.h"
#include "win\loader\hm_wmp_loader_depot.h"
#include "win\loader\hm_wmp_loader_base.h"
#include "AtlConv.h"
#include <assert.h>

hm_loader_wmp::hm_loader_wmp()
: m_pWMPLoaderDepot( new hm_wmp_loader_depot() ) {

}

hm_loader_wmp::~hm_loader_wmp() {
	if ( m_pWMPLoaderDepot ) {
		delete m_pWMPLoaderDepot;
		m_pWMPLoaderDepot = NULL;
	}
}

bool hm_loader_wmp::load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot ) {
	if ( !pManifestData || !pHMUIRoot ) return false;
	Json::Value& rootVal = pManifestData->getRootVal();
	Json::Value aryWMPVal;
	Json::Value wmpVal;
	Json::Value valTmp;
	unsigned int uIndex, uCount;
	hm_wndmsgprocessor_i* pWMPNew = NULL;
	hm_wmp_loader_base* pWMPLoader = NULL;

	aryWMPVal = rootVal[ "WndMsgProcessor" ];
	if ( !aryWMPVal.isArray() ) {
		assert( FALSE );
		return false;
	}
	uCount = aryWMPVal.size();
	for ( uIndex = 0; uIndex < uCount; ++uIndex ) {
		wmpVal = aryWMPVal[ uIndex ];
		if ( !wmpVal.isObject() ) {
			assert( FALSE );
			return false;
		}
		// get pWMPLoader.
		pWMPLoader = m_pWMPLoaderDepot->getWMPLoader( wmpVal );
		if ( !pWMPLoader ) {
			HM_LOADER_SKIP();
			continue;
		}
		// create pWMPNew.
		pWMPNew = pWMPLoader->createWMP( wmpVal, pHMUIRoot );
		if ( !pWMPNew ) {
			HM_LOADER_SKIP();
			continue;
		}
	}

	return true;
}

bool hm_loader_wmp::load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot ) {
	return true;
}
