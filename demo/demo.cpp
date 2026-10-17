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
#include "hm_layout_scale.h"
#include <vector>

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

// ---- tab chapters ( §1..§4 ) : keelobj groups switched by visibility ----
static const unsigned int arrChapterGroups[4][17] = {
	{ 3, 19, 10, 11, 29, 30, 31, 32, 33, 34, 35, 36, 48, 49, 50, 51, 0 },  // §1 基础交互（+ 原生接管按钮 + 8格状态矩阵 + TGL-10 开关 + RAD-11 单选组）
	{ 21, 22, 23, 24, 25, 26, 52, 53, 0, 0, 0, 0, 0, 0, 0, 0, 0 },           // §2 文本与数据（字阶标本 + PRG-12 进度条）
	{ 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                // §3 菜单系统
	{ 2, 28, 38, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },              // §4 滚动与容器（滚动井+内容行+滚动条）
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
	// 原生接管按钮（ §1 对照标本）随章节显隐。
	if ( g_hBtnTest1 )
		::ShowWindow( g_hBtnTest1, ( uChapterIndex == 0 ) ? SW_SHOW : SW_HIDE );
	::InvalidateRect( g_hWnd, NULL, FALSE );
}

// §3 : open the skinned popup menu for root menu item uRootItemIndex.
// creates a borderless popup window, connects it to the skin ( prop id 21 ->
// MsgSink 12 / keelobj K46 / hm_wmp_popupmenu ), then HMWM_BINDMENU binds the
// submenu HMENU - the WMP resizes the window to fit and draws the items.
void OpenSkinPopupMenu( unsigned int uRootItemIndex ) {
	// §3 menu-bar items: resolve the submenu and let the ENGINE popup take
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
	setWMPDisplayText( g_pWMPEvtLog, szBuf );
	// invalidate ONLY the event log panel: scrollbar dragging logs one event
	// per mousemove, and a full-window invalidate here made every control
	// repaint on each of those ( g_pWMPEvtLog already proves K18 exists ).
	hm_keel_obj* pKeelObjEvtLog = NULL;
	if ( hm_get_keelobj( g_pHMRoot, KEELOBJID_EVENTLOG, &pKeelObjEvtLog ) && pKeelObjEvtLog && pKeelObjEvtLog->getLayoutObj() )
		::InvalidateRect( g_hWnd, &pKeelObjEvtLog->getLayoutObj()->m_rect, FALSE );
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
	UNREFERENCED_PARAMETER(lpCmdLine);

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

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
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

   // tab chapters : §1 基础交互 is the default visible section.
   SwitchChapter( 0 );

   // §4 scroll view: the well ( K2 ) clips its children, the content rows
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

   // §1 RAD-11 radio group ( K49..K51 / WMP72..74, hm_wmp_radio ): GDI is
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

   ShowWindow( g_hWnd, nCmdShow );
   UpdateWindow( g_hWnd );


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
	//button: test1 — 原生控件换肤对照标本（ §1 基础交互）。
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