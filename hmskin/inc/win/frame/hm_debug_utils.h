#ifndef __HM_DEBUG_UTILS_H__
#define __HM_DEBUG_UTILS_H__
#include "windows.h"
#include "win\hm_export_defs.h"

// invalidate/paint-rect logging for "why did this repaint" diagnosis:
// gated on the exe-dir "hmskin.trace" marker, appends "tick tag (l,t,r,b)"
// to "hmskin_invalidate.log" next to the exe ( prc NULL = FULL window ).
HMLIB_API void hm_util_invlog( const TCHAR* szTag, const RECT* prc );

#endif //__HM_DEBUG_UTILS_H__
