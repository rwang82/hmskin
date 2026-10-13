#include "stdafx.h"
#include "common\hm_wndmsg_publisher.h"
#include "win\hm_event_defs.h"

//////////////////////////////////////////////////////////////////////////
static bool IsObsInContainer( hm_wndmsg_publisher::obs_ptr_type pObserver, hm_wndmsg_publisher::obs_ptr_container_type* pObsContainer );
//////////////////////////////////////////////////////////////////////////
//
hm_wndmsg_observer::hm_wndmsg_observer()
: m_pWndMsgSource( NULL ) {}
//
hm_wndmsg_observer::~hm_wndmsg_observer() {
	if ( m_pWndMsgSource )
		m_pWndMsgSource->UnRegObserver( this );
	m_pWndMsgSource = NULL;
}
//
void hm_wndmsg_observer::switchWndMsgSource( hm_wndmsg_publisher* pWndMsgSource )
{
	if ( m_pWndMsgSource == pWndMsgSource )
		return;
	wndmsg_container_type containerWndMsg;
	wndmsg_container_type::iterator itWndMsg, iendWndMsg;
	
	// cancel old relationship
	if ( m_pWndMsgSource ) {
		m_pWndMsgSource->UnRegObserver( this );
	} 
	
	//
	m_pWndMsgSource = pWndMsgSource;
	
	// create new relationship
	if ( m_pWndMsgSource ) {
		containerWndMsg.clear();
		pushbackWndMsgNeed( containerWndMsg );
		iendWndMsg = containerWndMsg.end();
		for ( itWndMsg = containerWndMsg.begin(); itWndMsg != iendWndMsg; ++itWndMsg ) {
			m_pWndMsgSource->orderWndMsg2Back( *itWndMsg, this );
		}
		m_pWndMsgSource->RegObserver( this );
	}
}
//////////////////////////////////////////////////////////////////////////
//
hm_wndmsg_publisher::hm_wndmsg_publisher()
{}
//
hm_wndmsg_publisher::~hm_wndmsg_publisher(){
	//
	::OutputDebugString( _T("enter ~hm_wndmsg_publisher\n") );
	cancelAllWndMsg();
	::OutputDebugString( _T("exit ~hm_wndmsg_publisher\n") );
}
//
BOOL hm_wndmsg_publisher::_hasObserver( hm_wndmsg_observer* pObserver ) {
	obs_ptr_container_type::iterator itObsvr, iendObsvr;

	iendObsvr = m_containerObsvr.end();
	for ( itObsvr = m_containerObsvr.begin(); itObsvr!=iendObsvr; ++itObsvr ) {
		if ( (*itObsvr) == pObserver )
			return TRUE;
	}
	return FALSE;
}
//
void hm_wndmsg_publisher::RegObserver( hm_wndmsg_observer* pObserver ) {
	obs_ptr_container_type::iterator itObsvr, iendObsvr;

	iendObsvr = m_containerObsvr.end();
	for ( itObsvr = m_containerObsvr.begin(); itObsvr!=iendObsvr; ++itObsvr ) {
		if ( *itObsvr == pObserver )
			return;
	}
	m_containerObsvr.push_back( pObserver );
}
//
void hm_wndmsg_publisher::notifyWndMsg( HWND hWnd, UINT message, WPARAM& wParam, LPARAM& lParam, WNDPROCPARAM* pParamWndProc ){
	if ( !pParamWndProc ) return;
	wndmsg2obss_map_type::iterator itObss;
	obs_ptr_container_type* pObsContainer = NULL;
	obs_ptr_container_type::iterator itObs, iendObs;
	CWPSTRUCT wmsgParam; 
	BOOL bContinue;

	itObss = m_mapWndMsg2Obsvrs.find( message );
	if ( itObss == m_mapWndMsg2Obsvrs.end() )
		return;
	pObsContainer = itObss->second;
	//
	wmsgParam.hwnd = hWnd;
	wmsgParam.message = message;
	wmsgParam.wParam = wParam;
	wmsgParam.lParam = lParam;
	pParamWndProc->m_bValidRes = false;
	pParamWndProc->m_lResult = 0;
	bContinue = true;
	iendObs = pObsContainer->end();
	for ( itObs = pObsContainer->begin(); itObs!=iendObs; ++itObs )
	{
		(*itObs)->update( this, HM_EVENT_WND_MSG, (LPARAM)&wmsgParam, (LPARAM)pParamWndProc, &bContinue );
		if ( !bContinue ) {
			pParamWndProc->m_flagEatMsg |= EATMSGFLAG_DISABLE_OTHER_MSGOBSRV;
		}
		if ( pParamWndProc->m_flagEatMsg & EATMSGFLAG_DISABLE_OTHER_MSGOBSRV )	{
			break;
		}
	}
	
	// param change can pass to next processor.
	wParam = wmsgParam.wParam;
	lParam = wmsgParam.lParam;
}
//
void hm_wndmsg_publisher::orderWndMsg2Front( UINT message, obs_ptr_type pObserver ){
	wndmsg2obss_map_type::iterator itObss;
	obs_ptr_container_type* pObsContainer = NULL;
	obs_ptr_container_type::iterator itObs, iendObs;
	
	//find pObsContainer
	itObss = m_mapWndMsg2Obsvrs.find( message );
	if ( itObss != m_mapWndMsg2Obsvrs.end() ) {
		pObsContainer = itObss->second;
	} else {
		pObsContainer = new obs_ptr_container_type();
		m_mapWndMsg2Obsvrs[ message ] = pObsContainer;
		//order new wndmsg from source
		_doOrderWndMsgFromSource( message );
	}
	//save pObserver
	if ( IsObsInContainer( pObserver, pObsContainer ) )
		return; //make sure every pObserver register one message only once.
	pObsContainer->push_front( pObserver );
}
//
void hm_wndmsg_publisher::orderWndMsg2Back( UINT message, obs_ptr_type pObserver ){
	wndmsg2obss_map_type::iterator itObss;
	obs_ptr_container_type* pObsContainer = NULL;
	obs_ptr_container_type::iterator itObs, iendObs;
	
	//find pObsContainer
	itObss = m_mapWndMsg2Obsvrs.find( message );
	if ( itObss != m_mapWndMsg2Obsvrs.end() ) {
		pObsContainer = itObss->second;
	} else {
		pObsContainer = new obs_ptr_container_type();
		m_mapWndMsg2Obsvrs[ message ] = pObsContainer;
		//order new wndmsg from source
		_doOrderWndMsgFromSource( message );
	}
	//save pObserver
	if ( IsObsInContainer( pObserver, pObsContainer ) )
		return; //make sure every pObserver register one message only once.
	pObsContainer->push_back( pObserver );
}
//
bool hm_wndmsg_publisher::hasOrderWndMsg( UINT message, obs_ptr_type pObserver ) const {
	if ( !pObserver ) return false;
	wndmsg2obss_map_type::const_iterator c_itObss;
	//
	c_itObss = m_mapWndMsg2Obsvrs.find( message );
	if ( c_itObss == m_mapWndMsg2Obsvrs.end() ) 
		return false;
	//
	return IsObsInContainer( pObserver, c_itObss->second );
}
//
void hm_wndmsg_publisher::cancelWndMsg( UINT message, obs_ptr_type pObserver ){
	if ( !pObserver ) return;
	wndmsg2obss_map_type::iterator itObss;
	obs_ptr_container_type* pObsContainer = NULL;
	obs_ptr_container_type::iterator itObs, iendObs;
	
	itObss = m_mapWndMsg2Obsvrs.find( message );
	if ( itObss == m_mapWndMsg2Obsvrs.end() )
		return;
	pObsContainer = itObss->second;
	//
	iendObs = pObsContainer->end();
	for ( itObs = pObsContainer->begin(); itObs!=iendObs; ++itObs )	{
		if ( *itObs == pObserver ) {
			pObsContainer->erase( itObs );
			break;
		}
	}
	//
	_checkCancelWndMsg();
}
//
void hm_wndmsg_publisher::UnRegObserver( obs_ptr_type pObserver ){
	if ( !pObserver ) return;
	wndmsg2obss_map_type::iterator itObss, iendObss;
	obs_ptr_container_type* pObsContainer = NULL;
	obs_ptr_container_type::iterator itObs, iendObs;
	
	iendObss = m_mapWndMsg2Obsvrs.end();
	for ( itObss = m_mapWndMsg2Obsvrs.begin(); itObss!=iendObss; ++itObss ) {
		pObsContainer = itObss->second;
		//
		iendObs = pObsContainer->end();
		for ( itObs = pObsContainer->begin(); itObs!=iendObs; ++itObs )	{
			if ( *itObs == pObserver ) {
				pObsContainer->erase( itObs );
				break;
			}
		}
	}
	//
	_checkCancelWndMsg();
	//
	iendObs = m_containerObsvr.end();
	for ( itObs = m_containerObsvr.begin(); itObs!=iendObs; ++itObs ) {
		if ( *itObs == pObserver ) {
			m_containerObsvr.erase( itObs );
			break;
		}
	}
}
//
void hm_wndmsg_publisher::cancelAllWndMsg(){
	// need more code here.
	_checkCancelWndMsg();
	//
	wndmsg2obss_map_type::iterator itObss,iendObss;
	obs_ptr_container_type* pObsContainer = NULL;
	obs_ptr_container_type::iterator itObs, iendObs;

	iendObss = m_mapWndMsg2Obsvrs.end();
	for ( itObss = m_mapWndMsg2Obsvrs.begin(); itObss!=iendObss; ++itObss )
	{
		pObsContainer = itObss->second;
		if ( !pObsContainer )
			continue;
		iendObs = pObsContainer->end();
		for ( itObs = pObsContainer->begin(); itObs!=iendObs; ++itObs ) {
			(*itObs)->clearWndMsgSource();
		}
		pObsContainer->clear();
		delete pObsContainer;
		pObsContainer = NULL;
		itObss->second = NULL;
	}
	//
	m_mapWndMsg2Obsvrs.clear();

	//
	iendObs = m_containerObsvr.end();
	for ( itObs = m_containerObsvr.begin(); itObs!=iendObs; ++itObs ) {
		(*itObs)->clearWndMsgSource();
	}
	m_containerObsvr.clear();
}

