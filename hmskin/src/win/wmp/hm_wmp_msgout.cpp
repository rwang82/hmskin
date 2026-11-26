#include "stdafx.h"
#include "win\wmp\hm_wmp_msgout.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_uiroot.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_event_defs.h"
#include "win\hm_wndmsg_defs.h"
#include "win\paint_system\hm_ps_drawtool_i.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include "win\hook_system\hm_hs_whellengine.h"
#include "hmskin_event_defs.h"
#include <gdiplus.h>
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
// built-in text item: drawtool text + selection rects.
class hm_msgout_item_text : public hm_msgout_item_i, public hm_msgout_textsel_i {
public:
	hm_msgout_item_text( hm_wmp_msgout* pOwner, hm_wmp_msgout::string_type strText )
	: m_pOwner( pOwner ), m_strText( strText ) {
		m_nTextW = 0; m_nTextH = 0;
	}

	// word-wrap measurement against the bubble inner width; the owner supplies
	// font/margins/alignment ( its cfg ) through the shared helpers.
	virtual int measure( int nRowW ) {
		const hm_wmp_msgout::MSGOUT_CFG& cfg = m_pOwner->m_cfg;
		int nAvail = nRowW - cfg.m_nAvatarLen - cfg.m_nPadH;
		if ( nAvail < 40 )
			nAvail = 40;
		int nMaxBubble = nAvail * cfg.m_nMaxBubblePct / 100;
		int nTextW = 0, nTextH = 0;
		if ( !m_pOwner->_measureTextExtent( m_strText.c_str(), nMaxBubble - 2 * cfg.m_nPadH, nTextW, nTextH ) ) {
			HDC hDC = ::GetDC( NULL );
			HFONT hFontOld = (HFONT)::SelectObject( hDC, cfg.m_hFontText );
			RECT rcText = { 0, 0, nMaxBubble - 2 * cfg.m_nPadH, 0 };
			::DrawText( hDC, m_strText.c_str(), -1, &rcText,
				DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX );
			::SelectObject( hDC, hFontOld );
			::ReleaseDC( NULL, hDC );
			nTextW = rcText.right - rcText.left;
			nTextH = rcText.bottom - rcText.top;
		}
		m_nTextW = nTextW;
		m_nTextH = nTextH;
		m_nBubbleW = min( nMaxBubble, nTextW + 2 * cfg.m_nPadH );
		int nBubbleH = nTextH + 2 * cfg.m_nPadV;
		return max( nBubbleH, cfg.m_nAvatarLen );
	}

	virtual void render( HDC hDC, const RECT& rcRow, hm_ps_drawtool_i* pDrawTool ) {
		const hm_wmp_msgout::MSGOUT_CFG& cfg = m_pOwner->m_cfg;
		m_pOwner->_drawText( hDC, m_strText.c_str(), rcRow, cfg.m_pRawClrText, true, false );
	}
	// the bubble hugs the wrapped text ( measure() sized it ).
	virtual void getBubbleSize( SIZE& size ) {
		size.cx = m_nBubbleW;
		size.cy = m_nTextH + 2 * m_pOwner->m_cfg.m_nPadV;
	}

	// proportional char mapping ( v1b ): chars per line estimated from the
	// wrap width - exact copy text, approximate highlight rects.
	virtual bool hitTestChar( POINT ptContent, int& nCharIdx ) {
		int nLen = (int)m_strText.length();
		if ( nLen == 0 || m_nTextW <= 0 )
			return false;
		int nX = ptContent.x;
		if ( nX < 0 ) nX = 0;
		if ( nX > m_nTextW ) nX = m_nTextW;
		nCharIdx = nX * nLen / max( 1, m_nTextW );
		if ( nCharIdx > nLen ) nCharIdx = nLen;
		return true;
	}

	virtual void getSelRects( int nCharS, int nCharE, std::vector<RECT>& containerRects ) {
		int nLen = (int)m_strText.length();
		if ( nLen == 0 || m_nTextW <= 0 )
			return;
		// the select-all sentinel ( 0x7FFFFFFF end ) must clamp before the
		// inclusive-end +1 below, or every rect comes out garbage.
		if ( nCharS < 0 ) nCharS = 0;
		if ( nCharS > nLen ) nCharS = nLen;
		if ( nCharE > nLen ) nCharE = nLen;
		const hm_wmp_msgout::MSGOUT_CFG& cfg = m_pOwner->m_cfg;
		int nInnerW = m_nBubbleW - 2 * cfg.m_nPadH;
		int nLineH = max( 1, m_pOwner->_lineHeight() );
		int nLines = max( 1, ( m_nTextH + nLineH - 1 ) / nLineH );
		int nCharPerLine = max( 1, nLen / nLines );
		for ( int nLine = 0; nLine < nLines; ++nLine ) {
			int nLS = nLine * nCharPerLine;
			int nLE = min( nLen, nLS + nCharPerLine );
			if ( nLE < nCharS || nLS > nCharE )
				continue;   // line outside the span
			int nXS = ( max( nCharS, nLS ) - nLS ) * nInnerW / max( 1, nCharPerLine );
			int nXE = ( min( nCharE + 1, nLE ) - nLS ) * nInnerW / max( 1, nCharPerLine );
			RECT rc = { nXS, nLine * nLineH, nXE, ( nLine + 1 ) * nLineH };
			containerRects.push_back( rc );
		}
	}

