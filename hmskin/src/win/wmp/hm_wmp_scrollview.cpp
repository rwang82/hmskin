#include "stdafx.h"
#include "win\wmp\hm_wmp_scrollview.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\frame\hm_uiroot.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_debug_utils.h"
#include "win\hm_event_defs.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
hm_wmp_scrollview::SCROLLVIEW_CFG::SCROLLVIEW_CFG()
: m_idWMPSearchBar( 0 )
, m_idKeelObjContent( 0 )
, m_idKeelObjBar( 0 )
, m_idWMPContent( 0 )
, m_uContentExtentW( 0 )
, m_uContentExtentH( 0 )
, m_uArrowLen( 20 )
, m_uMinThumbLen( 20 )
, m_nFixedThumbLen( -1 )
, m_bLayoutScroll( FALSE )
, m_nWheelStep( 60 )
, m_bAnchorBottom( FALSE )
{
}

hm_wmp_scrollview::hm_wmp_scrollview( const SCROLLVIEW_CFG& cfgScaled, hm_uiroot* pUIRoot,
	hmskin_event_listener_base* pEventListener )
: hm_wndmsgprocessor_i( pEventListener )
, m_cfg( cfgScaled )
, m_pHMUIRoot( pUIRoot )
, m_pWMPBar( NULL )
, m_pKeelObjViewport( NULL )
, m_uContentExtentW( cfgScaled.m_uContentExtentW )
, m_uContentExtentH( cfgScaled.m_uContentExtentH )
, m_uFlag( FLAG_SVIEW_CONTENT_DIRTY )
, m_bWired( false )
, m_bRangeBound( false )
{
	::SetRect( &m_rcContentVisible, 0, 0, 0, 0 );
	m_core.setExtent( (int)cfgScaled.m_uContentExtentH );
	m_core.setThumbCfg( cfgScaled.m_nFixedThumbLen, cfgScaled.m_uMinThumbLen );
}

hm_wmp_scrollview::~hm_wmp_scrollview() {
	// shutdown contract ( header ): every referenced WMP has a smaller id and
	// died first - its ~hm_cmn_subject::EraseAllRegister already removed our
	// subscriptions. the keelobj tree died even earlier and took the
	// POSCHANGED registration with it. touch nothing here.
	m_pWMPBar = NULL;
	m_pKeelObjViewport = NULL;
}

//
MSGBIND_BEGIN( hm_wmp_scrollview )
MSGBIND( WM_MOUSEWHEEL, _OnMouseWheel )
MSGBIND_END()

ORDERWNDMSG_BEGIN( hm_wmp_scrollview )
ORDERWNDMSG( WM_MOUSEWHEEL )
ORDERWNDMSG_END()

