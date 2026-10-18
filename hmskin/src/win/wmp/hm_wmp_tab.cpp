#include "stdafx.h"
#include "win\wmp\hm_wmp_tab.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_pos_decoders.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hm_event_defs.h"
#include "hmskin_event_defs.h"
#include <assert.h>
//
#define TABHEADER_DEFAULT_MEMBITMAP_WIDTH (200)
#define TABHEADER_DEFAULT_MEMBITMAP_HEIGHT (200)
hm_wmp_tabheader::hm_wmp_tabheader( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_uIndexSel( 0 )
, m_uIndexHov( INDEX_NONE )
, m_uIndexPre( INDEX_NONE )
, m_uBufIndexSel( INDEX_NONE )
, m_uBufIndexHov( INDEX_NONE )
, m_uBufIndexPre( INDEX_NONE )
, m_hMemDC( NULL )
, m_hMemBitmap( NULL )
, m_hMemBitmapOld( NULL ) {
	::memset( &m_sizeMemBitmap, 0, sizeof(SIZE) );
	HDC hRealDC;

	hRealDC = ::GetDC( NULL );
	_Init( hRealDC, TABHEADER_DEFAULT_MEMBITMAP_WIDTH, TABHEADER_DEFAULT_MEMBITMAP_HEIGHT );
	::ReleaseDC( NULL, hRealDC );
	_createDistrictObjs();
}

hm_wmp_tabheader::~hm_wmp_tabheader() {
	delAllTabBtns();
	_UnInit();
}

MSGBIND_BEGIN( hm_wmp_tabheader )
	MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
	MSGBIND( WM_LBUTTONUP, _OnLButtonUp )
	MSGBIND( WM_LBUTTONDBLCLK, _OnLButtonDblClk )
	MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
	MSGBIND( WM_MOUSELEAVE, _OnMouseLeave )
	MSGBIND( WM_KEYDOWN, _OnKeyDown )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_tabheader )
	ORDERWNDMSG( WM_LBUTTONDOWN )
	ORDERWNDMSG( WM_LBUTTONUP )
	ORDERWNDMSG( WM_LBUTTONDBLCLK )
	ORDERWNDMSG( WM_MOUSEMOVE )
	ORDERWNDMSG( WM_MOUSELEAVE )
	ORDERWNDMSG( WM_KEYDOWN )
ORDERWNDMSG_END()
//
void hm_wmp_tabheader::_createDistrictObjs() {

	_setDistrictObj( EDISTRICTID_BK, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_TABBTN_NOR, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_TABBTN_HOV, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_TABBTN_PRE, new hm_districtobj() );
	_setDistrictObj( EDISTRICTID_TABBTN_SEL, new hm_districtobj() );
		
}
//
void hm_wmp_tabheader::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent ) {
	case HMKEELOBJEVENT_DRAWSELF:
		{
			HEDSPARAM* pHEDSParam = NULL;
			RECT rcCanvas;

			pHEDSParam = (HEDSPARAM*)pEventParam;
			if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
				return;

			// state switch = cache miss by definition ( safety net: the
			// interaction handlers invalidate too, this catches programmatic
			// changes ). with BUF_NONE it is pure bookkeeping.
			if ( ( m_uIndexSel != m_uBufIndexSel ) || ( m_uIndexHov != m_uBufIndexHov )
			     || ( m_uIndexPre != m_uBufIndexPre ) ) {
				invalidateBuffer();
				m_uBufIndexSel = m_uIndexSel;
				m_uBufIndexHov = m_uIndexHov;
				m_uBufIndexPre = m_uIndexPre;
			}
			_drawWithStrategy( pHEDSParam->m_hDCCanvas, rcCanvas );
		}
		break;
	default:
		break;
	}
}

