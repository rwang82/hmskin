#include "stdafx.h"
#include "win\frame\hm_uiroot.h"
#include "win\loader\hm_wmp_loader_base.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "common\FuncPack.h"
#include "json.h"
#include <assert.h>

static const char STR_WMPCLSNAME[] = "WMPClsName";
static const char STR_WMPIDWMP[] = "idWMP";

hm_wndmsgprocessor_i* hm_wmp_loader_base::createWMP( const Json::Value& valWMP, hm_uiroot* pHMUIRoot ) {
	if ( !pHMUIRoot )
		return NULL;
	m_dblScale = pHMUIRoot->m_dblLayoutScale;
	hm_wndmsgprocessor_i* pWMPNew = NULL;
	unsigned int uWMPId;
	// create pWMPNew
	pWMPNew = _createWMP( valWMP, pHMUIRoot );
	if ( !pWMPNew ) {
		return NULL;
	}
	CFuncPack fpkDelWMPNew( gfnDelAnyObj< hm_wndmsgprocessor_i >, pWMPNew );
	// set WMPId to pWMPNew.
	if ( _getWMPId( valWMP, uWMPId ) ) {
		pWMPNew->_setId( uWMPId );
	}
	// fill pWMPNew Attribute.
	if ( !_fillWMP( valWMP, pWMPNew ) ) {
		return NULL;
	}
	fpkDelWMPNew.Cancel();
	return pWMPNew;
}

bool hm_wmp_loader_base::_getWMPName( const Json::Value& valWMP, std::string& strWMPName ) {
	Json::Value valWMPName;

	valWMPName = valWMP[ STR_WMPCLSNAME ];
	if ( !valWMPName.isString() ) {
		assert( false );
		return false;
	}
	strWMPName = valWMPName.asString();
	return true;
}

bool hm_wmp_loader_base::_getWMPId( const Json::Value& valWMP, unsigned int& uWMPId ) const {
	Json::Value valWMPId;

	valWMPId = valWMP[ STR_WMPIDWMP ];
	if ( !valWMPId.isInt() ) {
		assert( false );
		return false;
	}
	uWMPId = ( unsigned int )valWMPId.asInt();
	return true;
}


