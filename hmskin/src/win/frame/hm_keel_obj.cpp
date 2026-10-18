#include "stdafx.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\resource_system\hm_rs_uires_i.h"
#include "win\paint_system\hm_ps_paint_layer_i.h"
#include "common\hm_cmn_envcfg.h"
#include "win\frame\hm_debug_utils.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\hm_layoutobj_defs.h"
#include "Imm.h"
#include <assert.h>
//////////////////////////////////////////////////////////////////////////
#define KEELOBJ_INVALID_ID (0xFFFFFFFF)
//////////////////////////////////////////////////////////////////////////
#define HMKEELOBJ_FLAG_NONE (0x00000000)
#define HMKEELOBJ_FLAG_VISIBLE (0x00000001)
#define HMKEELOBJ_FLAG_CLIPCHILDREN (0x00000002)
//////////////////////////////////////////////////////////////////////////
hm_keel_obj::hm_keel_obj()
: m_uFlag( HMKEELOBJ_FLAG_VISIBLE )
, m_pLayoutObj( NULL )
, m_pParentObj( NULL )
, m_uId( KEELOBJ_INVALID_ID )
{
	Register( this, HMKEELOBJEVENT_LAYOUTOBJ_CHANGED );
}
//
hm_keel_obj::~hm_keel_obj()
{
	//delete all layout obj
	_delAllLayoutObj();
	//
	UnRegister( this, HMKEELOBJEVENT_LAYOUTOBJ_CHANGED );
	// drop the state-changed subscriptions on all WMPs: WMPs live in the
	// factory and may outlive this keelobj - a stale observer pointer would
	// dangle at their next Notify.
	{
		wmp_container_type::iterator itWMP, iendWMP;
		iendWMP = m_containerWMP.end();
		for ( itWMP = m_containerWMP.begin(); itWMP!=iendWMP; ++itWMP ) {
			(*itWMP)->UnRegister( this, HM_EVENT_WMP_STATECHANGED );
		}
	}
}
// 
void hm_keel_obj::update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue )
{
	switch ( idEvent )
	{
	case HM_EVENT_WND_MSG:
		{
			CWPSTRUCT* pWP = (CWPSTRUCT*)pEventParam;
			msg_sink_i::WNDPROCPARAM* pParamWndProc = (msg_sink_i::WNDPROCPARAM*)pEventParam2;

			// hidden keel objects neither react to nor eat window messages.
			// ( e.g. controls hidden by tab-chapter switching used to redraw
			//   themselves on mouse hover and even stayed clickable. )
			{
				BOOL bVisibleChain = isVisible();
				for ( hm_keel_obj* pParent = getParent(); bVisibleChain && pParent; pParent = pParent->getParent() )
					bVisibleChain = pParent->isVisible();
				if ( !bVisibleChain )
					break;
			}

			// Notify
			notifyWndMsg( pWP->hwnd, pWP->message, pWP->wParam, pWP->lParam, pParamWndProc );

			if ( !pParamWndProc || !( pParamWndProc->m_flagEatMsg & EATMSGFLAG_DISABLE_SOURCE ) ) {
				_doWndMsg( pWP, pParamWndProc );
			}

			// check if need disable other msg ob
			if ( pParamWndProc
				&& ( EATMSGFLAG_DISABLE_OTHER_MSGOBSRV == ( pParamWndProc->m_flagEatMsg & EATMSGFLAG_DISABLE_OTHER_MSGOBSRV ) )
				&& pbContinue ) {
				*pbContinue = false;
			}
		}
		break;
	case HMKEELOBJEVENT_DRAWSUBOBJS:
		{
			HDC hDC = ( HDC )pEventParam;
			HWND hWndCanvas = (HWND)pEventParam2;

			doDrawAll( hDC, hWndCanvas );
		}
		break;
	case HMKEELOBJEVENT_LAYOUTOBJ_CHANGED:
		{
			hm_layout_obj* pLayoutObjNew = (hm_layout_obj*)pEventParam;
			hm_layout_obj* pLayoutObjOld = (hm_layout_obj*)pEventParam2;

			// unregister cmn::ID_EVENT_POSCHANGED from pLayoutObjOld
			if ( pLayoutObjOld )
				pLayoutObjOld->UnRegister( this, cmn::ID_EVENT_POSCHANGED );

			// register cmn::ID_EVENT_POSCHANGED from pLayoutObjNew
			if ( pLayoutObjNew )
				pLayoutObjNew->Register( this, cmn::ID_EVENT_POSCHANGED );
		}
		break;
	case cmn::ID_EVENT_POSCHANGED:
		{
			Notify( HMKEELOBJEVENT_POSCHANGED, (LPARAM)this );
		}
		break;
	case HM_EVENT_WMP_STATECHANGED:
		{
			// one of my WMPs changed state from outside the message flow
			// ( e.g. checkbox setCheck by radio-group logic ). the owner
			// holds the rect/canvas knowledge - invalidate it and let the
			// paint cycle redraw; the WMP never learns about keelobjs.
			hm_layout_obj* pLayoutObjSelf = getLayoutObj();
			const coordinate_system_hwnd* pCoordSysHwnd = pLayoutObjSelf
				? ( const coordinate_system_hwnd* )pLayoutObjSelf->getCoordinateSystem() : NULL;
			if ( pCoordSysHwnd && pCoordSysHwnd->getHWnd() ) {
				hm_util_invlog( _T("state-inv"), &pLayoutObjSelf->m_rect );
				::InvalidateRect( pCoordSysHwnd->getHWnd(), &pLayoutObjSelf->m_rect, FALSE );
			}
		}
		break;
	default:
		break;
	}
	
}
//
void hm_keel_obj::pushbackWndMsgNeed( wndmsg_container_type& containerWndMsgNeed ) {
	wndmsg_container_type containerWndMsgOrdered;
	wndmsg_container_type::iterator itWMOd, iendWMOd;
	wndmsg_container_type::iterator itWMNeed, iendWMNeed;
	wmp_container_type::iterator itWMP, iendWMP;
	hm_wndmsgprocessor_i* pWndMsgProcessor = NULL;
	
    // push all wndmsg ordered to need.
	if ( containerWndMsgNeed.size() == 0 ) {
		pushbackAllWndMsgOrdered( containerWndMsgNeed );
	} else {
		pushbackAllWndMsgOrdered( containerWndMsgOrdered );
		iendWMOd = containerWndMsgOrdered.end();
		for ( itWMOd = containerWndMsgOrdered.begin(); itWMOd!=iendWMOd; ++itWMOd ) {
			iendWMNeed = containerWndMsgNeed.end();
			for ( itWMNeed = containerWndMsgNeed.begin(); itWMNeed!=iendWMNeed; ++itWMNeed ) {
				if ( *itWMOd == *itWMNeed )
					break;
			}
			//
			if ( itWMNeed == iendWMNeed )
				containerWndMsgNeed.push_back( *itWMOd );
		}
	}

	// push all wndmsg need by window message processor.
	iendWMP = m_containerWMP.end();
	for ( itWMP = m_containerWMP.begin(); itWMP!=iendWMP; ++itWMP ) {
		pWndMsgProcessor = *itWMP;
		if ( pWndMsgProcessor ) {
			hm_wndmsgprocessor_i::msg_container_type containerWndMsg;
			hm_wndmsgprocessor_i::msg_container_type::iterator itWndMsg, iendWndMsg;
						
			if ( containerWndMsgNeed.size() == 0 ) {
				pWndMsgProcessor->getWndMsgs( containerWndMsgNeed );
			} else {
				pWndMsgProcessor->getWndMsgs( containerWndMsg );
				iendWndMsg = containerWndMsg.end();
				for ( itWndMsg = containerWndMsg.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
					// check itWndMsg if need to add to containerWndMsgNeed.
					iendWMNeed = containerWndMsgNeed.end();
					for ( itWMNeed = containerWndMsgNeed.begin(); itWMNeed!=iendWMNeed; ++itWMNeed ) {
						if ( *itWndMsg == *itWMNeed )
							break;
					}
					//
					if ( itWMNeed == iendWMNeed )
						containerWndMsgNeed.push_back( *itWndMsg );
				}
			}
		}
	}


}
//
void hm_keel_obj::_doWndMsg( CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc )
{
	if ( !pWP || !pParamWndProc ) return;
	wmp_container_type::iterator itWMP, iendWMP;
	hm_wndmsgprocessor_i* pWndMsgProcessor = NULL;
	
	iendWMP = m_containerWMP.end();
	for ( itWMP = m_containerWMP.begin(); itWMP!=iendWMP; ++itWMP ) {
		pWndMsgProcessor = *itWMP;
		if ( !pWndMsgProcessor ) {
			assert( false );		
			continue;
		}
		pWndMsgProcessor->doWndMsg( this, pWP, pParamWndProc );
	}
}
//
bool hm_keel_obj::_IsNeedWndMsg( unsigned int message ) {
	wndmsg_container_type containerWndMsg;
	wndmsg_container_type::iterator itWndMsg, iendWndMsg;
	wmp_container_type::iterator itWMP, iendWMP;
	hm_wndmsgprocessor_i* pWndMsgProcessor = NULL;
    
	// check wndmsg order
	containerWndMsg.clear();
	pushbackAllWndMsgOrdered( containerWndMsg );
	iendWndMsg = containerWndMsg.end();
	for ( itWndMsg = containerWndMsg.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
		if ( *itWndMsg == message )
			return true;
	}
	
	// check wndmsg processor
	iendWMP = m_containerWMP.end();
	for ( itWMP = m_containerWMP.begin(); itWMP!=iendWMP; ++itWMP ) {
		pWndMsgProcessor = *itWMP;
		if ( pWndMsgProcessor ) {
			hm_wndmsgprocessor_i::msg_container_type containerWndMsg;
			hm_wndmsgprocessor_i::msg_container_type::iterator itWndMsg, iendWndMsg;

			pWndMsgProcessor->getWndMsgs( containerWndMsg );
			iendWndMsg = containerWndMsg.end();
			for ( itWndMsg = containerWndMsg.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
				if ( *itWndMsg == message )	
					return true;
			}
		}
	}
	
	return false;
}

