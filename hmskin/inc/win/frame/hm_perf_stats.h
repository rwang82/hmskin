#ifndef __HM_PERF_STATS_H__
#define __HM_PERF_STATS_H__
#include "win\hm_export_defs.h"
#include <windows.h>

// text-draw perf aggregation for backend A/B ( tools/smoke_test.py --perf ).
// the hot path is a few integer adds - no I/O, no locks; the report lands in
// hmskin_perf.log next to the exe when hm_lib_shutdown runs, or on an
// explicit hm_perf_dump() call. instrumented in hm_ps_drawtool_d2d and
// hm_ps_drawtool_gdiplus ( their text draw is the A/B subject ).

#define HM_PERF_TOOL_D2D 0
#define HM_PERF_TOOL_GP  1
#define HM_PERF_TOOL_COUNT 2

// hot path. dbMicros = elapsed microseconds of one text draw.
// nCacheHit tags the D2D layout-cache outcome ( -1 = n/a, 0 = miss,
// 1 = hit ) so the report can split the D2D histogram into "round trip
// only" vs "construction + round trip" populations.
HMLIB_API void hm_perf_text_draw_ex( int nTool, double dbMicros, unsigned int uChars, int nCacheHit );

// write the report file. returns FALSE when nothing was collected or the
// file could not be created. does NOT reset the counters ( idempotent ).
HMLIB_API BOOL hm_perf_dump();

// RAII timing guard: destructor records one timed draw on EVERY return path
// of the function it lives in. shared by the draw-tool backends.
class hm_perf_text_guard {
public:
	hm_perf_text_guard( int nTool, unsigned int uChars )
		: m_nTool( nTool ), m_uChars( uChars ), m_nCacheHit( -1 ) {
		::QueryPerformanceCounter( &m_t0 );
	}
	// tag the sample after the fact ( the cache outcome is only known once
	// the layout lookup has run inside the guarded function )
	void setCacheHit( int nCacheHit ) { m_nCacheHit = nCacheHit; }
	~hm_perf_text_guard() {
		LARGE_INTEGER t1, f;
		::QueryPerformanceCounter( &t1 );
		::QueryPerformanceFrequency( &f );
		double us = (double)( t1.QuadPart - m_t0.QuadPart ) * 1e6 / (double)f.QuadPart;
		hm_perf_text_draw_ex( m_nTool, us, m_uChars, m_nCacheHit );
	}
private:
	LARGE_INTEGER m_t0;
	int m_nTool;
	unsigned int m_uChars;
	int m_nCacheHit;
};

#endif //__HM_PERF_STATS_H__
