#include "stdafx.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_pos_decoders.h"
#include "win\frame\hm_layout_obj.h"
#include "win\hm_event_defs.h"


//////////////////////////////////////////////////////////
hm_wmp_scrollbar::hm_wmp_scrollbar( ENUMDIRECTIONTYPE eDirectType, hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener )
, m_eDirectType( eDirectType )
, m_pCurStateObj( NULL )
, m_pStateObjDrag( NULL )
, m_pStateObjNonDrag( NULL )
, m_uMouseStatusFlags( 0 )
, m_eCursorPos( ECP_OUTSIDE )
, m_nDistanceLBtnDown( 0 )
, m_nValMin( -10 )
, m_nValMax( 90 )
, m_nValCur( 40 )
, m_pHEDSParamCache( NULL )
{
	m_pHEDSParamCache = new HEDSPARAM();
	//
	memset( &m_ptTrackThumbLBtnDown, 0, sizeof(POINT) );
	//
	switch ( eDirectType ) {
	case L2R_TYPE: 
		{
			m_pPosDecoderStartThumb = new hm_posdecoder_dock( hm_posdecoder_dock::DOCK_LEFT );
			m_pPosDecoderEndThumb = new hm_posdecoder_dock( hm_posdecoder_dock::DOCK_RIGHT );
			m_pPosDecoderTrackThumb = new hm_posdecoder_slider( hm_posdecoder_slider::SLIDER_L2R );
		}
		break;
	case T2B_TYPE:
		{
			m_pPosDecoderStartThumb = new hm_posdecoder_dock( hm_posdecoder_dock::DOCK_TOP );
			m_pPosDecoderEndThumb = new hm_posdecoder_dock( hm_posdecoder_dock::DOCK_BOTTOM );
			m_pPosDecoderTrackThumb = new hm_posdecoder_slider( hm_posdecoder_slider::SLIDER_T2B );
		}
		break;
	default:
		break;
	}
	//
	_createDistrictObjs();
	//
	_initStateObj();
}

hm_wmp_scrollbar::~hm_wmp_scrollbar() {
	//
	_uninitStateObj();
	//
	_uninitDistricts();
	//
	delete m_pPosDecoderStartThumb;
	m_pPosDecoderStartThumb = NULL;
	//
	delete m_pPosDecoderEndThumb;
	m_pPosDecoderEndThumb = NULL;
	//
	delete m_pPosDecoderTrackThumb;
	m_pPosDecoderTrackThumb = NULL;
	//
	if ( m_pHEDSParamCache )
		delete m_pHEDSParamCache;
	m_pHEDSParamCache = NULL;
}
//
MSGBIND_BEGIN( hm_wmp_scrollbar )
	MSGBIND( WM_LBUTTONDOWN, _OnLButtonDown )
	MSGBIND( WM_LBUTTONDBLCLK, _OnLButtonDblClk )
	MSGBIND( WM_LBUTTONUP, _OnLButtonUp )
	MSGBIND( WM_RBUTTONDOWN, _OnRButtonDown )
	MSGBIND( WM_RBUTTONUP, _OnRButtonUp )
	MSGBIND( WM_MOUSEMOVE, _OnMouseMove )
	MSGBIND( WM_MOUSELEAVE, _OnMouseLeave )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_scrollbar )
	ORDERWNDMSG( WM_LBUTTONDOWN )
	ORDERWNDMSG( WM_LBUTTONDBLCLK )
	ORDERWNDMSG( WM_LBUTTONUP )
	ORDERWNDMSG( WM_RBUTTONDOWN )
	ORDERWNDMSG( WM_RBUTTONUP )
	ORDERWNDMSG( WM_MOUSEMOVE )
	ORDERWNDMSG( WM_MOUSELEAVE )