void hm_keel_obj::pushbackWMP( hm_wndmsgprocessor_i* pWndMsgProcessor ) {
	if ( !pWndMsgProcessor ) 
		return;
	if ( hasWMP( pWndMsgProcessor ) )
		return;
	hm_wndmsgprocessor_i::msg_container_type containerWndMsg;
	hm_wndmsgprocessor_i::msg_container_type::iterator itWndMsg, iendWndMsg;
	
	pWndMsgProcessor->getWndMsgs( containerWndMsg );
	iendWndMsg = containerWndMsg.end();
	for ( itWndMsg = containerWndMsg.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
		if ( !m_pWndMsgSource->hasOrderWndMsg( *itWndMsg, this ) ) {
			m_pWndMsgSource->orderWndMsg2Back( *itWndMsg, this );
		}
	}
	// subscribe to the WMP's out-of-message state changes: when a state API
	// fires HM_EVENT_WMP_STATECHANGED ( e.g. checkbox setCheck from radio
	// group logic ), the owner invalidates its own rect. dependency stays
	// owner -> member, the WMP never learns about keelobjs.
	pWndMsgProcessor->Register( this, HM_EVENT_WMP_STATECHANGED );
	m_containerWMP.push_back( pWndMsgProcessor );
}

bool hm_keel_obj::hasWMP( const hm_wndmsgprocessor_i* pWndMsgProcessor ) const{
	wmp_container_type::const_iterator citWMP, ciendWMP;
	
	ciendWMP = m_containerWMP.end();
	for ( citWMP = m_containerWMP.begin(); citWMP!=ciendWMP; ++citWMP ) {
		if ( *citWMP == pWndMsgProcessor )
			return true;
	}
	return false;
}

