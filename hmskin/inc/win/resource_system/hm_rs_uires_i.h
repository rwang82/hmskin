#ifndef __HM_RS_UIRES_I_H__
#define __HM_RS_UIRES_I_H__
#include "WinDef.h"
#include "common\hm_cmn_pattern_observer.h"
#include "win\hm_export_defs.h"
//
class hm_ps_drawtool_i;

// broadcast by a uires when its CONTENT changes ( e.g. text via setText ):
// subscribers - typically the owning WMP's pixel cache - react by
// invalidating. declared at the ABSTRACT level: the framework subscribes
// any uires without knowing the concrete resource class.
#define HM_UIRES_EVENT_CONTENTCHANGED ( (UINT)0x0000A101 )

class HMLIB_API hm_rs_uires_i : public cmn::hm_cmn_subject
{
public:
	hm_rs_uires_i() {};
	virtual ~hm_rs_uires_i() {};

public:
	// rcCanvas - specify draw hm_rs_uires_i to which rectangle part of HDC.
	virtual void show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool ) = 0;
	//
	virtual bool loadRawRes( hm_ps_drawtool_i* pDrawTool ) = 0;
	//
	virtual bool unloadRawRes( hm_ps_drawtool_i* pDrawTool ) = 0;
};




#endif //__HM_RS_UIRES_I_H__