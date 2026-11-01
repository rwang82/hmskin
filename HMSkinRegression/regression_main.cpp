// HMSkinRegression: headless engine-logic regression gate ( see
// run_regression.cmd ). exit code = number of FAILED cases ( 0 = green ).
// everything here runs WITHOUT windows - the GUI layers ( smoke golden,
// e2e interaction ) stay in tools/ and need an interactive session.

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "hmskin.h"
#include "hmskin_event_listener_base.h"

static int s_nFail = 0;
static int s_nPass = 0;

#define CHECK( cond, name ) \
	do { \
		if ( cond ) { ++s_nPass; printf( "PASS %s\n", name ); } \
		else { ++s_nFail; printf( "FAIL %s\n", name ); } \
	} while ( 0 )

// minimal listener: startup asserts a non-null one, the gate never fires
// a real event ( no windows -> no interaction ).
class regression_listener : public hmskin_event_listener_base {
public:
	virtual void onHKSkinEvent( unsigned int, void*, void* ) {}
};

// demo manifest ids ( see demo/demo.cpp defines / demo.hm.manifest )
#define RG_KEELOBJ_MAINBKGND 1
#define RG_KEELOBJ_EVENTLOG  18
#define RG_KEELOBJ_MISSING   99999
#define RG_WMPID_EVENTLOG    35

static bool _file_contains( const TCHAR* szPath, const char* szNeedle ) {
	FILE* fp = _tfopen( szPath, _T( "rb" ) );
	if ( !fp )
		return false;
	char szBuf[ 8192 ];
	size_t n;
	bool bFound = false;
	while ( ( n = fread( szBuf, 1, sizeof( szBuf ) - 1, fp ) ) > 0 ) {
		szBuf[ n ] = '\0';
		if ( strstr( szBuf, szNeedle ) ) {
			bFound = true;
			break;
		}
	}
	fclose( fp );
	return bFound;
}

int main() {
	printf( "HMSkinRegression: begin ( headless engine gate )\n" );

	// ---- startup contract: a listener is required ( fail fast, no assert ) ----
	hm_startup_param paramNull( (HINSTANCE)::GetModuleHandle( NULL ), NULL, 96 );
	hm_uiroot* pRootNull = NULL;
	CHECK( hm_lib_startup( &paramNull, _T( "demo.hm.manifest" ), &pRootNull ) == false,
	       "startup without listener refused" );

	regression_listener listener;
	hm_startup_param param( (HINSTANCE)::GetModuleHandle( NULL ), &listener, 144 );
	hm_uiroot* pRoot = NULL;
	if ( !hm_lib_startup( &param, _T( "demo.hm.manifest" ), &pRoot ) || !pRoot ) {
		printf( "FAIL startup ( manifest missing from the output dir? )\n" );
		return 1;
	}
	++s_nPass; printf( "PASS startup + manifest load\n" );

	// ---- DPI math ( uDpi = 144 -> 1.5x ) ----
	CHECK( hm_layout_scale() == 1.5, "dpi scale factor 1.5" );
	CHECK( hm_layout_scale_px( 96 ) == 144, "dpi px(96) == 144" );
	CHECK( hm_layout_scale_px( 0 ) == 0, "dpi px(0) == 0" );

	// ---- single-uiroot contract ( second startup refused ) ----
	hm_uiroot* pRoot2 = (hm_uiroot*)1;
	CHECK( hm_lib_startup( &param, _T( "demo.hm.manifest" ), &pRoot2 ) == false,
	       "second uiroot refused" );

	// ---- keelobj tree dump ----
	const HWND hWndFake = (HWND)0x1234;   // mapping fails -> rectRaw nodes, still valid
	unsigned int nNodes = hm_keelobj_dump_tree( pRoot, hWndFake, _T( "hmskin_tree.regression.json" ) );
	CHECK( nNodes > 50, "dump_tree node count > 50" );
	CHECK( _file_contains( _T( "hmskin_tree.regression.json" ), "\"hmskin_tree\"" ),
	       "dump_tree json has tree key" );
	CHECK( _file_contains( _T( "hmskin_tree.regression.json" ), "\"parentId\"" ),
	       "dump_tree json has parentId" );

	// ---- id-based keelobj API contracts ----
	RECT rc = { 0, 0, 0, 0 };
	BOOL bOk = hm_keelobj_get_layout_rect( pRoot, RG_KEELOBJ_MAINBKGND, &rc );
	CHECK( bOk, "get_layout_rect existing id" );
	CHECK( hm_keelobj_get_layout_rect( pRoot, RG_KEELOBJ_MISSING, &rc ) == false,
	       "get_layout_rect missing id -> false" );
	CHECK( hm_keelobj_set_visible( pRoot, RG_KEELOBJ_MAINBKGND, TRUE ) == true,
	       "set_visible existing id" );
	CHECK( hm_keelobj_set_visible( pRoot, RG_KEELOBJ_MISSING, TRUE ) == false,
	       "set_visible missing id -> false" );
	CHECK( hm_keelobj_set_clip_children( pRoot, RG_KEELOBJ_MISSING, TRUE ) == false,
	       "set_clip_children missing id -> false" );

	// ---- text set/get roundtrip ( first text uires of the EVT log WMP ) ----
	CHECK( hm_wmp_set_text( pRoot, RG_KEELOBJ_EVENTLOG, RG_WMPID_EVENTLOG,
	                        _T( "REGRESSION_ROUNDTRIP" ) ) == true,
	       "set_text existing wmp" );
	unsigned int uNeed = hm_wmp_get_text( pRoot, RG_KEELOBJ_EVENTLOG, RG_WMPID_EVENTLOG,
	                                      NULL, 0 );
	CHECK( uNeed == 20 + 1, "get_text length query == len+1" );
	TCHAR szBuf[ 64 ] = { 0 };
	unsigned int uGot = hm_wmp_get_text( pRoot, RG_KEELOBJ_EVENTLOG, RG_WMPID_EVENTLOG,
	                                     szBuf, 64 );
	CHECK( uGot == 20 && _tcscmp( szBuf, _T( "REGRESSION_ROUNDTRIP" ) ) == 0,
	       "get_text roundtrip text" );
	CHECK( hm_wmp_set_text( pRoot, RG_KEELOBJ_MISSING, RG_WMPID_EVENTLOG, _T("x") ) == false,
	       "set_text missing keelobj -> false" );
	CHECK( hm_wmp_get_text( pRoot, RG_KEELOBJ_MISSING, RG_WMPID_EVENTLOG, szBuf, 64 ) == 0,
	       "get_text missing keelobj -> 0" );

	// ---- renderer override contract ----
	CHECK( hm_set_drawtool_all( pRoot, HM_DRAWTOL_GDI ) == true,
	       "set_drawtool_all valid enum" );
	CHECK( hm_set_drawtool_all( pRoot, 99 ) == false,
	       "set_drawtool_all invalid enum -> false" );

	// ---- disconnect safety on unknown handles ( no crash + idempotent ) ----
	CHECK( hm_wndpropid_disconnect( pRoot, hWndFake, 21 ) == true,
	       "disconnect unknown hwnd -> true ( idempotent no-op )" );

	// ---- shutdown ( must simply complete ) ----
	hm_lib_shutdown( pRoot );
	++s_nPass; printf( "PASS shutdown\n" );

	printf( "HMSkinRegression: %d passed, %d failed\n", s_nPass, s_nFail );
	printf( "HMSkinRegression: %s\n", s_nFail ? "FAILED" : "ALL GREEN" );
	return s_nFail;
}
