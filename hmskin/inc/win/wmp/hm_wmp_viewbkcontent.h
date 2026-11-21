#ifndef __HM_WMP_WMP_VIEWBKCONTENT_H__
#define __HM_WMP_WMP_VIEWBKCONTENT_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"
//
#define HMVBKC_FLAG_WHELLSTOPED ( 0x00000001 )
class HMLIB_API hm_wmp_viewbkcontent : public hm_wmp_display {
public:
	hm_wmp_viewbkcontent( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_viewbkcontent();

	// interface of hm_wndmsgprocessor_i
public:
	DECLARE_MSGBIND()
private:
	virtual void _getAllDistrictId( district_id_container_type& containerIds ){};

	// interface of hm_wmp_display
protected:
	virtual void _createDistrictObjs();

	// interface of hm_cmn_observer_i
public:
	virtual void update( void* pSubject, UINT idEvent, LPARAM pEventParam, LPARAM pEventParam2, BOOL* pbContinue );

protected:
	void _doDrawSkinRaw( HDC hDC, const RECT& rcCanvas );
	void _mapRectContentVisible2Canvas( const RECT& rcContentVisible, const RECT& rcCanvas, RECT& rcContentV2Canvas );

private:
	void _OnMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnHMMouseWhell( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

protected:
	HWND m_hWndCanvas;
	unsigned int m_uFlag;
};

#endif //__HM_WMP_WMP_VIEWBKCONTENT_H__