	virtual void getPlainText( std::basic_string<TCHAR>& strOut ) { strOut = m_strText; }

	hm_wmp_msgout* m_pOwner;
	hm_wmp_msgout::string_type m_strText;
	int m_nTextW, m_nTextH, m_nBubbleW;
};

//////////////////////////////////////////////////////////////////////////
// built-in image item: file path ( png / bmp / jpg through GDI+ ) or a
// borrowed HBITMAP, block-level ( own row ), scaled down to the clamp.
class hm_msgout_item_image : public hm_msgout_item_i {
public:
	hm_msgout_item_image( hm_wmp_msgout* pOwner, const TCHAR* szPath, HBITMAP hBmp )
	: m_hBmp( hBmp ), m_pImage( NULL ), m_nW( 0 ), m_nH( 0 ) {
		m_pOwner = pOwner;
		if ( szPath && szPath[ 0 ] )
			m_strPath = szPath;
		if ( m_strPath.empty() && !hBmp )
			m_bInvalid = true;
		else
			_load();
	}
	~hm_msgout_item_image() {
		delete m_pImage;
	}
	void _load() {
		if ( m_hBmp ) {
			Gdiplus::Bitmap* pBmp = Gdiplus::Bitmap::FromHBITMAP( m_hBmp, NULL );
			if ( pBmp && pBmp->GetLastStatus() == Gdiplus::Ok && pBmp->GetWidth() > 0 ) {
				m_pImage = pBmp;
				m_nW = (int)pBmp->GetWidth();
				m_nH = (int)pBmp->GetHeight();
				return;
			}
			delete pBmp;
		}
		if ( m_strPath.length() ) {
			Gdiplus::Image* pImage = Gdiplus::Image::FromFile( m_strPath.c_str() );
			if ( pImage && pImage->GetLastStatus() == Gdiplus::Ok && pImage->GetWidth() > 0 ) {
				m_pImage = pImage;
				m_nW = (int)pImage->GetWidth();
				m_nH = (int)pImage->GetHeight();
				return;
			}
			delete pImage;
		}
		m_bInvalid = true;
	}
	bool m_bInvalid = false;
	hm_wmp_msgout::string_type m_strPath;
	HBITMAP m_hBmp;              // borrowed: the host keeps ownership
	Gdiplus::Image* m_pImage;    // owned
	int m_nW, m_nH;

	virtual int measure( int nRowW ) {
		// clamp to ~240 ( 96-baseline ), never upscale.
		double dblScale = m_pOwner->m_pHMUIRoot ? m_pOwner->m_pHMUIRoot->m_cfg.m_dblLayoutScale : 1.0;
		int nMax = (int)( 240 * dblScale + 0.5 );
		int nW = m_nW, nH = m_nH;
		if ( nW > nMax || nH > nMax ) {
			double dblFit = min( (double)nMax / nW, (double)nMax / nH );
			nW = (int)( nW * dblFit );
			nH = (int)( nH * dblFit );
		}
		m_nDrawW = nW; m_nDrawH = nH;
		return max( nH, 1 );
	}
	virtual void render( HDC hDC, const RECT& rcRow, hm_ps_drawtool_i* pDrawTool ) {
		if ( !m_pImage || m_nDrawW <= 0 || m_nDrawH <= 0 )
			return;
		Gdiplus::Graphics graph( hDC );
		graph.SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );
		graph.DrawImage( m_pImage, rcRow.left, rcRow.top, m_nDrawW, m_nDrawH );
	}
	// the bubble hugs the scaled image.
	virtual void getBubbleSize( SIZE& size ) {
		size.cx = m_nDrawW + 2 * m_pOwner->m_cfg.m_nPadH;
		size.cy = m_nDrawH + 2 * m_pOwner->m_cfg.m_nPadV;
	}
	hm_wmp_msgout* m_pOwner = NULL;
	int m_nDrawW = 0, m_nDrawH = 0;
};

