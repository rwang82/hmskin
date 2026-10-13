#ifndef __HM_WMP_BUTTON_NCA_H__
#define __HM_WMP_BUTTON_NCA_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"
//
#define NCABTN_FLAG_LBTNDOWN (0x00000001)
#define NCABTN_FLAG_TRACEMOUSELEAVE (0x00000002)
#define NCABTN_FLAG_DISABLE (0x00000004)
//
class hm_keel_obj;
// 
class HMLIB_API hm_wmp_button_nca: public hm_wmp_display
{
public:
	enum ENUMBTNSTATE{ 
		NCBTN_STATE_NORMAL_ACTIVE= 0, 
		NCBTN_STATE_HOVER_ACTIVE, 
		NCBTN_STATE_PRESS_ACTIVE, 
		NCBTN_STATE_DISABLE_ACTIVE,
		NCBTN_STATE_NORMAL_INACTIVE, 
		NCBTN_STATE_HOVER_INACTIVE, 
		NCBTN_STATE_PRESS_INACTIVE, 
		NCBTN_STATE_DISABLE_INACTIVE,
	};
	enum ENUMDISTRICTID {
		EDISTRICTID_BK_NOR_ACTIVE = 0,
		EDISTRICTID_BK_HOV_ACTIVE,
		EDISTRICTID_BK_PRE_ACTIVE,
		EDISTRICTID_BK_DIS_ACTIVE,
		EDISTRICTID_BK_NOR_INACTIVE,
		EDISTRICTID_BK_HOV_INACTIVE,
		EDISTRICTID_BK_PRE_INACTIVE,
		EDISTRICTID_BK_DIS_INACTIVE
	};

public:
	hm_wmp_button_nca( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_button_nca();

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
	//
private:
	void _OnNCActivate( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	//
private:
	void _doDrawSkin( HDC hDC, const RECT& rcCanvas, ENUMBTNSTATE eBtnState );
	ENUMBTNSTATE _getCurState();

private:
	void _OnNCLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	unsigned int m_uBtnStateFlags;
	bool m_bActive;
};



#endif //__HM_WMP_BUTTON_NCA_H__
