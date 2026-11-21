#include "stdafx.h"
#include "win\wndctrl_repository\hm_wndctrl_base.h"
#include "tchar.h"
#include <assert.h>

const TCHAR hm_wndctrl_base::s_szClassName[] = _T("hm_wnd");
hm_wndctrl_base::map_hwnd2cls_type hm_wndctrl_base::s_mapHwnd2Obj;
BOOL hm_wndctrl_base::s_bRegisted = FALSE;
ATOM hm_wndctrl_base::s_clsAtom = 0;

hm_wndctrl_base::hm_wndctrl_base():
m_hWnd(NULL)
{
	assert( s_bRegisted );
}

hm_wndctrl_base::~hm_wndctrl_base()
{}

BOOL hm_wndctrl_base::CreateWnd(
				  DWORD dwExStyle,
				  LPCTSTR lpWindowName,
				  DWORD dwStyle,
				  int x,
				  int y,
				  int nWidth,
				  int nHeight,
				  HWND hWndParent,
				  HMENU hMenu,
				  HINSTANCE hInstance)
{
	HWND hWnd;

	//hWnd = ::CreateWindowEx( dwExStyle, s_szClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, this );
	hWnd = ::CreateWindowEx( dwExStyle, this->_wndClsName(), lpWindowName, dwStyle, x, y, nWidth, nHeight, NULL, hMenu, hInstance, this );
	
	if ( !hWnd ) {
		DWORD dwErr = ::GetLastError();
		int a = 0; 
		return FALSE;
	}
	
	return TRUE;
}

LRESULT hm_wndctrl_base::WndProc( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbIsCallDefWndProc )
{
	if ( pbIsCallDefWndProc )
	{
		*pbIsCallDefWndProc = TRUE;
	}
	
	return 0;
}

// shared registrar: one callback for every wndctrl-flavored class; the
// HWND->object map in WindowProcCallBack routes per-window dispatch.
ATOM hm_wndctrl_base::_regWndCls( HINSTANCE hInstance, LPCTSTR szClsName, UINT nStyle, HBRUSH hbrBackground )
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= nStyle;
	wcex.lpfnWndProc	= (WNDPROC)WindowProcCallBack;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= hbrBackground;
	wcex.lpszMenuName	= (LPCTSTR)NULL;
	wcex.lpszClassName	= szClsName;
	wcex.hIconSm		= NULL;
	
	return RegisterClassEx( &wcex );
}

ATOM hm_wndctrl_base::RegWndClass( HINSTANCE hInstance )
{
	if ( s_bRegisted )
		return s_clsAtom;
	s_clsAtom = _regWndCls( hInstance, s_szClassName, CS_HREDRAW | CS_VREDRAW, (HBRUSH)(COLOR_WINDOW+1) );
	s_bRegisted = TRUE;
	return s_clsAtom;
}

LRESULT CALLBACK hm_wndctrl_base::WindowProcCallBack( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	hm_wndctrl_base* pThisWndCls = NULL;
	if ( uMsg != WM_NCCREATE )
		pThisWndCls = s_mapHwnd2Obj[ hwnd ];
	
	switch( uMsg ) 
	{
	case WM_NCCREATE:
		{
			LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
			pThisWndCls = ( hm_wndctrl_base* )( pCS->lpCreateParams );
			s_mapHwnd2Obj[ hwnd ] = pThisWndCls;
			pThisWndCls->m_hWnd = hwnd;
		}
		break;
	case WM_NCDESTROY:
		{
			pThisWndCls->m_hWnd = NULL;
			s_mapHwnd2Obj.erase( hwnd );
		}
		break;
	default:
		{
			pThisWndCls = s_mapHwnd2Obj[ hwnd ];
			if ( pThisWndCls )
			{
				BOOL bIsCallDefWndProc = TRUE;
				LRESULT lr;
						
				if ( ::IsWindow( pThisWndCls->m_hWnd ) )
				{
					lr = pThisWndCls->WndProc( uMsg, wParam, lParam, &bIsCallDefWndProc );
					if ( !bIsCallDefWndProc )
						return lr;	
				}
			}
		}
		break;
	}
	
	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}





