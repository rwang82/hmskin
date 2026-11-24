#include "stdafx.h"
#include "win\wmp\hm_wmp_msgout.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_event_defs.h"
#include "win\frame\hm_debug_utils.h"
#include "win\hm_wndmsg_defs.h"
#include "win\paint_system\hm_ps_drawtool_i.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include "win\hook_system\hm_hs_whellengine.h"
#include <gdiplus.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
hm_wmp_msgout::MSGOUT_CFG::MSGOUT_CFG()
: m_pRawFontText( NULL )
, m_pRawClrText( NULL )
, m_pRawClrSystem( NULL )
, m_hFontText( NULL )
, m_clrText( RGB( 232, 236, 241 ) )
, m_clrBubbleSelf( RGB( 0x95, 0xEC, 0x69 ) )
, m_clrBubblePeer( RGB( 0xFF, 0xFF, 0xFF ) )
, m_clrSystem( RGB( 139, 147, 158 ) )
, m_clrAvatarSelf( RGB( 0x95, 0xEC, 0x69 ) )
, m_clrAvatarPeer( RGB( 0x8B, 0x93, 0x9E ) )
, m_nAvatarLen( 40 )
, m_nMaxBubblePct( 80 )
, m_nMsgGap( 10 )
, m_nPadH( 12 )
, m_nPadV( 8 )
, m_nCornerLen( 8 )
{
}

hm_wmp_msgout::hm_wmp_msgout( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_nContentH( 0 )
, m_nCanvasW( 0 )
, m_bWheelStopped( false )
{
	// 60fps inertia ticks: half the per-tick distance at double the
	// cadence - the same total distance reads far smoother ( FRAME24's
	// 41ms ticks with 30-90px steps looked jumpy on fast flicks ).
	WHELLENGINE_INSTANCE->setFrameMode( hm_hs_whellengine::FRAME60 );
}

hm_wmp_msgout::~hm_wmp_msgout() {
}

//
MSGBIND_BEGIN( hm_wmp_msgout )
MSGBIND( HMWM_MOUSEWHEEL, _OnHMMouseWhell )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_msgout )
ORDERWNDMSG( HMWM_MOUSEWHEEL )
ORDERWNDMSG_END()

void hm_wmp_msgout::_createDistrictObjs() {
	// no districts - everything renders directly in _render.
}

void hm_wmp_msgout::_getAllDistrictId( district_id_container_type& containerIds ) {
	UNREFERENCED_PARAMETER( containerIds );
}

void hm_wmp_msgout::setConfigure( const MSGOUT_CFG& cfgScaled ) {
	m_cfg = cfgScaled;
}

void hm_wmp_msgout::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	UNREFERENCED_PARAMETER( pSubject );
	UNREFERENCED_PARAMETER( pEventParam2 );
	UNREFERENCED_PARAMETER( pbContinue );
	switch ( idEvent ) {
	case HM_EVENT_DRAWSELF:
		{
			HEDSPARAM* pHEDSParam = (HEDSPARAM*)pEventParam;
			if ( !pHEDSParam )
				return;
			RECT rcCanvas;
			if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas,
					coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
				return;
			_render( pHEDSParam->m_hDCCanvas, rcCanvas );
		}
		break;
	case HM_EVENT_KEELOBJ_POSCHANGED:
		{ // the canvas ( translated by the scroll offset, sized by the viewport )
			// settled - a width change invalidates every cached bubble rect.
			hm_keel_obj* pKeelObj = (hm_keel_obj*)pSubject;
			hm_layout_obj* pLayout = pKeelObj ? pKeelObj->getLayoutObj() : NULL;
			if ( !pLayout )
				return;
			int nCanvasW = pLayout->m_rect.right - pLayout->m_rect.left;
			if ( nCanvasW > 0 && nCanvasW != m_nCanvasW ) {
				m_nCanvasW = nCanvasW;
				_measureAll();
			}
		}
		break;
	default:
		break;
	}
}

