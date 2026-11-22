// HMSkinRegression: headless engine-logic regression gate ( see
// run_regression.cmd ). exit code = number of FAILED cases ( 0 = green ).
// everything here runs WITHOUT windows - the GUI layers ( smoke golden,
// e2e interaction ) stay in tools/ and need an interactive session.

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "hmskin.h"
#include "hmskin_event_listener_base.h"
#include "win\hm_wndmsg_defs.h"

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
	// crash-resilient output: unbuffered stdout keeps every PASS line that
	// preceded a crash inside the harness log ( the failing step is then the
	// first missing one ).
	setvbuf( stdout, NULL, _IONBF, 0 );
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
		// warm_render: offscreen first-render pass ( startup-flash fix ) -
		// contract checks only, the visual effect is demo/e2e territory.
		CHECK( hm_lib_warm_render( pRoot, hWndMap ) == true,
		       "warm_render valid args" );
		CHECK( hm_lib_warm_render( NULL, hWndMap ) == false,
		       "warm_render null root -> false" );
		CHECK( hm_lib_warm_render( pRoot, NULL ) == false,
		       "warm_render null hwnd -> false" );
		CHECK( hm_wndpropid_disconnect( pRoot, hWndMap, 258 ) == true,
		       "mapcase disconnect" );
		::DestroyWindow( hWndMap );
	}

	// ---- scrollview orchestration ( WMP87 hm_wmp_scrollview on the K2 well:
	// wheel -> layout scroll offset, value range, clamp, bar fine-step ) ----
	// SendMessage drives the dispatcher subclass synchronously - no message
	// pump needed headless; the wheel point rides in lParam ( screen coords ),
	// the same contract real hardware input uses.
	{
		printf( "scrollcase: begin\n" );
		// window big enough for the FULL demo layout at the session scale
		// ( 1100x750@96 x1.5 ): a narrow window collapses the left panel
		// ( right inset 430@96=645px ) and degenerates K2 to zero width.
		HWND hWndScroll = ::CreateWindowEx( 0, _T("STATIC"), _T("rgscroll"),
			WS_OVERLAPPEDWINDOW, 0, 0, 1700, 1200, NULL, NULL,
			(HINSTANCE)::GetModuleHandle( NULL ), NULL );
		CHECK( hWndScroll != NULL, "scrollcase window created" );
		CHECK( hm_wndpropid_connect( pRoot, hWndScroll, 258 ) == true,
		       "scrollcase connect PropId 258" );
		// sec.4 is hidden by default ( chapter mechanism ) - show the trio.
		hm_keelobj_set_visible( pRoot, 2, TRUE );
		hm_keelobj_set_visible( pRoot, 38, TRUE );
		hm_keelobj_set_visible( pRoot, 47, TRUE );
		printf( "scrollcase: wired, dumping initial rects\n" );

		const TCHAR* szDumpScroll = _T( "hmskin_tree.scrollcase.json" );
		hm_keelobj_dump_tree( pRoot, hWndScroll, szDumpScroll );
		RECT rcK2s = { 0, 0, 0, 0 }, rcK38s = { 0, 0, 0, 0 };
		CHECK( _find_node_rect( szDumpScroll, 2, &rcK2s ) && _find_node_rect( szDumpScroll, 38, &rcK38s ),
		       "scrollcase K2/K38 rects dumped" );
		const int nStep = hm_layout_scale_px( 60 );      // WheelStep96
		const int nContentH = hm_layout_scale_px( 600 ); // ContentExtent96.H
		const int nTop0 = rcK38s.top;
		const int nRange = nContentH - ( rcK2s.bottom - rcK2s.top );
		CHECK( rcK38s.bottom - rcK38s.top == nContentH, "scrollcase content height == 600@96" );
		CHECK( nRange > nStep, "scrollcase content taller than viewport" );

		// wheel points in CLIENT coords ( from the dumped viewport rect ),
		// content-side and bar-strip-side, then translated to screen.
		POINT ptContent = { rcK2s.left + hm_layout_scale_px( 50 ), rcK2s.top + hm_layout_scale_px( 50 ) };
		POINT ptBar = { rcK2s.right - hm_layout_scale_px( 9 ),
		                ( rcK2s.top + rcK2s.bottom ) / 2 };
		::ClientToScreen( hWndScroll, &ptContent );
		::ClientToScreen( hWndScroll, &ptBar );
		LPARAM lpContent = MAKELPARAM( ptContent.x, ptContent.y );
		LPARAM lpBar = MAKELPARAM( ptBar.x, ptBar.y );
		WPARAM wpDown1 = MAKEWPARAM( 0, (SHORT)( -1 * WHEEL_DELTA ) );
		WPARAM wpUp1 = MAKEWPARAM( 0, (SHORT)( 1 * WHEEL_DELTA ) );
		// "overshoot" deltas must stay inside a 16-bit wheel-delta field:
		// +-10 notches ( 1200 ) far exceeds the range yet truncates nowhere.
		WPARAM wpUpBig = MAKEWPARAM( 0, (SHORT)( 10 * WHEEL_DELTA ) );
		WPARAM wpDownBig = MAKEWPARAM( 0, (SHORT)( -10 * WHEEL_DELTA ) );

		// (a) one notch over the content == exactly one wheel step.
		printf( "scrollcase: step (a) wheel -1 over content\n" );
		::SendMessage( hWndScroll, WM_MOUSEWHEEL, wpDown1, lpContent );
		hm_keelobj_dump_tree( pRoot, hWndScroll, szDumpScroll );
		CHECK( _find_node_rect( szDumpScroll, 38, &rcK38s ) && rcK38s.top == nTop0 - nStep,
		       "scrollcase wheel -1 notch -> one step" );

		// (b) huge wheel clamps at the bottom ( contentH - viewportH ).
		printf( "scrollcase: step (b) wheel overshoot down\n" );
		::SendMessage( hWndScroll, WM_MOUSEWHEEL, wpDownBig, lpContent );
		hm_keelobj_dump_tree( pRoot, hWndScroll, szDumpScroll );
		CHECK( _find_node_rect( szDumpScroll, 38, &rcK38s ) && rcK38s.top == nTop0 - nRange,
		       "scrollcase wheel overshoot clamps at range" );

		// (c) step back up one notch, then wheel over the BAR STRIP: the
		// scrollbar's own 3px fine step, through the same TRACKTHUMB chain.
		printf( "scrollcase: step (c) bar-strip fine wheel\n" );
		::SendMessage( hWndScroll, WM_MOUSEWHEEL, wpUp1, lpContent );
		::SendMessage( hWndScroll, WM_MOUSEWHEEL, wpDown1, lpBar );
		hm_keelobj_dump_tree( pRoot, hWndScroll, szDumpScroll );
		CHECK( _find_node_rect( szDumpScroll, 38, &rcK38s ) && rcK38s.top == nTop0 - nRange + nStep - 3,
		       "scrollcase bar-wheel -1 notch -> 3px fine step" );

		// (d) huge wheel up returns exactly to the top.
		printf( "scrollcase: step (d) wheel overshoot up\n" );
		::SendMessage( hWndScroll, WM_MOUSEWHEEL, wpUpBig, lpContent );
		hm_keelobj_dump_tree( pRoot, hWndScroll, szDumpScroll );
		CHECK( _find_node_rect( szDumpScroll, 38, &rcK38s ) && rcK38s.top == nTop0,
		       "scrollcase wheel up clamps back at top" );

		CHECK( hm_wndpropid_disconnect( pRoot, hWndScroll, 258 ) == true,
		       "scrollcase disconnect" );
		::DestroyWindow( hWndScroll );
	}

	// ---- popup menu work-area flip ( native TrackPopupMenu parity ) ----
	// BINDMENU sizes the window from the HMENU; a corner anchor must flip
	// the menu back into the work area ( anchor becomes right/bottom edge ).
	// the anchor-edge assertions are menu-size independent, so DPI rounding
	// of the MenuCfg cannot wobble them.
	{
		printf( "popupcase: begin\n" );
		RECT rcWork;
		::SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );
		HWND hWndPop = ::CreateWindowEx( 0, _T("STATIC"), _T("rgpopup"),
			WS_POPUP, rcWork.right - 10, rcWork.bottom - 10, 40, 40, NULL, NULL,
			(HINSTANCE)::GetModuleHandle( NULL ), NULL );
		CHECK( hWndPop != NULL, "popupcase window created" );
		CHECK( hm_wndpropid_connect( pRoot, hWndPop, 21 ) == true,
		       "popupcase connect PopupSkin PropId 21" );
		HMENU hMenu = ::CreatePopupMenu();
		MENUITEMINFO mii;
		memset( &mii, 0, sizeof( mii ) );
		mii.cbSize = sizeof( mii );
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.fType = MFT_STRING;
		mii.dwTypeData = _T("rg item");
		mii.wID = 301;
		::InsertMenuItem( hMenu, 0, TRUE, &mii );
		// item 1 disabled, item 2 distinct id: keyboard DOWN must SKIP the
		// disabled middle item and land on 2.
		mii.fMask |= MIIM_STATE;
		mii.fState = MF_DISABLED | MF_GRAYED;
		::InsertMenuItem( hMenu, 1, TRUE, &mii );
		mii.fMask = MIIM_STRING | MIIM_ID;
		mii.fState = 0;
		mii.wID = 302;
		::InsertMenuItem( hMenu, 2, TRUE, &mii );
		// trailing separator: a 1px slot, not a full item row.
		mii.fMask = MIIM_FTYPE;
		mii.fType = MFT_SEPARATOR;
		mii.dwTypeData = NULL;
		::InsertMenuItem( hMenu, 3, TRUE, &mii );

		::SendMessage( hWndPop, HMWM_BINDMENU, (WPARAM)hMenu, 0 );
		RECT rcPop;
		::GetWindowRect( hWndPop, &rcPop );
		CHECK( rcPop.right == rcWork.right - 10 && rcPop.bottom == rcWork.bottom - 10,
		       "popupcase corner anchor flips to right/bottom aligned" );
		CHECK( rcPop.left >= rcWork.left && rcPop.top >= rcWork.top
		       && rcPop.right <= rcWork.right && rcPop.bottom <= rcWork.bottom,
		       "popupcase flipped rect inside work area" );

		// mid-work-area anchor: nothing overflows, left/top must stay put.
		::MoveWindow( hWndPop, rcWork.left + 40, rcWork.top + 40, 40, 40, FALSE );
		::SendMessage( hWndPop, HMWM_BINDMENU, (WPARAM)hMenu, 0 );
		::GetWindowRect( hWndPop, &rcPop );
		CHECK( rcPop.left == rcWork.left + 40 && rcPop.top == rcWork.top + 40,
		       "popupcase mid-screen anchor keeps left/top" );
		// separator geometry: 3 full items + a 1px slot, 5 gaps total.
		{
			const int s6 = hm_layout_scale_px( 6 ), h25 = hm_layout_scale_px( 25 );
			CHECK( rcPop.bottom - rcPop.top == 5 * s6 + 3 * h25 + 1,
			       "popupcase separator thins to a 1px slot" );
		}

		// keyboard path ( native parity ): ESC dismisses; on a fresh window
		// DOWN selects item 0 and RETURN activates - WM_COMMAND must land in
		// the queue of the still-alive owner window.
		::SendMessage( hWndPop, WM_KEYDOWN, VK_ESCAPE, 0 );
		CHECK( !::IsWindow( hWndPop ), "popupcase ESC dismisses the popup" );
		// release the slot like popupmenu_show does after its modal pump,
		// or the next connect on PropId 21 hits the exclusive-slot guard.
		CHECK( hm_wndpropid_disconnect( pRoot, hWndPop, 21 ) == true,
		       "popupcase ESC slot released" );

		HWND hWndOwner = ::CreateWindowEx( 0, _T("STATIC"), _T("rgpopowner"),
			WS_POPUP, 0, 0, 10, 10, NULL, NULL,
			(HINSTANCE)::GetModuleHandle( NULL ), NULL );
		hWndPop = ::CreateWindowEx( 0, _T("STATIC"), _T("rgpopup2"),
			WS_POPUP, rcWork.left + 40, rcWork.top + 40, 40, 40, NULL, NULL,
			(HINSTANCE)::GetModuleHandle( NULL ), NULL );
		CHECK( hWndPop != NULL && hWndOwner != NULL, "popupcase key window created" );
		CHECK( hm_wndpropid_connect( pRoot, hWndPop, 21 ) == true,
		       "popupcase key window connect" );
		::SendMessage( hWndPop, HMWM_BINDMENU, (WPARAM)hMenu, (LPARAM)hWndOwner );
		// separator band clicks: absorbed, menu stays ( VM-verified native );
		// a blank-margin click dismisses ( also VM-verified ).
		{
			const int s6 = hm_layout_scale_px( 6 ), h25 = hm_layout_scale_px( 25 );
			::SendMessage( hWndPop, WM_LBUTTONDOWN, MK_LBUTTON,
			               MAKELPARAM( s6 + 10, 4 * s6 + 3 * h25 ) );
			::SendMessage( hWndPop, WM_LBUTTONDOWN, MK_LBUTTON,
			               MAKELPARAM( s6 + 10, 4 * s6 + 3 * h25 - s6 + 1 ) );
			CHECK( ::IsWindow( hWndPop ), "popupcase separator band clicks keep the popup" );
			::SendMessage( hWndPop, WM_LBUTTONDOWN, MK_LBUTTON,
			               MAKELPARAM( 2, h25 / 2 ) );
		}
		CHECK( !::IsWindow( hWndPop ), "popupcase blank-margin click dismisses" );
		CHECK( hm_wndpropid_disconnect( pRoot, hWndPop, 21 ) == true,
		       "popupcase separator-click slot released" );

		hWndPop = ::CreateWindowEx( 0, _T("STATIC"), _T("rgpopup3"),
			WS_POPUP, rcWork.left + 40, rcWork.top + 40, 40, 40, NULL, NULL,
			(HINSTANCE)::GetModuleHandle( NULL ), NULL );
		CHECK( hWndPop != NULL, "popupcase key window recreated" );
		CHECK( hm_wndpropid_connect( pRoot, hWndPop, 21 ) == true,
		       "popupcase key window reconnect" );
		::SendMessage( hWndPop, HMWM_BINDMENU, (WPARAM)hMenu, (LPARAM)hWndOwner );
		::SendMessage( hWndPop, WM_KEYDOWN, VK_DOWN, 0 );
		::SendMessage( hWndPop, WM_KEYDOWN, VK_DOWN, 0 );   // skips the disabled middle item
		::SendMessage( hWndPop, WM_KEYDOWN, VK_RETURN, 0 );
		MSG msgK;
		BOOL bGotCmd = FALSE;
		while ( ::PeekMessage( &msgK, NULL, 0, 0, PM_REMOVE ) ) {
			if ( msgK.message == WM_COMMAND && LOWORD( msgK.wParam ) == 302 )
				bGotCmd = TRUE;
		}
		CHECK( bGotCmd, "popupcase key down skips disabled, return -> WM_COMMAND 302" );
		CHECK( !::IsWindow( hWndPop ), "popupcase return closes the popup" );

		::DestroyMenu( hMenu );
		CHECK( hm_wndpropid_disconnect( pRoot, hWndPop, 21 ) == true,
		       "popupcase disconnect" );
		::DestroyWindow( hWndOwner );
	}

	// ---- shutdown ( must simply complete ) ----
	hm_lib_shutdown( pRoot );
	++s_nPass; printf( "PASS shutdown\n" );

	printf( "HMSkinRegression: %d passed, %d failed\n", s_nPass, s_nFail );
	printf( "HMSkinRegression: %s\n", s_nFail ? "FAILED" : "ALL GREEN" );
	return s_nFail;
}