void hm_wmp_tabheader::_render( HDC hDC, const RECT& rcCanvas ) {
	// the composed strip for the snapshotted state
	_doDrawSkinRaw( hDC, rcCanvas, m_uBufIndexSel, m_uBufIndexPre, m_uBufIndexHov );
}
//
void hm_wmp_tabheader::_OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcKeelObj;
	RECT rcTabBtn;
	POINT ptClient;
	unsigned int uTabBtnIndex;
	unsigned int uTabBtnCount = m_containerTabBtns.size();
	unsigned int uIndexSelOld = m_uIndexSel;
	HDC hDC;

	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	rcTabBtn.top = rcKeelObj.top;
	rcTabBtn.bottom = rcKeelObj.bottom;
	for ( uTabBtnIndex = 0; uTabBtnIndex<uTabBtnCount; ++uTabBtnIndex ) {
		rcTabBtn.left = uTabBtnIndex*( rcKeelObj.right - rcKeelObj.left ) / uTabBtnCount + rcKeelObj.left;
		rcTabBtn.right = ( uTabBtnIndex + 1 )*( rcKeelObj.right - rcKeelObj.left ) / uTabBtnCount + rcKeelObj.left;
		if ( ::PtInRect( &rcTabBtn, ptClient ) ) {
			m_uIndexSel = uTabBtnIndex;
			break;
		}
	}
	
	if ( uIndexSelOld != m_uIndexSel ) {
		hDC = ::GetDC( pWP->hwnd );
		if ( uIndexSelOld != INDEX_NONE ) {
			_doDrawBtnRaw( hDC, rcKeelObj, uIndexSelOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );	
		}
		if ( m_uIndexSel != INDEX_NONE ) {
			_doDrawBtnRaw( hDC, rcKeelObj, m_uIndexSel, hm_wmp_tabheader::EDISTRICTID_TABBTN_SEL );
		}
		::ReleaseDC( pWP->hwnd, hDC );
		invalidateBuffer();   // partial draw changed the visible state
		//
		//Notify( HMWMPTABEVENT_SELECTCHANGED, m_uIndexSel, uIndexSelOld );
		HKEREPARAM hkereParam;
		hkereParam.m_pEventSender = this;
		hkereParam.m_uEvent = HMWMPTABEVENT_SELECTCHANGED;
		hkereParam.m_lEventParam = m_uIndexSel;
		hkereParam.m_lEventParam2 = uIndexSelOld;
		pKeelObj->Notify( HMKEELOBJEVENT_RELAYEVENT, (LPARAM)&hkereParam );
		//
		if ( m_pEventListener ) {
			m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_TABBTN_CLICK, (void*)pKeelObj, (void*)(UINT_PTR)m_uIndexSel );
		}
	}

}
//
void hm_wmp_tabheader::_OnLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
}
//
void hm_wmp_tabheader::_OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
}
//
void hm_wmp_tabheader::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	RECT rcTabBtn;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	unsigned int uTabBtnIndex;
	unsigned int uTabBtnCount = m_containerTabBtns.size();
	unsigned int uTabBtnWidth;
	BOOL bNeedDraw = FALSE;
	unsigned int uIndexHovOld = m_uIndexHov;
	unsigned int uIndexPreOld = m_uIndexPre;
	unsigned int uIndexSelOld = m_uIndexSel;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	// get uTabBtnWidth
	rcTabBtn.top = rcKeelObj.top;
	rcTabBtn.bottom = rcKeelObj.bottom;
	if ( uTabBtnCount == 0 )
		return;
	uTabBtnWidth = ( rcKeelObj.right - rcKeelObj.left ) / uTabBtnCount;
	for ( uTabBtnIndex = 0; uTabBtnIndex<uTabBtnCount; ++uTabBtnIndex ) {
		rcTabBtn.left = uTabBtnIndex*( rcKeelObj.right - rcKeelObj.left ) / uTabBtnCount + rcKeelObj.left;
		rcTabBtn.right = ( uTabBtnIndex + 1 )*( rcKeelObj.right - rcKeelObj.left ) / uTabBtnCount + rcKeelObj.left;
		if ( ::PtInRect( &rcTabBtn, ptClient ) ) {
			if ( pWP->wParam & MK_LBUTTON ) {
				if ( uTabBtnIndex != this->m_uIndexPre ) {
					_setTabBtnPre( uTabBtnIndex );
				}
			} else {
				if ( uTabBtnIndex != this->m_uIndexHov ) {
					_setTabBtnHov( uTabBtnIndex );
				}
			}
			break;
		}
	}
	if ( uTabBtnIndex == uTabBtnCount ) {
		_setTabBtnPre( INDEX_NONE );
		_setTabBtnHov( INDEX_NONE );
	}

	// leave tracking is base-class mechanics now ( see hm_wmp_display ).
	armMouseLeaveTracking( pWP->hwnd );

	hDC = ::GetDC( pWP->hwnd );
	if ( uIndexHovOld != m_uIndexHov ) {
		if ( ( m_uIndexHov != INDEX_NONE ) && ( m_uIndexHov != m_uIndexSel ) )
			_doDrawBtnRaw( hDC, rcKeelObj, m_uIndexHov, hm_wmp_tabheader::EDISTRICTID_TABBTN_HOV );	
		if ( ( uIndexHovOld != INDEX_NONE ) && ( uIndexHovOld != m_uIndexSel ) )
			_doDrawBtnRaw( hDC, rcKeelObj, uIndexHovOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );
	}
	if ( uIndexPreOld != m_uIndexPre ) {
		if ( ( m_uIndexPre != INDEX_NONE ) && ( m_uIndexPre != m_uIndexSel ) )
			_doDrawBtnRaw( hDC, rcKeelObj, m_uIndexPre, hm_wmp_tabheader::EDISTRICTID_TABBTN_PRE );
		if ( ( uIndexPreOld != INDEX_NONE ) && ( uIndexPreOld != m_uIndexSel ) )
			_doDrawBtnRaw( hDC, rcKeelObj, uIndexPreOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );
	}
	if ( ( uIndexHovOld != m_uIndexHov ) || ( uIndexPreOld != m_uIndexPre ) )
		invalidateBuffer();   // partial draw changed the visible state
	::ReleaseDC( pWP->hwnd, hDC );


	//if ( bNeedDraw ) {
	//	hDC = ::GetDC( pWP->hwnd );
	//	//_doDrawSkin( hDC, rcKeelObj, m_uIndexSel, m_uIndexPre, m_uIndexHov );
	//	if ( uIndexHovOld != m_uIndexHov ) {
	//		if ( ( m_uIndexHov != INDEX_NONE ) && ( m_uIndexHov != m_uIndexSel ) )
	//			_doDrawBtnRaw( hDC, rcKeelObj, m_uIndexHov, hm_wmp_tabheader::EDISTRICTID_TABBTN_HOV );	
	//		if ( ( uIndexHovOld != INDEX_NONE ) && ( uIndexHovOld != m_uIndexSel ) )
	//			_doDrawBtnRaw( hDC, rcKeelObj, uIndexHovOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );
	//	}
	//	::ReleaseDC( pWP->hwnd, hDC );
	//}

	
	
}
//
void hm_wmp_tabheader::_setTabBtnHov( unsigned int uIndexHov ) {
	m_uIndexHov = uIndexHov;
	m_uIndexPre = INDEX_NONE;
}
//
void hm_wmp_tabheader::_setTabBtnPre( unsigned int uIndexPre ) {
	m_uIndexHov = INDEX_NONE;
	m_uIndexPre = uIndexPre;
}
//
void hm_wmp_tabheader::_OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	unsigned int uIndexHovOld = m_uIndexHov;
	unsigned int uIndexPreOld = m_uIndexPre;

	// the mouse left the window while a tab was highlighted: clear and
	// repaint it NOR. without this the hover state survived until the next
	// mouse entry ( stuck highlight when sliding out past the last tab ).
	onLeaveArrived();
	if ( ( uIndexHovOld == INDEX_NONE ) && ( uIndexPreOld == INDEX_NONE ) )
		return;
	_setTabBtnPre( INDEX_NONE );
	_setTabBtnHov( INDEX_NONE );

	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj )
		return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	hDC = ::GetDC( pWP->hwnd );
	if ( ( uIndexHovOld != INDEX_NONE ) && ( uIndexHovOld != m_uIndexSel ) )
		_doDrawBtnRaw( hDC, rcKeelObj, uIndexHovOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );
	if ( ( uIndexPreOld != INDEX_NONE ) && ( uIndexPreOld != m_uIndexSel ) )
		_doDrawBtnRaw( hDC, rcKeelObj, uIndexPreOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );
	::ReleaseDC( pWP->hwnd, hDC );
	invalidateBuffer();   // partial draw changed the visible state
}
//
void hm_wmp_tabheader::_OnKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	// keyboard accessibility ( DESIGN ): arrow keys cycle the chapters with
	// wrap-around. WM_KEYDOWN reaches the focused window ( the canvas ),
	// the dispatcher fans it out to this WMP. runs the exact same
	// select/redraw/notify path as a click.
	unsigned int uTabBtnCount = m_containerTabBtns.size();
	unsigned int uIndexSelOld = m_uIndexSel;
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcKeelObj;
	HDC hDC;
	int nStep = 0;

	if ( uTabBtnCount == 0 )
		return;
	if ( pWP->wParam == VK_LEFT )
		nStep = -1;
	else if ( pWP->wParam == VK_RIGHT )
		nStep = 1;
	else
		return;
	m_uIndexSel = ( m_uIndexSel + nStep + uTabBtnCount ) % uTabBtnCount;
	if ( m_uIndexSel == uIndexSelOld )
		return;

	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	hDC = ::GetDC( pWP->hwnd );
	_doDrawBtnRaw( hDC, rcKeelObj, uIndexSelOld, hm_wmp_tabheader::EDISTRICTID_TABBTN_NOR );
	_doDrawBtnRaw( hDC, rcKeelObj, m_uIndexSel, hm_wmp_tabheader::EDISTRICTID_TABBTN_SEL );
	::ReleaseDC( pWP->hwnd, hDC );
	invalidateBuffer();   // partial draw changed the visible state
	//
	HKEREPARAM hkereParam;
	hkereParam.m_pEventSender = this;
	hkereParam.m_uEvent = HMWMPTABEVENT_SELECTCHANGED;
	hkereParam.m_lEventParam = m_uIndexSel;
	hkereParam.m_lEventParam2 = uIndexSelOld;
	pKeelObj->Notify( HMKEELOBJEVENT_RELAYEVENT, (LPARAM)&hkereParam );
	//
	if ( m_pEventListener ) {
		m_pEventListener->onHKSkinEvent( HMSKIN_EVENT_TABBTN_CLICK, (void*)pKeelObj, (void*)(UINT_PTR)m_uIndexSel );
	}
	// the arrows are consumed here.
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
}

