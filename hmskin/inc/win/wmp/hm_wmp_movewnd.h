#ifndef __HM_WMP_MOVEWND_H__
#define __HM_WMP_MOVEWND_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\resource_system\hm_rs_wndmark.h"
//
class hm_uiroot;
//
class hm_wmp_movewnd : public hm_wndmsgprocessor_i {
public:
	hm_wmp_movewnd( hmskin_event_listener_base* pEventListener = NULL );
	virtual ~hm_wmp_movewnd();

	// interface
public:
	virtual void doWndMsg( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	virtual void getWndMsgs( msg_container_type& containerWndMsg );


	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

public:
	void setUIRoot( hm_uiroot* pHMUIRoot );
	void setWndMark( const hm_rs_wndmark& wndMark );

private:
	void _OnHMKEELOBJEVENT_POSCHANGED( void* pSubject, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );	

private:
	hm_rs_wndmark m_wndMark;
	hm_uiroot* m_pHMUIRoot;
};


#endif //__HM_WMP_MOVEWND_H__