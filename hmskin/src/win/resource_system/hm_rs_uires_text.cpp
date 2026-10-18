#include "stdafx.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include "win\paint_system\hm_ps_drawtool_i.h"
//
hm_uires_text_data::hm_uires_text_data()
: m_tstrData( _T("") )
//, m_idRawResFont( hm_rs_rawres_font_factory::ERR_OBJ_ID )
//, m_idRawResColor( hm_rs_rawres_color_factory::ERR_OBJ_ID )
, m_idRawResFont( 0 )
, m_idRawResColor( 4 )
, m_nAlignment( 0 )
, m_eAlginment( EALGINMENT_LEFT )
, m_eTextFlag( TEXTFLAG_SINGLELINE ) {
}
//
hm_uires_text_data::~hm_uires_text_data() {

}

void hm_uires_text_data::setAlignment( int nAlignment ) {
	if ( nAlignment == 1 ) {
		m_eAlginment = EALGINMENT_VCENTER;
	} else if ( nAlignment == 2 ) {
		m_eAlginment = EALGINMENT_RIGHT;
	} else {
		m_eAlginment = EALGINMENT_LEFT;
	}
}

//
hm_rs_uires_text::hm_rs_uires_text( const hm_uires_text_data& data )
: m_data( data )
, m_pRawResFont( NULL )
, m_pRawResColor( NULL )
{
	m_pRawResFont = RAWFONTFACTORY_INSTANCE->getObj( data.m_idRawResFont );
	m_pRawResColor = RAWCOLORFACTORY_INSTANCE->getObj( data.m_idRawResColor );
}
//
hm_rs_uires_text::~hm_rs_uires_text()
{
}

void hm_rs_uires_text::show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool ) 
{
	if ( !pDrawTool ) return;
	bool bMultiLine = false;

	bMultiLine = m_data.m_eTextFlag & hm_uires_text_data::TEXTFLAG_MULTILINE;
	pDrawTool->draw( hDC, rcCanvas, &m_data, m_pRawResFont, m_pRawResColor, bMultiLine );

}
//
bool hm_rs_uires_text::loadRawRes( hm_ps_drawtool_i* pDrawTool ) {
	if ( !pDrawTool ) return false;
	hm_rs_rawres_font* pRawResFont = NULL;
	hm_rs_rawres_color* pRawResColor = NULL;

	pRawResFont = getRawResFont();
	if ( !pDrawTool->loadRes( pRawResFont ) )
		return false;

	pRawResColor = getRawResColor();
	if ( !pDrawTool->loadRes( pRawResColor ) ) {
		pDrawTool->unloadRes( pRawResFont );
		return false;
	}

	return true;
}
//
bool hm_rs_uires_text::unloadRawRes( hm_ps_drawtool_i* pDrawTool ) {
	if ( !pDrawTool ) return false;
	hm_rs_rawres_font* pRawResFont = NULL;
	hm_rs_rawres_color* pRawResColor = NULL;

	pRawResFont = getRawResFont();
	pRawResColor = getRawResColor();

	pDrawTool->unloadRes( pRawResFont );
	pDrawTool->unloadRes( pRawResColor );

	return true;
}
//
hm_rs_rawres_font* hm_rs_uires_text::getRawResFont() {
	if ( m_pRawResFont ) return m_pRawResFont;
	m_pRawResFont = RAWFONTFACTORY_INSTANCE->getObj( m_data.m_idRawResFont );
	return m_pRawResFont;
}
//
hm_rs_rawres_color* hm_rs_uires_text::getRawResColor() {
	if ( m_pRawResColor ) return m_pRawResColor;
	m_pRawResColor = RAWCOLORFACTORY_INSTANCE->getObj( m_data.m_idRawResColor );
	return m_pRawResColor;
}
//
void hm_rs_uires_text::setFontId( unsigned int uFontId ) {
	if ( uFontId == m_data.m_idRawResFont )
		return;
	m_data.m_idRawResFont = uFontId;
	m_pRawResFont = RAWFONTFACTORY_INSTANCE->getObj( m_data.m_idRawResFont );
}
//
void hm_rs_uires_text::setColorId( unsigned int uColorId ) {
	if ( uColorId == m_data.m_idRawResColor )
		return;
	m_data.m_idRawResColor = uColorId;
	m_pRawResColor = RAWCOLORFACTORY_INSTANCE->getObj( m_data.m_idRawResColor );
}

void hm_rs_uires_text::setText( const tstring_type& tstrText ) {
	m_data.m_tstrData = tstrText;
	// observer broadcast: pixel caches ( and future consumers, e.g.
	// re-layout on length change ) react without the caller knowing
	Notify( HM_UIRES_EVENT_TEXTCHANGED );
}