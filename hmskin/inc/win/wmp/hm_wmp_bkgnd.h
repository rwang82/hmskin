#ifndef __HM_WMP_BKGND_H__
#define __HM_WMP_BKGND_H__
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\hm_export_defs.h"
//
class HMLIB_API hm_wmp_bkgnd: public hm_wmp_display
{	
public:
	enum ENUMDISTRICTID {
		EDISTRICTID_BK = 0
	};
public:
	hm_wmp_bkgnd( hmskin_event_listener_base* pEventListener );
	virtual ~hm_wmp_bkgnd();

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
	
};




#endif //__HM_WMP_BKGND_H__