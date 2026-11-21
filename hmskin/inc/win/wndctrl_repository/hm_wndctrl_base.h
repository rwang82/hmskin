#ifndef __HM_WNDCTRL_BASE_H__
#define __HM_WNDCTRL_BASE_H__
#include "windows.h"
//#include "QHMLib.h"
#include <map>

class hm_wndctrl_base
{
public:
	typedef hm_wndctrl_base class_type;
	typedef std::map< HWND, class_type* > map_hwnd2cls_type;
	
public: 
	hm_wndctrl_base(); 
	virtual ~hm_wndctrl_base();
	
	//interface
public:
	virtual LRESULT WndProc( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbIsCallDefWndProc );
	
	//operator
public:
	static ATOM RegWndClass( HINSTANCE hInstance );
	
	//
protected:
	virtual BOOL CreateWnd( 
		DWORD dwExStyle,
		LPCTSTR lpWindowName,
		DWORD dwStyle,
		int x,
		int y,
		int nWidth,
		int nHeight,
		HWND hWndParent,
		HMENU hMenu,
		HINSTANCE hInstance
		);
protected:
	// class-name provider: derived windowed controls register a flavored
	// class via _regWndCls and return its name here for CreateWnd.
	virtual LPCTSTR _wndClsName() const { return s_szClassName; }
	static ATOM _regWndCls( HINSTANCE hInstance, LPCTSTR szClsName, UINT nStyle, HBRUSH hbrBackground );

private:
	static LRESULT CALLBACK WindowProcCallBack( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static map_hwnd2cls_type s_mapHwnd2Obj;
	static const TCHAR s_szClassName[MAX_CLASS_NAME];
	static BOOL s_bRegisted;
	static ATOM s_clsAtom;
	
public:
	HWND m_hWnd;
};

#endif //__HM_WNDCTRL_BASE_H__