ORDERWNDMSG_END()
//
void hm_wmp_scrollbar::_createDistrictObjs() {
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_BKGND, new hm_districtobj() );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_NOR, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_HOV, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_PRE, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_NOR, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_HOV, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_PRE, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_NOR, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_HOV, new hm_districtobj(  ) );
	_setDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_PRE, new hm_districtobj(  ) );
}
//
void hm_wmp_scrollbar::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue ) {
	switch ( idEvent ) {
	case HMKEELOBJEVENT_DRAWSELF:
		{
			HEDSPARAM* pParam = NULL;
			RECT rcCanvas;
			POINT ptCursorS2L;
			
			pParam = (HEDSPARAM*)pEventParam;
			if ( !pParam->m_pLayoutObjCanvas->mapPosition( pParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
				return;
			//
			if ( !::GetCursorPos( &ptCursorS2L ) )
				return;
			//
			if ( !pParam->m_pLayoutObjCanvas->screen2LayoutObj( ptCursorS2L ) )
				return;
			
			// save HEDSPARAM to cache
			if ( m_pHEDSParamCache != pParam ) {
				memcpy( m_pHEDSParamCache, pParam, sizeof(HEDSPARAM) );
			}
			
			if ( m_pCurStateObj ) {
				//TCHAR szMsg[ 100 ];
				//_stprintf_s( szMsg, _T("m_nValCur:%d\n"), m_nValCur );
				//::OutputDebugString( szMsg );
				m_pCurStateObj->_OnDrawSkinRaw( this, pParam->m_hDCCanvas, rcCanvas, _getCursorPos( rcCanvas, ptCursorS2L ) );
			}
		}
		break;
	default:
		break;
	}
}

void hm_wmp_scrollbar::drawSelf() {
	if ( !m_pHEDSParamCache ) return;
	BOOL bContinue = false;
	RECT rcCanvas;
	POINT ptCursorS2L;
	
	if ( !m_pHEDSParamCache->m_pLayoutObjCanvas->mapPosition( m_pHEDSParamCache->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
		return;
	if ( !::GetCursorPos( &ptCursorS2L ) )
		return;
	//
	if ( !m_pHEDSParamCache->m_pLayoutObjCanvas->screen2LayoutObj( ptCursorS2L ) )
		return;
	
	m_pHEDSParamCache->m_hDCCanvas = ::GetDC( m_pHEDSParamCache->m_hWndCanvas );
	
	m_pCurStateObj->_doDrawSkin( this, m_pHEDSParamCache->m_hDCCanvas, rcCanvas, _getCursorPos( rcCanvas, ptCursorS2L ) );

	//update( NULL, HMKEELOBJEVENT_DRAWSELF, (long)m_pHEDSParamCache, 0, &bContinue );

	::ReleaseDC( m_pHEDSParamCache->m_hWndCanvas, m_pHEDSParamCache->m_hDCCanvas );
}

void hm_wmp_scrollbar::_uninitDistricts() {
	hm_districtobj* pDistrictObj = NULL;

	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_BKGND, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_NOR, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_HOV, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_START_PRE, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_NOR, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_HOV, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_END_PRE, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_NOR, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_HOV, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
	if ( _getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_PRE, &pDistrictObj ) ) {
		pDistrictObj->setPosDecoder( NULL );
	}
}

void hm_wmp_scrollbar::_initStateObj() {
	if ( !m_pStateObjDrag )
		m_pStateObjDrag = new scrollbar_state_dragtrackthumb();
	if ( !m_pStateObjNonDrag )
		m_pStateObjNonDrag = new scrollbar_state_nondragtrackthumb();

	m_pCurStateObj = m_pStateObjNonDrag;
}

void hm_wmp_scrollbar::_uninitStateObj() {
	m_pCurStateObj = NULL;

	if ( m_pStateObjDrag ) {
		delete m_pStateObjDrag;
		m_pStateObjDrag = NULL;
	}
	if ( m_pStateObjNonDrag ) {
	    delete m_pStateObjNonDrag;
		m_pStateObjNonDrag = NULL;
	}
}

void hm_wmp_scrollbar::_switchState( ENUMSTATETYPE eStateType ) {
	switch ( eStateType )
	{
	case ESTATE_DRAG:
		{
			m_pCurStateObj = m_pStateObjDrag;
		}
		break;
	case ESTATE_NONDRAG:
		{
			m_pCurStateObj = m_pStateObjNonDrag;
		}
		break;
	case ESTATE_UNKNOWN:
	default:
		{
			m_pCurStateObj = NULL;
		}
		break;
	}
}

hm_wmp_scrollbar::ENUMSTATETYPE hm_wmp_scrollbar::_getCurState() {
	if ( !m_pCurStateObj )
		return ESTATE_UNKNOWN;
	return m_pCurStateObj->getEState();
}

bool hm_wmp_scrollbar::_isSameRect( const RECT& rcL, const RECT& rcR ) const {
	return ( ( rcL.left == rcR.left )
		&& ( rcL.right == rcR.right )
		&& ( rcL.top == rcR.top )
		&& ( rcL.bottom == rcR.bottom ) );
}

bool hm_wmp_scrollbar::_moveTrackThumb( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, const POINT& ptCursorClient ) {
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	RECT rcTrackThumbRefRect;
	POINT ptCursorCalc;
	int nDistanceTmp;
	int nMaxDistance;

	ptCursorCalc = ptCursorClient;
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return false;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );

	// calc ptCursorCalc
	if ( !_getTrackThumbRefRect( rcKeelObj, rcTrackThumbRefRect ) ) 
		return false;

	switch( m_eDirectType )
	{
	case L2R_TYPE:
		{
			nMaxDistance = rcTrackThumbRefRect.right - rcTrackThumbRefRect.left;
			nDistanceTmp = m_nDistanceLBtnDown + ( (__int16)ptCursorCalc.x - (int)m_ptTrackThumbLBtnDown.x );
		}
		break;
	case T2B_TYPE:
		{
			nMaxDistance = rcTrackThumbRefRect.bottom - rcTrackThumbRefRect.top;
			nDistanceTmp = m_nDistanceLBtnDown + ( (__int16)ptCursorCalc.y - (int)m_ptTrackThumbLBtnDown.y );
		}
		break;
	default:
		return false;
		break;
	}
	if ( nMaxDistance == 0 ) return false;
	
	//
	m_pPosDecoderTrackThumb->m_distance = nDistanceTmp < 0 ? 0 : ( ( nDistanceTmp > nMaxDistance ) ? nMaxDistance : nDistanceTmp );
 
	// adjust m_nValCur
	m_nValCur = m_nValMin + ( ( m_nValMax - m_nValMin ) * m_pPosDecoderTrackThumb->m_distance / nMaxDistance );
	TCHAR szMsg[50];
	_stprintf_s( szMsg, _T("m_nValCur:%d\n"), m_nValCur );
	::OutputDebugString( szMsg );
	
	// send HM_EVENT_TRACKTHUMB_MOVED notify event.
	HETMPARAM* pHETMParam = new HETMPARAM();
	pHETMParam->m_nPosCur = ( m_nValCur - m_nValMin );
	pHETMParam->m_nPosMax = ( m_nValMax - m_nValMin );
	pHETMParam->m_pWMPScrollbar = this;
	Notify( HM_EVENT_TRACKTHUMB_MOVED, (LPARAM)pHETMParam );
	delete pHETMParam;
	pHETMParam = NULL;

	return true;
}

bool hm_wmp_scrollbar::_updateCurVal2TrackThumbPos( const RECT& rcKeelObj ) {
	RECT rcTrackThumbRefRect;
	
	if ( !_getTrackThumbRefRect( rcKeelObj, rcTrackThumbRefRect ) ) 
		return false;
	
	switch ( m_pPosDecoderTrackThumb->m_eSliderType ) {
	case hm_posdecoder_slider::SLIDER_L2R:
		{
			m_pPosDecoderTrackThumb->m_distance = ( rcTrackThumbRefRect.right - rcTrackThumbRefRect.left ) * ( m_nValCur - m_nValMin ) / ( m_nValMax - m_nValMin );
		} break;
	case hm_posdecoder_slider::SLIDER_T2B: 
		{
			m_pPosDecoderTrackThumb->m_distance = ( rcTrackThumbRefRect.bottom - rcTrackThumbRefRect.top ) * ( m_nValCur - m_nValMin ) / ( m_nValMax - m_nValMin );
		} break;
	default:
		return false;
	}

	return true;
}

bool hm_wmp_scrollbar::_getTrackThumbRefRect( const RECT& rcKeelObj, RECT& rcTrackThumbRefRect ) {
	RECT rcStartThumb;
	RECT rcEndThumb;
	int nThickness;

	// get rcStartThumb
	if ( m_pPosDecoderStartThumb ) {
		m_pPosDecoderStartThumb->getPosition( rcKeelObj, rcStartThumb );
	} else {
		rcStartThumb = rcKeelObj;
		if ( m_eDirectType == L2R_TYPE ) {
			rcStartThumb.right = rcKeelObj.left;
		} else if ( m_eDirectType == T2B_TYPE ) {
			rcStartThumb.bottom = rcKeelObj.top;
		} else
			return false;
	}

	// get rcEndThumb
	if ( m_pPosDecoderEndThumb ) {
		m_pPosDecoderEndThumb->getPosition( rcKeelObj, rcEndThumb );
	} else {
		rcEndThumb = rcKeelObj;
		if ( m_eDirectType == L2R_TYPE ) {
			rcEndThumb.left = rcKeelObj.right;
		} else if ( m_eDirectType == T2B_TYPE ) {
			rcEndThumb.top = rcKeelObj.bottom;
		} else
			return false;
	}
	
	// get track thumb thickness
	if ( !m_pPosDecoderTrackThumb )
		return false;
	nThickness = m_pPosDecoderTrackThumb->m_nThickness;
	
	// get rcTrackThumbRefRect
	rcTrackThumbRefRect = rcKeelObj;
	if  ( m_eDirectType == L2R_TYPE ) {
		rcTrackThumbRefRect.left = rcStartThumb.right + nThickness / 2;
		rcTrackThumbRefRect.right = rcEndThumb.left - ( nThickness - nThickness / 2 );
	} else if ( m_eDirectType == T2B_TYPE ) {
		rcTrackThumbRefRect.top = rcStartThumb.bottom + nThickness / 2;
		rcTrackThumbRefRect.bottom = rcEndThumb.top - ( nThickness - nThickness / 2 );
	} else
		return false;
	
	return true;
}

hm_wmp_scrollbar::ENUMCURSORPOS hm_wmp_scrollbar::_getCursorPos( const RECT& rcKeelObj, const POINT& curPos ) {
	RECT rcDest;
	RECT rcTrackThumbRefRect;
	
	// check start thumb
	if ( m_pPosDecoderStartThumb ) {
		if ( m_pPosDecoderStartThumb->getPosition( rcKeelObj, rcDest ) ) {
			if ( ::PtInRect( &rcDest, curPos ) )
				return ECP_STARTTHUMB;
		}
	}

	// check stop thumb
	if ( m_pPosDecoderEndThumb ) {
		if ( m_pPosDecoderEndThumb->getPosition( rcKeelObj, rcDest ) ) {
			if ( ::PtInRect( &rcDest, curPos ) )
				return ECP_ENDTHUMB;
		}
	}
	
	// check track thumb
	if ( m_pPosDecoderTrackThumb ) {
		if ( _getTrackThumbRefRect( rcKeelObj, rcTrackThumbRefRect ) ) {
			if ( m_pPosDecoderTrackThumb->getPosition( rcTrackThumbRefRect, rcDest ) ) {
				if ( ::PtInRect( &rcDest, curPos ) )
					return ECP_TRACKTHUMB;
			}
		}
	}
	
	// check ECP_STARTSPACE | ECP_ENDSPACE
	if ( ::PtInRect( &rcKeelObj, curPos ) ) {
		POINT ptTrackThumbCenter;

		// get track thumb center position
		if ( !m_pPosDecoderTrackThumb ) 
			return ECP_BKGND;
		if ( !m_pPosDecoderTrackThumb->getPosition( rcKeelObj, rcDest ) ) 
			return ECP_BKGND;
		ptTrackThumbCenter.x = ( rcDest.left + rcDest.right ) / 2;
		ptTrackThumbCenter.y = ( rcDest.top + rcDest.bottom ) / 2;

		switch ( m_eDirectType ) {
		case L2R_TYPE:
			{
				return ( curPos.x < ptTrackThumbCenter.x ) ? ECP_STARTSPACE : ECP_ENDSPACE;
			}
			break;
		case T2B_TYPE:
			{
				return ( curPos.y < ptTrackThumbCenter.y ) ? ECP_STARTSPACE : ECP_ENDSPACE;
			}
			break;
		default:
			break;
		}
		// need more code here to distinguish ECP_STARTSPACE and ECP_ENDSPACE.
		return ECP_BKGND;
	}

	//
	return ECP_OUTSIDE;
}

bool hm_wmp_scrollbar::setTrackThumbLen( unsigned int uLen ) {
	if ( !m_pPosDecoderTrackThumb ) return false;
	
	m_pPosDecoderTrackThumb->m_nThickness = uLen;
	return true;
}

bool hm_wmp_scrollbar::setValRange( int nValMin, int nValMax ) {
	if ( nValMax < nValMin ) return false;
	if ( !m_pPosDecoderTrackThumb ) return false;
	
	m_nValMin = nValMin;
	m_nValMax = nValMax;
	m_nValCur = ( m_nValCur < m_nValMin ) ? m_nValMin : ( ( m_nValCur > m_nValMax ) ? m_nValMax : m_nValCur );

	return true;
}

bool hm_wmp_scrollbar::setCurValue( int nValCur ) {
	if ( ( nValCur < m_nValMin ) || ( nValCur > m_nValMax ) ) return false;

	m_nValCur = nValCur;

	return true;
}

void hm_wmp_scrollbar::_OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( m_pCurStateObj )
		m_pCurStateObj->_OnLButtonDown( this, pKeelObj, pWP, pParamWndProc );

	if ( m_uMouseStatusFlags & SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN ) {
		m_pCurStateObj = m_pStateObjDrag;
	} else {
		m_pCurStateObj = m_pStateObjNonDrag;
	}
}

void hm_wmp_scrollbar::_OnLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( m_pCurStateObj )
		m_pCurStateObj->_OnLButtonDblClk( this, pKeelObj, pWP, pParamWndProc );
}