//////////////////////////////////////////////////////////////////////////
hm_wmp_msgout::MSGOUT_CFG::MSGOUT_CFG()
: m_idWMPSearchBar( 0 )
, m_idKeelObjBar( 0 )
, m_pRawFontText( NULL )
, m_pRawClrText( NULL )
, m_pRawClrSystem( NULL )
, m_hFontText( NULL )
, m_clrText( RGB( 232, 236, 241 ) )
, m_clrBubbleSelf( RGB( 0x95, 0xEC, 0x69 ) )
, m_clrBubblePeer( RGB( 0xFF, 0xFF, 0xFF ) )
, m_clrSystem( RGB( 139, 147, 158 ) )
, m_clrAvatarSelf( RGB( 0x95, 0xEC, 0x69 ) )
, m_clrAvatarPeer( RGB( 0x8B, 0x93, 0x9E ) )
, m_clrSelHilite( RGB( 0x33, 0x66, 0xCC ) )
, m_nAvatarLen( 40 )
, m_nMaxBubblePct( 80 )
, m_nMsgGap( 10 )
, m_nPadH( 12 )
, m_nPadV( 8 )
, m_nCornerLen( 8 )
, m_nWheelStep( 60 )
, m_nFixedThumbLen( -1 )
, m_uMinThumbLen( 20 )
, m_uArrowLen( 20 )
{
}

hm_wmp_msgout::hm_wmp_msgout( hm_uiroot* pUIRoot, hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_pHMUIRoot( pUIRoot )
, m_pWMPBar( NULL )
, m_pKeelObjViewport( NULL )
, m_nContentH( 0 )
, m_nCanvasW( 0 )
, m_nLineH( 0 )
, m_bWired( false )
, m_bWheelStopped( false )
, m_bSelDragging( false )
, m_nSelItemAnchor( -1 ), m_nSelCharAnchor( 0 )
, m_nSelItemActive( -1 ), m_nSelCharActive( 0 ) {
	// 60fps inertia ticks: half the per-tick distance at double the
	// cadence - the same total distance reads far smoother ( FRAME24's
	// 41ms ticks with 30-90px steps looked jumpy on fast flicks ).
	WHELLENGINE_INSTANCE->setFrameMode( hm_hs_whellengine::FRAME60 );
}

hm_wmp_msgout::~hm_wmp_msgout() {
	for ( size_t i = 0; i < m_containerItems.size(); ++i )
		delete m_containerItems[ i ].m_pItem;
	m_containerItems.clear();
	m_pWMPBar = NULL;
	m_pKeelObjViewport = NULL;
}

//
MSGBIND_BEGIN( hm_wmp_msgout )
MSGBIND( WM_MOUSEWHEEL, _OnMouseWheel )
MSGBIND( HMWM_MOUSEWHEEL, _OnHMMouseWhell )
MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
MSGBIND( WM_LBUTTONUP, _OnLButtonUp )
MSGBIND( WM_KEYDOWN, _OnKeyDown )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_msgout )
ORDERWNDMSG( WM_MOUSEWHEEL )
ORDERWNDMSG( HMWM_MOUSEWHEEL )
ORDERWNDMSG( WM_LBUTTONDOWN )
ORDERWNDMSG( WM_MOUSEMOVE )
ORDERWNDMSG( WM_LBUTTONUP )
ORDERWNDMSG( WM_KEYDOWN )
ORDERWNDMSG_END()

void hm_wmp_msgout::_createDistrictObjs() {
	// no districts - everything renders directly in _render.
}

void hm_wmp_msgout::_getAllDistrictId( district_id_container_type& containerIds ) {
	UNREFERENCED_PARAMETER( containerIds );
}

void hm_wmp_msgout::setConfigure( const MSGOUT_CFG& cfgScaled ) {
	m_cfg = cfgScaled;
	m_core.setThumbCfg( cfgScaled.m_nFixedThumbLen, cfgScaled.m_uMinThumbLen );
}

bool hm_wmp_msgout::appendItem( hm_msgout_item_i* pItem, int eSender ) {
	if ( !pItem )
		return false;
	ItemSlot slot;
	slot.m_pItem = pItem;
	slot.m_pTextSel = dynamic_cast< hm_msgout_textsel_i* >( pItem );
	slot.m_eSender = eSender;
	::SetRect( &slot.m_rcRow, 0, 0, 0, 0 );
	if ( m_nCanvasW > 0 ) {
		int nH = pItem->measure( m_nCanvasW );
		slot.m_rcRow.right = m_nCanvasW;
		slot.m_rcRow.bottom = nH;
		::OffsetRect( &slot.m_rcRow, 0, m_nContentH );
		m_nContentH += nH + m_cfg.m_nMsgGap;
	}
	m_containerItems.push_back( slot );
	if ( m_nCanvasW > 0 )
		_relayoutAndNotify();
	return true;
}

bool hm_wmp_msgout::appendText( int eSender, const TCHAR* szText ) {
	if ( !szText || !szText[ 0 ] )
		return false;
	return appendItem( new hm_msgout_item_text( this, szText ), eSender );
}

