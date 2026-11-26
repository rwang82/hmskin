#include "stdafx.h"
#include "win\wmp\hm_scroll_core.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"

hm_scroll_core::hm_scroll_core()
: m_nTop( 0 )
, m_nViewH( 0 )
, m_nExtentH( 0 )
, m_nFixedThumbLen( -1 )
, m_uMinThumbLen( 20 ) {
}

void hm_scroll_core::setThumbCfg( int nFixedThumbLen, unsigned int uMinThumbLen ) {
	m_nFixedThumbLen = nFixedThumbLen;
	m_uMinThumbLen = uMinThumbLen;
}

void hm_scroll_core::setViewport( int nViewH ) {
	m_nViewH = nViewH > 0 ? nViewH : 0;
}

void hm_scroll_core::setExtent( int nExtentH ) {
	m_nExtentH = nExtentH > 0 ? nExtentH : 0;
}

int hm_scroll_core::getMaxTop() const {
	int nMax = m_nExtentH - m_nViewH;
	return nMax > 0 ? nMax : 0;
}

bool hm_scroll_core::isAtBottom() const {
	// +2px tolerance, ported from the scrollview's legacy rule.
	return ( m_nTop + m_nViewH ) + 2 >= m_nExtentH;
}

bool hm_scroll_core::onExtentGrown( int nExtentH ) {
	bool bAtBottom = isAtBottom();   // against the OLD extent
	setExtent( nExtentH );
	if ( !bAtBottom )
		return false;   // do-not-disturb: keep the position
	return scrollTo( getMaxTop() );
}

bool hm_scroll_core::scrollTo( int nTop ) {
	if ( nTop < 0 )
		nTop = 0;
	int nMax = getMaxTop();
	if ( nTop > nMax )
		nTop = nMax;
	if ( nTop == m_nTop )
		return false;
	m_nTop = nTop;
	return true;
}

void hm_scroll_core::reset() {
	m_nTop = 0;
}

void hm_scroll_core::syncBar( hm_wmp_scrollbar* pBar, hm_keel_obj* pKeelObjBar ) {
	if ( !pBar )
		return;
	// value range == the pixel overflow; value unit == 1 scaled px.
	pBar->setValRange( 0, getMaxTop() );
	pBar->setCurValue( m_nTop );

	// thumb length: fixed ( golden-look pinning ) or viewport/content ratio
	// floored at the configured minimum.
	int nThumbLen;
	if ( m_nFixedThumbLen >= 0 ) {
		nThumbLen = m_nFixedThumbLen;
	} else {
		int nTrackTotal = 0;
		hm_layout_obj* pLayoutBar = pKeelObjBar ? pKeelObjBar->getLayoutObj() : NULL;
		if ( pLayoutBar )
			nTrackTotal = pLayoutBar->m_rect.bottom - pLayoutBar->m_rect.top;
		nTrackTotal -= pBar->getStartThumbPosDecoder()->m_space1
			+ pBar->getStartThumbPosDecoder()->m_nThickness;
		nTrackTotal -= pBar->getEndThumbPosDecoder()->m_space1
			+ pBar->getEndThumbPosDecoder()->m_nThickness;
		nThumbLen = (int)m_uMinThumbLen;
		if ( m_nExtentH && nTrackTotal > 0 ) {
			nThumbLen = nTrackTotal * m_nViewH / m_nExtentH;
			if ( nThumbLen < (int)m_uMinThumbLen )
				nThumbLen = (int)m_uMinThumbLen;
		}
	}
	pBar->setTrackThumbLen( (unsigned int)nThumbLen );
}