void hm_wmp_scrollbar::_OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( m_pCurStateObj )
		m_pCurStateObj->_OnLButtonUp( this, pKeelObj, pWP, pParamWndProc );

	if ( m_uMouseStatusFlags & SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN ) {
		m_pCurStateObj = m_pStateObjDrag;
	} else {
		m_pCurStateObj = m_pStateObjNonDrag;
	}
}

void hm_wmp_scrollbar::_OnRButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}

void hm_wmp_scrollbar::_OnRButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}

void hm_wmp_scrollbar::_OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	if ( m_pCurStateObj )
		m_pCurStateObj->_OnMouseMove( this, pKeelObj, pWP, pParamWndProc );
}

void hm_wmp_scrollbar::_OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
    if ( m_pCurStateObj )
		m_pCurStateObj->_OnMouseLeave( this, pKeelObj, pWP, pParamWndProc );
}

void hm_wmp_scrollbar::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_BKGND );
	containerIds.push_back( EDISTRICTID_THUMB_START_NOR );
	containerIds.push_back( EDISTRICTID_THUMB_START_HOV );
	containerIds.push_back( EDISTRICTID_THUMB_START_PRE );
	containerIds.push_back( EDISTRICTID_THUMB_END_NOR );
	containerIds.push_back( EDISTRICTID_THUMB_END_HOV );
	containerIds.push_back( EDISTRICTID_THUMB_END_PRE );
	containerIds.push_back( EDISTRICTID_THUMB_TRACK_NOR );
	containerIds.push_back( EDISTRICTID_THUMB_TRACK_HOV );
	containerIds.push_back( EDISTRICTID_THUMB_TRACK_PRE );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