//
void hm_wmp_scrollview::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent )
	{
	case HM_EVENT_KEELOBJ_POSCHANGED:
		{ // relayed by the viewport keelobj after its layout settled.
			hm_keel_obj* pKeelObjViewport = (hm_keel_obj*)pSubject;
			if ( !pKeelObjViewport )
				break;
			m_pKeelObjViewport = pKeelObjViewport;
			if ( !_ensureWiring() )
				break;
			_recalcFromViewport( pKeelObjViewport );
			m_bRangeBound = true;
		}
		break;
	case HM_EVENT_TRACKTHUMB_MOVED:
		{ // subject = the scrollbar WMP; drag and wheel-over-bar both arrive here.
			if ( !_ensureWiring() || !m_pKeelObjViewport || !m_pKeelObjViewport->getLayoutObj() )
				break;
			HETMPARAM* pHETMParam = (HETMPARAM*)pEventParam;
			if ( !pHETMParam || !pHETMParam->m_pWMPScrollbar || pHETMParam->m_pWMPScrollbar != m_pWMPBar )
				break;
			// lazy wiring may have happened just above ( first interaction
			// BEFORE any POSCHANGED reached us, e.g. headless ) - the range
			// must be bound once before first use.
			if ( !m_bRangeBound ) {
				_recalcFromViewport( m_pKeelObjViewport );
				m_bRangeBound = true;
			}
			unsigned int uExtentW, uExtentH;
			if ( !_getContentExtent( uExtentW, uExtentH, m_pKeelObjViewport ) )
				break;
			int nViewH = m_pKeelObjViewport->getLayoutObj()->m_rect.bottom
				- m_pKeelObjViewport->getLayoutObj()->m_rect.top;
			int nScrollable = (int)uExtentH - nViewH;
			int nTop = 0;
			if ( nScrollable > 0 && pHETMParam->m_nPosMax > 0 )
				nTop = nScrollable * pHETMParam->m_nPosCur / pHETMParam->m_nPosMax;
			_scrollTo( m_pKeelObjViewport, nTop, true );
		}
		break;
	case HM_EVENT_CONTENT_EXTENT:
		{ // subject = the content WMP; it re-measured itself.
			HECTPARAM* pHECTParam = (HECTPARAM*)pEventParam;
			if ( !pHECTParam )
				break;
			if ( m_cfg.m_bAnchorBottom ) {
				// chat-list mode: decide against the OLD extent before overwriting.
				// at-bottom ( stick-to-latest ) -> pin to the new bottom; scrolled
				// up ( do-not-disturb ) -> keep the position, the non-dirty clamp
				// path preserves a still-valid top. never sets the (0,0) dirty reset.
				bool bAtBottom = m_core.isAtBottom();   // against the OLD extent
				m_uContentExtentW = ( pHECTParam->m_cxExtent > 0 ) ? (unsigned int)pHECTParam->m_cxExtent : 0;
				m_uContentExtentH = ( pHECTParam->m_cyExtent > 0 ) ? (unsigned int)pHECTParam->m_cyExtent : 0;
				m_core.setExtent( (int)m_uContentExtentH );
				if ( m_bWired && m_pKeelObjViewport ) {
					if ( bAtBottom ) {
						// re-bind the bar range FIRST: _scrollTo syncs the value
						// and paints the thumb while the range is still 0 when the
						// extent jumps 0 -> large ( divide-by-zero in the bar ).
						_updateScrollBarPos( m_pKeelObjViewport );
						_scrollTo( m_pKeelObjViewport, m_core.getMaxTop(), false );
					} else {
						// keeps top; re-syncs the bar range/thumb for the new extent.
						_recalcFromViewport( m_pKeelObjViewport );
					}
				}
			} else {
				m_uContentExtentW = ( pHECTParam->m_cxExtent > 0 ) ? (unsigned int)pHECTParam->m_cxExtent : 0;
				m_uContentExtentH = ( pHECTParam->m_cyExtent > 0 ) ? (unsigned int)pHECTParam->m_cyExtent : 0;
				m_uFlag |= FLAG_SVIEW_CONTENT_DIRTY;
				if ( m_bWired && m_pKeelObjViewport )
					_recalcFromViewport( m_pKeelObjViewport );
			}
		}
		break;
	case HM_EVENT_MOUSEWHEEL:
		{ // subject = the content WMP; inertial ticks as pixel distances
		  // ( positive = towards the content top, top decreases ).
			if ( !m_bWired || !m_pKeelObjViewport || !m_pKeelObjViewport->getLayoutObj() )
				break;
			int nDistance = (int)pEventParam;
			if ( nDistance )
				_scrollTo( m_pKeelObjViewport, m_rcContentVisible.top - nDistance, false );
		}
		break;
	default:
		break;
	}
}

//
hm_keel_obj* hm_wmp_scrollview::_getKeelObj( unsigned int uId ) const {
	if ( !m_pHMUIRoot || !m_pHMUIRoot->m_pKeelObjFactory || !uId )
		return NULL;
	return m_pHMUIRoot->m_pKeelObjFactory->getObj( uId );
}