bool hm_wmp_tabheader::pushbackTabBtn( tabbtninfo* pTabBtnInfo ) {
	if ( !pTabBtnInfo ) return false;
	m_containerTabBtns.push_back( pTabBtnInfo );
	return true;
}
//
bool hm_wmp_tabheader::delTabBtn( unsigned int uTabBtnIndex ) {
	if ( uTabBtnIndex >= _getTabBtnCount() ) return false;
	tabbtninfo* pTabBtnInfo = m_containerTabBtns[ uTabBtnIndex ];
	
	delete pTabBtnInfo;
	pTabBtnInfo = NULL;
	m_containerTabBtns.erase( m_containerTabBtns.begin() + uTabBtnIndex );
	return true;
}
//
void hm_wmp_tabheader::delAllTabBtns() {
	tabbtninfo_container_type::iterator itTabBtnInfo, iendTabBtnInfo;
	tabbtninfo* pTabBtnInfo = NULL;

	iendTabBtnInfo = m_containerTabBtns.end();
	for ( itTabBtnInfo = m_containerTabBtns.begin(); itTabBtnInfo!=iendTabBtnInfo; ++itTabBtnInfo ) {
		pTabBtnInfo = *itTabBtnInfo;
		delete pTabBtnInfo;
		pTabBtnInfo = NULL;
	}
	m_containerTabBtns.clear();
}
//
const tabbtninfo* hm_wmp_tabheader::getTabBtn( unsigned int uTabBtnIndex ) {
	// the comparisons were INVERTED for 9 years ( >= / < ): valid indices
	// returned NULL while out-of-range ones walked into operator[] -
	// flipped to the correct direction.
	if ( m_containerTabBtns.size() == 0
	|| m_containerTabBtns.size() <= uTabBtnIndex ) return NULL;
	return m_containerTabBtns[ uTabBtnIndex ];
}
//
bool hm_wmp_tabheader::hasTabBtn( unsigned int uTabBtndIndex ) {
	return ( m_containerTabBtns.size() > 0 
	&& m_containerTabBtns.size() > uTabBtndIndex );
}
//
unsigned int hm_wmp_tabheader::getTabBtnCount() {
	return _getTabBtnCount();
}
//
void hm_wmp_tabheader::_Init( HDC hRealDC, unsigned int uWidthNew, unsigned int uHeightNew ) {
	assert( !m_hMemDC );
	assert( !m_hMemBitmap );

	m_hMemDC = ::CreateCompatibleDC( hRealDC );
	m_hMemBitmap = ::CreateCompatibleBitmap( hRealDC, uWidthNew, uHeightNew );
	m_hMemBitmapOld = (HBITMAP)::SelectObject( m_hMemDC, m_hMemBitmap );

	m_sizeMemBitmap.cx = uWidthNew;
	m_sizeMemBitmap.cy = uHeightNew;
}
//
void hm_wmp_tabheader::_UnInit() {
	assert( m_hMemDC );
	assert( m_hMemBitmap );
	HBITMAP hBitmapTmp;

	hBitmapTmp = (HBITMAP)::SelectObject( m_hMemDC, m_hMemBitmapOld );
	assert( hBitmapTmp == m_hMemBitmap );
	::DeleteObject( m_hMemBitmap );
	m_hMemBitmap = NULL;
	::DeleteDC( m_hMemDC );
	m_hMemDC = NULL;
}

