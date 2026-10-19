#include "stdafx.h"
#include "win\resource_system\hm_rs_wndmark.h"
#include <assert.h>
//
hm_rs_hwnddepository::hm_rs_hwnddepository() {

}

hm_rs_hwnddepository::~hm_rs_hwnddepository() {

}

BOOL hm_rs_hwnddepository::bindHWND2PropId( HWND hWnd, propid_type propId ) {
	if ( !hWnd || _isHWNDBind2PropId( hWnd ) || _isPropIdBind2HWND( propId ) ) {
		// one propId <-> one HWND: a reject usually means a stale entry
		// leaked ( release build: diagnose via DebugView ).
		::OutputDebugString( _T("hmskin: bindHWND2PropId rejected ( hwnd or propId already bound ).\n") );
		assert( FALSE );
		return FALSE;
	}
	m_mapPropId2HWND[ propId ] = hWnd;
	return TRUE;
}

BOOL hm_rs_hwnddepository::bindHWND2CtrlId( HWND hWnd, ctrlid_type ctrlId ) {
	if ( !hWnd || _isHWNDBind2CtrlId( hWnd ) || _isCtrlIdBind2HWND( ctrlId ) ) {
		return FALSE;
	}
	m_mapCtrlId2HWND[ ctrlId ] = hWnd;
	return TRUE;
}

void hm_rs_hwnddepository::unBindHWND( HWND hWnd ) {
	propid2hwnd_map_type::iterator itPropId2HWND, iendPropId2HWND;
	ctrlid2hwnd_map_type::iterator itCtrlId2HWND, iendCtrlId2HWND;
	BOOL bFound = FALSE;

	//
	iendPropId2HWND = m_mapPropId2HWND.end();
	for ( itPropId2HWND = m_mapPropId2HWND.begin(); itPropId2HWND!=iendPropId2HWND; ++itPropId2HWND ) {
		if ( itPropId2HWND->second == hWnd ) {
			m_mapPropId2HWND.erase( itPropId2HWND );
			bFound = TRUE;
			break;
		}
	}
	//
	iendCtrlId2HWND = m_mapCtrlId2HWND.end();
	for ( itCtrlId2HWND = m_mapCtrlId2HWND.begin(); itCtrlId2HWND!=iendCtrlId2HWND; ++itCtrlId2HWND ) {
		if ( itCtrlId2HWND->second == hWnd ) {
			m_mapCtrlId2HWND.erase( itCtrlId2HWND );
			bFound = TRUE;
			break;
		}
	}
	if ( !bFound )
		::OutputDebugString( _T("hmskin: unBindHWND found no binding ( double disconnect? ).\n") );
}

HWND hm_rs_hwnddepository::propId2HWND( propid_type propId ) {
	propid2hwnd_map_type::iterator itPropId2HWND;

	itPropId2HWND = m_mapPropId2HWND.find( propId );
	if ( itPropId2HWND == m_mapPropId2HWND.end() )
		return NULL;
	return itPropId2HWND->second;
}

HWND hm_rs_hwnddepository::ctrlId2HWND( ctrlid_type ctrlId ) {
	ctrlid2hwnd_map_type::iterator itCtrlId2HWND;

	itCtrlId2HWND = m_mapCtrlId2HWND.find( ctrlId );
	if ( itCtrlId2HWND == m_mapCtrlId2HWND.end() )
		return NULL;
	return itCtrlId2HWND->second;
}

BOOL hm_rs_hwnddepository::_isPropIdBind2HWND( propid_type propId ) {
	return m_mapPropId2HWND.find( propId ) != m_mapPropId2HWND.end();
}

BOOL hm_rs_hwnddepository::_isCtrlIdBind2HWND( ctrlid_type ctrlId ) {
	return m_mapCtrlId2HWND.find( ctrlId ) != m_mapCtrlId2HWND.end();
}

BOOL hm_rs_hwnddepository::_isHWNDBind2PropId( HWND hWnd ) {
	propid2hwnd_map_type::iterator itPropId2HWND, iendPropId2HWND;

	iendPropId2HWND = m_mapPropId2HWND.end();
	for ( itPropId2HWND = m_mapPropId2HWND.begin(); itPropId2HWND!=iendPropId2HWND; ++itPropId2HWND ) {
		if ( itPropId2HWND->second == hWnd )
			return TRUE;
	}
	//
	return FALSE;
}

BOOL hm_rs_hwnddepository::_isHWNDBind2CtrlId( HWND hWnd ) {
	ctrlid2hwnd_map_type::iterator itCtrlId2HWND, iendCtrlId2HWND;

	iendCtrlId2HWND = m_mapCtrlId2HWND.end();
	for ( itCtrlId2HWND = m_mapCtrlId2HWND.begin(); itCtrlId2HWND!=iendCtrlId2HWND; ++itCtrlId2HWND ) {
		if ( itCtrlId2HWND->second == hWnd )
			return TRUE;
	}
	//
	return FALSE;
}

BOOL hm_rs_hwnddepository::_isBindHWND( HWND hWnd ) {
	return _isHWNDBind2PropId( hWnd ) || _isHWNDBind2CtrlId( hWnd );
}

HWND hm_rs_hwnddepository::wndMark2HWND( const hm_rs_wndmark& wndMark ) {
	switch ( wndMark.m_eflag )
	{
	case hm_rs_wndmark::WNDMARK_WNDPROPID:
		{
			return propId2HWND( wndMark.m_uWndPropID );
		}
		break;
	case hm_rs_wndmark::WNDMARK_CTRLID:
		{
			return ctrlId2HWND( wndMark.m_uCtrlID );
		}
		break;
	default:
		assert( FALSE );
		break;
	}

	assert( FALSE );
	return NULL;
}

BOOL hm_rs_hwnddepository::bindHWND2WndMark( HWND hWnd, const hm_rs_wndmark& wndMark ) {
	switch ( wndMark.m_eflag )
	{
	case hm_rs_wndmark::WNDMARK_CTRLID:
		{
			return bindHWND2CtrlId( hWnd, wndMark.m_uCtrlID );
		}
		break;
	case hm_rs_wndmark::WNDMARK_WNDPROPID:
		{
			return bindHWND2PropId( hWnd, wndMark.m_uWndPropID );
		}
		break;
	default:
		assert( FALSE );
		break;
	}

	assert( FALSE );
	return FALSE;
}

