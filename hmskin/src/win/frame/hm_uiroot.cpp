#include "stdafx.h"
#include "win\frame\hm_uiroot.h"
#include "win\loader\hm_loader_wndstyle.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_keel_obj.h"
#include "win\resource_system\hm_rs_wndmark.h"
#include <assert.h>
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FACTORY( hm_uiroot )
//////////////////////////////////////////////////////////////////////////
hm_uiroot::hm_uiroot( hmskin_event_listener_base* pEventListener )
: m_pEventListener( pEventListener )
, m_pHWNDDespository( new hm_rs_hwnddepository() )
, m_pMsgSinkFactory( new msg_sink_factory_type() )
, m_pWndMark2CSIdTable( new hm_wmark_csid_table_type() )
, m_pCoordSysFactory( new hm_coordsys_factory_type() )
, m_pWndMark2MSIdTable( new hm_wmark_msid_table_type() )
, m_pKeelObjFactory( new hm_keel_obj_factory() )
, m_pWMPFactory( new hm_wmp_factory() )
, m_nWndCornerPref( HM_WNDCORNER_NOTSET )
{
	_Init();
}

hm_uiroot::~hm_uiroot() {
	if ( m_pWMPFactory )
		delete m_pWMPFactory;
	m_pWMPFactory = NULL;

	if ( m_pKeelObjFactory )
		delete m_pKeelObjFactory;
	m_pKeelObjFactory = NULL;

	if ( m_pWndMark2MSIdTable )
		delete m_pWndMark2MSIdTable;
	m_pWndMark2MSIdTable = NULL;

	if ( m_pCoordSysFactory )
		delete m_pCoordSysFactory;
	m_pCoordSysFactory = NULL;

	if ( m_pWndMark2CSIdTable )
		delete m_pWndMark2CSIdTable;
	m_pWndMark2CSIdTable = NULL;

	if ( m_pMsgSinkFactory )
		delete m_pMsgSinkFactory;
	m_pMsgSinkFactory= NULL;

	if ( m_pHWNDDespository )
		delete m_pHWNDDespository;
	m_pHWNDDespository = NULL;

	m_pEventListener = NULL;

}

void hm_uiroot::_Init() {
	assert( m_pEventListener );
}
