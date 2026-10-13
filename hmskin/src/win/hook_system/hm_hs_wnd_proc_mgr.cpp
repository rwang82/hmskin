#include "stdafx.h"
#include "win\hook_system\hm_hs_wnd_proc_mgr.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////////
//
WndProcMgr* WndProcMgr::GetInstance()
{
	static WndProcMgr wndProcMgr;
	return &wndProcMgr;
}

WndProcMgr::WndProcMgr()
{}

WndProcMgr::~WndProcMgr()
{}

BOOL WndProcMgr::Attach( HWND hWndDest, WNDPROC pfnWndProcNew )
{
	WNDPROC pfnWndProcOld;

	if ( WasAttached( hWndDest ) || !::IsWindow( hWndDest ) )
		return FALSE; //can't attach twice
	
	pfnWndProcOld = (WNDPROC)::SetWindowLongPtr( hWndDest, GWLP_WNDPROC, (LONG_PTR)pfnWndProcNew );
	m_mapHwnd2WndProc[ hWndDest ] = pfnWndProcOld;
	
	return TRUE;
}
	
BOOL WndProcMgr::Detach( HWND hWndDest )
{
	WNDPROC pfnWndProcOld;

	if ( !WasAttached( hWndDest ) )
		return FALSE;
	
	pfnWndProcOld = m_mapHwnd2WndProc[ hWndDest ];
	::SetWindowLongPtr( hWndDest, GWLP_WNDPROC, (LONG_PTR)pfnWndProcOld );

	return TRUE;
}

BOOL WndProcMgr::WasAttached( HWND hWndDest )
{
	if ( m_mapHwnd2WndProc.size() == 0 )
		return false;
	return m_mapHwnd2WndProc.find( hWndDest ) != m_mapHwnd2WndProc.end();
}

WNDPROC WndProcMgr::GetWndProcOld( HWND hWndDest )
{
	assert( WasAttached( hWndDest ) );
	return m_mapHwnd2WndProc[ hWndDest ];
}

