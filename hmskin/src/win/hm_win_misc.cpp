#include "stdafx.h"
#include "win\hm_win_misc.h"

BOOL GetCurModuleDirPath( tstring_type& strModuleDirPath ) {
	TCHAR szCurModulePath[ MAX_PATH +1 ];
	DWORD dwRet;
	TCHAR* posT;

	dwRet = ::GetModuleFileName( NULL, szCurModulePath, MAX_PATH );
	if ( dwRet == 0 )
		return FALSE;
	
	posT = _tcsrchr( szCurModulePath, _T('\\') );
	posT+=1;
	*posT = _T('\0');

	strModuleDirPath = szCurModulePath;
	return TRUE;
}

HFONT GetMenuLabelFont() {
	static HFONT hFontMenuLabel = NULL;
	if ( !hFontMenuLabel ) {
		LOGFONT logFont;
		memset( &logFont, 0, sizeof( LOGFONT ) );
		logFont.lfHeight = -12;
		logFont.lfWeight = FW_NORMAL;
		logFont.lfCharSet = GB2312_CHARSET;
		_tcscpy_s( logFont.lfFaceName, _T( "Microsoft YaHei UI" ) );
		hFontMenuLabel = ::CreateFontIndirect( &logFont );
	}
	return hFontMenuLabel;
}

BOOL MultiByteToWideChar( const std::string& strMultiByte, std::basic_string<WCHAR>& wstrWideChar, unsigned int uCodePage, DWORD dwFlags ) {
#define DEFAULT_WCHARTMPBUF_SIZE (64)
	int nRet;
	WCHAR wszData[ DEFAULT_WCHARTMPBUF_SIZE ];
	int cchWideChar;

	nRet = MultiByteToWideChar( uCodePage, dwFlags, strMultiByte.c_str(), -1, wszData, DEFAULT_WCHARTMPBUF_SIZE );
	if ( nRet != 0 ) { //success.
		wstrWideChar = wszData;
		return TRUE;
	} 

	//
	cchWideChar = MultiByteToWideChar( uCodePage, dwFlags, strMultiByte.c_str(), -1, NULL, 0 );
	WCHAR* pWideChatBuf = new WCHAR[ cchWideChar ];
	nRet = MultiByteToWideChar( uCodePage, dwFlags, strMultiByte.c_str(), -1, pWideChatBuf, cchWideChar );
	if ( nRet == 0 ) {
		delete []pWideChatBuf;
		return FALSE;
	}
	wstrWideChar = pWideChatBuf;
	delete []pWideChatBuf;
	return TRUE;
}

BOOL MultiByteToTChar( const std::string&  strMultiByte, std::basic_string<TCHAR>& tstrTChar ) {
#ifndef UNICODE
	wstrWideChar = strMultiByte;
	return TRUE;
#else 
	return MultiByteToWideChar( strMultiByte, tstrTChar );
#endif //
}