bool hm_wmp_msgout::appendImage( int eSender, const TCHAR* szImagePath, HBITMAP hBmp ) {
	hm_msgout_item_image* pItem = new hm_msgout_item_image( this, szImagePath, hBmp );
	if ( pItem->m_bInvalid ) {
		delete pItem;
		return false;
	}
	return appendItem( pItem, eSender );
}

void hm_wmp_msgout::clear() {
	for ( size_t i = 0; i < m_containerItems.size(); ++i )
		delete m_containerItems[ i ].m_pItem;
	m_containerItems.clear();
	m_nContentH = 0;
	m_nSelItemAnchor = m_nSelItemActive = -1;
	m_core.reset();
	if ( m_nCanvasW > 0 )
		_relayoutAndNotify();
}

void hm_wmp_msgout::_relayoutAndNotify() {
	_measureAll();
	_notifyExtent();
	bool bMoved = m_core.onExtentGrown( m_nContentH );   // stick ( was at bottom ) or DND
	if ( m_bWired ) {
		m_core.syncBar( m_pWMPBar, m_pKeelObjViewport );   // range first, then the moved value
		// a proportional thumb changes length with the extent - repaint it
		// even when the top stayed ( DND ); a fixed thumb only on moves.
		if ( m_pWMPBar && ( bMoved || m_cfg.m_nFixedThumbLen < 0 ) )
			m_pWMPBar->drawSelf();
	}
	_invalidateViewport();
}

void hm_wmp_msgout::_measureAll() {
	int nContentH = 0;
	for ( size_t i = 0; i < m_containerItems.size(); ++i ) {
		ItemSlot& slot = m_containerItems[ i ];
		int nH = slot.m_pItem->measure( m_nCanvasW );
		::SetRect( &slot.m_rcRow, 0, nContentH, m_nCanvasW, nContentH + nH );
		nContentH += nH + m_cfg.m_nMsgGap;
	}
	m_nContentH = nContentH;
}

void hm_wmp_msgout::_notifyExtent() {
	HECTPARAM hectParam;
	hectParam.m_cxExtent = m_nCanvasW;
	hectParam.m_cyExtent = m_nContentH;
	hectParam.m_pWMPSender = this;
	Notify( HM_EVENT_CONTENT_EXTENT, (LPARAM)&hectParam );
}

void hm_wmp_msgout::_ensureWiring() {
	if ( m_bWired )
		return;
	if ( !m_pHMUIRoot || !m_pHMUIRoot->m_pWMPFactory )
		return;
	hm_wndmsgprocessor_i* pWMPBarBase = m_pHMUIRoot->m_pWMPFactory->getObj( m_cfg.m_idWMPSearchBar );
	hm_wmp_scrollbar* pBar = dynamic_cast< hm_wmp_scrollbar* >( pWMPBarBase );
	if ( !pBar )
		return;
	m_pWMPBar = pBar;
	m_pWMPBar->Register( this, HM_EVENT_TRACKTHUMB_MOVED );
	m_pWMPBar->setStartThumbLen( m_cfg.m_uArrowLen );
	m_pWMPBar->setEndThumbLen( m_cfg.m_uArrowLen );
	m_bWired = true;
}

void hm_wmp_msgout::_scrollApplied( bool bRepaintBar ) {
	if ( m_bWired && m_pWMPBar ) {
		m_pWMPBar->setCurValue( m_core.getTop() );
		if ( bRepaintBar )
			m_pWMPBar->drawSelf();
	}
	_invalidateViewport();
}

void hm_wmp_msgout::_invalidateViewport() {
	if ( !m_pKeelObjViewport || !m_pKeelObjViewport->getLayoutObj() )
		return;
	hm_layout_obj* pLayout = m_pKeelObjViewport->getLayoutObj();
	const coordinate_system_hwnd* pCS = pLayout->getCoordinateSystem();
	HWND hWnd = pCS ? pCS->getHWnd() : NULL;
	if ( !hWnd )
		return;
	RECT rcViewport;
	if ( !pLayout->mapPosition( hWnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcViewport ) )
		return;
	// trim the bar strip ( dock-right shape ): the bar repaints itself.
	hm_keel_obj* pKOBar = ( m_pHMUIRoot && m_pHMUIRoot->m_pKeelObjFactory && m_cfg.m_idKeelObjBar )
		? m_pHMUIRoot->m_pKeelObjFactory->getObj( m_cfg.m_idKeelObjBar ) : NULL;
	hm_layout_obj* pLayBar = pKOBar ? pKOBar->getLayoutObj() : NULL;
	if ( pLayBar ) {
		RECT rcBar;
		if ( pLayBar->mapPosition( hWnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcBar )
			&& rcBar.left > rcViewport.left )
			rcViewport.right = min( rcViewport.right, rcBar.left );
	}
	::InvalidateRect( hWnd, &rcViewport, FALSE );
}

