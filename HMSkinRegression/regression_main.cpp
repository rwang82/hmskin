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

// one dump node line -> its mapped rect ( formatter is the dump's own,
// stable: `{ "id": N, ..., "rect": { "l": L, "t": T, "r": R, "b": B } }` )
static bool _find_node_rect( const TCHAR* szPath, int nId, RECT* prc ) {
	FILE* fp = _tfopen( szPath, _T( "rb" ) );
	if ( !fp )
		return false;
	char szLine[ 512 ];
	char szWant[ 32 ];
	sprintf_s( szWant, "\"id\": %d,", nId );
	bool bFound = false;
	while ( fgets( szLine, 512, fp ) ) {
		if ( strstr( szLine, szWant ) ) {
			const char* szRect = strstr( szLine, "\"l\":" );
			int l, t, r, b;
			if ( szRect && sscanf_s( szRect, "\"l\": %d, \"t\": %d, \"r\": %d, \"b\": %d",
			                          &l, &t, &r, &b ) == 4 ) {
				prc->left = l; prc->top = t; prc->right = r; prc->bottom = b;
				bFound = true;
			}
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

	// ---- NC-aware coordinate mapping ( 2026-11-01 menu-click regression ) ----
	// the dump maps every rect via mapPosition CLIENT. K6 ( wndarea root,
	// window origin ) must land at exactly -( clientOrigin - windowOrigin ),
	// measured independently here via Win32; K8 ( rootmenu, CS window type,
	// docked top: inset 20, offset 54, thickness 26 at 96dpi ) must land at
	// K6 + decoder geometry. the input-dependent conversion bug put every
	// rect at a constant offset - K8 off by inset*scale - menu and caption
	// clicks went dead while rendering stayed fine.
	{
		HWND hWndMap = ::CreateWindowEx( 0, _T("STATIC"), _T("rgmap"),
			WS_OVERLAPPEDWINDOW, 0, 0, 400, 300, NULL, NULL,
			(HINSTANCE)::GetModuleHandle( NULL ), NULL );
		CHECK( hWndMap != NULL, "mapcase window created" );
		CHECK( hm_wndpropid_connect( pRoot, hWndMap, 258 ) == true,
		       "mapcase connect PropId 258" );
		RECT rcK6 = { 0, 0, 0, 0 }, rcK8 = { 0, 0, 0, 0 };
		hm_keelobj_dump_tree( pRoot, hWndMap, _T( "hmskin_tree.mapcase.json" ) );
		BOOL bGot6 = _find_node_rect( _T( "hmskin_tree.mapcase.json" ), 6, &rcK6 );
		BOOL bGot8 = _find_node_rect( _T( "hmskin_tree.mapcase.json" ), 8, &rcK8 );
		POINT ptClient00 = { 0, 0 };
		RECT rcWndMap;
		::ClientToScreen( hWndMap, &ptClient00 );
		::GetWindowRect( hWndMap, &rcWndMap );
		int nDx = ptClient00.x - rcWndMap.left;   // window -> client shift
		int nDy = ptClient00.y - rcWndMap.top;
		CHECK( bGot6 && rcK6.left == -nDx && rcK6.top == -nDy,
		       "map K6 window-origin == -clientDelta ( win32 oracle )" );
		RECT rcWant8 = {
			hm_layout_scale_px( 20 ) + rcK6.left,  hm_layout_scale_px( 54 ) + rcK6.top,
			rcK6.right - hm_layout_scale_px( 20 ),  hm_layout_scale_px( 54 + 26 ) + rcK6.top };
		CHECK( bGot8 && abs( rcK8.left - rcWant8.left ) <= 1 && abs( rcK8.top - rcWant8.top ) <= 1
		       && abs( rcK8.right - rcWant8.right ) <= 1 && abs( rcK8.bottom - rcWant8.bottom ) <= 1,
		       "map K8 rootmenu == K6 + dock geometry ( off-origin invariant )" );
		CHECK( hm_wndpropid_disconnect( pRoot, hWndMap, 258 ) == true,
		       "mapcase disconnect" );
		::DestroyWindow( hWndMap );
	}

	// ---- shutdown ( must simply complete ) ----
	hm_lib_shutdown( pRoot );
	++s_nPass; printf( "PASS shutdown\n" );

	printf( "HMSkinRegression: %d passed, %d failed\n", s_nPass, s_nFail );
	printf( "HMSkinRegression: %s\n", s_nFail ? "FAILED" : "ALL GREEN" );
	return s_nFail;
}