bool hm_wmp_msgout::appendText( int eSender, const TCHAR* szText ) {
	if ( !szText || !szText[ 0 ] )
		return false;
	if ( m_nCanvasW <= 0 ) {
		// no canvas geometry yet ( e.g. headless early call ) - defer to the
		// next POSCHANGED, which re-measures everything.
		MsgItem msg;
		msg.m_eSender = eSender;
		msg.m_strText = szText;
		::SetRect( &msg.m_rcRow, 0, 0, 0, 0 );
		::SetRect( &msg.m_rcBubble, 0, 0, 0, 0 );
		m_containerMsgs.push_back( msg );
		return true;
	}
	MsgItem msg;
	msg.m_eSender = eSender;
	msg.m_strText = szText;
	if ( !_measureItem( msg, m_nCanvasW ) )
		return false;
	// stack below the current content.
	::OffsetRect( &msg.m_rcRow, 0, m_nContentH );
	::OffsetRect( &msg.m_rcBubble, 0, m_nContentH );
	m_nContentH += ( msg.m_rcRow.bottom - msg.m_rcRow.top ) + m_cfg.m_nMsgGap;
	m_containerMsgs.push_back( msg );
	_notifyExtent( m_nCanvasW );
	return true;
}

void hm_wmp_msgout::clear() {
	m_containerMsgs.clear();
	m_nContentH = 0;
	if ( m_nCanvasW > 0 )
		_notifyExtent( m_nCanvasW );
}

// full re-measure at the current canvas width: re-stacks every item from the
// top and reports the new extent.
void hm_wmp_msgout::_measureAll() {
	int nContentH = 0;
	for ( size_t i = 0; i < m_containerMsgs.size(); ++i ) {
		MsgItem& msg = m_containerMsgs[ i ];
		if ( !_measureItem( msg, m_nCanvasW ) )
			continue;
		::OffsetRect( &msg.m_rcRow, 0, nContentH );
		::OffsetRect( &msg.m_rcBubble, 0, nContentH );
		nContentH += ( msg.m_rcRow.bottom - msg.m_rcRow.top ) + m_cfg.m_nMsgGap;
	}
	m_nContentH = nContentH;
	_notifyExtent( m_nCanvasW );
}

// measure ONE message against nRowW: fills rcRow ( full row band ) and
// rcBubble ( the bubble box with padding included ).
bool hm_wmp_msgout::_measureItem( MsgItem& msg, int nRowW ) {
	::SetRect( &msg.m_rcRow, 0, 0, nRowW, 0 );
	if ( msg.m_eSender == EMSGSENDER_SYSTEM ) {
		// centered one-liner; single-line measure at full width.
		int nTextH = 0;
		if ( _measureTextHeight( msg.m_strText.c_str(), nRowW, false, nTextH ) ) {
			msg.m_rcRow.bottom = nTextH + 2 * m_cfg.m_nPadV;
		} else {
			HDC hDC = ::GetDC( NULL );
			HFONT hFontOld = (HFONT)::SelectObject( hDC, m_cfg.m_hFontText );
			RECT rcText = { 0, 0, nRowW, 0 };
			::DrawText( hDC, msg.m_strText.c_str(), -1, &rcText, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX );
			::SelectObject( hDC, hFontOld );
			::ReleaseDC( NULL, hDC );
			msg.m_rcRow.bottom = ( rcText.bottom - rcText.top ) + 2 * m_cfg.m_nPadV;
		}
		::SetRect( &msg.m_rcBubble, 0, 0, 0, 0 );
		return true;
	}
	int nAvail = nRowW - m_cfg.m_nAvatarLen - m_cfg.m_nPadH;   // avatar + gap
	if ( nAvail < 40 )
		nAvail = 40;
	int nMaxBubble = nAvail * m_cfg.m_nMaxBubblePct / 100;
	// measure the text at the fixed bubble width, height grows ( word wrap ):
	// drawtool calcRect first ( three-backend parity ), GDI DrawText fallback.
	int nTextW = 0, nTextH = 0;
	if ( !_measureTextExtent( msg.m_strText.c_str(), nMaxBubble - 2 * m_cfg.m_nPadH, nTextW, nTextH ) ) {
		HDC hDC = ::GetDC( NULL );
		HFONT hFontOld = (HFONT)::SelectObject( hDC, m_cfg.m_hFontText );
		RECT rcText = { 0, 0, nMaxBubble - 2 * m_cfg.m_nPadH, 0 };
		::DrawText( hDC, msg.m_strText.c_str(), -1, &rcText,
			DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX );
		::SelectObject( hDC, hFontOld );
		::ReleaseDC( NULL, hDC );
		nTextW = rcText.right - rcText.left;
		nTextH = rcText.bottom - rcText.top;
	}
	int nBubbleW = min( nMaxBubble, nTextW + 2 * m_cfg.m_nPadH );
	int nBubbleH = nTextH + 2 * m_cfg.m_nPadV;
	msg.m_rcRow.bottom = max( nBubbleH, m_cfg.m_nAvatarLen );
	if ( msg.m_eSender == EMSGSENDER_PEER )
		::SetRect( &msg.m_rcBubble, m_cfg.m_nAvatarLen + m_cfg.m_nPadH, 0,
			m_cfg.m_nAvatarLen + m_cfg.m_nPadH + nBubbleW, nBubbleH );
	else // self: mirrored to the right
		::SetRect( &msg.m_rcBubble, nRowW - m_cfg.m_nAvatarLen - m_cfg.m_nPadH - nBubbleW, 0,
			nRowW - m_cfg.m_nAvatarLen - m_cfg.m_nPadH, nBubbleH );
	return true;
}