bool hm_wmp_msgout::_isCursorOverContent( hm_keel_obj* pKeelObj, HWND hWndMsg, POINT ptScreen ) const {
	hm_layout_obj* pLayout = pKeelObj ? pKeelObj->getLayoutObj() : NULL;
	if ( !pLayout )
		return false;
	RECT rcViewport;
	if ( !pLayout->mapPosition( hWndMsg, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcViewport ) )
		return false;
	POINT ptCursor = ptScreen;
	if ( !::ScreenToClient( hWndMsg, &ptCursor ) )
		return false;
	if ( !::PtInRect( &rcViewport, ptCursor ) )
		return false;
	// the bar strip handles its own wheel - stay out of it.
	hm_keel_obj* pKOBar = ( m_pHMUIRoot && m_pHMUIRoot->m_pKeelObjFactory && m_cfg.m_idKeelObjBar )
		? m_pHMUIRoot->m_pKeelObjFactory->getObj( m_cfg.m_idKeelObjBar ) : NULL;
	hm_layout_obj* pLayBar = pKOBar ? pKOBar->getLayoutObj() : NULL;
	if ( pLayBar ) {
		RECT rcBar;
		if ( pLayBar->mapPosition( hWndMsg, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcBar )
			&& ::PtInRect( &rcBar, ptCursor ) )
			return false;
	}
	return true;
}

void hm_wmp_msgout::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
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
		{ // the viewport settled - size + wire + initial scroll state.
			hm_keel_obj* pKeelObj = (hm_keel_obj*)pSubject;
			hm_layout_obj* pLayout = pKeelObj ? pKeelObj->getLayoutObj() : NULL;
			if ( !pLayout )
				return;
			m_pKeelObjViewport = pKeelObj;
			_ensureWiring();
			// a list without DRAWSELF ( manifest RegEvts 6 ) would never paint.
			assert( pKeelObj->HasRegister( (hm_wndmsgprocessor_i*)this, HM_EVENT_DRAWSELF ) );
			RECT rcViewport = pLayout->m_rect;
			int nViewH = rcViewport.bottom - rcViewport.top;
			int nBarW = 0;
			hm_keel_obj* pKOBar = ( m_pHMUIRoot && m_pHMUIRoot->m_pKeelObjFactory && m_cfg.m_idKeelObjBar )
				? m_pHMUIRoot->m_pKeelObjFactory->getObj( m_cfg.m_idKeelObjBar ) : NULL;
			hm_layout_obj* pLayBar = pKOBar ? pKOBar->getLayoutObj() : NULL;
			if ( pLayBar )
				nBarW = pLayBar->m_rect.right - pLayBar->m_rect.left;
			int nContentW = ( rcViewport.right - rcViewport.left ) - nBarW;
			m_core.setViewport( nViewH );
			if ( nContentW > 0 && nContentW != m_nCanvasW ) {
				m_nCanvasW = nContentW;
				_measureAll();
				m_core.setExtent( m_nContentH );
				_notifyExtent();
			}
			m_core.scrollTo( m_core.getTop() );   // re-clamp after the viewport resize
			if ( m_bWired )
				m_core.syncBar( m_pWMPBar, pKOBar );
			_invalidateViewport();
		}
		break;
	case HM_EVENT_TRACKTHUMB_MOVED:
		{ // subject = the scrollbar WMP; drag / arrows / bar-wheel.
			_ensureWiring();
			if ( !m_bWired || !m_pWMPBar )
				break;
			HETMPARAM* pHETMParam = (HETMPARAM*)pEventParam;
			if ( !pHETMParam || pHETMParam->m_pWMPScrollbar != m_pWMPBar || pHETMParam->m_nPosMax <= 0 )
				break;
			int nTop = pHETMParam->m_nPosCur;
			if ( m_core.scrollTo( nTop ) )
				_scrollApplied( false );
		}
		break;
	default:
		break;
	}
}

void hm_wmp_msgout::_OnMouseWheel( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP )
		return;
	POINT ptScreen;
	ptScreen.x = (int)(short)LOWORD( pWP->lParam );
	ptScreen.y = (int)(short)HIWORD( pWP->lParam );
	if ( !_isCursorOverContent( pKeelObj, pWP->hwnd, ptScreen ) )
		return;
	int nNotches = (int)GET_WHEEL_DELTA_WPARAM( pWP->wParam ) / WHEEL_DELTA;
	if ( nNotches == 0 )
		return;
	if ( m_core.scrollTo( m_core.getTop() - nNotches * m_cfg.m_nWheelStep ) )
		_scrollApplied( true );
}

