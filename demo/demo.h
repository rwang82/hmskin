#pragma once

#include "resource.h"
#include "tchar.h"

// append one line to the EVT-08 event log panel ( demo.cpp ).
void AppendEventLog( const TCHAR* szLine );

// switch the visible tab chapter ( 0..3 = §1..§4, demo.cpp ).
void SwitchChapter( unsigned int uChapterIndex );
