#ifndef __HM_UI_DEFAULTS_H__
#define __HM_UI_DEFAULTS_H__
#include "windows.h"

// engine-side fallback colors ( DESIGN.md tokens ): used when a manifest
// does not configure a color ( menu label colors etc. ). hosts may include
// this too to stay in sync ( e.g. the demo window-class brush ).
#define HM_UIDEF_TEXT_COLOR          RGB( 232, 236, 241 )     // text
#define HM_UIDEF_TEXT_TITLE_COLOR    RGB( 216, 224, 231 )     // text-title
#define HM_UIDEF_TEXT_MUTED_COLOR    RGB( 139, 147, 158 )     // text-muted
#define HM_UIDEF_TEXT_DISABLED_COLOR RGB( 91, 99, 110 )       // text-faint
#define HM_UIDEF_LINE_COLOR          RGB( 0x26, 0x2C, 0x34 )  // line ( hairline )
#define HM_UIDEF_BG_COLOR            RGB( 0x0F, 0x12, 0x16 )  // bg ( window base )

#endif //__HM_UI_DEFAULTS_H__
