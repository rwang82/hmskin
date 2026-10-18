#include "stdafx.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"


hm_wmp_display::hm_wmp_display( hmskin_event_listener_base* pEventListener )
: hm_wndmsgprocessor_i( pEventListener )
, m_pDrawTool( NULL )
, m_bLeaveTracking( FALSE )
, m_eBufStrategy( BUF_NONE )
, m_hBufBmp( NULL )
, m_bBufDirty( TRUE )
, m_uiresObserver() {
	m_uiresObserver.m_pOwner = this;
	memset( &m_rcBufCanvas, 0, sizeof( RECT ) );
}

void hm_wmp_display::registerShowObjEvents( hm_showobj* pShowObj ) {
	// subscribe the adapter to text-content broadcasts: setText on the
	// uires then invalidates this WMP's pixel cache automatically
	if ( !pShowObj || !pShowObj->m_pUIRes )
		return;
	hm_rs_uires_text* pText = dynamic_cast< hm_rs_uires_text* >( pShowObj->m_pUIRes );
	if ( !pText )
		return;
	if ( !pText->HasRegister( &m_uiresObserver, HM_UIRES_EVENT_TEXTCHANGED ) )
		pText->Register( &m_uiresObserver, HM_UIRES_EVENT_TEXTCHANGED );
}
//
hm_wmp_display::~hm_wmp_display() {
	_clearAllDistrictObjs();
	//if ( m_pDrawTool )
	//	delete m_pDrawTool;
	m_pDrawTool = NULL;
	//
	if ( m_hBufBmp ) {
		::DeleteObject( m_hBufBmp );
		m_hBufBmp = NULL;
	}
}
//
void hm_wmp_display::drawSelf( hm_keel_obj* pKeelObj, HWND hWndCanvas ) {
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcCanvas;
	HDC hDC;

	if ( !pKeelObj || !hWndCanvas )
		return;
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj )
		return;
	if ( !pLayoutObj->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;

	hDC = ::GetDC( hWndCanvas );
	if ( !hDC )
		return;
	_drawWithStrategy( hDC, rcCanvas );
	::ReleaseDC( hWndCanvas, hDC );
}
//
void hm_wmp_display::_drawWithStrategy( HDC hDC, const RECT& rcCanvas ) {
	// direct draw when: control declared BUF_NONE, or the target is a memory
	// DC ( the root WM_PAINT path - it composites offscreen already, a
	// nested per-draw buffer would only add a copy ). WindowFromDC returns
	// the owner window for screen DCs and NULL for memory DCs.
	// EXCEPTION: BUF_CACHE_SIZE is a pixel cache, not a per-draw buffer -
	// it opts into the paint path too, because its BitBlt is cheaper than
	// re-rendering ( that is its whole point ).
	if ( ( m_eBufStrategy == BUF_NONE )
	     || ( !::WindowFromDC( hDC ) && m_eBufStrategy != BUF_CACHE_SIZE ) ) {
		_render( hDC, rcCanvas );
		return;
	}

	// screen or paint DC with a buffering strategy: compose/serve offscreen,
	// blit once so no intermediate render step ( opaque bg fill -> bitmap
	// -> text ) is ever visible on screen.
	int nW = rcCanvas.right - rcCanvas.left;
	int nH = rcCanvas.bottom - rcCanvas.top;
	if ( ( nW <= 0 ) || ( nH <= 0 ) )
		return;
	HDC hMemDC = ::CreateCompatibleDC( hDC );
	if ( !hMemDC )
		return;
	HBITMAP hBmpDraw = NULL;
	HBITMAP hBmpOld = NULL;
	BOOL bBufOwner = FALSE;
	BOOL bRender = TRUE;

	if ( m_eBufStrategy == BUF_ALLOC ) {
		// click-rate controls: draws are rare, fresh bitmap per draw.
		hBmpDraw = ::CreateCompatibleBitmap( hDC, nW, nH );
		bBufOwner = TRUE;
	} else {
		// BUF_CACHE_SIZE pixel cache: (re)allocate only when the canvas
		// size changes; re-render only when dirty ( content change ) - a
		// clean cache serves the blit alone.
		if ( !m_hBufBmp || ( m_rcBufCanvas.right != nW ) || ( m_rcBufCanvas.bottom != nH ) ) {
			if ( m_hBufBmp )
				::DeleteObject( m_hBufBmp );
			m_hBufBmp = ::CreateCompatibleBitmap( hDC, nW, nH );
			m_rcBufCanvas.left = 0;
			m_rcBufCanvas.top = 0;
			m_rcBufCanvas.right = nW;
			m_rcBufCanvas.bottom = nH;
			m_bBufDirty = TRUE;
		}
		hBmpDraw = m_hBufBmp;
		bRender = m_bBufDirty;
	}
	if ( !hBmpDraw ) {
		::DeleteDC( hMemDC );
		return;
	}

	hBmpOld = ( HBITMAP )::SelectObject( hMemDC, hBmpDraw );
	if ( bRender ) {
		RECT rcMem = { 0, 0, nW, nH };
		_render( hMemDC, rcMem );
		m_bBufDirty = FALSE;
	}
	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, nW, nH, hMemDC, 0, 0, SRCCOPY );
	::SelectObject( hMemDC, hBmpOld );

	if ( bBufOwner )
		::DeleteObject( hBmpDraw );
	::DeleteDC( hMemDC );
}
//
void hm_wmp_display::setDrawTool( hm_ps_drawtool_i* pDrawTool ) { 
	// different drawtool need different resource, so when we change the drawtool, need reload resource that drawtool need.

	// unload resource
	if ( m_pDrawTool ) {
		map_id2district_type::iterator itDistrictObj, iendDistrictObj;
		hm_districtobj* pDistrictObj = NULL; 

		iendDistrictObj = m_mapId2District.end();
		for ( itDistrictObj = m_mapId2District.begin(); itDistrictObj!=iendDistrictObj; ++itDistrictObj ) {
			pDistrictObj = itDistrictObj->second;
			pDistrictObj->unloadRes( m_pDrawTool );
		}
		//
		//delete m_pDrawTool;
		m_pDrawTool = NULL;
	}
	
	m_pDrawTool = pDrawTool; 

	// load resource
	if ( m_pDrawTool ) {
		map_id2district_type::iterator itDistrictObj, iendDistrictObj;
		hm_districtobj* pDistrictObj = NULL;

		iendDistrictObj = m_mapId2District.end();
		for ( itDistrictObj = m_mapId2District.begin(); itDistrictObj!=iendDistrictObj; ++itDistrictObj ) {
			pDistrictObj = itDistrictObj->second;
			pDistrictObj->loadRes( m_pDrawTool );
		}
	}

};
//
bool hm_wmp_display::_setDistrictObj( district_id_type idDistrict, hm_districtobj* pDistrictObj ) {
	if ( !pDistrictObj ) return false;
	map_id2district_type::iterator itDistrictObj;

	//
	itDistrictObj = m_mapId2District.find( idDistrict );
	if ( itDistrictObj != m_mapId2District.end() ) {
		hm_districtobj* pDistrictObjOld = itDistrictObj->second;

		if ( m_pDrawTool )
			pDistrictObjOld->unloadRes( m_pDrawTool );
		// replaced district's uires die with it: the subject destructor
		// drops our subscriptions, nothing to clean by hand
		delete pDistrictObjOld;
		pDistrictObjOld = NULL;
		m_mapId2District.erase( itDistrictObj );
	}
	//
	m_mapId2District[ idDistrict ] = pDistrictObj;

	// remember the owner ( for showobjs pushed later ) and subscribe to
	// the text uires already inside
	pDistrictObj->m_pOwnerWMP = this;
	for ( unsigned int i = 0; i < pDistrictObj->getShowObjCount(); ++i )
		registerShowObjEvents( pDistrictObj->getShowObj( i ) );

	if ( m_pDrawTool )
		pDistrictObj->loadRes( m_pDrawTool );

	return true;
}
//
bool hm_wmp_display::getDistrictObj( district_id_type idDistrict, hm_districtobj** ppDistrictObj ) {
	return _getDistrictObj( idDistrict, ppDistrictObj );
}
//
bool hm_wmp_display::_hasDistrictObj( district_id_type idDistrict ) {
	map_id2district_type::iterator itDistrictObj;
	
	itDistrictObj = m_mapId2District.find( idDistrict );

	return itDistrictObj!=m_mapId2District.end();
}
//
bool hm_wmp_display::_getDistrictObj( district_id_type idDistrict, hm_districtobj** ppDistrictObj ) {
	map_id2district_type::iterator itDistrictObj;

	itDistrictObj = m_mapId2District.find( idDistrict );
	if ( itDistrictObj==m_mapId2District.end() ) return false;
	
	*ppDistrictObj = itDistrictObj->second;
	return true;
}
//
bool hm_wmp_display::_clearAllDistrictObjs() {
	map_id2district_type::iterator itDistrictObj, iendDistrictObj;

	iendDistrictObj = m_mapId2District.end();
	for ( itDistrictObj = m_mapId2District.begin(); itDistrictObj!=iendDistrictObj; ++itDistrictObj ) {
		delete itDistrictObj->second;
		itDistrictObj->second = NULL;
	}
	m_mapId2District.clear(); // 
	return true;
}
//
void hm_wmp_display::getAllDistrictId( district_id_container_type& containerIds ) {
	_getAllDistrictId( containerIds );
}
////////////////////////////////////////
hm_showobj::hm_showobj( hm_rs_uires_i* pUIRes, hm_pos_decoder_i* pPosDecoder )
: m_pUIRes( pUIRes )
, m_pPosDecoder( pPosDecoder )
{}
//
hm_showobj::~hm_showobj() {
	if ( m_pUIRes ) { 
		delete m_pUIRes;
		m_pUIRes = NULL;
	}
	if ( m_pPosDecoder ) {
		delete m_pPosDecoder;
		m_pPosDecoder = NULL;
	}
}
//
void hm_showobj::show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool ) {
	RECT rcShowObj;
	
	if ( !m_pUIRes ) return;
	if ( m_pPosDecoder ) {
		if ( !m_pPosDecoder->getPosition( rcCanvas, rcShowObj ) ) return;
	} else {
		rcShowObj = rcCanvas;
	}
	// 
	m_pUIRes->show( hDC, rcShowObj, pDrawTool );
}
//
bool hm_showobj::loadRes( hm_ps_drawtool_i* pDrawTool ) {
	if ( !m_pUIRes || !pDrawTool ) return false;
	
	m_pUIRes->loadRawRes( pDrawTool );
	return true;
}
//
bool hm_showobj::unloadRes( hm_ps_drawtool_i* pDrawTool ) {
	if ( !m_pUIRes || !pDrawTool ) return false;
	
	m_pUIRes->unloadRawRes( pDrawTool );
	return true;	
}
////////////////////////////////////////
//
hm_districtobj::hm_districtobj( hm_pos_decoder_i* pPosDecoder ) {
	m_pPosDecoder = pPosDecoder;
	m_pOwnerWMP = NULL;
};
//
hm_districtobj::~hm_districtobj() { 
	_clearAllShowObj(); 
	//
	if ( m_pPosDecoder ) {
		delete m_pPosDecoder;
		m_pPosDecoder = NULL;
	}

};
//
bool hm_districtobj::pushBackShowObj( hm_showobj* pShowObj ) {
	if ( !pShowObj ) return false;
	m_containerShowObjs.push_back( pShowObj );
	// late push into an installed district: join the owner's subscriptions
	if ( m_pOwnerWMP )
		m_pOwnerWMP->registerShowObjEvents( pShowObj );
	return true;
}
//
bool hm_districtobj::pushFrontShowObj( hm_showobj* pShowObj ) {
	if ( !pShowObj ) return false;
	m_containerShowObjs.push_front( pShowObj );
	// late push into an installed district: join the owner's subscriptions
	if ( m_pOwnerWMP )
		m_pOwnerWMP->registerShowObjEvents( pShowObj );
	return true;
}
//
void hm_districtobj::_clearAllShowObj() {
	showobj_container_type::iterator itShowObj, iendShowObj;
	hm_showobj* pShowObj = NULL;

	iendShowObj = m_containerShowObjs.end();
	for ( itShowObj = m_containerShowObjs.begin(); itShowObj!=iendShowObj; ++itShowObj ) {
		pShowObj = (*itShowObj);
		delete pShowObj;
	}
	m_containerShowObjs.clear();
}
//
void hm_districtobj::show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool, void* pContext ) {
	RECT rcDistrict;
	showobj_container_type::iterator itShowObj, iendShowObj;
	hm_showobj* pShowObj = NULL;

	// get rcDistrict
	if ( m_pPosDecoder ) {
		m_pPosDecoder->getPosition( rcCanvas, rcDistrict );
	} else {
		rcDistrict = rcCanvas;
	}
	
	// show showobj
	iendShowObj = m_containerShowObjs.end();
	for ( itShowObj = m_containerShowObjs.begin(); itShowObj!=iendShowObj; ++itShowObj ) {
		pShowObj = *itShowObj;
		if ( !pShowObj ) continue;
		pShowObj->show( hDC, rcDistrict, pDrawTool );
	}

}
//
void hm_districtobj::getDistrictRect( const RECT& rcCanvas, RECT& rcDistrictObj ) {
	if ( m_pPosDecoder ) {
		m_pPosDecoder->getPosition( rcCanvas, rcDistrictObj );
	} else {
		rcDistrictObj = rcCanvas;
	}
}
//
bool hm_districtobj::loadRes( hm_ps_drawtool_i* pDrawTool ) {
	if ( !pDrawTool ) return false;
	showobj_container_type::iterator itShowObj, iendShowObj;
	hm_showobj* pShowObj = NULL;

	iendShowObj = m_containerShowObjs.end();
	for ( itShowObj = m_containerShowObjs.begin(); itShowObj!=iendShowObj; ++itShowObj ) {
		pShowObj = *itShowObj;
		if ( !pShowObj ) continue;
		pShowObj->loadRes( pDrawTool );
	}
	return true;
}
//
bool hm_districtobj::unloadRes( hm_ps_drawtool_i* pDrawTool ) {
	if ( !pDrawTool ) return false;
	showobj_container_type::iterator itShowObj, iendShowObj;
	hm_showobj* pShowObj = NULL;
	
	iendShowObj = m_containerShowObjs.end();
	for ( itShowObj = m_containerShowObjs.begin(); itShowObj!=iendShowObj; ++itShowObj ) {
		pShowObj = *itShowObj;
		if ( !pShowObj ) continue;
		pShowObj->unloadRes( pDrawTool );
	}
	return true;
}

hm_showobj* hm_districtobj::getShowObj( unsigned int uIndex ) {
	if ( uIndex >= m_containerShowObjs.size() ) {
		return NULL;
	}
	return m_containerShowObjs[ uIndex ];
} 