void hm_keel_obj::delWMP( hm_wndmsgprocessor_i* pWndMsgProcessor ) {
	wmp_container_type::iterator itWMP, iendWMP;
	bool bFindWMP = false;

	iendWMP = m_containerWMP.end();
	for ( itWMP = m_containerWMP.begin(); itWMP!=iendWMP; ++itWMP ) {
		if ( *itWMP == pWndMsgProcessor ) {
			m_containerWMP.erase( itWMP );
			bFindWMP = true;
			break;
		}
	}
	if ( !bFindWMP )
		return;

	// mirror of pushbackWMP's Register: the detached WMP may outlive this
	// keelobj ( WMPs live in the factory ), drop the subscription.
	pWndMsgProcessor->UnRegister( this, HM_EVENT_WMP_STATECHANGED );

	// check if need cancel some wndmsg from m_pWndMsgSource
	if ( m_pWndMsgSource ) {
		hm_wndmsgprocessor_i::msg_container_type containerWndMsgProcess;
		hm_wndmsgprocessor_i::msg_container_type::iterator itWndMsg, iendWndMsg;
		
		pWndMsgProcessor->getWndMsgs( containerWndMsgProcess );
		iendWndMsg = containerWndMsgProcess.end();
		for ( itWndMsg = containerWndMsgProcess.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
			if ( !_IsNeedWndMsg( *itWndMsg ) ) {
				m_pWndMsgSource->cancelWndMsg( *itWndMsg, this );
			}
		}
	}
}

