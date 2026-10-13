#include "stdafx.h"
#include "win\loader\hm_loader_wndstyle.h"
#include "win\frame\hm_uiroot.h"
#include <assert.h>

hm_loader_wndstyle::hm_loader_wndstyle() {}
hm_loader_wndstyle::~hm_loader_wndstyle() {}

bool hm_loader_wndstyle::load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot ) {
	if ( !pManifestData || !pHMUIRoot )
		return false;
	Json::Value& rootVal = pManifestData->getRootVal();
	Json::Value valStyle;
	Json::Value valCorner;

	valStyle = rootVal[ "WndStyle" ];
	if ( valStyle.isNull() )
		return true; // optional section : absent means "don't touch system default"
	if ( !valStyle.isObject() )
		return false;

	valCorner = valStyle[ "Corner" ];
	if ( valCorner.isNull() )
		return true;
	if ( !valCorner.isString() )
		return false;

	std::string strCorner = valCorner.asString();
	if ( strCorner == "DEFAULT" )
		pHMUIRoot->m_nWndCornerPref = HM_WNDCORNER_DEFAULT;
	else if ( strCorner == "DONOTROUND" )
		pHMUIRoot->m_nWndCornerPref = HM_WNDCORNER_DONOTROUND;
	else if ( strCorner == "ROUND" )
		pHMUIRoot->m_nWndCornerPref = HM_WNDCORNER_ROUND;
	else if ( strCorner == "ROUNDSMALL" )
		pHMUIRoot->m_nWndCornerPref = HM_WNDCORNER_ROUNDSMALL;
	else {
		assert( false ); // unknown WndStyle.Corner value
		return false;
	}
	return true;
}

bool hm_loader_wndstyle::load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot ) {

	return true;
}