//
void hm_wmp_tabheader::_UpdateMemBitmap( HDC hRealDC, unsigned int uWidthNew, unsigned int uHeightNew ) {
	if ( ( m_sizeMemBitmap.cx >= ( long )uWidthNew ) && ( m_sizeMemBitmap.cy >= ( long )uHeightNew ) )
		return;
	
	_UnInit();
	_Init( hRealDC, uWidthNew, uHeightNew );
}
//
void hm_wmp_tabheader::_doDrawSkin( HDC hDC, const RECT& rcCanvas, unsigned int uIndexSel, unsigned int uIndexPre, unsigned int uIndexHover ) {
	RECT rcCanvasMem;

	_UpdateMemBitmap( hDC, (rcCanvas.right - rcCanvas.left), ( rcCanvas.bottom - rcCanvas.top ) );
	::SetRect( &rcCanvasMem, 0, 0, m_sizeMemBitmap.cx, m_sizeMemBitmap.cy );
	_doDrawSkinRaw( m_hMemDC, rcCanvasMem, uIndexSel, uIndexPre, uIndexHover );
	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, rcCanvasMem.right, rcCanvasMem.bottom, m_hMemDC, 0, 0, SRCCOPY );
}
//
void hm_wmp_tabheader::_doDrawSkinRaw( HDC hDC, const RECT& rcCanvas, unsigned int uIndexSel, unsigned int uIndexPre, unsigned int uIndexHover ) {
	if ( rcCanvas.left >= rcCanvas.right ) return;
	hm_districtobj* pDistrictObj = NULL;
	unsigned int uTabBtnWidth;
	unsigned int uTabBtnIndex;
	unsigned int uTabBtnCount = m_containerTabBtns.size();
	hm_posdecoder_dock* pPosDecoder = NULL;

	// draw background
	if ( _getDistrictObj( EDISTRICTID_BK, &pDistrictObj ) ) {
		pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
	}
	// 
	if ( uTabBtnCount == 0 ) return;
	uTabBtnWidth = ( rcCanvas.right - rcCanvas.left ) / uTabBtnCount;
	pPosDecoder = new hm_posdecoder_dock( hm_posdecoder_dock::DOCK_LEFT, 0, 0, 0, 0);
	for ( uTabBtnIndex = 0; uTabBtnIndex < uTabBtnCount; ++uTabBtnIndex ) {
		pPosDecoder->m_space1 = uTabBtnIndex*( rcCanvas.right - rcCanvas.left ) / uTabBtnCount;
		pPosDecoder->m_nThickness = ( uTabBtnIndex + 1 ) * ( rcCanvas.right - rcCanvas.left ) / uTabBtnCount - uTabBtnIndex * ( rcCanvas.right - rcCanvas.left ) / uTabBtnCount;
		if ( uTabBtnIndex == uIndexSel ) {
			if ( !_getDistrictObj( EDISTRICTID_TABBTN_SEL, &pDistrictObj ) ) 
				continue;
		} else if ( uTabBtnIndex == uIndexPre ) {
			if ( !_getDistrictObj( EDISTRICTID_TABBTN_PRE, &pDistrictObj ) ) 
				continue;
		} else if ( uTabBtnIndex == uIndexHover ) {
			if ( !_getDistrictObj( EDISTRICTID_TABBTN_HOV, &pDistrictObj ) ) 
				continue;
		} else {
			if ( !_getDistrictObj( EDISTRICTID_TABBTN_NOR, &pDistrictObj ) ) 
				continue;
		}
		//
		_setText( pDistrictObj, uTabBtnIndex );
		//
		pDistrictObj->setPosDecoder( pPosDecoder );
		pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
		pDistrictObj->setPosDecoder( NULL );
		//
	}
	delete pPosDecoder;
	pPosDecoder = NULL;
}

