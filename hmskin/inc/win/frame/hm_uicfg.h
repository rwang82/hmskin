#ifndef __HM_UICFG_H__
#define __HM_UICFG_H__

// session config: inputs frozen once load completes - startup param ( DPI )
// + manifest sections ( WndStyle, PopupSkin ).
class hm_uicfg {
public:
	hm_uicfg()
	: m_dblLayoutScale( 1.0 )
	, m_uPopupPropId( 0 )
	, m_nWndCornerPref( -1 ) {}

	double m_dblLayoutScale;      // = dpi/96, loaders scale px at parse time
	unsigned int m_uPopupPropId;  // manifest PopupSkin PropId, 0 = popups off
	int m_nWndCornerPref;         // see hm_loader_wndstyle.h, -1 = not configured
};

#endif //__HM_UICFG_H__
