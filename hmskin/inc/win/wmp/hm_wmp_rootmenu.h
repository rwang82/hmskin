#ifndef __HM_WMP_ROOTMENU_H__
#define __HM_WMP_ROOTMENU_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "common\hm_cmn_pattern_observer.h"
#include <string>
#include "tchar.h"
#include <deque>
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"
//
class hm_ps_canvas;
//
struct HMLIB_API RootMenuCfg{
	RootMenuCfg(){
		m_uLenMenuItem = 60;
		m_uSpaceH = 0;
		m_uSpaceV = 0;
	};
	~RootMenuCfg(){};
	unsigned int m_uLenMenuItem;
	unsigned int m_uSpaceH;
	unsigned int m_uSpaceV;
};

#define RMFLAG_BTNDOWN (0x00000001)
class HMLIB_API hm_wmp_rootmenu : public hm_wmp_display
{
public:
	typedef enum _ENUMROOTMENUITEMSTATE {
		ERMIS_NORMAL = 0,
		ERMIS_HOVER, 
		ERMIS_PRESS,
		ERMIS_DISABLE
	} ENUMROOTMENUITEMSTATE;
	typedef enum _ENUMDISTRICTID {
		EDISTRICTID_BKGND = 0,
		EDISTRICTID_MENUITEM_NOR,
		EDISTRICTID_MENUITEM_HOV,
		EDISTRICTID_MENUITEM_PRE,
		EDISTRICTID_MENUITEM_DIS
	} ENUMDISTRICTID;
	typedef struct _rmiteminfo {
		ENUMROOTMENUITEMSTATE m_eRMIState;
		std::basic_string< TCHAR > m_strText;
	} rmiteminfo;
	typedef std::deque< rmiteminfo > rmii_container_type;

public:
	hm_wmp_rootmenu( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_rootmenu();

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
	void setConfigure( const RootMenuCfg& rmCfg );

private:
	void _OnNCLButtonDown( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCLButtonDblClk( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCLButtonUp( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCMouseLeave( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnNCHitTest( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
	void _OnMouseMove( hm_keel_obj* pKeelObj, CWPSTRUCT* pWP, msg_sink_i::WNDPROCPARAM* pParamWndProc );
		
private:
	void _doDrawSkin( HDC hDC, const RECT& rcCanvas, HWND hWnd, const POINT& ptScreen );
	void _doDrawSkinRaw( HDC hDC, const RECT& rcCanvas, POINT ptInCanvas, HMENU hRootMenu );
	void _doDrawBackground( HDC hDC, const RECT& rcCanvas );
	void _doDrawMenuItem( HDC hDC, const RECT& rcDMICanvas, unsigned int uIndex, const rmiteminfo& rmiinfo );
	bool _getCurMenuItemIndex( const RECT& rcMDICanvas, HWND hWnd, const POINT& ptScreen, int& curMIIndex );

private:
	unsigned int m_uLenMenuItem;
	unsigned int m_uSpaceH;
	unsigned int m_uSpaceV;
	unsigned int m_uRootMenuFlags;
	int m_nCurMIIndex;
	hm_ps_canvas* m_pMemCanvas;
};

#endif //__HM_WMP_ROOTMENU_H__