hm_wndmsgprocessor_i* hm_keel_obj::getWMP( unsigned int uWMPId ) {
	wmp_container_type::iterator itWMP, iendWMP;
	bool bFindWMP = false;
	hm_wndmsgprocessor_i* pWMPDest = NULL;

	iendWMP = m_containerWMP.end();
	for ( itWMP = m_containerWMP.begin(); itWMP != iendWMP; ++itWMP ) {
		pWMPDest = ( *itWMP );
		if ( pWMPDest->getId() == uWMPId ) {
			return pWMPDest;
		}
	}

	assert( false );
	return NULL;
}

////
//void hm_keel_obj::pushbackWMP( hm_wmp_id_type wmpId )
//{
//	if ( !hasWMP( wmpId ) ) {
//		m_containerWMPId.push_back( wmpId );
//		
//		// check is need order wndmsg by keel obj
//		if ( m_pWndMsgSource ) {
//			hm_wndmsgprocessor_i* pWndMsgProcessor = NULL;
//			hm_wndmsgprocessor_i::msg_container_type containerWndMsg;
//			hm_wndmsgprocessor_i::msg_container_type::iterator itWndMsg, iendWndMsg;
//			
//			pWndMsgProcessor = WMPFACTORY_INSTANCE->getObj( wmpId );
//			if ( pWndMsgProcessor ) {
//				pWndMsgProcessor->getWndMsgs( containerWndMsg );
//				
//				iendWndMsg = containerWndMsg.end();
//				for ( itWndMsg = containerWndMsg.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
//					if ( !m_pWndMsgSource->hasOrderWndMsg( *itWndMsg, this ) ) {
//						m_pWndMsgSource->orderWndMsg2Back( *itWndMsg, this );
//					}
//				}
//			}
//		}
//	}
//}
////
//bool hm_keel_obj::hasWMP( hm_wmp_id_type wmpId )
//{
//	wmpid_container_type::iterator itWMP, iendWMP;
//
//	iendWMP = m_containerWMPId.end();
//	for ( itWMP = m_containerWMPId.begin(); itWMP!=iendWMP; ++itWMP ) {
//		if ( *itWMP == wmpId )	
//			return true;
//	}
//	
//	return false;
//}
////
//void hm_keel_obj::delWMP( hm_wmp_id_type wmpId )
//{
//	wmpid_container_type::iterator itWMP, iendWMP;
//
//	iendWMP = m_containerWMPId.end();
//	for ( itWMP = m_containerWMPId.begin(); itWMP!=iendWMP; ++itWMP ) {
//		if ( *itWMP == wmpId ) {
//			m_containerWMPId.erase( itWMP );
//			break;
//		}
//	}
//
//	// check if need cancel some wndmsg from m_pWndMsgSource
//	if ( m_pWndMsgSource ) {
//		hm_wndmsgprocessor_i* pWndMsgProcessor = NULL; 
//		hm_wndmsgprocessor_i::msg_container_type containerWndMsgProcess;
//		hm_wndmsgprocessor_i::msg_container_type::iterator itWndMsg, iendWndMsg;
//		
//		pWndMsgProcessor = WMPFACTORY_INSTANCE->getObj( wmpId );
//		pWndMsgProcessor->getWndMsgs( containerWndMsgProcess );
//		iendWndMsg = containerWndMsgProcess.end();
//		for ( itWndMsg = containerWndMsgProcess.begin(); itWndMsg!=iendWndMsg; ++itWndMsg ) {
//			if ( !_IsNeedWndMsg( *itWndMsg ) ) {
//				m_pWndMsgSource->cancelWndMsg( *itWndMsg, this );
//			}
//		}
//	}
//}
//
void hm_keel_obj::_doOrderWndMsgFromSource( wnd_msg_type message ){
	if ( m_pWndMsgSource )
		m_pWndMsgSource->orderWndMsg2Back( message, this );
}
//
void hm_keel_obj::_doCancelWndMsgFromSource( wnd_msg_type message ){
	if ( m_pWndMsgSource )
		m_pWndMsgSource->cancelWndMsg( message, this );
}


