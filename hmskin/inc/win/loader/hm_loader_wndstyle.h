#ifndef __HM_LOADER_WNDSTYLE_H__
#define __HM_LOADER_WNDSTYLE_H__
#include "win\loader\hm_loader_base.h"

// window style adaptation for modern windows ( 11+ ).
// manifest section ( optional ):
//   "WndStyle": { "Corner": "DEFAULT" | "DONOTROUND" | "ROUND" | "ROUNDSMALL" }
// values map to DWM_WINDOW_CORNER_PREFERENCE ( applied via DwmSetWindowAttribute
// when a window is connected, see hm_wndpropid_connect ). on windows 10 and
// older the call is a harmless no-op.
enum hm_wndcorner_pref {
	HM_WNDCORNER_NOTSET     = -1, // section absent : don't touch system default
	HM_WNDCORNER_DEFAULT    = 0,  // DWMWCP_DEFAULT : follow system ( rounded on win11 )
	HM_WNDCORNER_DONOTROUND = 1,  // DWMWCP_DONOTROUND : square corners ( legacy look )
	HM_WNDCORNER_ROUND      = 2,  // DWMWCP_ROUND : force normal rounding
	HM_WNDCORNER_ROUNDSMALL = 3   // DWMWCP_ROUNDSMALL : force small radius
};
#define HM_DWMWA_WINDOW_CORNER_PREFERENCE (33)

class hm_loader_wndstyle : public hm_loader_base {
public:
	hm_loader_wndstyle();
	virtual ~hm_loader_wndstyle();

	// interface of hm_loader_base
public:
	virtual bool load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot );
	virtual bool load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot );
};

#endif //__HM_LOADER_WNDSTYLE_H__