// inertial wheel ticks ( whellengine ): translate the decay into a pixel
// distance and apply directly; the engine self-parks at zero.
void hm_wmp_msgout::_OnHMMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP )
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
	if ( nDistance && m_core.scrollTo( m_core.getTop() - nDistance ) )
		_scrollApplied( true );
	if ( nDistance == 0 ) {
		m_bWheelStopped = true;
		WHELLENGINE_INSTANCE->pauseTriggerThread();
	}
}

// row geometry in canvas coords: peer = avatar left, bubble right of it;
// self mirrored. the bubble hugs the item's measured box ( 0 = fill the band
// between the avatar and the opposite edge ), content = bubble minus padding.
void hm_wmp_msgout::_rowRects( const ItemSlot& slot, const RECT& rcCanvas, int nTop,
                               RECT& rcRow, RECT& rcBubble, RECT& rcContent ) {
	rcRow = slot.m_rcRow;
	::OffsetRect( &rcRow, rcCanvas.left, rcCanvas.top - nTop );
	SIZE sizeBubble = { 0, 0 };
	slot.m_pItem->getBubbleSize( sizeBubble );
	int nAvail = ( rcRow.right - rcRow.left ) - m_cfg.m_nAvatarLen - m_cfg.m_nPadH;
	if ( sizeBubble.cx <= 0 || sizeBubble.cx > nAvail )
		sizeBubble.cx = max( 0, nAvail );
	if ( sizeBubble.cy <= 0 || sizeBubble.cy > rcRow.bottom - rcRow.top )
		sizeBubble.cy = rcRow.bottom - rcRow.top;
	rcBubble = rcRow;
	if ( slot.m_eSender == EMSGSENDER_SELF ) {
		rcBubble.right = rcRow.right - m_cfg.m_nAvatarLen - m_cfg.m_nPadH;
		rcBubble.left = rcBubble.right - sizeBubble.cx;
	} else {
		rcBubble.left = rcRow.left + m_cfg.m_nAvatarLen + m_cfg.m_nPadH;
		rcBubble.right = rcBubble.left + sizeBubble.cx;
	}
	rcBubble.bottom = rcBubble.top + sizeBubble.cy;
	rcContent = rcBubble;
	::InflateRect( &rcContent, -m_cfg.m_nPadH, -m_cfg.m_nPadV );
}

// text line pitch for the selection model ( one wrapped line of a probe ).
int hm_wmp_msgout::_lineHeight() {
	if ( m_nLineH <= 0 ) {
		int nW = 0, nH = 0;
		if ( !_measureTextExtent( _T( "Wg" ), 10000, nW, nH ) || nH <= 0 )
			nH = m_cfg.m_nPadV + 16;
		m_nLineH = nH;
	}
	return m_nLineH;
}

