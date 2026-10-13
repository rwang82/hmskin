#ifndef __HM_WMP_BKGND_NCA_H__
#define __HM_WMP_BKGND_NCA_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"

class HMLIB_API hm_wmp_bkgnd_nca: public hm_wmp_display
{
public:
	enum ENUMDISTRICTID {
		EDISTRICTID_CAPTION_ACTIVE = 0,
		EDISTRICTID_BOTTOM_ACTIVE,
		EDISTRICTID_LEFT_ACTIVE,
		EDISTRICTID_RIGHT_ACTIVE,
		EDISTRICTID_CAPTION_INACTIVE,
		EDISTRICTID_BOTTOM_INACTIVE,
		EDISTRICTID_LEFT_INACTIVE,
		EDISTRICTID_RIGHT_INACTIVE
	};
public:
	hm_wmp_bkgnd_nca( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_bkgnd_nca();

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds );

	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );
	
public:
	void setThickL( unsigned int uThick );
	void setThickR( unsigned int uThick );
	void setThickT( unsigned int uThick );
	void setThickB( unsigned int uThick );
	//
private:
	void _OnNCActivate( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCCalcSize( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnSysKeyDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnSysKeyUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
			
	//
private:
	void _doDrawCaption( HDC hDC, HWND hWndDest );
	void _doDrawBottom( HDC hDC, HWND hWndDest );
	void _doDrawLeft( HDC hDC, HWND hWndDest );
	void _doDrawRight( HDC hDC, HWND hWndDest );
	void _doDrawSysBtn( HDC hDC, HWND hWndDest );
	//
	int _Pt2HitTestCode( HWND hWnd, const POINT& ptWnd );

private:
	bool m_bActive;
	unsigned int m_uThickL;
	unsigned int m_uThickR;
	unsigned int m_uThickT;
	unsigned int m_uThickB;
};


#endif //__HM_WMP_BKGND_NCA_H__