#ifndef __HM_WIN_MISC_H__
#define __HM_WIN_MISC_H__
#include <string>
#include "tchar.h"

typedef std::basic_string<TCHAR> tstring_type;

BOOL GetCurModuleDirPath( tstring_type& strModuleDirPath );

// note: manifest/jsoncpp strings are always UTF-8 ( Json::Value::asString
// returns UTF-8 ), so the default code page must be CP_UTF8 - CP_ACP would
// only work by accident on a GBK-locale system with GBK-encoded manifests.
BOOL MultiByteToWideChar( const std::string& strMultiByte, std::basic_string<WCHAR>& wstrWideChar, unsigned int CodePage = CP_UTF8, DWORD dwFlags = 0 );

BOOL MultiByteToTChar( const std::string&  strMultiByte, std::basic_string<TCHAR>& tstrTChar );


#endif //__HM_WIN_MISC_H__