bool IsObsInContainer( hm_wndmsg_publisher::obs_ptr_type pObserver, hm_wndmsg_publisher::obs_ptr_container_type* pObsContainer )
{
	hm_wndmsg_publisher::obs_ptr_container_type::iterator itObs, iendObs;
	
	iendObs = pObsContainer->end();
	for ( itObs = pObsContainer->begin(); itObs!=iendObs; ++itObs )
	{
		if ( *itObs == pObserver )
			return true;
	}
	
	return false;
}

void hm_wndmsg_publisher::pushbackAllWndMsgOrdered( wndmsg_container_type& containerWndMsg ) const
{
	wndmsg2obss_map_type::const_iterator citObss, ciendObss;

	if ( m_mapWndMsg2Obsvrs.size() == 0 )
		return;
	//
	ciendObss = m_mapWndMsg2Obsvrs.end();
	for ( citObss = m_mapWndMsg2Obsvrs.begin(); citObss!=ciendObss; ++citObss ) {
		containerWndMsg.push_back( citObss->first );
	}
}

void hm_wndmsg_publisher::getWndMsgOrdered( obs_ptr_type pObserver, wndmsg_container_type& containerWndMsg ) const
{
	wndmsg2obss_map_type::const_iterator citObss, ciendObss;
	obs_ptr_container_type* pcontainerObs;
	obs_ptr_container_type::const_iterator citObs, ciendObs;

	containerWndMsg.clear();
	if ( m_mapWndMsg2Obsvrs.size() == 0 )
		return;
	//
	ciendObss = m_mapWndMsg2Obsvrs.end();
	for( citObss = m_mapWndMsg2Obsvrs.begin(); citObss!=ciendObss; ++citObss ) {
		pcontainerObs = citObss->second;
		ciendObs = pcontainerObs->end();
		for( citObs = pcontainerObs->begin(); citObs!=ciendObs; ++citObs ) {
			if ( (*citObs) == pObserver ) {
				containerWndMsg.push_back( citObss->first );
				break;
			}
		}
	}
}

