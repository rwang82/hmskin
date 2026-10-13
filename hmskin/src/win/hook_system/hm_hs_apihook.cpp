#include "stdafx.h"
#include <assert.h>
#include "win\hook_system\hm_hs_apihook.h"
#include "AtlConv.h"
#include "win\frame\hm_hook_trackpopupmenu.h"

bool hm_hs_apihook::hookOnTrackPopupMenu( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	if ( _isRegHook( szAPIName, szLibFileName ) ) 
		return false;
	if ( !_regHookTrackPopupMenu( szAPIName, szLibFileName ) )
		return false;
	
	//
	_hookStart( szAPIName, szLibFileName );
	
	return true;
}

bool hm_hs_apihook::hookOff( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	apiinfo_container_type::iterator itAPIInfo, iendAPIInfo;
	
	if ( !_isRegHook( szAPIName, szLibFileName ) ) 
		return false;
	if ( _isHookStarted( szAPIName, szLibFileName ) ) {
		if ( !_hookStop( szAPIName, szLibFileName ) )
			return false;
	}

	return _unregHook( szAPIName, szLibFileName );
}
//
bool hm_hs_apihook::_regHookTrackPopupMenu( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	if ( _isRegHook( szAPIName, szLibFileName ) ) return false;
	apiinfo* pAPIInfo = NULL;
	void* pfnOrigin = NULL;
	HMODULE hModule;

	USES_CONVERSION;
	hModule = LoadLibrary( szLibFileName ); // just increase a referrence count
	if ( !hModule ) return false;
	pfnOrigin = GetProcAddress( hModule, T2A(szAPIName) );
	if ( !pfnOrigin ) {
		::FreeLibrary( hModule );
		return false;
	}
	
	pAPIInfo = new apiinfo();
	//
	_prepareCodeHookTrackPopupMenu( pfnOrigin, pAPIInfo->m_originCodeBuf, pAPIInfo->m_newCodeBuf );
	pAPIInfo->m_strAPIName = szAPIName;
	pAPIInfo->m_strLibFileName = szLibFileName;
	pAPIInfo->m_bHooked = false;
	pAPIInfo->m_pfnOriginAddr = pfnOrigin;
	pAPIInfo->m_hModule = hModule;
	pAPIInfo->m_dwPID = ::GetCurrentProcessId();
	
	//
	m_containerAPIInfo.push_back( pAPIInfo );
	return true;
}

bool hm_hs_apihook::_unregHook( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	if ( !_isRegHook( szAPIName, szLibFileName ) ) return false;
	if ( _isHookStarted( szAPIName, szLibFileName ) ) {
		apiinfo_container_type::iterator itAPIInfo, iendAPIInfo;
		iendAPIInfo = m_containerAPIInfo.end();
		for ( itAPIInfo = m_containerAPIInfo.begin(); itAPIInfo!=iendAPIInfo; ++itAPIInfo ) {
			if ( ( _tcscmp( (*itAPIInfo)->m_strAPIName.c_str(), szAPIName ) == 0 )
			&& ( _tcscmp( (*itAPIInfo)->m_strLibFileName.c_str(), szLibFileName ) == 0 ) ) {
				if ( (*itAPIInfo)->m_bHooked ) {
					_hookStop( (*itAPIInfo) );
				}
				//
				::FreeLibrary( (*itAPIInfo)->m_hModule );
				delete (*itAPIInfo);
				m_containerAPIInfo.erase( itAPIInfo );
				//
				return true;
			}
		}
	}

	return false;
}

bool hm_hs_apihook::_isRegHook( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	apiinfo_container_type::iterator itAPIInfo, iendAPIInfo;
	
	iendAPIInfo = m_containerAPIInfo.end();
	for ( itAPIInfo = m_containerAPIInfo.begin(); itAPIInfo!=iendAPIInfo; ++itAPIInfo ) {
		if ( ( _tcscmp( (*itAPIInfo)->m_strAPIName.c_str(), szAPIName ) == 0 )
			&& ( _tcscmp( (*itAPIInfo)->m_strLibFileName.c_str(), szLibFileName ) == 0 ) ) {
			return true;
		}
	}
	
	return false;
}

