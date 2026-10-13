#ifndef __HM_WMP_BUTTON_H__
#define __HM_WMP_BUTTON_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"
//
#define BTN_FLAG_LBTNDOWN (0x00000001)
#define BTN_FLAG_TRACEMOUSELEAVE (0x00000002)
#define BTN_FLAG_DISABLE (0x00000004)
//
class HMLIB_API hm_wmp_button: public hm_wmp_display
{
public:
	enum ENUMBTNSTATE{ BTN_STATE_NORMAL= 0, BTN_STATE_HOVER, BTN_STATE_PRESS, BTN_STATE_DISABLE };
	enum ENUMDISTRICTID {
		EDISTRICTID_BK_NOR = 0,
		EDISTRICTID_BK_HOV,
		EDISTRICTID_BK_PRE,
		EDISTRICTID_BK_DIS
	};

public:
	hm_wmp_button( hmskin_event_listener_base* pHMSkinEventListener );
	virtual ~hm_wmp_button();

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
	
private:
	void _doDrawSkin( HDC hDC, const RECT& rcCanvas, ENUMBTNSTATE eBtnState );
	ENUMBTNSTATE _getCurState();

private:
	void _OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnRButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnRButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnSetText( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnGetText( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	unsigned int m_uBtnStateFlags;
};






#endif //__HM_WMP_BUTTON_H__