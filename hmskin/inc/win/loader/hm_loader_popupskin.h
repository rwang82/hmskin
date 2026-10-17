#ifndef __HM_LOADER_POPUPSKIN_H__
#define __HM_LOADER_POPUPSKIN_H__
#include "win\loader\hm_loader_base.h"

// engine popup menus ( optional manifest section ):
//   "PopupSkin": { "PropId": 21 }
// declares which skin PropId the ENGINE popup windows connect to ( the
// MsgSink/CoordinateSystem/keelobj/WMP behind that id draw the popup -
// hm_wmp_popupmenu ). absent = engine popups disabled; the transparent
// TrackPopupMenu hook then denies calls ( opt-in only ).
class hm_loader_popupskin : public hm_loader_base {
public:
	hm_loader_popupskin();
	virtual ~hm_loader_popupskin();

	// interface of hm_loader_base
public:
	virtual bool load( hm_manifest_json* pManifestData, hm_uiroot* pHMUIRoot );
	virtual bool load( hm_manifest_xml* pManifestData, hm_uiroot* pHMUIRoot );
};

#endif //__HM_LOADER_POPUPSKIN_H__