bool hm_wmp_scrollview::_ensureWiring() {
	if ( m_bWired )
		return true;
	if ( !m_pHMUIRoot || !m_pHMUIRoot->m_pWMPFactory || !m_pHMUIRoot->m_pKeelObjFactory )
		return false;
	// the WMP manifest section parses BEFORE the KeelObj section, so at
	// construction time none of the keelobjs exist - resolve lazily. Register
	// is NON-idempotent ( duplicate registration asserts ): every id must
	// resolve before the first subscription runs, and m_bWired stays false
	// on any partial failure so a later entry point retries cleanly.
	hm_wndmsgprocessor_i* pWMPBarBase = m_pHMUIRoot->m_pWMPFactory->getObj( m_cfg.m_idWMPSearchBar );
	hm_wmp_scrollbar* pWMPBar = dynamic_cast< hm_wmp_scrollbar* >( pWMPBarBase );
	if ( !pWMPBar )
		return false;
	if ( !_getKeelObj( m_cfg.m_idKeelObjContent ) )
		return false;
	hm_wndmsgprocessor_i* pWMPContent = NULL;
	if ( m_cfg.m_idWMPContent ) {
		pWMPContent = m_pHMUIRoot->m_pWMPFactory->getObj( m_cfg.m_idWMPContent );
		if ( !pWMPContent )
			return false;
	}
	// all resolutions passed - subscriptions are safe now.
	m_pWMPBar = pWMPBar;
	m_pWMPBar->Register( this, HM_EVENT_TRACKTHUMB_MOVED );
	if ( pWMPContent ) {
		pWMPContent->Register( this, HM_EVENT_CONTENT_EXTENT );
		// inertial wheel ticks ( whellengine ) arrive as pixel distances.
		pWMPContent->Register( this, HM_EVENT_MOUSEWHEEL );
	}
	m_pWMPBar->setStartThumbLen( m_cfg.m_uArrowLen );
	m_pWMPBar->setEndThumbLen( m_cfg.m_uArrowLen );
	if ( m_cfg.m_nFixedThumbLen >= 0 )
		m_pWMPBar->setTrackThumbLen( (unsigned int)m_cfg.m_nFixedThumbLen );
	m_bWired = true;
	return true;
}

bool hm_wmp_scrollview::_getContentExtent( unsigned int& uW, unsigned int& uH, hm_keel_obj* pKeelObjViewport ) const {
	if ( m_uContentExtentW || m_uContentExtentH ) {
		uW = m_uContentExtentW;
		uH = m_uContentExtentH;
		return true;
	}
	// no static/dynamic extent declared: measure the content keelobj rect
	// ( a scroll offset translates it but never resizes it ).
	hm_keel_obj* pKeelObjContent = _getKeelObj( m_cfg.m_idKeelObjContent );
	hm_layout_obj* pLayoutContent = pKeelObjContent ? pKeelObjContent->getLayoutObj() : NULL;
	if ( !pLayoutContent ) {
		uW = uH = 0;
		return false;
	}
	uW = (unsigned int)( pLayoutContent->m_rect.right - pLayoutContent->m_rect.left );
	uH = (unsigned int)( pLayoutContent->m_rect.bottom - pLayoutContent->m_rect.top );
	return true;
}

// visible-window settle: horizontal math inline ( the core is vertical ),
// vertical clamping delegated to hm_scroll_core - dirty resets to the top,
// otherwise a still-valid top survives and an overshoot re-clamps.
void hm_wmp_scrollview::_calcContentVisibleRect( const SIZE& sizeCanvas ) {
	if ( FLAG_SVIEW_CONTENT_DIRTY & m_uFlag )
		m_core.reset();
	else
		m_core.scrollTo( m_core.getTop() );   // clamp-only apply

	m_rcContentVisible.left = 0;
	m_rcContentVisible.right = ( long )min( m_uContentExtentW, (unsigned int)sizeCanvas.cx );
	m_rcContentVisible.top = m_core.getTop();
	m_rcContentVisible.bottom = m_rcContentVisible.top
		+ ( long )min( (unsigned int)m_core.getExtentH(), (unsigned int)sizeCanvas.cy );

	// cancel FLAG_SVIEW_CONTENT_DIRTY flag
	m_uFlag &= ~FLAG_SVIEW_CONTENT_DIRTY;
}