scrollbar_state_nondragtrackthumb::scrollbar_state_nondragtrackthumb()
: m_hBitmapMemCanvasCache( NULL ) {
	memset( &m_rcCanvasMemCache, 0, sizeof( RECT ) );
}
scrollbar_state_nondragtrackthumb::~scrollbar_state_nondragtrackthumb() {
	//
	if ( m_hBitmapMemCanvasCache ) {
		DeleteObject( m_hBitmapMemCanvasCache );
		m_hBitmapMemCanvasCache = NULL;
	}
}

void scrollbar_state_nondragtrackthumb::_OnLButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	if ( !::PtInRect( &rcKeelObj, ptClient ) )
		return;
	//
	pParamWndProc->m_flagEatMsg = EATMSGFLAG_DISABLE_OLD_WNDPROC | EATMSGFLAG_DISABLE_OTHER_MSGSINK | EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
	pParamWndProc->m_bValidRes = true;
	pParamWndProc->m_lResult = 0;
	//
	eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
	switch ( eCursorPos ) {
	case hm_wmp_scrollbar::ECP_OUTSIDE:
		return;
	case hm_wmp_scrollbar::ECP_STARTTHUMB: 
		{
			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_STARTTHUMB_LBTNDOWN;
		}
		break;
	case hm_wmp_scrollbar::ECP_ENDTHUMB: 
		{
			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_ENDTHUMB_LBTNDOWN;
		}
		break;
	case hm_wmp_scrollbar::ECP_TRACKTHUMB:
		{
			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN;
			pScrollBarCtx->m_ptTrackThumbLBtnDown = ptClient;
			pScrollBarCtx->m_nDistanceLBtnDown = pScrollBarCtx->m_pPosDecoderTrackThumb->m_distance;
			// switch state to drag.
			//pScrollBarCtx->_switchState( hm_wmp_scrollbar::ESTATE_DRAG );
		}
		break;
	case hm_wmp_scrollbar::ECP_STARTSPACE:
		{
			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_STARTSPACE_LBTNDOWN;
		}
		break;
	case hm_wmp_scrollbar::ECP_ENDSPACE:
		{
			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_ENDSPACE_LBTNDOWN;
		}
		break;
	case hm_wmp_scrollbar::ECP_BKGND:
	default:
		{
			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_BKGND_LBTNDOWN;
		}
		break;
	}
	//
	if ( ::GetCapture() != pWP->hwnd )
		::SetCapture( pWP->hwnd );
	
	//
	hDC = ::GetDC( pWP->hwnd );
	_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
	::ReleaseDC( pWP->hwnd, hDC );
}

void scrollbar_state_nondragtrackthumb::_OnLButtonDblClk( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	_OnLButtonDown( pScrollBarCtx, pKeelObj, pWP, pParamWndProc );
}

void scrollbar_state_nondragtrackthumb::_OnLButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {

	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	BOOL bLBtnDownOld;
	hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	bLBtnDownOld = ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_LBTNDOWN );
	if ( bLBtnDownOld ) {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof( TRACKMOUSEEVENT );
		tme.dwFlags = TME_LEAVE|TME_CANCEL;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = pWP->hwnd;

		::TrackMouseEvent( &tme );
		//
		::ReleaseCapture();
	}
	pScrollBarCtx->m_uMouseStatusFlags &= ~SCRBAR_FLAG_LBTNDOWN; // cancel left button down flag.
	
	if ( ::PtInRect( &rcKeelObj, ptClient ) ) {

		eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
		hDC = ::GetDC( pWP->hwnd );
		_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
		::ReleaseDC( pWP->hwnd, hDC );
	} else if ( bLBtnDownOld ) {
		eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
		hDC = ::GetDC( pWP->hwnd );
		_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
		::ReleaseDC( pWP->hwnd, hDC );
	}

	// send message to outside
	if ( bLBtnDownOld && ::PtInRect( &rcKeelObj, ptClient ) ) {
		// need more code here.

		//notify parent
	}
}

