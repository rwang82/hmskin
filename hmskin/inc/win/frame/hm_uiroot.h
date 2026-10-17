#ifndef __HM_UI_ROOT_H__
#define __HM_UI_ROOT_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "common\hm_cmn_pattern_factory.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
//
class msg_sink_i_factory;
class coordinate_system_hwnd_factory;
class hm_keel_obj_factory;
class hm_rs_hwnddepository;
class hm_wmp_factory;
class hmskin_event_listener_base;
//
class hm_uiroot {
public:
	hm_uiroot( hmskin_event_listener_base* pEventListener = NULL );
	virtual ~hm_uiroot();

private:
	void _Init();

public:
	hm_rs_hwnddepository* m_pHWNDDespository;
	msg_sink_i_factory* m_pMsgSinkFactory;
	hm_wmark_csid_table_type* m_pWndMark2CSIdTable;
	hm_wmark_msid_table_type* m_pWndMark2MSIdTable;
	coordinate_system_hwnd_factory* m_pCoordSysFactory;
	hm_keel_obj_factory* m_pKeelObjFactory;
	hm_wmp_factory* m_pWMPFactory;
	hmskin_event_listener_base* m_pEventListener;
	// DPI layout scale of this skin session ( set by hm_lib_startup from
	// hm_startup_param::m_uDpi BEFORE the manifest loads; loaders read it
	// at parse time. the session owner - hm_layout_scale() mirrors it
	// process-wide as an app-facing convenience ).
	double m_dblLayoutScale;

	// window style adaptation, see hm_loader_wndstyle.h ( -1 = not configured ).
	int m_nWndCornerPref;

};

//
DECLARE_FACTORY_EXPORTS( hm_uiroot );
#define hm_uiroot_factory_type FACTORY_TYPE( hm_uiroot )
extern HMLIB_API hm_uiroot_factory_type g_factoryUIRoot;
#define HMUIROOTFACTORY_INSTANCE (&g_factoryUIRoot)

#endif //__HM_UI_ROOT_H__