void hm_hs_apihook::_prepareCodeHookTrackPopupMenu( void* pfnOrigin, unsigned char* originCodeBuf, unsigned char* newCodeBuf ) {
	// snapshot the original prologue we are about to overwrite.
	// note: the hook fully replaces the API ( no call-through to the original ),
	// so no instruction-boundary trampoline is needed - the snapshot is only
	// used to restore the bytes on unhook.
	memcpy( originCodeBuf, pfnOrigin, HM_HOOK_PATCH_SIZE );
#if defined(_M_ARM64)
	// 16-byte absolute jump: LDR X17,[PC,#8] ; BR X17 ; <8-byte target>
	// ( x17/IP1 is the linker veneer scratch register - safe to clobber;
	//   AArch64 instructions are fixed 4 bytes, so any 4-instruction
	//   prologue slot can host the patch without boundary decoding )
	UINT_PTR uTarget = (UINT_PTR)&hmTrackPopupMenu;
	newCodeBuf[0] = 0x51; // LDR X17,[PC,#8] = 0x58000051 ( little-endian )
	newCodeBuf[1] = 0x00;
	newCodeBuf[2] = 0x00;
	newCodeBuf[3] = 0x58;
	newCodeBuf[4] = 0x20; // BR X17 = 0xD61F0220 ( little-endian )
	newCodeBuf[5] = 0x02;
	newCodeBuf[6] = 0x1F;
	newCodeBuf[7] = 0xD6;
	memcpy( newCodeBuf + 8, &uTarget, sizeof( uTarget ) );
#elif defined(_WIN64)
	// 12-byte absolute jump: mov rax, hmTrackPopupMenu ; jmp rax
	// ( rax is volatile per the x64 ABI; the stub never returns,
	//   so clobbering it before the original prologue runs is safe )
	UINT_PTR uTarget = (UINT_PTR)&hmTrackPopupMenu;
	newCodeBuf[0] = 0x48;
	newCodeBuf[1] = 0xB8;
	memcpy( newCodeBuf + 2, &uTarget, sizeof( uTarget ) );
	newCodeBuf[10] = 0xFF;
	newCodeBuf[11] = 0xE0;
#else
	// 5-byte relative jump: jmp hmTrackPopupMenu
	INT_PTR nRel32 = (INT_PTR)&hmTrackPopupMenu - ( (INT_PTR)pfnOrigin + 5 );
	newCodeBuf[0] = 0xE9;
	memcpy( newCodeBuf + 1, &nRel32, 4 );
#endif
}

hm_hs_apihook::hm_hs_apihook() {}
hm_hs_apihook::~hm_hs_apihook() {}

bool hm_hs_apihook::_hookStart( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	apiinfo_container_type::iterator itAPIInfo, iendAPIInfo;

	iendAPIInfo = m_containerAPIInfo.end();
	for ( itAPIInfo = m_containerAPIInfo.begin(); itAPIInfo!=iendAPIInfo; ++itAPIInfo ) {
		if ( ( _tcscmp( (*itAPIInfo)->m_strAPIName.c_str(), szAPIName ) == 0 )
		&& ( _tcscmp( (*itAPIInfo)->m_strLibFileName.c_str(), szLibFileName ) == 0 ) ) {
			if ( !(*itAPIInfo)->m_bHooked ) {
				_hookStart( *itAPIInfo );
			}
			return true;
		}
	}
	return false;
}

bool hm_hs_apihook::_hookStop( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	apiinfo_container_type::iterator itAPIInfo, iendAPIInfo;

	iendAPIInfo = m_containerAPIInfo.end();
	for ( itAPIInfo = m_containerAPIInfo.begin(); itAPIInfo!=iendAPIInfo; ++itAPIInfo ) {
		if ( ( _tcscmp( (*itAPIInfo)->m_strAPIName.c_str(), szAPIName ) == 0 )
		&& ( _tcscmp( (*itAPIInfo)->m_strLibFileName.c_str(), szLibFileName ) == 0 ) ) {
			if ( (*itAPIInfo)->m_bHooked ) {
				_hookStop( *itAPIInfo );
			}
			return true;
		}
	}
	return false;
}