// content-space rendering: rcCanvas is the viewport band; items render at
// ( row.top - scrollTop ) - the ClipChildren culls what falls outside.
void hm_wmp_msgout::_render( HDC hDC, const RECT& rcCanvas ) {
	int nTop = m_core.getTop();
	int nSelS = ( m_nSelItemAnchor >= 0 && m_nSelItemActive >= 0 )
		? min( m_nSelItemAnchor, m_nSelItemActive ) : -1;
	int nSelE = nSelS < 0 ? -1 : max( m_nSelItemAnchor, m_nSelItemActive );
	Gdiplus::Graphics graphics( hDC );
	graphics.SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
	Gdiplus::Color clrSel;
	clrSel.SetFromCOLORREF( m_cfg.m_clrSelHilite );
	Gdiplus::SolidBrush brushSel( Gdiplus::Color( 70, clrSel.GetR(), clrSel.GetG(), clrSel.GetB() ) );
	std::vector<RECT> containerRects;
	for ( size_t i = 0; i < m_containerItems.size(); ++i ) {
		const ItemSlot& slot = m_containerItems[ i ];
		RECT rcRow, rcBubble, rcContent;
		_rowRects( slot, rcCanvas, nTop, rcRow, rcBubble, rcContent );
		if ( rcRow.top >= rcCanvas.bottom || rcRow.bottom <= rcCanvas.top )
			continue;   // outside the viewport band
		if ( slot.m_eSender == EMSGSENDER_SYSTEM ) {
			_drawText( hDC, ((hm_msgout_item_text*)slot.m_pItem)->m_strText.c_str(), rcRow,
				m_cfg.m_pRawClrSystem ? m_cfg.m_pRawClrSystem : m_cfg.m_pRawClrText, false, true );
			continue;
		}
		// avatar color block.
		RECT rcAvatar = rcRow;
		if ( slot.m_eSender == EMSGSENDER_PEER ) {
			rcAvatar.right = rcAvatar.left + m_cfg.m_nAvatarLen;
		} else {
			rcAvatar.right = rcRow.right;
			rcAvatar.left = rcAvatar.right - m_cfg.m_nAvatarLen;
		}
		rcAvatar.top += max( 0, ( ( rcRow.bottom - rcRow.top ) - m_cfg.m_nAvatarLen ) / 2 );
		rcAvatar.bottom = rcAvatar.top + m_cfg.m_nAvatarLen;
		Gdiplus::Color clrAvatar;
		clrAvatar.SetFromCOLORREF( slot.m_eSender == EMSGSENDER_PEER ? m_cfg.m_clrAvatarPeer : m_cfg.m_clrAvatarSelf );
		Gdiplus::SolidBrush brushAvatar( clrAvatar );
		graphics.FillRectangle( &brushAvatar, rcAvatar.left, rcAvatar.top,
			rcAvatar.right - rcAvatar.left, rcAvatar.bottom - rcAvatar.top );
		// bubble ( block-level items like images draw their own contents ).
		Gdiplus::Color clrBubble;
		clrBubble.SetFromCOLORREF( slot.m_eSender == EMSGSENDER_PEER ? m_cfg.m_clrBubblePeer : m_cfg.m_clrBubbleSelf );
		Gdiplus::SolidBrush brushBubble( clrBubble );
		int nCor = min( m_cfg.m_nCornerLen, ( rcBubble.right - rcBubble.left ) / 2 );
		Gdiplus::GraphicsPath pathBubble;
		pathBubble.AddArc( rcBubble.left, rcBubble.top, 2 * nCor, 2 * nCor, 180, 90 );
		pathBubble.AddArc( rcBubble.right - 2 * nCor, rcBubble.top, 2 * nCor, 2 * nCor, 270, 90 );
		pathBubble.AddArc( rcBubble.right - 2 * nCor, rcBubble.bottom - 2 * nCor, 2 * nCor, 2 * nCor, 0, 90 );
		pathBubble.AddArc( rcBubble.left, rcBubble.bottom - 2 * nCor, 2 * nCor, 2 * nCor, 90, 90 );
		pathBubble.CloseFigure();
		graphics.FillPath( &brushBubble, &pathBubble );
		// selection highlight UNDER the text ( item-relative rects ).
		if ( (int)i >= nSelS && (int)i <= nSelE && slot.m_pTextSel ) {
			containerRects.clear();
			int nCS = ( (int)i == nSelS ) ? min( m_nSelCharAnchor, m_nSelCharActive ) : 0;
			int nCE = ( (int)i == nSelE ) ? max( m_nSelCharAnchor, m_nSelCharActive ) : 0x7FFFFFFF;
			slot.m_pTextSel->getSelRects( nCS, nCE, containerRects );
			for ( size_t j = 0; j < containerRects.size(); ++j ) {
				RECT rc = containerRects[ j ];
				::OffsetRect( &rc, rcContent.left, rcContent.top );
				graphics.FillRectangle( &brushSel, rc.left, rc.top,
					rc.right - rc.left, rc.bottom - rc.top );
			}
		}
		// item contents ( text or image ) inside the bubble.
		slot.m_pItem->render( hDC, rcContent, m_pDrawTool );
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
                               hm_rs_rawres_color* pRawClr, bool bMultiLine, bool bCenter, int nCharS, int nCharE ) {
	// selection range ( nCharS..nCharE ): GDI+ translucent highlight under
	// the drawtool pass. char rects come from the wrap metrics.
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


// selection: LButtonDown anchors, drag extends, LButtonUp settles; the span
// is ( item, char ) pairs - cross-message copy aggregates getPlainText.
void hm_wmp_msgout::_OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP )
		return;
	// WM_LBUTTONDOWN carries CLIENT coords; the shared hit-test speaks screen.
	POINT ptClient;
	ptClient.x = (long)(short)LOWORD( pWP->lParam );
	ptClient.y = (long)(short)HIWORD( pWP->lParam );
	POINT ptScreen = ptClient;
	::ClientToScreen( pWP->hwnd, &ptScreen );
	if ( !_isCursorOverContent( pKeelObj, pWP->hwnd, ptScreen ) )
		return;
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_CUT_ALL;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	int nTop = m_core.getTop();
	m_bSelDragging = true;
	m_nSelItemAnchor = m_nSelItemActive = -1;
	for ( size_t i = 0; i < m_containerItems.size(); ++i ) {
		RECT rcRow = m_containerItems[ i ].m_rcRow;
		::OffsetRect( &rcRow, 0, -nTop );
		if ( ::PtInRect( &rcRow, ptClient ) ) {
			m_nSelItemAnchor = m_nSelItemActive = (int)i;
			if ( m_containerItems[ i ].m_pTextSel ) {
				POINT ptContent = { ptClient.x - rcRow.left, ptClient.y - rcRow.top };
				m_containerItems[ i ].m_pTextSel->hitTestChar( ptContent, m_nSelCharAnchor );
				m_nSelCharActive = m_nSelCharAnchor;
			} else {
				m_nSelCharAnchor = m_nSelCharActive = 0;
			}
			break;
		}
	}
	if ( ::GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	_invalidateViewport();
}

void hm_wmp_msgout::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !m_bSelDragging || !pKeelObj || !pWP )
		return;
	POINT ptClient;
	ptClient.x = (long)(short)LOWORD( pWP->lParam );
	ptClient.y = (long)(short)HIWORD( pWP->lParam );
	int nTop = m_core.getTop();
	int nLen = (int)m_containerItems.size();
	for ( int i = 0; i < nLen; ++i ) {
		RECT rcRow = m_containerItems[ i ].m_rcRow;
		::OffsetRect( &rcRow, 0, -nTop );
		if ( ::PtInRect( &rcRow, ptClient ) ) {
			m_nSelItemActive = i;
			if ( m_containerItems[ i ].m_pTextSel ) {
				POINT ptContent = { ptClient.x - rcRow.left, ptClient.y - rcRow.top };
				m_containerItems[ i ].m_pTextSel->hitTestChar( ptContent, m_nSelCharActive );
			}
			break;
		}
	}
	_invalidateViewport();
}

