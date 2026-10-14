#include "stdafx.h"
#include "win\hook_system\hm_hs_mouse_proc_hook.h"
#include "win\hook_system\hm_hs_whellengine.h"
#include <deque>
#include <map>
#include "tchar.h"
///////////////////////////////////////////
static HHOOK s_hMHook = NULL;
LRESULT CALLBACK HMLibMouseHookProc( int nCode, WPARAM wParam, LPARAM lParam );
///////////////////////////////////////////
static bool bMWhellForward = true;
static HHOOK s_hMLLHook = NULL;
LRESULT CALLBACK HMLibMouseLowLevelMouseProc( int nCode, WPARAM wParam, LPARAM lParam );
///////////////////////////////////////////
bool hs_mouse_proc_hook_start( HINSTANCE hInstance )
{
	if ( !s_hMHook )
	{
		s_hMHook = ::SetWindowsHookEx( WH_MOUSE, HMLibMouseHookProc, (HINSTANCE)hInstance, ::GetCurrentThreadId() );
	}
	else
		return true;

	return s_hMHook != NULL;
}

void hs_mouse_proc_hook_stop()
{
	if ( s_hMHook )
	{
		if ( ::UnhookWindowsHookEx( s_hMHook ) )
			s_hMHook = NULL;
	}
}

LRESULT CALLBACK HMLibMouseHookProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	static std::map< HWND, BOOL > s_mapHwnd2CaptionDown;

	if(nCode == HC_ACTION)
	{
		// get a pointer to the mouse hook struct.
		PMOUSEHOOKSTRUCT mhs = (PMOUSEHOOKSTRUCT) lParam;

		// intercept messages for left button down and up
		switch(wParam)
		{
		case WM_NCLBUTTONDOWN:
			{
				//::OutputDebugString(_T("WM_NCLBUTTONDOWN.\n"));
				// if the message is from your window and
				// the hit test indicates title bar
				if( mhs->wHitTestCode == HTCAPTION ) {
					s_mapHwnd2CaptionDown[ mhs->hwnd ] = TRUE;
					::OutputDebugString( _T("[hook] rec nclbtndown\n") );
				}
			}
			break;
		case WM_NCLBUTTONDBLCLK:
			{
				//::OutputDebugString(_T("WM_NCLBUTTONDBLCLK.\n"));
				// if the message is from your window and
				// the hit test indicates title bar
				if( mhs->wHitTestCode == HTCAPTION )
					s_mapHwnd2CaptionDown[ mhs->hwnd ] = TRUE;
			}
			break;
		case WM_NCLBUTTONUP:
			{
				//::OutputDebugString(_T("WM_NCLBUTTONUP.\n"));
				std::map< HWND, BOOL >::iterator it;

				it = s_mapHwnd2CaptionDown.find( mhs->hwnd );
				if ( it != s_mapHwnd2CaptionDown.end() )
					s_mapHwnd2CaptionDown.erase( it );
			}
			break;

		case WM_LBUTTONUP:
			{
				//::OutputDebugString(_T("WM_LBUTTONUP.\n"));
				std::map< HWND, BOOL >::iterator it;

				it = s_mapHwnd2CaptionDown.find( mhs->hwnd );
				if( it != s_mapHwnd2CaptionDown.end() )
				{
					s_mapHwnd2CaptionDown.erase( it );
					::OutputDebugString( _T("[hook] synth nclbtnup\n") );
					::PostMessage( mhs->hwnd, WM_NCLBUTTONUP, HTCAPTION, MAKELONG(mhs->pt.x,mhs->pt.y) );
				}
			}
			break;
		case WM_MOUSEWHEEL:
			{	
				//::OutputDebugString( _T("WM_MOUSEWHEEL in HMLibMouseHookProc\n") );
				WHELLTAGDATA tagData;
				MOUSEHOOKSTRUCT* stMouseHook = (MOUSEHOOKSTRUCT*)lParam;
				
				memset( &tagData, 0, sizeof( tagData ) );
				tagData.m_hWnd = stMouseHook->hwnd;
				tagData.m_pt = stMouseHook->pt;
				tagData.m_uTime = ::GetTickCount();
				tagData.m_uFlag |= ( bMWhellForward ? WTDFLAG_WHEEL_FORWORD : 0 ); 
				WHELLENGINE_INSTANCE->tag( tagData );
				
				static std::deque< std::basic_string< TCHAR > > containerSpan;
				static DWORD s_dwLastTime = 0;
				DWORD dwCurTime;
				BOOL bFirst;
				
				bFirst = (s_dwLastTime == 0);
				dwCurTime = ::GetTickCount();

				if ( bFirst ) {
					s_dwLastTime = dwCurTime;
					break;
				} else {
					TCHAR szMsg[100];

					_stprintf_s( szMsg, _T("WM_MOUSEWHEEL: bMWhellForward:%d, %d ms\n"), bMWhellForward, dwCurTime - s_dwLastTime );	
					::OutputDebugString( szMsg );
				//	containerSpan.push_back( szMsg );
					s_dwLastTime = dwCurTime;

					//if ( containerSpan.size() >= 100 ) {
					//	std::deque< std::basic_string< TCHAR > >::iterator itTime, iendTime;
					//	iendTime = containerSpan.end();
					//	for ( itTime = containerSpan.begin(); itTime!=iendTime; ++itTime ) {
					//		::OutputDebugString( itTime->c_str() );							
					//	}
					//	
					//	containerSpan.clear();
					//}
				}
				
			}
			break;
		default:
			break;
		}
	}
	// let the messages through to the next hook
	return ::CallNextHookEx( s_hMHook, nCode, wParam, lParam );

}

bool hs_mouse_ll_proc_hook_start( HINSTANCE hInstance ) {
	if ( !s_hMLLHook )
	{
		s_hMLLHook = ::SetWindowsHookEx( WH_MOUSE_LL, HMLibMouseLowLevelMouseProc, (HINSTANCE)hInstance, 0 );
	}
	else
		return true;

	return s_hMLLHook != NULL;
}

void hs_mouse_ll_proc_hook_stop() {
	if ( s_hMLLHook )
	{
		if ( ::UnhookWindowsHookEx( s_hMLLHook ) )
			s_hMLLHook = NULL;
	}
}

LRESULT CALLBACK HMLibMouseLowLevelMouseProc( int nCode, WPARAM wParam, LPARAM lParam ) {
	
	if ( nCode == HC_ACTION ) {
		PMSLLHOOKSTRUCT mllhs = (PMSLLHOOKSTRUCT)lParam;
		switch( wParam ) {
		case WM_MOUSEWHEEL:
			{	
				__int16 zDelta = HIWORD( mllhs->mouseData );
			//	::OutputDebugString( _T("WM_MOUSEWHEEL in HMLibMouseLowLevelMouseProc \n") );
				bMWhellForward = ( zDelta >= 0 );
			} break;
		default:
			break;
		}
	}
	
	// let the messages through to the next hook
	return ::CallNextHookEx( s_hMHook, nCode, wParam, lParam );
}