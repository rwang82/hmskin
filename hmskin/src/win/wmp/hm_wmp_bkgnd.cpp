#include "stdafx.h"
#include "win\wmp\hm_wmp_bkgnd.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_layout_obj.h"
#include "win\hm_event_defs.h"

hm_wmp_bkgnd::hm_wmp_bkgnd( hmskin_event_listener_base* pEventListener )
: hm_wmp_display( pEventListener ){
	_createDistrictObjs();
}
//
hm_wmp_bkgnd::~hm_wmp_bkgnd(){}
//
MSGBIND_BEGIN( hm_wmp_bkgnd )
MSGBIND_END()
//
ORDERWNDMSG_BEGIN( hm_wmp_bkgnd )
ORDERWNDMSG_END()
//
void hm_wmp_bkgnd::_createDistrictObjs() {
	hm_districtobj* pDistrictObj = NULL;

	pDistrictObj = new hm_districtobj();
	_setDistrictObj( EDISTRICTID_BK, pDistrictObj );
}
//
void hm_wmp_bkgnd::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue )
{
	switch ( idEvent ) {
		case HMKEELOBJEVENT_DRAWSELF:  
			{ 
				HEDSPARAM* pHEDSParam = NULL;
				RECT rcCanvas;

				pHEDSParam = (HEDSPARAM*)pEventParam;
				if ( !pHEDSParam->m_pLayoutObjCanvas->mapPosition( pHEDSParam->m_hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcCanvas ) )
					return;
				
				_doDrawSkin( pHEDSParam->m_hDCCanvas, rcCanvas );
			}
			break;
		default:
			break;
	}
}
//
void hm_wmp_bkgnd::_doDrawSkin( HDC hDC, const RECT& rcCanvas )
{
	// both paint ( memory DC of the root composite ) and interaction draws
	// funnel through the strategy dispatcher: BUF_NONE renders direct (
	// byte-identical to the old district->show call ), a declared buffer
	// strategy composes offscreen / serves the pixel cache instead
	_drawWithStrategy( hDC, rcCanvas );
}

void hm_wmp_bkgnd::_render( HDC hDC, const RECT& rcCanvas )
{
	hm_districtobj* pDistrictObj = NULL;

	if ( !_getDistrictObj( EDISTRICTID_BK, &pDistrictObj ) )
		return;
	pDistrictObj->show( hDC, rcCanvas, m_pDrawTool );
}

void hm_wmp_bkgnd::_getAllDistrictId( district_id_container_type& containerIds ) {
	containerIds.push_back( EDISTRICTID_BK );
}