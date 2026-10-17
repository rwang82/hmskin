#pragma once

#include "resource.h"
#include "tchar.h"

// main window ( demo.cpp ).
extern HWND g_hWnd;

// hmskin ui root ( demo.cpp ).
class hm_uiroot;
extern hm_uiroot* g_pHMRoot;

// caption button keelobj ids ( manifest K40/K41/K42, used by the event listener ).
#define KEELOBJID_NCABTN_MIN (40)
#define KEELOBJID_NCABTN_MAX (41)
#define KEELOBJID_NCABTN_CLOSE (42)

// append one line to the EVT-08 event log panel ( demo.cpp ).
void AppendEventLog( const TCHAR* szLine );

// open the skinned popup menu for root menu item uRootItemIndex ( demo.cpp ).
void OpenSkinPopupMenu( unsigned int uRootItemIndex );

// switch the visible tab chapter ( 0..3 = sec.1..sec.4, demo.cpp ).
void SwitchChapter( unsigned int uChapterIndex );
