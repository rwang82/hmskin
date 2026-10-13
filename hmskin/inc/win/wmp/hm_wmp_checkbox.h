#ifndef __HM_WMP_CHECKBOX_H__
#define __HM_WMP_CHECKBOX_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"

#define HM_CHECK_FLAG_NONE (0x00000000)
#define HM_CHECK_FLAG_LBTDOWN (0x00000001)
#define HM_CHECK_FLAG_TRACEMOSELEAVE (0x00000002)
#define HM_CHECK_CMDEVT_DOWN (500)

class HMLIB_API hm_wmp_checkbox : public hm_wmp_display {
public:
	typedef enum _ECHECKBOXSTATUS {
		ENOCHECK_STATUS_NORMAL = 0,
		ENOCHECK_STATUS_HOVER = 1,
		ENOCHECK_STATUS_PRESS = 2,
		ECHECK_STATUS_NORMAL = 3,
		ECHECK_STATUS_HOVER = 4,
		ECHECK_STATUS_PRESS = 5,
		ECHECK_STATUS_DISABLE = 6,
		ENOCHECK_STATUS_DISABLE = 7
	} ECHECKBOXSTATUS;
	typedef enum _ENUMDISTRICTID {
		EDISTRICTID_BK_INVALID = -1,
		EDISTRICTID_BK_NOCHECK_NOR = 0,
		EDISTRICTID_BK_NOCHECK_HOV,
		EDISTRICTID_BK_NOCHECK_PRE,
		EDISTRICTID_BK_NOCHECK_DIS,
		EDISTRICTID_BK_CHECK_NOR,
		EDISTRICTID_BK_CHECK_HOV,
		EDISTRICTID_BK_CHECK_PRE,
		EDISTRICTID_BK_CHECK_DIS
	} EDISTRICTID;

public:
	hm_wmp_checkbox( hmskin_event_listener_base* pHMSkinEventListener );
	virtual ~hm_wmp_checkbox();

public:
	bool isCheck();

public:
	DECLARE_MSGBIND()
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds );

protected:
	virtual void _createDistrictObjs();

public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

private:
	void _doDrawSkin( HDC hDC, const RECT& rcCanvas );
	ECHECKBOXSTATUS _getCurState();
	EDISTRICTID _getCurDistrictId();

private:
	void _OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	ECHECKBOXSTATUS m_eCheckStatus;
	unsigned int m_uFlag;

};


#endif //__HM_WMP_CHECKBOX_H__