// inertial wheel ticks ( whellengine ): translate the decay into a pixel
// distance and hand it to the scrollview via the engine wheel event; the
// engine self-parks once the decay reaches zero ( viewbkcontent pattern ).
void hm_wmp_msgout::_OnHMMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP || !pParamWndProc )
		return;
	unsigned int uNoTriggerTagCount = (unsigned int)pWP->wParam;
	wtd_container_type* pWTDContainer = (wtd_container_type*)pWP->lParam;
	int nDistance = WHELLHELPER_INSTANCE->getDistancePixel( uNoTriggerTagCount, pWTDContainer, 2000 );
	if ( nDistance == 0 ) {
		if ( m_bWheelStopped )
			return;
	} else {
		m_bWheelStopped = false;
	}
	Notify( HM_EVENT_MOUSEWHEEL, (LPARAM)nDistance );
	if ( nDistance == 0 ) {
		m_bWheelStopped = true;
		WHELLENGINE_INSTANCE->pauseTriggerThread();
	}
}

// drawtool text helpers ( three-backend parity ): the rawres pointers come
// from the manifest ( loader-retained ); stack-constructed text data is
// safe - no backend reads the id fields.
bool hm_wmp_msgout::_measureTextExtent( const TCHAR* szText, int nW, int& nWOut, int& nHOut ) {
	if ( !m_pDrawTool || !m_cfg.m_pRawFontText || !m_cfg.m_pRawClrText || !szText )
		return false;
	RECT rc = { 0, 0, nW, 0 };
	if ( !m_pDrawTool->calcRect( NULL, rc, szText, m_cfg.m_pRawFontText, m_cfg.m_pRawClrText, true ) )
		return false;
	nWOut = rc.right - rc.left;
	nHOut = rc.bottom - rc.top;
	return true;
}

bool hm_wmp_msgout::_measureTextHeight( const TCHAR* szText, int nW, bool bMultiLine, int& nHOut ) {
	if ( !m_pDrawTool || !m_cfg.m_pRawFontText || !m_cfg.m_pRawClrText || !szText )
		return false;
	RECT rc = { 0, 0, nW, 0 };
	if ( !m_pDrawTool->calcRect( NULL, rc, szText, m_cfg.m_pRawFontText, m_cfg.m_pRawClrText, bMultiLine ) )
		return false;
	nHOut = rc.bottom - rc.top;
	return true;
}

void hm_wmp_msgout::_drawText( HDC hDC, const TCHAR* szText, const RECT& rc,
                               hm_rs_rawres_color* pRawClr, bool bMultiLine, bool bCenter ) {
	if ( m_pDrawTool && m_cfg.m_pRawFontText && pRawClr ) {
		hm_uires_text_data data;
		data.m_tstrData = szText;
		data.m_idRawResFont = 0;
		data.m_idRawResColor = 0;
		data.m_eTextFlag = (hm_uires_text_data::ENUMTEXTFALG)( bMultiLine ? hm_uires_text_data::TEXTFLAG_MULTILINE : hm_uires_text_data::TEXTFLAG_SINGLELINE );
		data.m_eAlginment = bCenter ? hm_uires_text_data::EALGINMENT_RIGHT : hm_uires_text_data::EALGINMENT_LEFT;
		m_pDrawTool->draw( hDC, rc, &data, m_cfg.m_pRawFontText, pRawClr, bMultiLine );
		return;
	}
	// GDI fallback ( no DrawTool configured ).
	HFONT hFontOld = (HFONT)::SelectObject( hDC, m_cfg.m_hFontText );
	int nBKModeOld = ::SetBkMode( hDC, TRANSPARENT );
	COLORREF clrOld = ::SetTextColor( hDC, pRawClr ? pRawClr->getCOLORREF() : m_cfg.m_clrText );
	::DrawText( hDC, szText, -1, (RECT*)&rc, ( bMultiLine ? DT_WORDBREAK : 0 )
		| DT_NOPREFIX | ( bCenter ? ( DT_CENTER | DT_VCENTER | DT_SINGLELINE ) : 0 ) );
	::SetTextColor( hDC, clrOld );
	::SetBkMode( hDC, nBKModeOld );
	::SelectObject( hDC, hFontOld );
}

