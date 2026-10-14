#pragma once

#include "resource.h"
#include "tchar.h"

// main window ( demo.cpp ).
extern HWND g_hWnd;

// caption button keelobj ids ( manifest K40/K41/K42 normal, K44/K43/K45 zoomed,
// used by the event listener and the WM_SIZE visibility swap in demo.cpp ).
#define KEELOBJID_NCABTN_MIN (40)
#define KEELOBJID_NCABTN_MAX (41)
#define KEELOBJID_NCABTN_CLOSE (42)
#define KEELOBJID_NCABTN_RESTORE (43)
#define KEELOBJID_NCABTN_MIN_Z (44)
#define KEELOBJID_NCABTN_CLOSE_Z (45)

// append one line to the EVT-08 event log panel ( demo.cpp ).
void AppendEventLog( const TCHAR* szLine );

// switch the visible tab chapter ( 0..3 = §1..§4, demo.cpp ).
void SwitchChapter( unsigned int uChapterIndex );
