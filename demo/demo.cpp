// demo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "demo.h"
#include "hmskin_cpp.h"
#include "hmskin_event_listener_demo.h"
#include "hmskin_helper.h"
#include "win\hm_wndmsg_defs.h"
#include "win\wmp\hm_wmp_scrollbar.h"
#include "win\wmp\hm_wmp_checkbox.h"
#include "win\wmp\hm_wndmsgprocessor_i.h"
#include "win\wmp\hm_wmp_display.h"
#include "win\frame\hm_uiroot.h"
#include "win\frame\hm_debug_utils.h"
#include "win\paint_system\hm_ps_drawtool_gdi.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "win\paint_system\hm_ps_drawtool_d2d.h"
#include "hm_layout_scale.h"
#include <vector>
#include <exception>
#include <stdio.h>
#include <gdiplus.h>
#pragma comment( lib, "gdiplus.lib" )

//////////////////////////////////////////////////////////
#define MAINWND_WNDPROPID (258)
#define KEELOBJID_MAINBKGND (1)
#define KEELOBJID_OWNDRAWBTN (3)
#define KEELOBJID_MAINTAB (4)
#define WMPID_MAINBKGND (1)
#define WMPID_MAINBKGND_LAYOUT (2)
#define WMPID_MAINBKGND_DRAWTRIGGER (3)
#define WMPID_OWNDRAWBTN (4)
#define WMPID_MAINTAB (6)
#define WMPID_MAINTAB_LAYOUT (7)
#define WMPID_MAINTAB_DRAWTRIGGER (8)
#define RAWID_RESBMP_BTN_NOR (1)
#define RAWID_RESBMP_BTN_HOV (2)
#define RAWID_RESBMP_BTN_PRE (3)
#define RAWID_RESBMP_TABBTNS (4)
#define KEELOBJID_EVENTLOG (18)
#define WMPID_EVENTLOG (35)

//hm_rawres_pic_factory::obj_id_type RAWID_RESBMP_BTN_PRE;
bool createUI( HWND hWndParent );
void AppendEventLog( const TCHAR* szLine );
void SwitchChapter( unsigned int uChapterIndex );
hmskin_event_listener_demo g_HMSkinEventListener;
hm_uiroot* g_pHMRoot = NULL;
void* g_pWMPEvtLog = NULL;
//////////////////////////////////////////////////////////
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND g_hWnd;
HWND g_hBtnTest1 = NULL;
// "--dump-tree <path>" mode: dump the keelobj tree after the first frame
// settles and exit ( empty = interactive mode, no dump ).
TCHAR g_szDumpTreePath[ MAX_PATH ] = _T("");
TCHAR g_szAgentDumpPath[ MAX_PATH ] = _T("");   // WM_APP+3 target, set by the agent thread
TCHAR g_szAgentEvtText[ 1024 ];                 // WM_APP+4 fills, agent thread reads
volatile LONG g_nAgentEvtReady = 0;