void hm_wmp_tabheader::_doDrawBtnRaw( HDC hDC, const RECT& rcCanvas, unsigned int uTabBtnIndex, ENUMDISTRICTID eDistrictType ) {
	unsigned int uTabBtnCount = m_containerTabBtns.size();
	if ( !hDC || uTabBtnIndex >= uTabBtnCount )
		return;
	hm_districtobj* pDistrictObj = NULL;
	hm_posdecoder_dock* pPosDecoder = NULL;
	
	if ( !_getDistrictObj( eDistrictType, &pDistrictObj ) ) 
		return;
	//
	_setText( pDistrictObj, uTabBtnIndex );
	//
	pPosDecoder = new hm_posdecoder_dock( hm_posdecoder_dock::DOCK_LEFT, 0, 0, 0, 0);
	pPosDecoder->m_space1 = uTabBtnIndex*( rcCanvas.right - rcCanvas.left ) / uTabBtnCount;
	pPosDecoder->m_nThickness = ( uTabBtnIndex + 1 ) * ( rcCanvas.right - rcCanvas.left ) / uTabBtnCount - uTabBtnIndex * ( rcCanvas.right - rcCanvas.left ) / uTabBtnCount;;
	pDistrictObj->setPosDecoder( pPosDecoder );
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
	pDistrictObj->setPosDecoder( NULL );

	delete pPosDecoder;
	pPosDecoder = NULL;
}

