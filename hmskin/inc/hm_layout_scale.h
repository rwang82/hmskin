#ifndef __HM_LAYOUT_SCALE_H__
#define __HM_LAYOUT_SCALE_H__
#include "win\hm_export_defs.h"

// DPI layout scale ( M1-1 ): manifests are authored on a 96dpi baseline;
// the loader multiplies every geometry number by this factor at parse time
// ( the industry-standard "authoring unit -> runtime unit" choke point,
// same pattern as density-independent units in modern UI systems ). set
// once by hm_lib_startup from hm_startup_param::m_uDpi BEFORE the manifest
// loads; runtime geometry APIs then speak physical pixels.
HMLIB_API double hm_layout_scale();
// convenience: 96-baseline value -> physical, rounded.
HMLIB_API int hm_layout_scale_px( int nValue96 );

#endif //__HM_LAYOUT_SCALE_H__
