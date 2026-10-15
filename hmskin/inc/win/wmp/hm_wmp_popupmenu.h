#ifndef __HM_WMP_POPUPMENU_H__
#define __HM_WMP_POPUPMENU_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include <string>
#include "tchar.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"
//
class HMLIB_API dimension_menuwnd_i {
public:
	virtual bool getWndSize( HMENU hMenu, SIZE* pMenuWndSize ) = 0;
	virtual bool getMIRect( HMENU hMenu, unsigned int uMIIndex, RECT* pMIRect ) = 0;
};
//
class HMLIB_API hm_wmp_popupmenu : public hm_wmp_display {
public:
	typedef std::basic_string< TCHAR > string_type;	
	typedef enum _ENUMDISTRICTID {
		EDISTRICTID_BKGND = 0,
		EDISTRICTID_MENUITEM_NOR,
		EDISTRICTID_MENUITEM_HOV,
		EDISTRICTID_MENUITEM_PRE,
		EDISTRICTID_MENUITEM_DIS
	} ENUMDISTRICTID;
public:
	typedef struct _popmenu_item_info{
		string_type m_strText;
	} popmenu_item_info;
public:
	hm_wmp_popupmenu( dimension_menuwnd_i* pCalcMenuWndRectObj, hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_popupmenu();

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
	void _doDrawSkin( HDC hDC, const RECT& rcCanvas );

private:
	void _OnLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnRButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnDestroy( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnHMWMBindMenu( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnKillFocus( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );

private:
	HMENU m_hMenu;
	dimension_menuwnd_i* m_pDimensionMenuWndObj;
	int m_nHoverMIIndex;
};
//
class HMLIB_API dimension_menuwnd_default: public dimension_menuwnd_i {
public:
	dimension_menuwnd_default( unsigned int uMIHeight, unsigned int uMIWidth, unsigned int uSpace );
	virtual ~dimension_menuwnd_default();

public:
	virtual bool getWndSize( HMENU hMenu, SIZE* pMenuWndSize );
	virtual bool getMIRect( HMENU hMenu, unsigned int uMIIndex, RECT* pMIRect );

private:
	unsigned int m_uMIHeight;
	unsigned int m_uMIWidth;
	unsigned int m_uSpace;
};


#endif //__HM_WMP_POPUPMENU_H__