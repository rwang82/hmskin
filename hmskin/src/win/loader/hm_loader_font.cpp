#include "stdafx.h"
#include "win\loader\hm_loader_font.h"
#include "win\frame\hm_uiroot.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\hm_win_misc.h"

hm_loader_font::hm_loader_font() {

}

hm_loader_font::~hm_loader_font() {

}

bool hm_loader_font::load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot ) {
	if ( !pManifestData ) return false;
	Json::Value& rootVal = pManifestData->getRootVal();
	Json::Value aryFontVal;
	Json::Value fontVal;
	Json::Value valTmp;
	unsigned int uIndex, uCount;
	int nIdResFont;
	int nFontStyle;
	int nFontUnit;
	int nFontSize;
	std::string strTmp;
	std::basic_string< TCHAR > tstrFaceName;
	hm_rs_font_param fontParam;

	aryFontVal = rootVal[ "Font" ];
	if ( aryFontVal.isNull() )
		return false;
	if ( !aryFontVal.isArray() )
		return false;

	uCount = aryFontVal.size();
	for ( uIndex = 0; uIndex<uCount; ++uIndex ) {
		fontVal = aryFontVal[ uIndex ];
		if ( !fontVal.isObject() )
			return false;

		// id
		valTmp = fontVal["id"];
		if ( !valTmp.isInt() )
			return false;
		nIdResFont = valTmp.asInt();

		// face name
		valTmp = fontVal["FaceName"];
		if ( !valTmp.isString() )
			return false;
		strTmp = valTmp.asString();
		if ( !MultiByteToTChar( strTmp, tstrFaceName ) )
			return false;

		// style
		valTmp = fontVal["Style"];
		if ( !valTmp.isString() )
			return false;
		strTmp = valTmp.asString();
		if ( strcmp( "REGULAR", strTmp.c_str() ) == 0 ) {
			nFontStyle = FONTSTYLE_REGULAR;
		} else if ( strcmp( "BOLD", strTmp.c_str() ) == 0 ) {
			nFontStyle = FONTSTYLE_BOLD;
		} else if ( strcmp( "ITALIC", strTmp.c_str() ) == 0 ) {
			nFontStyle = FONTSTYLE_ITALIC;
		} else {
			return false;
		}

		// SizeUnit
		valTmp = fontVal["SizeUnit"];
		if ( !valTmp.isString() )
			return false;
		strTmp = valTmp.asString();
		if ( strcmp( "POINT", strTmp.c_str() ) == 0 ) {
			nFontUnit = FONTUNIT_POINT;
		} else if ( strcmp( "PIXEL", strTmp.c_str() ) == 0 ) {
			nFontUnit = FONTUNIT_PIXEL;
		} else {
			return false;
		}

		// Size
		valTmp = fontVal["Size"];
		if ( !valTmp.isInt() )
			return false;
		nFontSize = (int)( valTmp.asInt() * pHMUIRoot->m_dblLayoutScale + 0.5 );  // GDI+ converts POINT on a fixed 96dpi grid - scale to physical

		//
		fontParam.m_strFaceName = tstrFaceName;
		fontParam.m_uFlagStyle = nFontStyle;
		fontParam.m_uFlagSizeUnit = nFontUnit;
		fontParam.m_uSize = nFontSize;
		RAWFONTFACTORY_INSTANCE->createObj< hm_rs_rawres_font, hm_rs_font_param >( nIdResFont, fontParam );
	}
	
	
	

	return true;
}

bool hm_loader_font::load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot ) {

	return true;
}