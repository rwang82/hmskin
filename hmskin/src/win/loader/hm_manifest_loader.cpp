#include "stdafx.h"
#include "win\loader\hm_manifest_loader.h"
#include "win\loader\hm_manifest_creator.h"
#include "win\loader\hm_loader_wndstyle.h"
#include "win\loader\hm_loader_coordinatesystem.h"
#include "win\loader\hm_loader_rawpic.h"
#include "win\loader\hm_loader_font.h"
#include "win\loader\hm_loader_color.h"
#include "win\loader\hm_loader_wmp.h"
#include "win\loader\hm_loader_msgsink.h"
#include "win\loader\hm_loader_keelobj.h"

hm_manifest_loader::hm_manifest_loader( const TCHAR* szUIManifestFilePath )
: m_strManifestFilePath( szUIManifestFilePath ? szUIManifestFilePath : _T("") )
, m_pManifestData( NULL ) {
	init();
}

hm_manifest_loader::~hm_manifest_loader() {
	clear();
}

bool hm_manifest_loader::load( hm_uiroot* pHMUIRoot ) {
	if ( m_pManifestData || !pHMUIRoot ) return false;
	m_pManifestData = CreateManifestData( m_strManifestFilePath.c_str() );
	if ( !m_pManifestData ) return false;

	loader_container_type::iterator itLoader, iendLoader;
	hm_loader_base* pLoader = NULL;
	
	iendLoader = m_containerLoader.end();
	for ( itLoader = m_containerLoader.begin(); itLoader!=iendLoader; ++itLoader ) {
		pLoader = *itLoader;
		if ( !m_pManifestData->visit( pLoader, pHMUIRoot ) )
			return false;
	}
	return true;
}

void hm_manifest_loader::init() {
#define REG_LOADER_ITEM( cls_loader ) \
	m_containerLoader.push_back( new cls_loader() );

	REG_LOADER_ITEM( hm_loader_wndstyle );
	REG_LOADER_ITEM( hm_loader_coordinatesystem );
	REG_LOADER_ITEM( hm_loader_rawpic );
	REG_LOADER_ITEM( hm_loader_font );
	REG_LOADER_ITEM( hm_loader_color );
	REG_LOADER_ITEM( hm_loader_wmp ); // must before hm_loader_keelobj.
	REG_LOADER_ITEM( hm_loader_msgsink ); // must before hm_loader_keelobj.
	REG_LOADER_ITEM( hm_loader_keelobj );
}

void hm_manifest_loader::clear() {
	loader_container_type::iterator itLoader, iendLoader;
	hm_loader_base* pLoader = NULL;

	iendLoader = m_containerLoader.end();
	for ( itLoader = m_containerLoader.begin(); itLoader!=iendLoader; ++itLoader ) {
		pLoader = *itLoader;
		delete pLoader;
		pLoader = NULL;
	}
	m_containerLoader.clear();
	
	//
	if ( m_pManifestData )
		delete m_pManifestData;
	m_pManifestData = NULL;
}