void hm_wmp_msgout::_OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !m_bSelDragging )
		return;
	m_bSelDragging = false;
	if ( pWP && ::GetCapture() == pWP->hwnd )
		::ReleaseCapture();
}

// keyboard: WM_KEYDOWN 'A'/'C' with Ctrl held ( WM_CHAR mangles Ctrl+A ).
void hm_wmp_msgout::_OnKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP )
		return;
	if ( !( ::GetKeyState( VK_CONTROL ) & 0x8000 ) )
		return;
	if ( pWP->wParam == 'A' ) {
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;
		_selectAll();
		_invalidateViewport();
	} else if ( pWP->wParam == 'C' ) {
		pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;
		_copySelection();
	}
}

void hm_wmp_msgout::_selectAll() {
	if ( m_containerItems.empty() )
		return;
	m_nSelItemAnchor = 0;
	m_nSelCharAnchor = 0;
	m_nSelItemActive = (int)m_containerItems.size() - 1;
	m_nSelCharActive = 0x7FFFFFFF;
}

void hm_wmp_msgout::_copySelection() {
	if ( m_nSelItemAnchor < 0 || m_nSelItemActive < 0 )
		return;
	int nItemS = min( m_nSelItemAnchor, m_nSelItemActive );
	int nItemE = max( m_nSelItemAnchor, m_nSelItemActive );
	const coordinate_system_hwnd* pCS = ( m_pKeelObjViewport && m_pKeelObjViewport->getLayoutObj() )
		? m_pKeelObjViewport->getLayoutObj()->getCoordinateSystem() : NULL;
	HWND hWndCanvas = pCS ? pCS->getHWnd() : NULL;
	if ( !hWndCanvas )
		return;
	string_type strAll;
	for ( int i = nItemS; i <= nItemE; ++i ) {
		if ( i >= (int)m_containerItems.size() || !m_containerItems[ i ].m_pTextSel )
			continue;
		string_type strOne;
		m_containerItems[ i ].m_pTextSel->getPlainText( strOne );
		int nCS = ( i == nItemS ) ? min( m_nSelCharAnchor, m_nSelCharActive ) : 0;
		int nCE = ( i == nItemE ) ? max( m_nSelCharAnchor, m_nSelCharActive ) : (int)strOne.length();
		if ( nCS < 0 ) nCS = 0;
		if ( nCE > (int)strOne.length() ) nCE = (int)strOne.length();
		if ( strAll.length() )
			strAll += _T("\r\n");
		strAll += strOne.substr( nCS, nCE - nCS );
	}
	if ( !::OpenClipboard( hWndCanvas ) )
		return;
	::EmptyClipboard();
	HGLOBAL hMem = ::GlobalAlloc( GMEM_MOVEABLE, ( strAll.length() + 1 ) * sizeof( TCHAR ) );
	if ( hMem ) {
		LPTSTR psz = (LPTSTR)::GlobalLock( hMem );
		if ( psz ) {
			_tcscpy_s( psz, strAll.length() + 1, strAll.c_str() );
			::GlobalUnlock( hMem );
#ifdef UNICODE
			::SetClipboardData( CF_UNICODETEXT, hMem );
#else
			::SetClipboardData( CF_TEXT, hMem );
#endif
		}
	}
	::CloseClipboard();
	if ( m_pEventListener )
		m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_MSGOUT_COPY, NULL, (void*)( UINT_PTR )strAll.length() );
}