void scrollbar_state_nondragtrackthumb::_OnRButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {

}

void scrollbar_state_nondragtrackthumb::_OnRButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {

}

void scrollbar_state_nondragtrackthumb::_OnMouseMove( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	BOOL bMouse1stIn;
	hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	
	if ( !::PtInRect( &rcKeelObj, ptClient ) ) {
		if ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_TRACEMOUSELEAVE ) {
			// leave event.
			if ( !( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN ) ) {
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof( TRACKMOUSEEVENT );
				tme.dwFlags = TME_LEAVE|TME_CANCEL;
				tme.dwHoverTime = HOVER_DEFAULT;
				tme.hwndTrack = pWP->hwnd;

				::TrackMouseEvent( &tme );
			}
			//
			pScrollBarCtx->m_uMouseStatusFlags &= ~SCRBAR_FLAG_TRACEMOUSELEAVE;
			//
			eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
			hDC = ::GetDC( pWP->hwnd );
			_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
			::ReleaseDC( pWP->hwnd, hDC );
		}

		return;
	} else {
		// 
		bMouse1stIn = !( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_TRACEMOUSELEAVE );
		if ( bMouse1stIn ) {
			TRACKMOUSEEVENT tme;

			tme.cbSize = sizeof( TRACKMOUSEEVENT );
			tme.dwFlags = TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = pWP->hwnd;

			::TrackMouseEvent( &tme );

			pScrollBarCtx->m_uMouseStatusFlags |= SCRBAR_FLAG_TRACEMOUSELEAVE;
		}

		eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
		if ( eCursorPos != pScrollBarCtx->m_eCursorPos ) {
			hDC = ::GetDC( pWP->hwnd );
			_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
			::ReleaseDC( pWP->hwnd, hDC );
		}
	}
}

void scrollbar_state_nondragtrackthumb::_OnMouseLeave( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	::OutputDebugString( _T("enter OnMouseLeave..\n") );
}

void scrollbar_state_nondragtrackthumb::_OnDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) {
  _doDrawSkinRaw( pScrollBarCtx, hDC, rcCanvas, eCursorPos );
}

void scrollbar_state_nondragtrackthumb::_doDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) {
	::OutputDebugString(_T("  ------------  in scrollbar_state_nondragtrackthumb::_doDrawSkinRaw\n"));
	//COLORREF clrScrollBar = RGB( 160, 160, 80 );
	//
	//hm_util_draw_rect( hDC, rcCanvas, clrScrollBar );
	// m_uMouseStatusFlags 

	hm_districtobj* pDistrict = NULL;
	hm_wmp_display::district_id_type idDistrict;
	
	int nTickStart = 0;
	int nTickEnd = 0;
	TCHAR szMsg[ 1000 ];
	static int s_nDrawCount = 0;
	static int s_nDrawBKSum = 0;
	static int s_nDrawStartSum = 0;
	static int s_nDrawEndSum = 0;
	static int s_nDrawTrackSum = 0;

	++s_nDrawCount;

	// draw background
	nTickStart = ::GetTickCount();

	if ( pScrollBarCtx->_getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_BKGND, &pDistrict ) ) {
		pDistrict->show( hDC, rcCanvas, pScrollBarCtx->m_pDrawTool );
	}

	nTickEnd = ::GetTickCount();
	s_nDrawBKSum += (nTickEnd - nTickStart);
	_stprintf_s( szMsg, _T("bk: %d"), s_nDrawBKSum/s_nDrawCount );
	//::OutputDebugString( szMsg );
	
	// draw start thumb
	nTickStart = ::GetTickCount();

	if ( pScrollBarCtx->m_pPosDecoderStartThumb ) {
		//
		if ( eCursorPos == hm_wmp_scrollbar::ECP_STARTTHUMB )
			idDistrict = ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_STARTTHUMB_LBTNDOWN ) ? hm_wmp_scrollbar::EDISTRICTID_THUMB_START_PRE : hm_wmp_scrollbar::EDISTRICTID_THUMB_START_HOV;
		else
			idDistrict = hm_wmp_scrollbar::EDISTRICTID_THUMB_START_NOR;
		//
		if ( pScrollBarCtx->_getDistrictObj( idDistrict, &pDistrict ) ) {
			pDistrict->setPosDecoder( pScrollBarCtx->m_pPosDecoderStartThumb );
			pDistrict->show( hDC, rcCanvas, pScrollBarCtx->m_pDrawTool );
			pDistrict->setPosDecoder( NULL );			
		}
	}

	nTickEnd = ::GetTickCount();
	s_nDrawStartSum += (nTickEnd - nTickStart);
	_stprintf_s( szMsg, _T("start: %d"), s_nDrawStartSum/s_nDrawCount );
	//::OutputDebugString( szMsg );

	// draw end thumb
	nTickStart = ::GetTickCount();

	if ( pScrollBarCtx->m_pPosDecoderEndThumb ) {
		//
		if ( eCursorPos == hm_wmp_scrollbar::ECP_ENDTHUMB )
			idDistrict = ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_ENDTHUMB_LBTNDOWN ) ? hm_wmp_scrollbar::EDISTRICTID_THUMB_END_PRE : hm_wmp_scrollbar::EDISTRICTID_THUMB_END_HOV;
		else
			idDistrict = hm_wmp_scrollbar::EDISTRICTID_THUMB_END_NOR;
		//
		if ( pScrollBarCtx->_getDistrictObj( idDistrict, &pDistrict ) ) {
			pDistrict->setPosDecoder( pScrollBarCtx->m_pPosDecoderEndThumb );
			pDistrict->show( hDC, rcCanvas, pScrollBarCtx->m_pDrawTool );
			pDistrict->setPosDecoder( NULL );
		}
	}
	
	nTickEnd = ::GetTickCount();
	s_nDrawEndSum += (nTickEnd - nTickStart);
	_stprintf_s( szMsg, _T("end: %d"), s_nDrawEndSum/s_nDrawCount );
	//::OutputDebugString( szMsg );

	// draw track thumb
	nTickStart = ::GetTickCount();
	// update current value to trackthumb positon.
	pScrollBarCtx->_updateCurVal2TrackThumbPos( rcCanvas );
	
	if ( pScrollBarCtx->m_pPosDecoderTrackThumb ) {
		//
		if ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN ) 
			idDistrict = hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_PRE;
		else
			idDistrict = ( eCursorPos == hm_wmp_scrollbar::ECP_TRACKTHUMB ) ? hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_HOV : hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_NOR;
 
		//
		if ( pScrollBarCtx->_getDistrictObj( idDistrict, &pDistrict ) ) {
			RECT rcTrackThumbRefRect;

			if ( pScrollBarCtx->_getTrackThumbRefRect( rcCanvas, rcTrackThumbRefRect ) ) { // rcCanvas should be rcKeelObj.
				pDistrict->setPosDecoder( pScrollBarCtx->m_pPosDecoderTrackThumb );
				pDistrict->show( hDC, rcTrackThumbRefRect, pScrollBarCtx->m_pDrawTool );
				pDistrict->setPosDecoder( NULL );
			}
		}
	}

	nTickEnd = ::GetTickCount();
	s_nDrawTrackSum += (nTickEnd - nTickStart);
	_stprintf_s( szMsg, _T("track thumb: %d\n"), s_nDrawTrackSum/s_nDrawCount );
