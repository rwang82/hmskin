#ifndef __HM_WIN_MISC_H__
#define __HM_WIN_MISC_H__
#include <string>
#include "windows.h"
#include "tchar.h"

typedef std::basic_string<TCHAR> tstring_type;

BOOL GetCurModuleDirPath( tstring_type& strModuleDirPath );

// shared menu-label font ( YaHei UI 12px, GB2312 charset ): the menu WMPs
// draw labels with raw GDI DrawText and the DC default font ( bitmap System
// font ) has no CJK glyphs - localized labels need an explicit font.
HFONT GetMenuLabelFont();

// note: manifest/jsoncpp strings are always UTF-8 ( Json::Value::asString
// returns UTF-8 ), so the default code page must be CP_UTF8 - CP_ACP would
// only work by accident on a GBK-locale system with GBK-encoded manifests.
BOOL MultiByteToWideChar( const std::string& strMultiByte, std::basic_string<WCHAR>& wstrWideChar, unsigned int CodePage = CP_UTF8, DWORD dwFlags = 0 );

BOOL MultiByteToTChar( const std::string&  strMultiByte, std::basic_string<TCHAR>& tstrTChar );


#endif //__HM_WIN_MISC_H__