#include "stdafx.h"
#include "win\loader\hm_wmp_loader_depot.h"
#include "win\loader\hm_wmp_loaders.h"
#include "json.h"
#include <assert.h>
//
#define ADD_WMP_LOADER( wmp_cls_name ) \
	m_mapName2WMPLoader[ #wmp_cls_name ] = new wmp_cls_name##_loader();
const char STR_WMPCLSNAME[] = "WMPClsName";
//
hm_wmp_loader_depot::hm_wmp_loader_depot() {
	_init();
}

hm_wmp_loader_depot::~hm_wmp_loader_depot() {
	_unInit();
}

hm_wmp_loader_base* hm_wmp_loader_depot::getWMPLoader( const Json::Value& valWMP ) {
	name2wmploader_map_type::iterator itN2L, iendN2L;
	hm_wmp_loader_base* pWMPLoader = NULL;
	std::string strWMPName;

	if ( !_getWMPName( valWMP, strWMPName ) ) {
		assert( false );
		return NULL;
	}
	iendN2L = m_mapName2WMPLoader.end();
	for ( itN2L = m_mapName2WMPLoader.begin(); itN2L != iendN2L; ++itN2L ) {
		if ( strWMPName == itN2L->first ) {
			return itN2L->second;
		}
	}

	assert( false );
	return NULL;
}

void hm_wmp_loader_depot::_init() {

	ADD_WMP_LOADER( hm_wmp_bkgnd );
	ADD_WMP_LOADER( hm_wmp_bkgnd_nca );
	ADD_WMP_LOADER( hm_wmp_button );
	ADD_WMP_LOADER( hm_wmp_button_nca );
	ADD_WMP_LOADER( hm_wmp_dialogview );
	ADD_WMP_LOADER( hm_wmp_drawtrigger );
	ADD_WMP_LOADER( hm_wmp_drawtrigger_nca );
	ADD_WMP_LOADER( hm_wmp_layouttrigger );
	ADD_WMP_LOADER( hm_wmp_movewnd );
	ADD_WMP_LOADER( hm_wmp_popupmenu );
	ADD_WMP_LOADER( hm_wmp_rootmenu );
	ADD_WMP_LOADER( hm_wmp_scrollbar );
	ADD_WMP_LOADER( hm_wmp_tabheader );
	ADD_WMP_LOADER( hm_wmp_viewbkcontent );
	ADD_WMP_LOADER( hm_wmp_checkbox );
	ADD_WMP_LOADER( hm_wmp_radio );
}

void hm_wmp_loader_depot::_unInit() {
	name2wmploader_map_type::iterator itN2L, iendN2L;
	hm_wmp_loader_base* pWMPLoader = NULL;

	iendN2L = m_mapName2WMPLoader.end();
	for ( itN2L = m_mapName2WMPLoader.begin(); itN2L != iendN2L; ++itN2L ) {
		pWMPLoader = itN2L->second;
		if ( pWMPLoader ) {
			delete pWMPLoader;
			pWMPLoader = NULL;
		}
	}
	m_mapName2WMPLoader.clear();
}

bool hm_wmp_loader_depot::_getWMPName( const Json::Value& valWMP, std::string& strWMPName ) {
	Json::Value valWMPName;

	valWMPName = valWMP[ STR_WMPCLSNAME ];
	if ( !valWMPName.isString() ) {
		assert( false );
		return false;
	}
	strWMPName = valWMPName.asString();
	return true;
}