//	::OutputDebugString( szMsg );
	
	//save current cursor position.
	pScrollBarCtx->m_eCursorPos = eCursorPos;
}

void scrollbar_state_nondragtrackthumb::_doDrawSkin( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) {
	HDC hMemDC;
	HBITMAP hBitmapMem;
	HBITMAP hBitmapOld;
	RECT rcCanvasMem;
	
	rcCanvasMem.left = 0;
	rcCanvasMem.right = ( rcCanvas.right - rcCanvas.left );
	rcCanvasMem.top = 0;
	rcCanvasMem.bottom = ( rcCanvas.bottom - rcCanvas.top );
	if ( ( rcCanvasMem.right == 0 ) || ( rcCanvasMem.bottom == 0 ) )
		return;

	hMemDC = ::CreateCompatibleDC( hDC );
	if ( pScrollBarCtx->_isSameRect( rcCanvasMem, m_rcCanvasMemCache ) && m_hBitmapMemCanvasCache ) {
		hBitmapMem = m_hBitmapMemCanvasCache;
	} else {
		if ( m_hBitmapMemCanvasCache ) {
			DeleteObject( m_hBitmapMemCanvasCache );
			m_hBitmapMemCanvasCache = NULL;
		}
		hBitmapMem = ::CreateCompatibleBitmap( hDC, rcCanvasMem.right, rcCanvasMem.bottom );
		m_hBitmapMemCanvasCache = hBitmapMem;
		m_rcCanvasMemCache = rcCanvasMem;
	}

	hBitmapOld = (HBITMAP)::SelectObject( hMemDC, hBitmapMem );
	
	TCHAR szMsg[ 100 ];
	int nTickStart = ::GetTickCount();
	static int nDrawCount = 0;
	static int nTimeSum = 0;

	_doDrawSkinRaw( pScrollBarCtx, hMemDC, rcCanvasMem, eCursorPos );

	int nTickEnd = ::GetTickCount();
	nTimeSum += ( nTickEnd - nTickStart );
	_stprintf_s( szMsg, _T("%d _doDrawSkinRaw spend:%d s\n"), nDrawCount++, nTimeSum/nDrawCount );
//	::OutputDebugString( szMsg );

	//
	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, rcCanvasMem.right, rcCanvasMem.bottom, hMemDC, 0, 0, SRCCOPY );
	
	::SelectObject( hMemDC, hBitmapOld );
	//::DeleteObject( hBitmapMem );
	::DeleteDC( hMemDC );


}

///////////////////////////////////////////////////////////////////////////////////
scrollbar_state_dragtrackthumb::scrollbar_state_dragtrackthumb()
: m_hBitmapMemCanvasCache( NULL )
, m_hBitmapBK( NULL ) {
	memset( &m_rcCanvasMemCache, 0, sizeof( RECT ) );
	memset( &m_sizeBK, 0, sizeof( SIZE ) );
}
scrollbar_state_dragtrackthumb::~scrollbar_state_dragtrackthumb() {}
void scrollbar_state_dragtrackthumb::_OnLButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}
void scrollbar_state_dragtrackthumb::_OnLButtonDblClk( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}
void scrollbar_state_dragtrackthumb::_OnLButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	BOOL bLBtnDownOld;
	hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos;
	
	// get rcKeelObj.
	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	// get ptClient.
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
	//
	bLBtnDownOld = ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_LBTNDOWN );
	if ( bLBtnDownOld ) {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof( TRACKMOUSEEVENT );
		tme.dwFlags = TME_LEAVE|TME_CANCEL;
		tme.dwHoverTime = HOVER_DEFAULT;
		tme.hwndTrack = pWP->hwnd;

		::TrackMouseEvent( &tme );
		//
		::ReleaseCapture();
	}
	pScrollBarCtx->m_uMouseStatusFlags &= ~SCRBAR_FLAG_LBTNDOWN; // cancel left button down flag.
	
	if ( ::PtInRect( &rcKeelObj, ptClient ) ) {

		eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
		hDC = ::GetDC( pWP->hwnd );
		_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
		::ReleaseDC( pWP->hwnd, hDC );
	} else if ( bLBtnDownOld ) {

		eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
		hDC = ::GetDC( pWP->hwnd );
		_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
		::ReleaseDC( pWP->hwnd, hDC );
	}

	// send message to outside
	if ( bLBtnDownOld && ::PtInRect( &rcKeelObj, ptClient ) ) {
		// need more code here.

		//////notify parent
		//WPARAM wParam = MAKEWPARAM( GetDlgCtrlID( pWP->hwnd ), BN_CLICKED );
		//LPARAM lParam = (LPARAM)pWP->hwnd;
		//::SendMessage( ::GetParent( pWP->hwnd ), WM_COMMAND, wParam, lParam );
	}
}

