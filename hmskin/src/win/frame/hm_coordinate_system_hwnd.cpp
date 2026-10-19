#include "stdafx.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "win\frame\hm_uiroot.h"
#include "stdio.h"
////////////////////////////////////////////////////////////////
coordinate_system_hwnd::coordinate_system_hwnd( ENUMCSHTYPE eType )
: m_hWnd( NULL )
, m_eType( eType )
{}
//
bool coordinate_system_hwnd::isSameSystem( const coordinate_system_hwnd& objCoordSysHwnd ) const {
	return ( m_hWnd == objCoordSysHwnd.m_hWnd ) && ( m_eType == objCoordSysHwnd.m_eType );
}
////////////////////////////////////////////////////////////////
IMPLEMENT_FACTORY( coordinate_system_hwnd )
////////////////////////////////////////////////////////////////
bool ConnectHwnd2CoordSys( hm_uiroot* pHMUIRoot, HWND hWnd, const hm_rs_wndmark& wndMark )
{
	if ( !pHMUIRoot )
		return false;
	hm_wmark_csid_table_type::obj_id_container_type containerObjId;
	hm_wmark_csid_table_type::obj_id_container_type::iterator itObjId, iendObjId;
	coordinate_system_hwnd* pCoordSysWin = NULL;
	
	//
	if ( !pHMUIRoot->m_pWndMark2CSIdTable->getObjIdContainer( wndMark, &containerObjId ) ) {
		return false;
	}
	//
	iendObjId = containerObjId.end();
	for ( itObjId = containerObjId.begin(); itObjId!=iendObjId; ++itObjId )
	{
		pCoordSysWin = pHMUIRoot->m_pCoordSysFactory->getObj( *itObjId );
		if ( !pCoordSysWin )
			continue;
		pCoordSysWin->setHWnd( hWnd );
	}

	return true;
}
//
bool DisconnectHwnd2CoordSys( hm_uiroot* pHMUIRoot, HWND hWnd, const hm_rs_wndmark& wndMark ) 
{
	if ( !pHMUIRoot )
		return false;
	hm_wmark_csid_table_type::obj_id_container_type containerCSId;
	hm_wmark_csid_table_type::obj_id_container_type::iterator itCSId, iendCSId;
	coordinate_system_hwnd* pCoordSysHWnd = NULL;
	
	if ( !pHMUIRoot->m_pWndMark2CSIdTable->getObjIdContainer( wndMark, &containerCSId ) )
		return false;
	//
	iendCSId = 	containerCSId.end();
	for ( itCSId = containerCSId.begin(); itCSId!=iendCSId; ++itCSId )
	{
		pCoordSysHWnd = pHMUIRoot->m_pCoordSysFactory->getObj( *itCSId );
		if ( !pCoordSysHWnd )
			continue;
		pCoordSysHWnd->setHWnd( NULL );
	}

	return true;
}

//
bool hm_csid2wndmark_bind( hm_uiroot* pHMUIRoot, unsigned int csId, const hm_rs_wndmark& wndMark ) {
	if ( !pHMUIRoot )
		return false;
	return pHMUIRoot->m_pWndMark2CSIdTable->bindObjId2WndMark( csId, wndMark );
}