void hm_keel_obj::_pushbackSubObj( hm_keel_obj* pSubKeelObj ) {
	if ( !pSubKeelObj ) {
		assert( false );
		return;
	}
	if ( _hasSubObj( pSubKeelObj ) ) {
		assert( false );	
		return;
	}
	m_containerKeelObjs.push_back( pSubKeelObj );
}

void hm_keel_obj::_eraseSubObj( hm_keel_obj* pSubKeelObj ) {
	if ( !pSubKeelObj ) {
		assert( false );
		return;
	}
	keelobj_container_type::iterator itKeelObj, iendKeelObj;

	iendKeelObj = m_containerKeelObjs.end();
	for ( itKeelObj = m_containerKeelObjs.begin(); itKeelObj!=iendKeelObj; ++itKeelObj ) {
		if ( pSubKeelObj == (*itKeelObj) ) {
			m_containerKeelObjs.erase( itKeelObj );
			return;
		}
	}

	assert( false );
}

bool hm_keel_obj::_hasSubObj( hm_keel_obj* pSubKeelObj ) {
	if ( !pSubKeelObj ) {
		assert( false );
		return false;
	}
	keelobj_container_type::iterator itKeelObj, iendKeelObj;

	iendKeelObj = m_containerKeelObjs.end();
	for ( itKeelObj = m_containerKeelObjs.begin(); itKeelObj!=iendKeelObj; ++itKeelObj ) {
		if ( pSubKeelObj == (*itKeelObj) )
			return true;
	}

	return false;
}

hm_keel_obj* hm_keel_obj::getParent() {
	return m_pParentObj;
}

void hm_keel_obj::setParent( hm_keel_obj* pParentObj ) {
	if ( m_pParentObj == pParentObj )	
		return;
	//
	if ( m_pParentObj ) {
		m_pParentObj->UnRegister( this, HMKEELOBJEVENT_DRAWSUBOBJS );
	}
	//
	m_pParentObj = pParentObj;
	//
	if ( m_pParentObj ) {
		m_pParentObj->Register( this, HMKEELOBJEVENT_DRAWSUBOBJS );
	}
}
//
bool hm_keel_obj::hasLayoutObj() const
{
	return ( m_pLayoutObj != NULL );
}
//
hm_layout_obj* hm_keel_obj::getLayoutObj()
{
	return m_pLayoutObj;
}
//
const hm_layout_obj* hm_keel_obj::getLayoutObj() const
{
	return m_pLayoutObj;
}
//
void hm_keel_obj::setLayoutObj( hm_layout_obj* pLayoutObjNew )
{
	hm_layout_obj* pLayoutObjOld = NULL;

	pLayoutObjOld = m_pLayoutObj;
	m_pLayoutObj = pLayoutObjNew;

	Notify( HMKEELOBJEVENT_LAYOUTOBJ_CHANGED, (LPARAM)pLayoutObjNew, (LPARAM)pLayoutObjOld );

	if ( pLayoutObjOld ) {
		delete pLayoutObjOld;
		pLayoutObjOld = NULL;
	}
}
//
void hm_keel_obj::_delAllLayoutObj()
{
	if ( m_pLayoutObj ) {
		m_pLayoutObj->UnRegister( this, cmn::ID_EVENT_POSCHANGED );
		delete m_pLayoutObj;
		m_pLayoutObj = NULL;
	}
}
//
void hm_keel_obj::doDrawAll( HDC hDC, HWND hWndCanvas )
{
	doDrawAll( hDC, hWndCanvas, NULL );
}