void hm_wmp_scrollview::_recalcFromViewport( hm_keel_obj* pKeelObjViewport ) {
	hm_layout_obj* pLayoutViewport = pKeelObjViewport ? pKeelObjViewport->getLayoutObj() : NULL;
	if ( !pLayoutViewport )
		return;
	// the children's decoders observe the viewport layout, but their observers
	// may run AFTER our POSCHANGED relay - recalc them so the sizes read
	// below are current ( calcPosition is idempotent; the old scrollview did
	// the same ).
	hm_keel_obj* pKeelObjContent = _getKeelObj( m_cfg.m_idKeelObjContent );
	hm_keel_obj* pKeelObjBar = _getKeelObj( m_cfg.m_idKeelObjBar );
	if ( pKeelObjContent && pKeelObjContent->getLayoutObj() )
		pKeelObjContent->getLayoutObj()->calcPosition();
	if ( pKeelObjBar && pKeelObjBar->getLayoutObj() )
		pKeelObjBar->getLayoutObj()->calcPosition();

	SIZE sizeCanvas;
	sizeCanvas.cx = pLayoutViewport->m_rect.right - pLayoutViewport->m_rect.left;
	sizeCanvas.cy = pLayoutViewport->m_rect.bottom - pLayoutViewport->m_rect.top;
	unsigned int uExtentW = 0, uExtentH = 0;
	_getContentExtent( uExtentW, uExtentH, pKeelObjViewport );
	m_core.setViewport( sizeCanvas.cy );
	m_core.setExtent( (int)uExtentH );
	_calcContentVisibleRect( sizeCanvas );
	_updateScrollBarPos( pKeelObjViewport );
}

void hm_wmp_scrollview::_updateScrollBarPos( hm_keel_obj* pKeelObjViewport ) {
	if ( !m_bWired || !m_pWMPBar )
		return;
	hm_layout_obj* pLayoutViewport = pKeelObjViewport ? pKeelObjViewport->getLayoutObj() : NULL;
	if ( !pLayoutViewport )
		return;

	// range / value / thumb math lives in the scroll core.
	m_core.syncBar( m_pWMPBar, _getKeelObj( m_cfg.m_idKeelObjBar ) );
}

void hm_wmp_scrollview::_applyContentOffset( int nTop ) {
	hm_keel_obj* pKeelObjContent = _getKeelObj( m_cfg.m_idKeelObjContent );
	hm_layout_obj* pLayoutContent = pKeelObjContent ? pKeelObjContent->getLayoutObj() : NULL;
	if ( !pLayoutContent )
		return;
	POINT ptScrollOffset;
	ptScrollOffset.x = 0;
	ptScrollOffset.y = -nTop;
	pLayoutContent->setScrollOffset( ptScrollOffset );
	pLayoutContent->calcPosition();
}

void hm_wmp_scrollview::_scrollTo( hm_keel_obj* pKeelObjViewport, int nTop, bool bFromBar ) {
	hm_layout_obj* pLayoutViewport = pKeelObjViewport ? pKeelObjViewport->getLayoutObj() : NULL;
	if ( !pLayoutViewport )
		return;
	unsigned int uExtentW, uExtentH;
	if ( !_getContentExtent( uExtentW, uExtentH, pKeelObjViewport ) )
		return;
	int nViewH = pLayoutViewport->m_rect.bottom - pLayoutViewport->m_rect.top;
	m_core.setViewport( nViewH );
	m_core.setExtent( (int)uExtentH );
	m_core.scrollTo( nTop );
	nTop = m_core.getTop();

	m_rcContentVisible.top = nTop;
	m_rcContentVisible.bottom = nTop + min( (int)uExtentH, nViewH );
	m_uFlag &= ~FLAG_SVIEW_CONTENT_DIRTY;

	if ( m_cfg.m_bLayoutScroll )
		_applyContentOffset( nTop );

	// pure notification - content WMPs repaint through the normal paint
	// cycle, no offscreen dc rides along ( engine contract, hm_event_defs.h ).
	HEMVPARAM hemvParam;
	hemvParam.m_vertical.m_nContentPosStart = (int)m_rcContentVisible.top;
	hemvParam.m_vertical.m_nContentPosEnd = (int)m_rcContentVisible.bottom;
	hemvParam.m_horizontal.m_nContentPosStart = (int)m_rcContentVisible.left;
	hemvParam.m_horizontal.m_nContentPosEnd = (int)m_rcContentVisible.right;
	hemvParam.m_pViewContentKeelObj = _getKeelObj( m_cfg.m_idKeelObjContent );
	Notify( HM_EVENT_MOVE_VIEWCONTENT, (LPARAM)&hemvParam );

	// wheel / extent paths moved the value from outside the bar - sync it
	// back and repaint the thumb ( the drag path already did both itself ).
	if ( !bFromBar && m_pWMPBar ) {
		m_pWMPBar->setCurValue( nTop );
		m_pWMPBar->drawSelf();
	}

	_invalidateContentViewport( pKeelObjViewport );
}