void hm_wmp_msgout::_notifyExtent( int nCanvasW ) {
	HECTPARAM hectParam;
	hectParam.m_cxExtent = nCanvasW;
	hectParam.m_cyExtent = m_nContentH;
	hectParam.m_pWMPSender = this;
	Notify( HM_EVENT_CONTENT_EXTENT, (LPARAM)&hectParam );
}

// content-space rendering: rcCanvas is the ( scroll-translated ) content
// keelobj rect - items stack from its top. the viewport's ClipChildren does
// the culling.
void hm_wmp_msgout::_render( HDC hDC, const RECT& rcCanvas ) {
	Gdiplus::Graphics graphics( hDC );
	graphics.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	for ( size_t i = 0; i < m_containerMsgs.size(); ++i ) {
		const MsgItem& msg = m_containerMsgs[ i ];
		RECT rcRow = msg.m_rcRow;
		::OffsetRect( &rcRow, rcCanvas.left, rcCanvas.top );
		if ( rcRow.top >= rcCanvas.bottom || rcRow.bottom <= rcCanvas.top )
			continue;   // outside the viewport band ( ClipChildren culls anyway )
		if ( msg.m_eSender == EMSGSENDER_SYSTEM ) {
			_drawText( hDC, msg.m_strText.c_str(), rcRow,
				m_cfg.m_pRawClrSystem ? m_cfg.m_pRawClrSystem : m_cfg.m_pRawClrText, false, true );
			continue;
		}
		// avatar color block.
		RECT rcAvatar = rcRow;
		if ( msg.m_eSender == EMSGSENDER_PEER ) {
			rcAvatar.left += 0;
			rcAvatar.right = rcAvatar.left + m_cfg.m_nAvatarLen;
		} else {
			rcAvatar.right = rcRow.right;
			rcAvatar.left = rcAvatar.right - m_cfg.m_nAvatarLen;
		}
		rcAvatar.top += max( 0, ( ( rcRow.bottom - rcRow.top ) - m_cfg.m_nAvatarLen ) / 2 );
		rcAvatar.bottom = rcAvatar.top + m_cfg.m_nAvatarLen;
		Gdiplus::Color clrAvatar;
		clrAvatar.SetFromCOLORREF( msg.m_eSender == EMSGSENDER_PEER ? m_cfg.m_clrAvatarPeer : m_cfg.m_clrAvatarSelf );
		Gdiplus::SolidBrush brushAvatar( clrAvatar );
		graphics.FillRectangle( &brushAvatar, rcAvatar.left, rcAvatar.top,
			rcAvatar.right - rcAvatar.left, rcAvatar.bottom - rcAvatar.top );
		// bubble.
		RECT rcBubble = msg.m_rcBubble;
		::OffsetRect( &rcBubble, rcCanvas.left, rcCanvas.top );
		Gdiplus::Color clrBubble;
		clrBubble.SetFromCOLORREF( msg.m_eSender == EMSGSENDER_PEER ? m_cfg.m_clrBubblePeer : m_cfg.m_clrBubbleSelf );
		Gdiplus::SolidBrush brushBubble( clrBubble );
		int nCor = min( m_cfg.m_nCornerLen, ( rcBubble.right - rcBubble.left ) / 2 );
		Gdiplus::GraphicsPath pathBubble;
		pathBubble.AddArc( rcBubble.left, rcBubble.top, 2 * nCor, 2 * nCor, 180, 90 );
		pathBubble.AddArc( rcBubble.right - 2 * nCor, rcBubble.top, 2 * nCor, 2 * nCor, 270, 90 );
		pathBubble.AddArc( rcBubble.right - 2 * nCor, rcBubble.bottom - 2 * nCor, 2 * nCor, 2 * nCor, 0, 90 );
		pathBubble.AddArc( rcBubble.left, rcBubble.bottom - 2 * nCor, 2 * nCor, 2 * nCor, 90, 90 );
		pathBubble.CloseFigure();
		graphics.FillPath( &brushBubble, &pathBubble );
		// text inside the bubble.
		RECT rcText = rcBubble;
		::InflateRect( &rcText, -m_cfg.m_nPadH, -m_cfg.m_nPadV );
		_drawText( hDC, msg.m_strText.c_str(), rcText, m_cfg.m_pRawClrText, true, false );
	}
}