void scrollbar_state_dragtrackthumb::_OnRButtonDown( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}
void scrollbar_state_dragtrackthumb::_OnRButtonUp( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}

void scrollbar_state_dragtrackthumb::_OnMouseMove( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {
	POINT ptClient;
	RECT rcKeelObj;
	hm_layout_obj* pLayoutObj = NULL;
	HDC hDC;
	hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos;

	pLayoutObj = pKeelObj->getLayoutObj();
	if ( !pLayoutObj ) return;
	pLayoutObj->mapPosition( pWP->hwnd, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcKeelObj );
	ptClient.x = LOWORD( pWP->lParam );
	ptClient.y = HIWORD( pWP->lParam );
  
	if ( !( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN ) )
		return; //no drag trackthumb flag.
	
	//
	pScrollBarCtx->_moveTrackThumb( pKeelObj, pWP, ptClient );

	eCursorPos = pScrollBarCtx->_getCursorPos( rcKeelObj, ptClient );
	hDC = ::GetDC( pWP->hwnd );

	_doDrawSkin( pScrollBarCtx, hDC, rcKeelObj, eCursorPos );
    
	::ReleaseDC( pWP->hwnd, hDC );
}

void scrollbar_state_dragtrackthumb::_OnMouseLeave( hm_wmp_scrollbar* pScrollBarCtx, hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc ) {}
void scrollbar_state_dragtrackthumb::_OnDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) {}
void scrollbar_state_dragtrackthumb::_doDrawSkinRaw( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) {
  	//COLORREF clrScrollBar = RGB( 160, 160, 80 );
	//
	//hm_util_draw_rect( hDC, rcCanvas, clrScrollBar );
	// m_uMouseStatusFlags 

	hm_districtobj* pDistrict = NULL;
	hm_wmp_display::district_id_type idDistrict;
	int nTickStart = 0;
	int nTickEnd = 0;
	TCHAR szMsg[ 1000 ];
	static int s_nDrawCount = 0;
	static int s_nDrawBKSum = 0;
	static int s_nDrawStartSum = 0;
	static int s_nDrawEndSum = 0;
	static int s_nDrawTrackSum = 0;

	++s_nDrawCount;

	if ( ( m_sizeBK.cx != ( rcCanvas.right - rcCanvas.left ) )
		|| ( m_sizeBK.cy != ( rcCanvas.bottom - rcCanvas.top ) ) 
		|| ( !m_hBitmapBK ) ) { // need to prepare m_hBitmapBK
		if ( m_hBitmapBK ) {
			::DeleteObject( m_hBitmapBK );
			m_hBitmapBK = NULL;
		}
		// draw background
		nTickStart = ::GetTickCount();

		if ( pScrollBarCtx->_getDistrictObj( hm_wmp_scrollbar::EDISTRICTID_BKGND, &pDistrict ) ) {
			pDistrict->show( hDC, rcCanvas, pScrollBarCtx->m_pDrawTool );
		}

		nTickEnd = ::GetTickCount();
		s_nDrawBKSum += (nTickEnd - nTickStart);
		_stprintf_s( szMsg, _T("bk: %d"), s_nDrawBKSum/s_nDrawCount );
		::OutputDebugString( szMsg );
		
		// draw start thumb
		nTickStart = ::GetTickCount();

		if ( pScrollBarCtx->m_pPosDecoderStartThumb ) {
			//
			if ( eCursorPos == hm_wmp_scrollbar::ECP_STARTTHUMB )
				idDistrict = ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_STARTTHUMB_LBTNDOWN ) ? hm_wmp_scrollbar::EDISTRICTID_THUMB_START_PRE : hm_wmp_scrollbar::EDISTRICTID_THUMB_START_HOV;
			else
				idDistrict = hm_wmp_scrollbar::EDISTRICTID_THUMB_START_NOR;
			//
			if ( pScrollBarCtx->_getDistrictObj( idDistrict, &pDistrict ) ) {
				pDistrict->setPosDecoder( pScrollBarCtx->m_pPosDecoderStartThumb );
				pDistrict->show( hDC, rcCanvas, pScrollBarCtx->m_pDrawTool );
				pDistrict->setPosDecoder( NULL );			
			}
		}

		nTickEnd = ::GetTickCount();
		s_nDrawStartSum += (nTickEnd - nTickStart);
		_stprintf_s( szMsg, _T("start: %d"), s_nDrawStartSum/s_nDrawCount );
		::OutputDebugString( szMsg );

		// draw end thumb
		nTickStart = ::GetTickCount();

		if ( pScrollBarCtx->m_pPosDecoderEndThumb ) {
			//
			if ( eCursorPos == hm_wmp_scrollbar::ECP_ENDTHUMB )
				idDistrict = ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_ENDTHUMB_LBTNDOWN ) ? hm_wmp_scrollbar::EDISTRICTID_THUMB_END_PRE : hm_wmp_scrollbar::EDISTRICTID_THUMB_END_HOV;
			else
				idDistrict = hm_wmp_scrollbar::EDISTRICTID_THUMB_END_NOR;
			//
			if ( pScrollBarCtx->_getDistrictObj( idDistrict, &pDistrict ) ) {
				pDistrict->setPosDecoder( pScrollBarCtx->m_pPosDecoderEndThumb );
				pDistrict->show( hDC, rcCanvas, pScrollBarCtx->m_pDrawTool );
				pDistrict->setPosDecoder( NULL );
			}
		}
		
		nTickEnd = ::GetTickCount();
		s_nDrawEndSum += (nTickEnd - nTickStart);
		_stprintf_s( szMsg, _T("end: %d"), s_nDrawEndSum/s_nDrawCount );
		::OutputDebugString( szMsg );

		// 
		HDC hMemDC = ::CreateCompatibleDC( hDC );
		HBITMAP hBitmapOld = NULL;
		m_hBitmapBK = ::CreateCompatibleBitmap( hDC, ( rcCanvas.right - rcCanvas.left ), ( rcCanvas.bottom - rcCanvas.top ) );
		hBitmapOld = ( HBITMAP )::SelectObject( hMemDC, m_hBitmapBK );
		
		::BitBlt( hMemDC, 0, 0, ( rcCanvas.right - rcCanvas.left ), ( rcCanvas.bottom - rcCanvas.top ), hDC, rcCanvas.left, rcCanvas.top, SRCCOPY );
		
		::SelectObject( hMemDC, hBitmapOld );
		::DeleteDC( hMemDC );
		 
	} else {
		HDC hMemDC = ::CreateCompatibleDC( hDC );
		HBITMAP hBitmapOld = NULL;
		hBitmapOld = ( HBITMAP )::SelectObject( hMemDC, m_hBitmapBK );

		::BitBlt( hDC, rcCanvas.left, rcCanvas.top, m_sizeBK.cx, m_sizeBK.cy, hMemDC, 0, 0, SRCCOPY );

		::SelectObject( hMemDC, hBitmapOld );
		::DeleteDC( hMemDC );
	}
	m_sizeBK.cx = rcCanvas.right - rcCanvas.left;
	m_sizeBK.cy = rcCanvas.bottom - rcCanvas.top;

	// draw track thumb
	nTickStart = ::GetTickCount();
	// calc track thumb position.
	//pScrollBarCtx->_updateCurVal2TrackThumbPos( rcCanvas ); // no need in this state.
	
	if ( pScrollBarCtx->m_pPosDecoderTrackThumb ) {

		//
		if ( pScrollBarCtx->m_uMouseStatusFlags & SCRBAR_FLAG_TRACKTHUMB_LBTNDOWN ) 
			idDistrict = hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_PRE;
		else
			idDistrict = ( eCursorPos == hm_wmp_scrollbar::ECP_TRACKTHUMB ) ? hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_HOV : hm_wmp_scrollbar::EDISTRICTID_THUMB_TRACK_NOR;
 
		//
		if ( pScrollBarCtx->_getDistrictObj( idDistrict, &pDistrict ) ) {
			RECT rcTrackThumbRefRect;

			if ( pScrollBarCtx->_getTrackThumbRefRect( rcCanvas, rcTrackThumbRefRect ) ) { // rcCanvas should be rcKeelObj.				
				pDistrict->setPosDecoder( pScrollBarCtx->m_pPosDecoderTrackThumb );
				pDistrict->show( hDC, rcTrackThumbRefRect, pScrollBarCtx->m_pDrawTool );
				pDistrict->setPosDecoder( NULL );
			}
		}
	}

	nTickEnd = ::GetTickCount();
	s_nDrawTrackSum += (nTickEnd - nTickStart);
	_stprintf_s( szMsg, _T("track thumb: %d\n"), s_nDrawTrackSum/s_nDrawCount );
	::OutputDebugString( szMsg );
	
	//save current cursor position.
	pScrollBarCtx->m_eCursorPos = eCursorPos;
}

