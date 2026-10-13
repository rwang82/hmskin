#include "stdafx.h"
#include "win\hook_system\hm_hs_wnd_proc_hook.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_msg_sink.h"
#include "win\frame\hm_coordinate_system_hwnd.h"
#include "win\frame\hm_uiroot.h"
#include "win\hook_system\hm_hs_wnd_msg_dispatch.h"
#include "tchar.h"
#include "hmskin.h"
#include <assert.h>
//////////////////////////////////////////////////
static HHOOK s_WndHook = NULL;
static unsigned int s_uRefCountWndHook = 0;
static LRESULT CALLBACK HMLibHookWndProc( int nCode, WPARAM wParam, LPARAM lParam );
static bool GetWndPropId( HWND hWnd, unsigned int* pidWndProp );
static bool GetWndCtrlId( HWND hWnd, unsigned int* pidCtrl );
static void BindHWND2WndMark( HWND hWnd, const hm_rs_wndmark& wndMark );
struct EHMUIRPParam {
	HWND m_hWnd;
	hm_rs_wndmark m_wndMark;
};
static void EnumHMUIRootProc( hm_uiroot* pHMUIRoot, void* pParam );
//////////////////////////////////////////////////
bool hs_wnd_proc_hook_start( HINSTANCE hInstance )
{
	if ( s_WndHook ) {
		s_uRefCountWndHook++;
		return true;
	}
	
	s_WndHook = ::SetWindowsHookEx( WH_CALLWNDPROC, HMLibHookWndProc, (HINSTANCE)hInstance, ::GetCurrentThreadId() );
	if ( s_WndHook ) {
		assert( s_uRefCountWndHook == 0 );
		s_uRefCountWndHook = 1;
	}
	return s_WndHook != NULL;
}
 
void hs_wnd_proc_hook_stop()
{
	if ( !s_WndHook )
		return;
	if ( s_uRefCountWndHook == 0 ) {
		assert( false );
	} else {
		--s_uRefCountWndHook;
	}

	if ( s_uRefCountWndHook == 0 )	{
		::UnhookWindowsHookEx( s_WndHook );
		s_WndHook = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK HMLibHookWndProc( int nCode, WPARAM wParam, LPARAM lParam )
{	
	CWPSTRUCT* pWP = NULL;
		
	pWP = ( CWPSTRUCT* )lParam;
	switch( pWP->message ) 
	{
	case WM_CREATE:
		{
			TCHAR szClsName[260];
			TCHAR szCtrlId[50];
			int nSize;
			int nCtrlId;
			
			nSize = ::GetClassName( pWP->hwnd, szClsName, 260 );
			nCtrlId = ::GetDlgCtrlID( pWP->hwnd );

			::OutputDebugString( _T("WM_CREATE.") );
			_stprintf_s( szCtrlId, _T("0x%016zx"), (size_t)pWP->hwnd );
			::OutputDebugString( szCtrlId );
			::OutputDebugString( _T(" ClassName:") );
			::OutputDebugString( szClsName );
			::OutputDebugString( _T(" CtrlId:") );
			_stprintf_s( szCtrlId, _T("%d"), nCtrlId );
			::OutputDebugString( szCtrlId );
			::OutputDebugString( _T("\n") );

			hm_rs_wndmark wndMark;
			//before window be created, we can't set prop id to window.

			//	
			if ( GetWndCtrlId( pWP->hwnd, &wndMark.m_uCtrlID ) ) {
				wndMark.m_eflag = hm_rs_wndmark::WNDMARK_CTRLID;
			} else {
				break; //do not hook this window.
			}

			BindHWND2WndMark( pWP->hwnd, wndMark );
			
		}
		break;
	case WM_DESTROY:
		{ //program will not call this. because, unhook function will be called before window destory.
			printf( "%s.\n", "WM_DESTROY" );
		}
		break;
	default:
		{}
		break;
	}


	return CallNextHookEx( s_WndHook, nCode, wParam, lParam );
}
//
void BindHWND2WndMark( HWND hWnd, const hm_rs_wndmark& wndMark ) {
	EHMUIRPParam paramEHMUIRP;

	paramEHMUIRP.m_hWnd = hWnd;
	paramEHMUIRP.m_wndMark = wndMark;
	HMUIROOTFACTORY_INSTANCE->enumObjs( EnumHMUIRootProc, &paramEHMUIRP );
}

void EnumHMUIRootProc( hm_uiroot* pHMUIRoot, void* pParam ) {
	if ( !pHMUIRoot || !pParam ) {
		assert( false );
		return;
	}
	EHMUIRPParam* pEHMUIRPParam = NULL;

	pEHMUIRPParam = (EHMUIRPParam*)pParam;
	
	//
	pHMUIRoot->m_pHWNDDespository->bindHWND2WndMark( pEHMUIRPParam->m_hWnd, pEHMUIRPParam->m_wndMark );
	//
	ConnectHwnd2MsgSink( pHMUIRoot, pEHMUIRPParam->m_hWnd, pEHMUIRPParam->m_wndMark );
	//
	ConnectHwnd2CoordSys( pHMUIRoot, pEHMUIRPParam->m_hWnd, pEHMUIRPParam->m_wndMark );
}
//
bool GetWndPropId( HWND hWnd, unsigned int* pidWndProp )
{
	if ( !hWnd || !pidWndProp )
		return false;
	HGLOBAL hMem;
	TCHAR* pszWndPropId = NULL;

	hMem = (HGLOBAL)::GetProp( hWnd, UUID_TSTR_WNDPROP_ID );
	if ( !hMem )
		return false;
	//
	pszWndPropId = (TCHAR*)::GlobalLock( hMem );
	if ( !pszWndPropId )
		return false;
	//
	*pidWndProp = _ttoi( pszWndPropId );
	//
	::GlobalUnlock( hMem );
	
	return true;
}
//
bool GetWndCtrlId( HWND hWnd, unsigned int* pidCtrl )
{
	if ( !hWnd || !pidCtrl )
		return false;
	int ctrlId;

	ctrlId = GetDlgCtrlID( hWnd );
	if ( ctrlId == 0 )
		return false;
	//
	*pidCtrl = ctrlId;

	return true;
}
