#include "stdafx.h"
#include "win\loader\hm_loader_popupskin.h"
#include "win\frame\hm_uiroot.h"
#include <assert.h>

hm_loader_popupskin::hm_loader_popupskin() {}
hm_loader_popupskin::~hm_loader_popupskin() {}

bool hm_loader_popupskin::load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot ) {
	if ( !pManifestData || !pHMUIRoot )
		return false;
	Json::Value& rootVal = pManifestData->getRootVal();
	Json::Value valPopupSkin;
	Json::Value valPropId;

	valPopupSkin = rootVal[ "PopupSkin" ];
	if ( valPopupSkin.isNull() )
		return true; // optional section: engine popups stay disabled
	if ( !valPopupSkin.isObject() )
		return false;

	valPropId = valPopupSkin[ "PropId" ];
	if ( !valPropId.isInt() ) {
		assert( false );
		return false;
	}
	pHMUIRoot->m_uPopupPropId = (unsigned int)valPropId.asInt();
	return true;
}

bool hm_loader_popupskin::load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot ) {
	return true;
}
