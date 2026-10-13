#include "stdafx.h"
#include "hmskin_event_listener_demo.h"
#include "hmskin_event_defs.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_uiroot.h"
#include "demo.h"

// every hmskin event lands in the EVT-08 event log ( see DESIGN.md ).
hmskin_event_listener_demo::hmskin_event_listener_demo() {
}

hmskin_event_listener_demo::~hmskin_event_listener_demo() {
}

void hmskin_event_listener_demo::onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
	hm_keel_obj* pKeelObj = ( hm_keel_obj* )lParam1;
	TCHAR szLine[ 64 ];

	switch ( uEventId )
	{
	case HMSKIN_EVENT_BTN_CLICK:
		_stprintf_s( szLine, 64, _T("BTN-%02d CLICK"), pKeelObj ? pKeelObj->getId() : 0 );
		break;
	case HMSKIN_EVENT_CHECKBOX_CLICK:
		_stprintf_s( szLine, 64, _T("CHK-02 -> %s"), lParam2 ? _T("true") : _T("false") );
		break;
	case HMSKIN_EVENT_TABBTN_CLICK:
		_stprintf_s( szLine, 64, _T("TAB-01 -> #%d"), (unsigned int)(UINT_PTR)lParam2 + 1 );
		break;
	case HMSKIN_EVENT_NCABTN_CLICK:
		_tcscpy_s( szLine, 64, _T("NCA-BTN CLICK") );
		break;
	default:
		_stprintf_s( szLine, 64, _T("EVT %u"), uEventId );
		break;
	}
	AppendEventLog( szLine );
}