void scrollbar_state_dragtrackthumb::_doDrawSkin( hm_wmp_scrollbar* pScrollBarCtx, HDC hDC, const RECT& rcCanvas, hm_wmp_scrollbar::ENUMCURSORPOS eCursorPos ) {
	HDC hMemDC;
	HBITMAP hBitmapMem;
	HBITMAP hBitmapOld;
	RECT rcCanvasMem;
	
	rcCanvasMem.left = 0;
	rcCanvasMem.right = ( rcCanvas.right - rcCanvas.left );
	rcCanvasMem.top = 0;
	rcCanvasMem.bottom = ( rcCanvas.bottom - rcCanvas.top );
	if ( ( rcCanvasMem.right == 0 ) || ( rcCanvasMem.bottom == 0 ) )
		return;

	hMemDC = ::CreateCompatibleDC( hDC );
	if ( pScrollBarCtx->_isSameRect( rcCanvasMem, m_rcCanvasMemCache ) && m_hBitmapMemCanvasCache ) {
		hBitmapMem = m_hBitmapMemCanvasCache;
	} else {
		if ( m_hBitmapMemCanvasCache ) {
			DeleteObject( m_hBitmapMemCanvasCache );
			m_hBitmapMemCanvasCache = NULL;
		}
		hBitmapMem = ::CreateCompatibleBitmap( hDC, rcCanvasMem.right, rcCanvasMem.bottom );
		m_hBitmapMemCanvasCache = hBitmapMem;
		m_rcCanvasMemCache = rcCanvasMem;
	}

	hBitmapOld = (HBITMAP)::SelectObject( hMemDC, hBitmapMem );
	
	TCHAR szMsg[ 100 ];
	int nTickStart = ::GetTickCount();
	static int nDrawCount = 0;
	static int nTimeSum = 0;

	_doDrawSkinRaw( pScrollBarCtx, hMemDC, rcCanvasMem, eCursorPos );

	int nTickEnd = ::GetTickCount();
	nTimeSum += ( nTickEnd - nTickStart );
	_stprintf_s( szMsg, _T("%d _doDrawSkinRaw spend:%d s\n"), nDrawCount++, nTimeSum/nDrawCount );
//	::OutputDebugString( szMsg );

	//
	::BitBlt( hDC, rcCanvas.left, rcCanvas.top, rcCanvasMem.right, rcCanvasMem.bottom, hMemDC, 0, 0, SRCCOPY );
	
	::SelectObject( hMemDC, hBitmapOld );
	//::DeleteObject( hBitmapMem );
	::DeleteDC( hMemDC );


}