#ifndef __HM_PERF_STATS_H__
#define __HM_PERF_STATS_H__
#include "win\hm_export_defs.h"

// text-draw perf aggregation for backend A/B ( tools/smoke_test.py --perf ).
// the hot path is a few integer adds - no I/O, no locks; the report lands in
// hmskin_perf.log next to the exe when hm_lib_shutdown runs, or on an
// explicit hm_lib_dump_perf() call. instrumented in hm_ps_drawtool_d2d and
// hm_ps_drawtool_gdiplus ( their text draw is the A/B subject ).

#define HM_PERF_TOOL_D2D 0
#define HM_PERF_TOOL_GP  1
#define HM_PERF_TOOL_COUNT 2

// hot path. dbMicros = elapsed microseconds of one text draw.
HMLIB_API void hm_perf_text_draw( int nTool, double dbMicros, unsigned int uChars );

// write the report file. returns FALSE when nothing was collected or the
// file could not be created. does NOT reset the counters ( idempotent ).
HMLIB_API BOOL hm_perf_dump();

#endif //__HM_PERF_STATS_H__