void hm_wmp_scrollview::_invalidateContentViewport( hm_keel_obj* pKeelObjViewport ) {
	hm_layout_obj* pLayoutViewport = pKeelObjViewport ? pKeelObjViewport->getLayoutObj() : NULL;
	if ( !pLayoutViewport )
		return;
	const coordinate_system_hwnd* pCoordSysHwnd = pLayoutViewport->getCoordinateSystem();
	HWND hWndCanvas = pCoordSysHwnd ? pCoordSysHwnd->getHWnd() : NULL;
	if ( !hWndCanvas )
		return;
	RECT rcViewport;
	if ( !pLayoutViewport->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcViewport ) )
		return;
	// trim the bar strip ( dock-right shape ): the bar repaints itself, a
	// wider invalidate would repaint it per scroll tick.
	hm_keel_obj* pKeelObjBar = _getKeelObj( m_cfg.m_idKeelObjBar );
	hm_layout_obj* pLayoutBar = pKeelObjBar ? pKeelObjBar->getLayoutObj() : NULL;
	if ( pLayoutBar ) {
		RECT rcBar;
		if ( pLayoutBar->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcBar )
			&& rcBar.left > rcViewport.left )
			rcViewport.right = min( rcViewport.right, rcBar.left );
	}
	hm_util_invlog( _T("scview-scroll"), &rcViewport );
	::InvalidateRect( hWndCanvas, &rcViewport, FALSE );
}

bool hm_wmp_scrollview::_isCursorOverViewport( hm_keel_obj* pKeelObjViewport, HWND hWndMsg, POINT ptScreen ) const {
	hm_layout_obj* pLayoutViewport = pKeelObjViewport ? pKeelObjViewport->getLayoutObj() : NULL;
	if ( !pLayoutViewport )
		return false;
	RECT rcViewport;
	if ( !pLayoutViewport->mapPosition( hWndMsg, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcViewport ) )
		return false;
	// the wheel event point rides in the message lParam ( SCREEN coords ):
	// real hardware input and synthetic posts agree on it, GetCursorPos
	// does not ( the real cursor may sit elsewhere during posted messages ).
	POINT ptCursor = ptScreen;
	if ( !::ScreenToClient( hWndMsg, &ptCursor ) )
		return false;
	if ( !::PtInRect( &rcViewport, ptCursor ) ) {
		hm_util_invlog( _T("scview-wheel-outside"), NULL );
		return false;
	}
	// the bar strip handles its own wheel - stay out of it.
	hm_keel_obj* pKeelObjBar = _getKeelObj( m_cfg.m_idKeelObjBar );
	hm_layout_obj* pLayoutBar = pKeelObjBar ? pKeelObjBar->getLayoutObj() : NULL;
	if ( pLayoutBar ) {
		RECT rcBar;
		if ( pLayoutBar->mapPosition( hWndMsg, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcBar )
			&& ::PtInRect( &rcBar, ptCursor ) ) {
			hm_util_invlog( _T("scview-wheel-onbar"), &rcBar );
			return false;
		}
	}
	return true;
}

void hm_wmp_scrollview::_OnMouseWheel( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( !pKeelObj || !pWP )
		return;
	// a dynamic content WMP brings the inertial wheel stream ( evt10 ) - the
	// raw viewport wheel must yield or every notch double-scrolls.
	if ( m_cfg.m_idWMPContent )
		return;
	if ( !_ensureWiring() )
		return;
	m_pKeelObjViewport = pKeelObj;
	// lazy wiring may have just happened ( first interaction before any
	// POSCHANGED reached us, e.g. a headless host ) - bind the range once.
	if ( !m_bRangeBound ) {
		_recalcFromViewport( pKeelObj );
		m_bRangeBound = true;
	}
	POINT ptScreen;
	ptScreen.x = (int)(short)LOWORD( pWP->lParam );
	ptScreen.y = (int)(short)HIWORD( pWP->lParam );
	if ( !_isCursorOverViewport( pKeelObj, pWP->hwnd, ptScreen ) )
		return;
	int nNotches = (int)GET_WHEEL_DELTA_WPARAM( pWP->wParam ) / WHEEL_DELTA;
	if ( nNotches == 0 )
		return;
	// wheel forward ( positive delta ) scrolls toward the content top.
	_scrollTo( pKeelObj, m_rcContentVisible.top - nNotches * m_cfg.m_nWheelStep, false );
}
