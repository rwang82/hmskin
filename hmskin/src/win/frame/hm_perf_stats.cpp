#include "stdafx.h"
#include "win\frame\hm_perf_stats.h"
#include <stdio.h>
#include <tchar.h>

// bucket edges in microseconds; last bucket is open-ended
static const long long s_bucketEdge[ 8 ] = { 50, 100, 200, 500, 1000, 2000, 5000, 0 };
static const TCHAR* s_toolName[ HM_PERF_TOOL_COUNT ] = { _T("D2D"), _T("GDI+") };

struct hm_perf_textstats {
	long long nCount;
	long long nSumUs;
	long long nMaxUs;
	long long nSumChars;
	long long nBucket[ 8 ];
	hm_perf_textstats() : nCount( 0 ), nSumUs( 0 ), nMaxUs( 0 ), nSumChars( 0 ) {
		for ( int i = 0; i < 8; i++ )
			nBucket[ i ] = 0;
	}
};
static hm_perf_textstats s_stats[ HM_PERF_TOOL_COUNT ];
// D2D sub-populations: cache hit ( pure DCRT round trip ) vs miss
// ( layout construction + round trip ) - the A/B discriminator
static hm_perf_textstats s_d2dHit, s_d2dMiss;

static void _accum_one( hm_perf_textstats& s, long long us, unsigned int uChars ) {
	s.nCount++;
	s.nSumUs += us;
	s.nSumChars += uChars;
	if ( us > s.nMaxUs )
		s.nMaxUs = us;
	int b = 0;
	while ( b < 7 && us >= s_bucketEdge[ b ] )
		b++;
	s.nBucket[ b ]++;
}

void hm_perf_text_draw_ex( int nTool, double dbMicros, unsigned int uChars, int nCacheHit ) {
	if ( nTool < 0 || nTool >= HM_PERF_TOOL_COUNT )
		return;
	long long us = (long long)( dbMicros + 0.5 );
	_accum_one( s_stats[ nTool ], us, uChars );
	if ( nTool == HM_PERF_TOOL_D2D && nCacheHit >= 0 )
		_accum_one( nCacheHit ? s_d2dHit : s_d2dMiss, us, uChars );
}

static long long _bucketPercentile( const hm_perf_textstats& s, int nPct ) {
	// approximate percentile from the histogram ( bucket = upper bound in us,
	// except the open-ended last one which reports its lower edge )
	long long target = ( s.nCount * nPct + 99 ) / 100;
	long long acc = 0;
	for ( int b = 0; b < 8; b++ ) {
		acc += s.nBucket[ b ];
		if ( acc >= target )
			return ( b < 7 ) ? s_bucketEdge[ b ] : s_bucketEdge[ 6 ];
	}
	return s.nMaxUs;
}

static void _print_stats( FILE* fp, const TCHAR* szName, const hm_perf_textstats& s ) {
	double avg = (double)s.nSumUs / (double)s.nCount;
	_ftprintf( fp, _T("[%s] count=%I64d avg=%.1fus p50<=%I64dus p95<=%I64dus max=%I64dus avg_len=%.0f\n"),
	           szName, s.nCount, avg,
	           _bucketPercentile( s, 50 ), _bucketPercentile( s, 95 ),
	           s.nMaxUs, (double)s.nSumChars / (double)s.nCount );
	_ftprintf( fp, _T("       buckets us <50/<100/<200/<500/<1000/<2000/<5000/rest: ") );
	for ( int b = 0; b < 8; b++ )
		_ftprintf( fp, _T("%I64d%s"), s.nBucket[ b ], ( b < 7 ) ? _T("/") : _T("\n") );
}

BOOL hm_perf_dump() {
	bool bAny = false;
	for ( int t = 0; t < HM_PERF_TOOL_COUNT; t++ ) {
		if ( s_stats[ t ].nCount > 0 )
			bAny = true;
	}
	if ( !bAny )
		return FALSE;

	TCHAR szPath[ MAX_PATH + 32 ];
	if ( ::GetModuleFileName( NULL, szPath, MAX_PATH ) == 0 )
		return FALSE;
	TCHAR* posSlash = _tcsrchr( szPath, _T('\\') );
	if ( !posSlash )
		return FALSE;
	*( posSlash + 1 ) = _T('\0');
	_tcscat_s( szPath, MAX_PATH + 32, _T("hmskin_perf.log") );

	FILE* fp = _tfopen( szPath, _T("w") );
	if ( !fp )
		return FALSE;
	_ftprintf( fp, _T("---- hmskin text-draw perf ----\n") );
	for ( int t = 0; t < HM_PERF_TOOL_COUNT; t++ ) {
		if ( s_stats[ t ].nCount > 0 )
			_print_stats( fp, s_toolName[ t ], s_stats[ t ] );
	}
	if ( s_d2dHit.nCount > 0 )
		_print_stats( fp, _T("D2D hit"), s_d2dHit );     // round trip only
	if ( s_d2dMiss.nCount > 0 )
		_print_stats( fp, _T("D2D miss"), s_d2dMiss );   // + layout construction
	if ( s_d2dHit.nCount + s_d2dMiss.nCount > 0 ) {
		_ftprintf( fp, _T("[D2D layout cache] hits=%I64d misses=%I64d (%.0f%% hit)\n"),
		           s_d2dHit.nCount, s_d2dMiss.nCount,
		           100.0 * (double)s_d2dHit.nCount
		               / (double)( s_d2dHit.nCount + s_d2dMiss.nCount ) );
	}
	fclose( fp );
	return TRUE;
}
