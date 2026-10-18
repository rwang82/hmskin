#ifndef __HM_DEBUG_UTILS_H__
#define __HM_DEBUG_UTILS_H__
#include "windows.h"
#include "win\hm_export_defs.h"

void hm_util_draw_rect( HDC hDC, const RECT& rcDraw );
void hm_util_draw_rect( HDC hDC, const RECT& rcDraw, const COLORREF& clr );
void hm_util_draw_line( HDC hDC, int x1, int y1, int x2, int y2, const COLORREF& clr = RGB( 0, 0, 0 ), int nWidth = 1, int fnPenStyle = PS_SOLID );

// invalidate/paint-rect logging for "why did this repaint" diagnosis:
// gated on the exe-dir "hmskin.trace" marker, appends "tick tag (l,t,r,b)"
// to "hmskin_invalidate.log" next to the exe ( prc NULL = FULL window ).
HMLIB_API void hm_util_invlog( const TCHAR* szTag, const RECT* prc );

#endif //__HM_DEBUG_UTILS_H__