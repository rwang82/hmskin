#include "stdafx.h"
#include "win\frame\hm_pos_decoders.h"

hm_posdecoder_dock::hm_posdecoder_dock( ENUMDOCKTYPE eDockType, int nSpace1, int nSpace2, int nSpace3, int nThickness )
: m_eDockType( eDockType )
, m_space1( nSpace1 )
, m_space2( nSpace2 )
, m_space3( nSpace3 )
, m_nThickness( nThickness ) {
	
}
//
hm_posdecoder_dock::~hm_posdecoder_dock() {}
//
bool hm_posdecoder_dock::getPosition( const RECT& rcRefObj, RECT& rcDestObj ) {
	switch ( m_eDockType ) {
	case DOCK_LEFT:
		{
			rcDestObj.left = rcRefObj.left + m_space1;
			rcDestObj.top = rcRefObj.top + m_space2;
			rcDestObj.right = rcDestObj.left + m_nThickness;
			rcDestObj.bottom = rcRefObj.bottom - m_space3;
		}
		break;
	case DOCK_TOP:
		{
			rcDestObj.left = rcRefObj.left + m_space2;
			rcDestObj.top = rcRefObj.top + m_space1;
			rcDestObj.right = rcRefObj.right - m_space3;
			rcDestObj.bottom = rcDestObj.top + m_nThickness;
		}
		break;
	case DOCK_RIGHT:
		{
			rcDestObj.right = rcRefObj.right - m_space1;
			rcDestObj.left = rcDestObj.right - m_nThickness;
			rcDestObj.top = rcRefObj.top + m_space2;
			rcDestObj.bottom = rcRefObj.bottom - m_space3;
		}
		break;
	case DOCK_BOTTOM:
		{
			rcDestObj.left = rcRefObj.left + m_space2;
			rcDestObj.right = rcRefObj.right - m_space3;
			rcDestObj.bottom = rcRefObj.bottom - m_space1;
			rcDestObj.top = rcDestObj.bottom - m_nThickness;
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}
//
hm_posdecoder_slider::hm_posdecoder_slider( ENUMSLIDERTYPE eSliderType, int space1, int space2, int distance, int nThickness ) 
: m_eSliderType( eSliderType )
, m_space1( space1 )
, m_space2( space2 )
, m_distance( distance )
, m_nThickness( nThickness )
{}
//
hm_posdecoder_slider::~hm_posdecoder_slider() 
{}
//
bool hm_posdecoder_slider::getPosition( const RECT& rcRefObj, RECT& rcDestObj ) {
	switch ( m_eSliderType )
	{
	case SLIDER_T2B:
		{
			rcDestObj.left = rcRefObj.left + m_space1;
			rcDestObj.right = rcRefObj.right - m_space2;
			rcDestObj.top = rcRefObj.top + m_distance - m_nThickness/2;
			rcDestObj.bottom = rcDestObj.top + m_nThickness;
		}
		break;
	case SLIDER_L2R:
		{
			rcDestObj.left = rcRefObj.left + m_distance - m_nThickness/2;
			rcDestObj.right = rcDestObj.left + m_nThickness;
			rcDestObj.top = rcRefObj.top + m_space1;
			rcDestObj.bottom = rcRefObj.bottom - m_space2;
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////
hm_posdecoder_offset::hm_posdecoder_offset( int nOffsetL, int nOffsetT, int nOffsetR, int nOffsetB )
: m_nOffsetL( nOffsetL )
, m_nOffsetT( nOffsetT )
, m_nOffsetR( nOffsetR )
, m_nOffsetB( nOffsetB ) {

}
//
hm_posdecoder_offset::~hm_posdecoder_offset()
{}
//
bool hm_posdecoder_offset::getPosition( const RECT& rcRefObj, RECT& rcDestObj ) {
	rcDestObj.left = rcRefObj.left + m_nOffsetL;
	rcDestObj.top = rcRefObj.top + m_nOffsetT;
	rcDestObj.right = rcRefObj.right - m_nOffsetR;
	rcDestObj.bottom = rcRefObj.bottom - m_nOffsetB;
	return true;
}