//void hm_wndmsg_publisher::_checkCancelWndMsg() {
//	if ( m_mapWndMsg2Obsvrs.size() == 0 ) return;
//	wndmsg2obss_map_type::reverse_iterator ritObss, riendObss;
//
//	//
//	for ( ritObss = m_mapWndMsg2Obsvrs.rbegin(); ritObss!=m_mapWndMsg2Obsvrs.rend(); ++ritObss ) {
//		if ( ritObss->second->size() != 0 ) 
//			continue;
//		_doCancelWndMsgFromSource( ritObss->first );
//		delete ritObss->second;
//		ritObss->second = NULL;
//		m_mapWndMsg2Obsvrs.erase( ritObss->first );
//		if ( m_mapWndMsg2Obsvrs.size() == 0 )
//			return;
//	}
//}

void hm_wndmsg_publisher::_checkCancelWndMsg() {
	if ( m_mapWndMsg2Obsvrs.size() == 0 ) return;
	wndmsg2obss_map_type::iterator itObss, iendObss;
	wndmsg_container_type containerWndMsg;
	wndmsg_container_type::iterator itWndMsg, iendWndMsg;
	
	// get containerWndMsg.
	iendObss = m_mapWndMsg2Obsvrs.end();
	for ( itObss = m_mapWndMsg2Obsvrs.begin(); itObss!=iendObss; ++itObss ) {
		containerWndMsg.push_back( itObss->first );
	}
	//
	iendWndMsg = containerWndMsg.end();
	for ( itWndMsg = containerWndMsg.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
		itObss = m_mapWndMsg2Obsvrs.find( *itWndMsg );
		if ( itObss == m_mapWndMsg2Obsvrs.end() ) 
			continue;
		if ( itObss->second->size() != 0 )
			continue;

		_doCancelWndMsgFromSource( itObss->first );
		delete itObss->second;
		m_mapWndMsg2Obsvrs.erase( itObss->first );
	}

}