void hm_wmp_tabheader::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_BK );
	containerIds.push_back( EDISTRICTID_TABBTN_NOR );
	containerIds.push_back( EDISTRICTID_TABBTN_HOV );
	containerIds.push_back( EDISTRICTID_TABBTN_PRE );
	containerIds.push_back( EDISTRICTID_TABBTN_SEL );
}

bool hm_wmp_tabheader::_setText( hm_districtobj* pDistrictObj, unsigned int uTabBtnIndex ) {
	if ( !pDistrictObj )
		return false;
	if ( uTabBtnIndex >= m_containerTabBtns.size() ) {
		assert( false );
		return false;
	}
	unsigned int uShowObjCount;
	hm_rs_uires_text* pUIText;
	tabbtninfo* pTabInfo;

	uShowObjCount = pDistrictObj->getShowObjCount();
	for ( unsigned int uIndex = 0; uIndex != uShowObjCount; ++uIndex ) {
		pUIText = dynamic_cast< hm_rs_uires_text* >( pDistrictObj->getShowObj( uIndex )->m_pUIRes );
		if ( pUIText ) {
			pTabInfo = m_containerTabBtns[ uTabBtnIndex ];
			pUIText->setText( pTabInfo->m_strTitle );
			return true;
		}
	}
	return false;
}

bool hm_wmp_tabheader::setText( hm_districtobj* pDistrictObj, unsigned int uTabBtnIndex ) {
	return _setText( pDistrictObj, uTabBtnIndex );
}

void hm_wmp_tabheader::pushDown( unsigned int uTabBtnIndex ) {
	if ( uTabBtnIndex >= m_containerTabBtns.size() ) {
		assert( false );
		return;
	}
	m_uIndexSel = uTabBtnIndex;
}