void hm_hs_apihook::_hookStart( apiinfo* pAPIInfo ) {
	if ( !pAPIInfo || pAPIInfo->m_bHooked ) return;
	HANDLE hProc;
	DWORD flOldProtect;
	DWORD flNewProtect;

	hProc = ::OpenProcess( PROCESS_ALL_ACCESS, 0, pAPIInfo->m_dwPID );

	flNewProtect = PAGE_READWRITE;
	::VirtualProtectEx(  hProc, pAPIInfo->m_pfnOriginAddr, HM_HOOK_PATCH_SIZE, flNewProtect, &flOldProtect);

	WriteProcessMemory( hProc, pAPIInfo->m_pfnOriginAddr, pAPIInfo->m_newCodeBuf, HM_HOOK_PATCH_SIZE, 0 );
	::FlushInstructionCache( hProc, pAPIInfo->m_pfnOriginAddr, HM_HOOK_PATCH_SIZE );

	flNewProtect = flOldProtect;
	::VirtualProtectEx( hProc, pAPIInfo->m_pfnOriginAddr, HM_HOOK_PATCH_SIZE, flNewProtect, &flOldProtect );

	::CloseHandle( hProc );

	pAPIInfo->m_bHooked = true;
}

void hm_hs_apihook::_hookStop( apiinfo* pAPIInfo ) {
	if ( !pAPIInfo || !pAPIInfo->m_bHooked ) return;
	HANDLE hProc;
	DWORD flOldProtect;
	DWORD flNewProtect;

	hProc = ::OpenProcess( PROCESS_ALL_ACCESS, 0, pAPIInfo->m_dwPID );

	flNewProtect = PAGE_READWRITE;
	::VirtualProtectEx(  hProc, pAPIInfo->m_pfnOriginAddr, HM_HOOK_PATCH_SIZE, flNewProtect, &flOldProtect);

	WriteProcessMemory( hProc, pAPIInfo->m_pfnOriginAddr, pAPIInfo->m_originCodeBuf, HM_HOOK_PATCH_SIZE, 0 );
	::FlushInstructionCache( hProc, pAPIInfo->m_pfnOriginAddr, HM_HOOK_PATCH_SIZE );

	flNewProtect = flOldProtect;
	::VirtualProtectEx( hProc, pAPIInfo->m_pfnOriginAddr, HM_HOOK_PATCH_SIZE, flNewProtect, &flOldProtect );

	::CloseHandle( hProc );
}
	
bool hm_hs_apihook::_isHookStarted( const TCHAR* szAPIName, const TCHAR* szLibFileName ) {
	apiinfo_container_type::iterator itAPIInfo, iendAPIInfo;
	
	iendAPIInfo = m_containerAPIInfo.end();
	for ( itAPIInfo = m_containerAPIInfo.begin(); itAPIInfo!=iendAPIInfo; ++itAPIInfo ) {	
		if ( ( _tcscmp( (*itAPIInfo)->m_strAPIName.c_str(), szAPIName ) == 0 )
			&& ( _tcscmp( (*itAPIInfo)->m_strLibFileName.c_str(), szLibFileName ) == 0 ) ) {
				return (*itAPIInfo)->m_bHooked;
		}
	}
	
	return false;
}

//BOOL WINAPI hm_hs_apihook::hmTrackPopupMenu(
//    HMENU hMenu,
//    UINT uFlags,
//    int x,
//    int y,
//    int nReserved,
//    HWND hWnd,
//	CONST RECT *prcRect) {
//		return ::hmTrackPopupMenu( hMenu, uFlags, x, y, nReserved, hWnd, prcRect );
//}