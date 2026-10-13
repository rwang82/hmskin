#ifndef __HM_HS_APIHOOK_H__
#define __HM_HS_APIHOOK_H__
#include <string>
#include <deque>
#include "tchar.h"

// size of the code patch written over the target API prologue.
// ARM64: 16-byte absolute jump ( LDR X17,#8 ; BR X17 ; <imm64> ),
//        x16/x17 are the inter-procedural scratch registers - safe to clobber.
// x64:   12-byte absolute jump ( 48 B8 <imm64> FF E0 : mov rax,imm64 ; jmp rax ),
//        because a 5-byte E9 rel32 jump can not reach beyond 2GB.
// x86:   classic 5-byte E9 rel32 jump.
#if defined(_M_ARM64)
static const int HM_HOOK_PATCH_SIZE  = 16;
#elif defined(_WIN64)
static const int HM_HOOK_PATCH_SIZE  = 12;
#else
static const int HM_HOOK_PATCH_SIZE  = 5;
#endif
static const int HM_HOOK_BUFFER_SIZE = 32;

class hm_hs_apihook{
	// type define
public:
	typedef std::basic_string< TCHAR > string_type;
	struct apiinfo {
		string_type m_strAPIName;
		string_type m_strLibFileName;
		unsigned char m_originCodeBuf[ HM_HOOK_BUFFER_SIZE ];
		unsigned char m_newCodeBuf[ HM_HOOK_BUFFER_SIZE ];
		bool m_bHooked;
		void* m_pfnOriginAddr;
		HMODULE m_hModule;
		DWORD m_dwPID;
	};
	typedef std::deque< apiinfo* > apiinfo_container_type;
	
	// operation
public:
	bool hookOnTrackPopupMenu( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	bool hookOff( const TCHAR* szAPIName, const TCHAR* szLibFileName );

	// helper
private:
	void _hookStart( apiinfo* pAPIInfo );
	void _hookStop( apiinfo* pAPIInfo );
	bool _hookStart( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	bool _hookStop( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	bool _isHookStarted( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	bool _unregHook( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	bool _isRegHook( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	//
	bool _regHookTrackPopupMenu( const TCHAR* szAPIName, const TCHAR* szLibFileName );
	void _prepareCodeHookTrackPopupMenu( void* pfnOrigin, unsigned char* originCodeBuf, unsigned char* newCodeBuf );

	//
private:
	//static BOOL WINAPI hmTrackPopupMenu(
 //   HMENU hMenu,
 //   UINT uFlags,
 //   int x,
 //   int y,
 //   int nReserved,
 //   HWND hWnd,
 //   CONST RECT *prcRect);

public:
	hm_hs_apihook();
	virtual ~hm_hs_apihook();

private:
	apiinfo_container_type m_containerAPIInfo;

};

extern hm_hs_apihook g_instanceAPIHook;
#define APIHOOK_INSTANCE (&g_instanceAPIHook)




#endif //__HM_HS_APIHOOK_H__