void hm_keel_obj::doDrawAll( HDC hDC, HWND hWndCanvas, const RECT* prcDirty )
{
	if ( !isVisible() )
		return;

	// dirty-rect culling: a keelobj outside the damaged area contributes
	// nothing to this frame - skip its entire subtree ( WMP draws + child
	// walk ). note: cull BEFORE the clip-children SaveDC too, an off-screen
	// scroll container can skip its clip entirely.
	if ( prcDirty ) {
		hm_layout_obj* pLayoutObjCull = getLayoutObj();
		RECT rcSelf;
		if ( pLayoutObjCull
			&& pLayoutObjCull->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcSelf )
			&& !::IntersectRect( &rcSelf, &rcSelf, prcDirty ) )
			return;   // no intersection = subtree invisible this frame
	}


	// scroll-container semantics: with clip-children on, this subtree may
	// never draw outside my rect ( a tall content row sliding under the
	// well edge gets cut at the edge ). SaveDC/RestoreDC keeps the clip
	// hermetic - later siblings are unaffected.
	int nDCSave = 0;
	if ( m_uFlag & HMKEELOBJ_FLAG_CLIPCHILDREN ) {
		hm_layout_obj* pLayoutObjSelf = getLayoutObj();
		const coordinate_system_hwnd* pCoordSysHwnd = pLayoutObjSelf
			? ( const coordinate_system_hwnd* )pLayoutObjSelf->getCoordinateSystem() : NULL;
		RECT rcSelf;
		if ( pCoordSysHwnd && pCoordSysHwnd->getHWnd()
			&& pLayoutObjSelf->mapPosition( hWndCanvas, coordinate_system_hwnd::CSH_CLIENT_TYPE, rcSelf ) ) {
			nDCSave = ::SaveDC( hDC );
			::IntersectClipRect( hDC, rcSelf.left, rcSelf.top, rcSelf.right, rcSelf.bottom );
		}
	}
	//
	_doDrawSelf( hDC, hWndCanvas );
	//
	_doDrawChildKeelObjs( hDC, hWndCanvas );
	//
	if ( nDCSave )
		::RestoreDC( hDC, nDCSave );
}
//
void hm_keel_obj::_doDrawSelf( HDC hDC, HWND hWndCanvas ) {
	HEDSPARAM param;

	param.m_hDCCanvas = hDC;
	param.m_hWndCanvas = hWndCanvas;
	param.m_pLayoutObjCanvas = m_pLayoutObj;
	Notify( HMKEELOBJEVENT_DRAWSELF, (LPARAM)(&param) );
}
//
void hm_keel_obj::_doDrawChildKeelObjs( HDC hDC, HWND hWndCanvas )
{
	Notify( HMKEELOBJEVENT_DRAWSUBOBJS, (LPARAM)hDC, (LPARAM)hWndCanvas );
}
//
void hm_keel_obj::setVisible( BOOL bVisible ) {
	if ( bVisible == ( ( m_uFlag & HMKEELOBJ_FLAG_VISIBLE ) == HMKEELOBJ_FLAG_VISIBLE ) )
		return;

	if ( bVisible )
		m_uFlag |= HMKEELOBJ_FLAG_VISIBLE;
	else
		m_uFlag &= ~HMKEELOBJ_FLAG_VISIBLE;
	//
	Notify( HMKEELOBJEVENT_SETVISIBLE, (LPARAM)bVisible );
}
//
BOOL hm_keel_obj::isVisible() const {
	return m_uFlag & HMKEELOBJ_FLAG_VISIBLE;
}
//
void hm_keel_obj::setClipChildren( BOOL bClipChildren ) {
	if ( bClipChildren )
		m_uFlag |= HMKEELOBJ_FLAG_CLIPCHILDREN;
	else
		m_uFlag &= ~HMKEELOBJ_FLAG_CLIPCHILDREN;
}
//
BOOL hm_keel_obj::isClipChildren() const {
	return m_uFlag & HMKEELOBJ_FLAG_CLIPCHILDREN;
}
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FACTORY( hm_keel_obj )
