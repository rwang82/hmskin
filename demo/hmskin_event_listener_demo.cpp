#include "stdafx.h"
#include "hmskin_event_listener_demo.h"
#include "hmskin_event_defs.h"
#include "hmskin_cpp.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_layout_obj.h"
#include "win\wmp\hm_wmp_checkbox.h"
#include "win\frame\hm_layout_decoders.h"
#include "win\frame\hm_uiroot.h"
#include "win\frame\hm_debug_utils.h"
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
		{ // checkbox-class WMPs: K10 = CHK-02 checkbox, K48 = TGL-10 toggle.
		  // ( radios are the hm_wmp_radio class and fire RADIO_CLICK below. )
			unsigned int uId = pKeelObj ? pKeelObj->getId() : 0;
			if ( uId == 48 )
				_stprintf_s( szLine, 64, _T("TGL-10 -> %s"), lParam2 ? _T("ON") : _T("OFF") );
			else
				_stprintf_s( szLine, 64, _T("CHK-02 -> %s"), lParam2 ? _T("true") : _T("false") );
		}
		break;
	case HMSKIN_EVENT_RADIO_CLICK:
		{ // RAD-11 ( K49..K51, hm_wmp_radio ): the clicked item stays checked
		  // by the engine, this side only unchecks siblings. setCheck
		  // invalidates via the STATECHANGED event, no view plumbing here.
			static const TCHAR* arrOptions[3] = { _T("GDI"), _T("GDI+"), _T("AUTO") };
			unsigned int uSel = pKeelObj ? ( pKeelObj->getId() - 49 ) : 0;
			if ( uSel > 2 )
				uSel = 2;
			for ( unsigned int uSibling = 0; uSibling < 3; ++uSibling ) {
				if ( uSibling == uSel )
					continue;
				hm_keel_obj* pKeelObjSibling = NULL;
				if ( hm_get_keelobj( g_pHMRoot, 49 + uSibling, &pKeelObjSibling ) && pKeelObjSibling ) {
					hm_wmp_checkbox* pWMPSibling = dynamic_cast< hm_wmp_checkbox* >( pKeelObjSibling->getWMP( 72 + uSibling ) );
					if ( pWMPSibling && pWMPSibling->isCheck() )
						pWMPSibling->setCheck( FALSE );
				}
			}
			_stprintf_s( szLine, 64, _T("RAD-11 -> %s"), arrOptions[ uSel ] );
		}
		break;
	case HMSKIN_EVENT_TABBTN_CLICK:
		_stprintf_s( szLine, 64, _T("TAB-01 -> #%d"), (unsigned int)(UINT_PTR)lParam2 + 1 );
		SwitchChapter( (unsigned int)(UINT_PTR)lParam2 );
		break;
	case HMSKIN_EVENT_NCABTN_CLICK:
		{
			unsigned int uBtnId = pKeelObj ? pKeelObj->getId() : 0;
			_stprintf_s( szLine, 64, _T("NCA-BTN #%u"), uBtnId );
			// caption buttons drive the real window commands ( manifest K40/K41/K42 ).
			if ( g_hWnd ) {
				if ( uBtnId == KEELOBJID_NCABTN_MIN )
					::SendMessage( g_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0 );
				else if ( uBtnId == KEELOBJID_NCABTN_MAX )
					::SendMessage( g_hWnd, WM_SYSCOMMAND, ::IsZoomed( g_hWnd ) ? SC_RESTORE : SC_MAXIMIZE, 0 );
				else if ( uBtnId == KEELOBJID_NCABTN_CLOSE )
					::SendMessage( g_hWnd, WM_SYSCOMMAND, SC_CLOSE, 0 );
			}
		}
		break;
	case HMSKIN_EVENT_ROOTMENU_CLICK:
		_stprintf_s( szLine, 64, _T("MENU-BAR -> #%u"), (unsigned int)(UINT_PTR)lParam2 + 1 );
		OpenSkinPopupMenu( (unsigned int)(UINT_PTR)lParam2 );
		break;
	case HMSKIN_EVENT_SCROLLBAR_CHANGED:
		{ // sec.4 : scrollbar value moves the content rows ( parallax demo,
		  // K38 base offset y=22, value 0..16 moves content up 0..16px ).
			int nVal = (int)(UINT_PTR)lParam2;
			_stprintf_s( szLine, 64, _T("SCB-03 -> %d"), nVal );
			hm_keel_obj* pKeelObjContent = NULL;
			if ( hm_get_keelobj( g_pHMRoot, 38, &pKeelObjContent ) && pKeelObjContent ) {
				hm_layout_obj* pLayoutObj = pKeelObjContent->getLayoutObj();
				hm_layoutdecoder_corner* pDecoder = pLayoutObj
					? dynamic_cast< hm_layoutdecoder_corner* >( pLayoutObj->getLayoutDecoder() ) : NULL;
				if ( pDecoder ) {
					pDecoder->m_space2 = hm_layout_scale_px( 22 ) - nVal;
					pLayoutObj->calcPosition();
					// invalidate ONLY the content rows rect: the scrollbar
					// redraws itself directly while dragging, a wider
					// invalidate would repaint every control per mousemove.
					hm_util_invlog( _T("scb-content"), &pLayoutObj->m_rect );
					::InvalidateRect( g_hWnd, &pLayoutObj->m_rect, FALSE );
				}
			}
		}
		break;
	case HMSKIN_EVENT_POPUPMENU_CLICK:
		{ // engine popup delivered WM_COMMAND to the owner already; log only.
			_stprintf_s( szLine, 64, _T("MENU-POP cmd=%u"), (unsigned int)(UINT_PTR)lParam2 );
		}
		break;
	default:
		_stprintf_s( szLine, 64, _T("EVT %u"), uEventId );
		break;
	}
	AppendEventLog( szLine );
}