// ---- tab chapters ( sec.1..sec.4 ) : keelobj groups switched by visibility ----
static const unsigned int arrChapterGroups[4][17] = {
	{ 3, 19, 10, 11, 29, 30, 31, 32, 33, 34, 35, 36, 48, 49, 50, 51, 0 },	// sec.1 basics (+ native-adoption button + 8-state checkbox grid)
	{ 21, 22, 23, 24, 25, 26, 52, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// sec.2 text & data ( type-scale specimens + PRG progress )
	{ 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// sec.3 menus
	{ 2, 28, 38, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },	// sec.4 scrolling & containers ( well + content rows + scrollbar )
};

void SwitchChapter( unsigned int uChapterIndex ) {
	if ( !g_pHMRoot || uChapterIndex >= 4 )
		return;
	for ( int nChapter = 0; nChapter < 4; ++nChapter ) {
		BOOL bVisible = ( nChapter == (int)uChapterIndex );
		for ( unsigned int i = 0; i < ( sizeof( arrChapterGroups[ 0 ] ) / sizeof( arrChapterGroups[ 0 ][ 0 ] ) ); ++i ) {
			unsigned int idKeelObj = arrChapterGroups[ nChapter ][ i ];
			if ( !idKeelObj )
				break;
			hm_keel_obj* pKeelObj = NULL;
			if ( hm_get_keelobj( g_pHMRoot, idKeelObj, &pKeelObj ) && pKeelObj )
				pKeelObj->setVisible( bVisible );
		}
	}
	// the native-adoption button ( sec.1 contrast specimen ) follows chapter visibility.
	if ( g_hBtnTest1 )
		::ShowWindow( g_hBtnTest1, ( uChapterIndex == 0 ) ? SW_SHOW : SW_HIDE );
	hm_util_invlog( _T("chapter"), NULL );
	::InvalidateRect( g_hWnd, NULL, FALSE );
}

// sec.3 : open the skinned popup menu for root menu item uRootItemIndex.
// creates a borderless popup window, connects it to the skin ( prop id 21 ->
// MsgSink 12 / keelobj K46 / hm_wmp_popupmenu ), then HMWM_BINDMENU binds the
// submenu HMENU - the WMP resizes the window to fit and draws the items.
void OpenSkinPopupMenu( unsigned int uRootItemIndex ) {
	// sec.3 menu-bar items: resolve the submenu and let the ENGINE popup take
	// over ( window/skin slot/command delivery are all engine-side now ).
	HMENU hRootMenu = ::GetMenu( g_hWnd );
	HMENU hSubMenu = ::GetSubMenu( hRootMenu, (int)uRootItemIndex );
	if ( !hSubMenu )
		return;
	RECT rcWnd;
	::GetWindowRect( g_hWnd, &rcWnd );
	hm_lib_popupmenu_show( g_pHMRoot, hSubMenu,
		rcWnd.left + hm_layout_scale_px( 20 + 5 + (int)uRootItemIndex * 60 ),
		rcWnd.top + hm_layout_scale_px( 80 ), g_hWnd );
}

// event log ( EVT-08 ) : prepend newest line, keep last 12, refresh the text WMP.
void AppendEventLog( const TCHAR* szLine ) {
	static std::vector< std::basic_string<TCHAR> > arrLines;
	static unsigned int uSeq = 0;
	if ( !g_pWMPEvtLog || !szLine )
		return;
	TCHAR szPrefixed[ 80 ];
	_stprintf_s( szPrefixed, 80, _T("%03d  %s"), ++uSeq, szLine );
	arrLines.insert( arrLines.begin(), std::basic_string<TCHAR>( szPrefixed ) );
	if ( arrLines.size() > 12 )
		arrLines.resize( 12 );
	std::basic_string<TCHAR> strAll;
	for ( size_t i = 0; i < arrLines.size(); ++i ) {
		if ( i ) strAll += _T("\n");
		strAll += arrLines[i];
	}
	TCHAR szBuf[ 1024 ];
	_tcsncpy_s( szBuf, 1024, strAll.c_str(), _TRUNCATE );
	hm_wmp_set_text( g_pHMRoot, KEELOBJID_EVENTLOG, WMPID_EVENTLOG, szBuf );
	// invalidate ONLY the event log panel: scrollbar dragging logs one event
	// per mousemove, and a full-window invalidate here made every control
	// repaint on each of those ( g_pWMPEvtLog already proves K18 exists ).
	hm_keel_obj* pKeelObjEvtLog = NULL;
	if ( hm_get_keelobj( g_pHMRoot, KEELOBJID_EVENTLOG, &pKeelObjEvtLog ) && pKeelObjEvtLog && pKeelObjEvtLog->getLayoutObj() ) {
		hm_util_invlog( _T("evtlog"), &pKeelObjEvtLog->getLayoutObj()->m_rect );
		::InvalidateRect( g_hWnd, &pKeelObjEvtLog->getLayoutObj()->m_rect, FALSE );
	}
}



// ============================================================
// AI-agent service thread ( v4 architecture, see tools/hmskin_mcp.py ):
// the ENGINE stays network-free - it only offers hmsh_* interfaces; the
// DEMO, as the host application, serves them over a local TCP endpoint
// ( 127.0.0.1:9321, one JSON request line in -> one JSON response line
// out ). the mac-side shell reaches it through an ssh -L tunnel, which
// sidesteps every logon-session / window-station wall.
// ops: ping | live | click <x,y> | shot | dump <path> | quit
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdarg.h>
#pragma comment( lib, "ws2_32.lib" )

#define HMSH_AGENT_PORT 9321

static void hmsh_agent_reply( SOCKET s, bool bOk, const char* szFmt, ... ) {
	// the wire protocol is pure ascii on purpose
	char szBody[ 512 ];
	va_list args;
	va_start( args, szFmt );
	vsprintf_s( szBody, 512, szFmt, args );
	va_end( args );
	char szLine[ 700 ];
	sprintf_s( szLine, 700, "{\"ok\": %s, \"data\": \"%s\"}\n", bOk ? "true" : "false", szBody );
	send( s, szLine, (int)strlen( szLine ), 0 );
}

static DWORD WINAPI hmsh_agent_thread( LPVOID ) {
	WSADATA wsa;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsa ) != 0 )
		return 1;
	SOCKET sListen = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( sListen == INVALID_SOCKET )
		return 1;
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons( HMSH_AGENT_PORT );
	InetPtonA( AF_INET, "127.0.0.1", &addr.sin_addr );
	if ( bind( sListen, ( sockaddr* )&addr, sizeof( addr ) ) != 0 || listen( sListen, 4 ) != 0 ) {
		closesocket( sListen );
		return 1;
	}
	for ( ; ; ) {
		SOCKET s = accept( sListen, NULL, NULL );
		if ( s == INVALID_SOCKET )
			continue;
		char szReq[ 1024 ] = { 0 };
		int n = recv( s, szReq, sizeof( szReq ) - 1, 0 );
		if ( n <= 0 ) { closesocket( s ); continue; }
		// parse: "op arg1 arg2" ( space separated, deliberately dumb )
		char szOp[ 32 ] = { 0 }, szA1[ MAX_PATH ] = { 0 };
		sscanf_s( szReq, "%31s %511s", szOp, (unsigned)sizeof( szOp ), szA1, (unsigned)sizeof( szA1 ) );
		if ( 0 == strcmp( szOp, "ping" ) ) {
			hmsh_agent_reply( s, true, "pong" );
		} else if ( 0 == strcmp( szOp, "quit" ) ) {
			hmsh_agent_reply( s, true, "bye" );
			closesocket( s );
			closesocket( sListen );
			::PostMessage( g_hWnd, WM_CLOSE, 0, 0 );
			return 0;
		} else if ( 0 == strcmp( szOp, "dump" ) ) {
			// narrow -> TCHAR into the global, then let the UI thread do the
			// walk ( WM_APP+3 ) - tree reads stay on the window thread
			size_t nConv = 0;
			mbstowcs_s( &nConv, g_szAgentDumpPath, MAX_PATH, szA1, _TRUNCATE );
			if ( g_szAgentDumpPath[ 0 ] ) {
				::PostMessage( g_hWnd, WM_APP + 3, 0, 0 );
				hmsh_agent_reply( s, true, "dumping" );
			} else {
				hmsh_agent_reply( s, false, "dump needs a path" );
			}
		} else if ( 0 == strcmp( szOp, "live" ) ) {
			::PostMessage( g_hWnd, WM_APP + 1, 0, 0 );   // async: poll the file
			hmsh_agent_reply( s, true, "hmskin_tree.live.json" );
		} else if ( 0 == strcmp( szOp, "shot" ) ) {
			::PostMessage( g_hWnd, WM_APP + 2, 0, 0 );
			hmsh_agent_reply( s, true, "_mcp_shot.png" );
		} else if ( 0 == strcmp( szOp, "shotscreen" ) ) {
			// desktop capture ( WM_APP+5 ): PrintWindow can serve a stale
			// redirection-surface frame after interaction direct draws -
			// the composed desktop cannot lie. e2e pixel diffs use this.
			::PostMessage( g_hWnd, WM_APP + 5, 0, 0 );
			hmsh_agent_reply( s, true, "_mcp_shot2.png" );
		} else if ( 0 == strcmp( szOp, "raise" ) ) {
			// topmost-toggle trick: reliably lifts the window above whatever
			// covers it ( e.g. the console running the tests ) without
			// needing foreground-activation rights
			::SetWindowPos( g_hWnd, HWND_TOPMOST, 0, 0, 0, 0,
			                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
			::SetWindowPos( g_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
			                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
			hmsh_agent_reply( s, true, "raised" );
		} else if ( 0 == strcmp( szOp, "lower" ) ) {
			// e2e epilogue: give the borrowed Z-order slot back ( sink below
			// the windows that covered the demo before the test raised it )
			::SetWindowPos( g_hWnd, HWND_BOTTOM, 0, 0, 0, 0,
			                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
			hmsh_agent_reply( s, true, "lowered" );
		} else if ( 0 == strcmp( szOp, "click" ) ) {
			// szA1 = "x,y" in CLIENT coords; posted from our own thread the
			// messages queue like real input
			int x = 0, y = 0;
			char szX[ 64 ] = { 0 };
			if ( sscanf_s( szA1, "%63[^,],%d", szX, (unsigned)sizeof( szX ), &y ) == 2 ) {
				x = atoi( szX );
				LPARAM lp = x | ( y << 16 );
				::PostMessage( g_hWnd, WM_LBUTTONDOWN, MK_LBUTTON, lp );
				Sleep( 60 );
				::PostMessage( g_hWnd, WM_LBUTTONUP, 0, lp );
				hmsh_agent_reply( s, true, "clicked %d,%d", x, y );
			} else {
				hmsh_agent_reply( s, false, "bad click args" );
			}
		} else if ( 0 == strcmp( szOp, "wheel" ) ) {
			// szA1 = "x,y,delta" in CLIENT coords ( delta in wheel notches,
			// e.g. -3 for three steps down ) - same synthetic-post pattern
			// as click; WM_MOUSEWHEEL carries the point in SCREEN coords,
			// so translate before posting
			int x = 0, y = 0, nDelta = 0;
			char szX[ 64 ] = { 0 };
			if ( sscanf_s( szA1, "%63[^,],%d,%d", szX, (unsigned)sizeof( szX ), &y, &nDelta ) == 3 ) {
				x = atoi( szX );
				POINT pt = { x, y };
				::ClientToScreen( g_hWnd, &pt );
				LPARAM lp = pt.x | ( pt.y << 16 );
				WPARAM wp = MAKEWPARAM( 0, (SHORT)( nDelta * WHEEL_DELTA ) );
				::PostMessage( g_hWnd, WM_MOUSEWHEEL, wp, lp );
				hmsh_agent_reply( s, true, "wheel %d,%d delta %d", x, y, nDelta );
			} else {
				hmsh_agent_reply( s, false, "bad wheel args" );
			}
		} else if ( 0 == strcmp( szOp, "ncclick" ) ) {
			// NC areas route through the real-input hook chain ( WH_MOUSE ),
			// not the subclass path - posted NC messages never reach the
			// rootmenu. so: real input. szA1 = "x,y" in CLIENT coords.
			int x = 0, y = 0;
			char szX[ 64 ] = { 0 };
			if ( sscanf_s( szA1, "%63[^,],%d", szX, (unsigned)sizeof( szX ), &y ) == 2 ) {
				x = atoi( szX );
				POINT pt = { x, y };
				::ClientToScreen( g_hWnd, &pt );
				::SetCursorPos( pt.x, pt.y );
				Sleep( 80 );
				INPUT inp[ 2 ] = { 0 };
				inp[ 0 ].type = INPUT_MOUSE;
				inp[ 0 ].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				inp[ 1 ].type = INPUT_MOUSE;
				inp[ 1 ].mi.dwFlags = MOUSEEVENTF_LEFTUP;
				::SendInput( 2, inp, sizeof( INPUT ) );
				hmsh_agent_reply( s, true, "ncclicked %d,%d", x, y );
			} else {
				hmsh_agent_reply( s, false, "bad ncclick args" );
			}
		} else if ( 0 == strcmp( szOp, "evtlog" ) ) {
			// UI-thread snapshot of the EVT log text ( WM_APP+4 ), utf-8 wire.
			// the protocol is ONE json line: raw newlines inside the text
			// would break json parsing - emit backslash-n two-char escapes
			::InterlockedExchange( &g_nAgentEvtReady, 0 );
			g_szAgentEvtText[ 0 ] = _T('\0');
			::PostMessage( g_hWnd, WM_APP + 4, 0, 0 );
			for ( int nWait = 0; nWait < 50 && !g_nAgentEvtReady; ++nWait )
				Sleep( 20 );
			char szUtf8[ 3072 ] = { 0 };
			WideCharToMultiByte( CP_UTF8, 0, g_szAgentEvtText, -1, szUtf8, sizeof( szUtf8 ), NULL, NULL );
			char szEsc[ 6144 ] = { 0 };
			size_t nOut = 0;
			for ( size_t i = 0; szUtf8[ i ] && nOut < sizeof( szEsc ) - 2; ++i ) {
				if ( szUtf8[ i ] == '\n' ) {
					szEsc[ nOut++ ] = '\\';
					szEsc[ nOut++ ] = 'n';
				} else if ( szUtf8[ i ] != '\r' ) {
					szEsc[ nOut++ ] = szUtf8[ i ];
				}
			}
			hmsh_agent_reply( s, true, "%s", szEsc );
		} else {
			hmsh_agent_reply( s, false, "unknown op" );
		}
		closesocket( s );
	}
}

static void hmsh_agent_start() {
	CreateThread( NULL, 0, hmsh_agent_thread, NULL, 0, NULL );
}

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	// DPI ( M1-1 ): declare awareness so DWM stops bitmap-stretching the
	// window on scaled displays; the skin lays out in physical pixels via
	// hm_startup_param::m_uDpi ( manifests stay 96dpi-authored ).
	// resolved dynamically: the header declarations are NTDDI-gated and
	// host apps may target older toolsets/SDKs.
	UINT uDpi = 96;
	{
		typedef BOOL (WINAPI *FN_SetDpiAwarenessCtx)( void* );
		typedef UINT (WINAPI *FN_GetDpiForSystem)( void );
		HMODULE hUser32 = ::GetModuleHandle( _T("user32.dll") );
		if ( hUser32 ) {
			FN_SetDpiAwarenessCtx pfnSetCtx = (FN_SetDpiAwarenessCtx)::GetProcAddress( hUser32, "SetProcessDpiAwarenessContext" );
			FN_GetDpiForSystem pfnGetDpi = (FN_GetDpiForSystem)::GetProcAddress( hUser32, "GetDpiForSystem" );
			if ( pfnSetCtx )
				pfnSetCtx( (void*)(INT_PTR)-4 );   // DPI_AWARENESS_CONTEXT_SYSTEM_AWARE
			if ( pfnGetDpi ) {
				UINT uDpiSys = pfnGetDpi();
				if ( uDpiSys )
					uDpi = uDpiSys;
			}
		}
	}
	MSG msg;
	HACCEL hAccelTable;
	hm_startup_param paramHMStartup( hInstance, &g_HMSkinEventListener, uDpi );
	TCHAR szCurModulePath[ MAX_PATH +1 ];
	DWORD dwRet;
	TCHAR* posT;
	std::basic_string< TCHAR > strModuleDirPath;
	std::basic_string< TCHAR > strUIManifest;

	dwRet = ::GetModuleFileName( NULL, szCurModulePath, MAX_PATH );
	if ( dwRet == 0 )
		return FALSE;
	posT = _tcsrchr( szCurModulePath, _T('\\') );
	posT+=1;
	*posT = _T('\0');
	strModuleDirPath = szCurModulePath;
	strUIManifest = strModuleDirPath + _T("demo.hm.manifest");

	//start up hm lib
	hm_lib_startup( &paramHMStartup, strUIManifest.c_str(), &g_pHMRoot );

	// "--drawtool D2D|Gdiplus|Gdi" - runtime backend override for A/B perf
	// runs ( tools/smoke_test.py --perf ). swaps the draw tool of every
	// display WMP after the manifest loaded; resource refcounts of both
	// backends are forwarding-compatible so no reload is needed.
	{
		const TCHAR* posArg = _tcsstr( lpCmdLine, _T("--drawtool") );
		if ( posArg && g_pHMRoot ) {
			posArg += _tcslen( _T("--drawtool") );
			while ( *posArg == _T(' ') )
				posArg++;
			hm_ps_drawtool_i* pTool = NULL;
			if ( 0 == _tcsnicmp( posArg, _T("D2D"), 3 ) )
				pTool = hm_ps_drawtool_d2d::getInstance();
			else if ( 0 == _tcsnicmp( posArg, _T("Gdiplus"), 7 ) )
				pTool = hm_ps_drawtool_gdiplus::getInstance();
			else if ( 0 == _tcsnicmp( posArg, _T("Gdi"), 3 ) )
				pTool = hm_ps_drawtool_gdi::getInstance();
			if ( pTool && g_pHMRoot->m_pWMPFactory ) {
				struct OverrideCtx { hm_ps_drawtool_i* pTool; } ctx = { pTool };
				g_pHMRoot->m_pWMPFactory->enumObjs(
				    []( hm_wndmsgprocessor_i* pWMP, void* pParam ) {
					    hm_wmp_display* pDisp = dynamic_cast< hm_wmp_display* >( pWMP );
					    if ( pDisp )
						    pDisp->setDrawTool( static_cast< OverrideCtx* >( pParam )->pTool );
				    }, &ctx );
			}
		}
	}

	// "--dump-tree <path>" - AI-agent tree-dump mode ( smoke --tree / hmskin_mcp ):
	// dump the keelobj tree after the first frame and exit. the same mode can
	// also be requested WITHOUT command-line args through a convention file
	// ( exe-dir "_dump_req", first line = output path ) - the shell launches
	// demo via explorer.exe which cannot pass arguments.
	{
		const TCHAR* posDump = _tcsstr( lpCmdLine, _T("--dump-tree") );
		if ( posDump ) {
			posDump += _tcslen( _T("--dump-tree") );
			while ( *posDump == _T(' ') || *posDump == _T('\"') )
				posDump++;
			int i = 0;
			while ( posDump[ i ] && posDump[ i ] != _T(' ') && posDump[ i ] != _T('\"') && i < MAX_PATH - 1 ) {
				g_szDumpTreePath[ i ] = posDump[ i ];
				i++;
			}
			g_szDumpTreePath[ i ] = _T('\0');
		}
		if ( !g_szDumpTreePath[ 0 ] ) {
			// convention-file variant: exe-dir "_dump_req" ( first line =
			// output path ), consumed and deleted. lets the explorer.exe
			// launch path request a dump ( explorer cannot pass args ).
			TCHAR szReq[ MAX_PATH + 32 ];
			if ( ::GetModuleFileName( NULL, szReq, MAX_PATH ) ) {
				TCHAR* posSlash = _tcsrchr( szReq, _T('\\') );
				if ( posSlash ) {
					*( posSlash + 1 ) = _T('\0');
					_tcscat_s( szReq, MAX_PATH + 32, _T("_dump_req") );
					FILE* fpReq = _tfopen( szReq, _T("r") );
					if ( fpReq ) {
						TCHAR szLine[ MAX_PATH ];
						if ( _fgetts( szLine, MAX_PATH, fpReq ) ) {
							TCHAR* posNl = _tcschr( szLine, _T('\n') );
							if ( posNl ) *posNl = _T('\0');
							TCHAR* posNl2 = _tcschr( szLine, _T('\r') );
							if ( posNl2 ) *posNl2 = _T('\0');
							if ( szLine[ 0 ] )
								_tcsncpy_s( g_szDumpTreePath, MAX_PATH, szLine, _TRUNCATE );
						}
						fclose( fpReq );
						::DeleteFile( szReq );
					}
				}
			}
		}
	}

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEMO));

	// engine throws ( e.g. refcount underflow ) escaping a WndProc would hit
	// std::terminate with a bare "abort()" dialog. catch them at the pump and
	// show the reason instead - much easier to diagnose.
	try {
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	}
	catch ( const std::exception& e ) {
		::MessageBoxA( NULL, e.what(), "demo: uncaught engine exception", MB_ICONERROR );
	}
	catch ( ... ) {
		::MessageBox( NULL, _T("unknown exception"), _T("demo: uncaught engine exception"), MB_ICONERROR );
	}

	//
 	hm_lib_shutdown( g_pHMRoot );

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEMO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DEMO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

	//	
   ////
   //hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
   //   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   // benchmark-bench layout: 1100x750, centered on screen ( see DESIGN.md )
   {
	   int nScrW = ::GetSystemMetrics( SM_CXSCREEN );
	   int nScrH = ::GetSystemMetrics( SM_CYSCREEN );
	   int nWndW = hm_layout_scale_px( 1100 );
	   int nWndH = hm_layout_scale_px( 750 );
	   g_hWnd = CreateWindow( szWindowClass, _T("hmskin Gallery — 皮肤基准台"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		   ( nScrW - nWndW ) / 2, ( nScrH - nWndH ) / 2, nWndW, nWndH, NULL, NULL, hInstance, NULL);
   }

   if ( !g_hWnd )
   {
      return FALSE;
   }

   ::createUI( g_hWnd );

   //
   hm_wndpropid_connect( g_pHMRoot, g_hWnd, MAINWND_WNDPROPID );

   // transparent native-popup skinning: right-click TrackPopupMenu calls are
   // intercepted and handled ENGINE-side ( PopupSkin slot + WM_COMMAND back
   // to us ) - this demo carries zero popup code of its own.
   hm_lib_enable_trackpopupmenu_hook( g_pHMRoot, TRUE );

   // event log ( EVT-08 ) : fetch the text WMP of keelobj 18 for runtime updates.
   {
	   hm_keel_obj* pKeelObjEvtLog = NULL;
	   if ( hm_get_keelobj( g_pHMRoot, KEELOBJID_EVENTLOG, &pKeelObjEvtLog ) && pKeelObjEvtLog )
		   g_pWMPEvtLog = pKeelObjEvtLog->getWMP( WMPID_EVENTLOG );
   }

   // tab chapters : sec.1 basics is the default visible section.
   SwitchChapter( 0 );

   // sec.4 scroll view: the well ( K2 ) clips its children, the content rows
   // ( K38 ) are taller than the well, the scrollbar range spans exactly
   // the overflow ( content height - well height ). thumb / arrow thicknesses
   // must equal the sbthumb_* / sbbtn_* bitmap dimensions ( gen_skin.py ).
   {
	   hm_keel_obj* pKeelObjWell = NULL;
	   hm_keel_obj* pKeelObjContent = NULL;
	   hm_keel_obj* pKeelObjScrollBar = NULL;
	   if ( hm_get_keelobj( g_pHMRoot, 2, &pKeelObjWell ) && pKeelObjWell )
		   pKeelObjWell->setClipChildren( TRUE );
	   if ( hm_get_keelobj( g_pHMRoot, 47, &pKeelObjScrollBar ) && pKeelObjScrollBar ) {
		   hm_wmp_scrollbar* pWMPScrollBar = dynamic_cast< hm_wmp_scrollbar* >( pKeelObjScrollBar->getWMP( 70 ) );
		   if ( pWMPScrollBar ) {
			   pWMPScrollBar->setStartThumbLen( hm_layout_scale_px( 20 ) );
			   pWMPScrollBar->setEndThumbLen( hm_layout_scale_px( 20 ) );
			   pWMPScrollBar->setTrackThumbLen( hm_layout_scale_px( 48 ) );
			   int nRange = 0;
			   if ( hm_get_keelobj( g_pHMRoot, 38, &pKeelObjContent ) && pKeelObjContent
				   && pKeelObjContent->getLayoutObj() && pKeelObjWell->getLayoutObj() ) {
				   RECT rcWell = pKeelObjWell->getLayoutObj()->m_rect;
				   RECT rcContent = pKeelObjContent->getLayoutObj()->m_rect;
				   nRange = ( rcContent.bottom - rcContent.top ) - ( rcWell.bottom - rcWell.top );
				   if ( nRange < 0 )
					   nRange = 0;
			   }
			   pWMPScrollBar->setValRange( 0, nRange );
		   }
	   }
   }

   // sec.1 RAD-11 radio group ( K49..K51 / WMP72..74, hm_wmp_radio ): GDI is
   // the default selection. stay-checked + RADIO_CLICK event are baked into
   // the radio class; sibling unchecking is driven by the event listener.
   {
	   hm_keel_obj* pKeelObjRadio = NULL;
	   if ( hm_get_keelobj( g_pHMRoot, 49, &pKeelObjRadio ) && pKeelObjRadio ) {
		   hm_wmp_checkbox* pWMPRadio = dynamic_cast< hm_wmp_checkbox* >( pKeelObjRadio->getWMP( 72 ) );
		   if ( pWMPRadio )
			   pWMPRadio->setCheck( TRUE );
	   }
   }

   // statusbar RAIL: live engine capability readout ( DESIGN.md ).
   {
	   // read the LIVE dpi via the same dynamic resolve as WinMain
	   UINT uRailDpi = 96;
	   {
		   typedef UINT (WINAPI *FN_GetDpiForSystem)( void );
		   HMODULE hUser32 = ::GetModuleHandle( _T("user32.dll") );
		   if ( hUser32 ) {
			   FN_GetDpiForSystem pfnGetDpi = (FN_GetDpiForSystem)::GetProcAddress( hUser32, "GetDpiForSystem" );
			   if ( pfnGetDpi && pfnGetDpi() )
				   uRailDpi = pfnGetDpi();
		   }
	   }
	   TCHAR szRail[ 128 ];
	   _stprintf_s( szRail, 128, _T("● LIVE   9-PATCH · 3-SEG · ALPHA · D2D · DPI %u"), uRailDpi );
	   hm_wmp_set_text( g_pHMRoot, 16, 37, szRail );
   }

   ShowWindow( g_hWnd, nCmdShow );
   UpdateWindow( g_hWnd );

   hmsh_agent_start();

   // AI-agent hooks, armed HERE and not inside WM_PAINT: the skin's
   // dispatcher routes registered messages ( WM_PAINT belongs to the
   // drawtrigger WMP ), so the demo's own wndproc never sees a WM_PAINT -
   // a dump trigger placed there would silently never fire.
   if ( g_szDumpTreePath[ 0 ] ) {
	   SetTimer( g_hWnd, 1, 600, NULL );   // one-shot dump mode, fires via WM_TIMER
   } else {
	   // every launch publishes ONE initial live-tree dump ( with the hwnd )
	   // so the shell can discover the handle without any interaction.
	   // direct call ( not PostMessage ): the queue hop proved unreliable
	   // here, and the layout is already settled at this point anyway.
	   TCHAR szLive0[ MAX_PATH + 48 ];
	   if ( ::GetModuleFileName( NULL, szLive0, MAX_PATH ) ) {
		   TCHAR* posSlash0 = _tcsrchr( szLive0, _T('\\') );
		   if ( posSlash0 ) {
			   *( posSlash0 + 1 ) = _T('\0');
			   _tcscat_s( szLive0, MAX_PATH + 48, _T("hmskin_tree.live.json") );
			   hmsh_dump_tree( g_pHMRoot, g_hWnd, szLive0 );
		   }
	   }
   }

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_RBUTTONDOWN:
		{	
			POINT ptCursor;
			MENUITEMINFO menuInfo;
			HMENU hPopMenu;
			
			::GetCursorPos( &ptCursor );
			hPopMenu = ::CreatePopupMenu();
			menuInfo.cbSize = sizeof( MENUITEMINFO );
			menuInfo.fMask = MIIM_STRING | MIIM_ID;   // without MIIM_ID InsertMenuItem ignores wID ( item id stays 0 = silent close )
			menuInfo.fType = MFT_STRING;
			menuInfo.dwTypeData = _T("rom sub menu1");
			menuInfo.wID = 208;   // transparent-hook specimen: id arrives via native WM_COMMAND
			::InsertMenuItem( hPopMenu, 0, TRUE, &menuInfo );
			::TrackPopupMenu( hPopMenu, TPM_LEFTALIGN|TPM_TOPALIGN, ptCursor.x, ptCursor.y, 0, hWnd, NULL );
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case 208:
			// right-click popup item ( transparent TrackPopupMenu hook ):
			// proof the engine delivers selections as native WM_COMMAND.
			AppendEventLog( _T("CTX-MENU 208 via WM_COMMAND") );
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_ERASEBKGND:
		// the skin paints the whole client area - skipping the class-brush
		// erase kills the white flash on repaints ( e.g. scrollbar dragging ).
		return 1;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		// dump-tree mode: one shot ( with a single retry when the tree came
		// out empty - e.g. the timer beat the skin loading ), then exit.
		if ( wParam == 1 && g_szDumpTreePath[ 0 ] ) {
			KillTimer( hWnd, 1 );
			static int nDumpAttempts = 0;
			unsigned int nNodes = hmsh_dump_tree( g_pHMRoot, hWnd, g_szDumpTreePath );
			nDumpAttempts++;
			if ( nNodes > 0 || nDumpAttempts >= 2 ) {
				PostQuitMessage( 0 );
			} else {
				SetTimer( hWnd, 1, 600, NULL );
			}
		}
		break;
	case ( WM_APP + 1 ): {
		// AI-agent live-tree channel ( tools/hmskin_mcp.py ): dump the CURRENT
		// tree state ( visibility after chapter switches etc. ) next to the exe
		// without exiting. the shell polls the file mtime.
		TCHAR szLive[ MAX_PATH + 48 ];
		if ( ::GetModuleFileName( NULL, szLive, MAX_PATH ) ) {
			TCHAR* posSlash = _tcsrchr( szLive, _T('\\') );
			if ( posSlash ) {
				*( posSlash + 1 ) = _T('\0');
				_tcscat_s( szLive, MAX_PATH + 48, _T("hmskin_tree.live.json") );
				hmsh_dump_tree( g_pHMRoot, hWnd, szLive );
			}
		}
		break;
	}
	case ( WM_APP + 2 ): {
		// AI-agent self-screenshot: PrintWindow OURSELVES into _mcp_shot.png
		// next to the exe ( GDI capture from another logon session is
		// unreliable - doing it in-session is exact ). triggered cross-session
		// via PostMessage, result consumed by file polling.
		TCHAR szShot[ MAX_PATH + 48 ];
		if ( ::GetModuleFileName( NULL, szShot, MAX_PATH ) ) {
			TCHAR* posSlash = _tcsrchr( szShot, _T('\\') );
			if ( posSlash ) {
				*( posSlash + 1 ) = _T('\0');
				_tcscat_s( szShot, MAX_PATH + 48, _T("_mcp_shot.png") );
				RECT rcWnd;
				::GetWindowRect( hWnd, &rcWnd );
				int nW = rcWnd.right - rcWnd.left;
				int nH = rcWnd.bottom - rcWnd.top;
				if ( nW > 0 && nH > 0 ) {
					HDC hDCMem = ::CreateCompatibleDC( NULL );
					HBITMAP hBmp = ::CreateCompatibleBitmap( ::GetDC( hWnd ), nW, nH );
					HGDIOBJ hOld = ::SelectObject( hDCMem, hBmp );
					::PrintWindow( hWnd, hDCMem, 2 /* PW_RENDERFULLCONTENT, NTDDI-gated in the SDK headers */ );
					::SelectObject( hDCMem, hOld );
					// save 32bpp BGRA as png via GDI+ ( already linked by the skin dll )
					Gdiplus::GdiplusStartupInput gdiplusStartupInput;
					ULONG_PTR ulGdiplusToken = 0;
					if ( Gdiplus::GdiplusStartup( &ulGdiplusToken, &gdiplusStartupInput, NULL ) == Gdiplus::Ok ) {
						{
							Gdiplus::Bitmap bmp( hBmp, NULL );
							CLSID clsidPng = { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e } };
							bmp.Save( szShot, &clsidPng, NULL );
						}
						Gdiplus::GdiplusShutdown( ulGdiplusToken );
					}
					::DeleteObject( hBmp );
					::DeleteDC( hDCMem );
				}
			}
		}
		break;
	}
	case ( WM_APP + 3 ): {
		// agent-requested one-shot dump to g_szAgentDumpPath ( stays alive,
		// unlike the --dump-tree boot mode which exits after dumping ).
		if ( g_szAgentDumpPath[ 0 ] ) {
			hmsh_dump_tree( g_pHMRoot, hWnd, g_szAgentDumpPath );
			g_szAgentDumpPath[ 0 ] = _T('\0');
		}
		break;
	}
	case ( WM_APP + 4 ): {
		// agent evtlog read: copy the EVT log text on the window thread
		if ( g_pWMPEvtLog ) {
			getWMPDisplayText( g_pWMPEvtLog, g_szAgentEvtText, 1024 );
			::InterlockedExchange( &g_nAgentEvtReady, 1 );
		}
		break;
	}
	case ( WM_APP + 5 ): {
		// agent desktop capture: BitBlt the window rect off the real screen
		// into _mcp_shot2.png ( see the "shotscreen" op for the rationale )
		RECT rcWnd;
		if ( ::GetWindowRect( hWnd, &rcWnd ) ) {
			int nW = rcWnd.right - rcWnd.left;
			int nH = rcWnd.bottom - rcWnd.top;
			HDC hScreen = ::GetDC( NULL );
			HDC hDCMem = ::CreateCompatibleDC( hScreen );
			HBITMAP hBmp = ::CreateCompatibleBitmap( hScreen, nW, nH );
			HGDIOBJ hOld = ::SelectObject( hDCMem, hBmp );
			::BitBlt( hDCMem, 0, 0, nW, nH, hScreen, rcWnd.left, rcWnd.top, SRCCOPY );
			::SelectObject( hDCMem, hOld );
			TCHAR szShot[ MAX_PATH + 48 ];
			if ( ::GetModuleFileName( NULL, szShot, MAX_PATH ) ) {
				TCHAR* posSlash = _tcsrchr( szShot, _T('\\') );
				if ( posSlash ) {
					*( posSlash + 1 ) = _T('\0');
					_tcscat_s( szShot, MAX_PATH + 48, _T("_mcp_shot2.png") );
					Gdiplus::GdiplusStartupInput gdiplusStartupInput;
					ULONG_PTR ulGdiplusToken = 0;
					if ( Gdiplus::GdiplusStartup( &ulGdiplusToken, &gdiplusStartupInput, NULL ) == Gdiplus::Ok ) {
						{
							Gdiplus::Bitmap bmp( hBmp, NULL );
							CLSID clsidPng = { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e } };
							bmp.Save( szShot, &clsidPng, NULL );
						}
						Gdiplus::GdiplusShutdown( ulGdiplusToken );
					}
				}
			}
			::DeleteObject( hBmp );
			::DeleteDC( hDCMem );
			::ReleaseDC( NULL, hScreen );
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


bool createUI( HWND hWndParent ) {
//button: test1 - contrast specimen: a native control adopted by the skin ( sec.1 ).
	// CLIENT coords ( not K12-relative ): the skin button row sits at client
	// y=152 ( K12 top 48 + space2 104 ), K19 ends at client x=352, so this
	// lands 16px to its right. 150x42 = bitmap native size, 1:1.
	{
		DWORD dwCtrlID = 20;
		g_hBtnTest1 = ::CreateWindow( _T( "Button" ), _T( "接管"), WS_VISIBLE | WS_CHILD,
			hm_layout_scale_px( 368 ), hm_layout_scale_px( 152 ), hm_layout_scale_px( 150 ), hm_layout_scale_px( 42 ), hWndParent, ( HMENU )dwCtrlID, NULL, NULL );
		if ( !g_hBtnTest1 ) {
			::OutputDebugString( _T("hBtnTest1 == NULL!\n") );
			return false;
		}
	}


	return true;
}