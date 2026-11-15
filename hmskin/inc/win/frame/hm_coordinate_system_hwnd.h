#ifndef __HM_COORDINATE_SYSTEM_HWND_H__
#define __HM_COORDINATE_SYSTEM_HWND_H__
#include "common\hm_cmn_patterns.h"
#include "common\hm_cmn_pattern_factory.h"
#include "common\hm_cmn_pattern_observer.h"
#include "win\resource_system\hm_rs_wndmark.h"
#include "win\hm_event_defs.h"
#include "win\hm_export_defs.h"
//
class hm_uiroot;
//
class HMLIB_API coordinate_system_hwnd : public cmn::hm_cmn_subject
{	
// type define.
public:
	enum ENUMCSHTYPE { CSH_CLIENT_TYPE, CSH_WINDOW_TYPE };
 
public:
	coordinate_system_hwnd( ENUMCSHTYPE eType = CSH_CLIENT_TYPE );
	~coordinate_system_hwnd(){};
	
public:
	bool isSameSystem( const coordinate_system_hwnd& objCoordSysHwnd ) const;
	void setHWnd( HWND hWnd ) { m_hWnd = hWnd; if ( hWnd ) Notify( HM_EVENT_COORDSYS_CHANGE ); }
	HWND getHWnd() const { return m_hWnd; }
	ENUMCSHTYPE getType() const { return m_eType; }
	
private:
	HWND m_hWnd;
	ENUMCSHTYPE m_eType;
};

//
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( coordinate_system_hwnd );
#pragma warning(pop)
#define hm_coordsys_factory_type FACTORY_TYPE( coordinate_system_hwnd )
//
#define hm_wmark_csid_table_type TWndMarkTable< hm_coordsys_factory_type::obj_id_type >
//
bool ConnectHwnd2CoordSys( hm_uiroot* pHMUIRoot, HWND hWnd, const hm_rs_wndmark& wndMark );
bool DisconnectHwnd2CoordSys( hm_uiroot* pHMUIRoot, HWND hWnd, const hm_rs_wndmark& wndMark );


EXTERN_C_START

HMLIB_API bool hm_csid2wndmark_bind( hm_uiroot* pHMUIRoot, unsigned int csId, const hm_rs_wndmark& wndMark );



EXTERN_C_END

#endif //__HM_COORDINATE_SYSTEM_HWND_H__