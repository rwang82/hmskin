#include "stdafx.h"
#include "hmskin_event_listener_demo.h"
#include "hmskin_event_defs.h"
#include "win\frame\hm_keel_obj.h"
#include "win\frame\hm_uiroot.h"
#include "win\wmp\hm_wmp_button.h"
#include "hmskin_helper.h"
//
extern hm_uiroot* g_pHMRoot;
extern HWND g_hWnd;
extern HWND g_hBtnTest1;
//
hmskin_event_listener_demo::hmskin_event_listener_demo() {
}

hmskin_event_listener_demo::~hmskin_event_listener_demo() {
}

void hmskin_event_listener_demo::onHKSkinEvent( unsigned int uEventId, void* lParam1, void* lParam2 ) {
	switch ( uEventId )
	{
		case HMSKIN_EVENT_BTN_CLICK:
		{
			hm_keel_obj* pKeelObj = NULL;
			TCHAR szMsg[ 50 ];
			hm_wmp_button* pWMPButtonL = NULL;
			hm_wmp_button* pWMPButtonR = NULL;
			TCHAR szBtnText[ 50 ];

			pKeelObj = ( hm_keel_obj* )lParam1;
			_stprintf_s( szMsg, _T("keelobjID:%d"), pKeelObj->getId() );
			//
			if ( pKeelObj->getId() == 3 ) { // left button keel object.
				pWMPButtonL = dynamic_cast< hm_wmp_button* >( pKeelObj->getWMP( 4 ) );
				if ( pWMPButtonL ) {
					//
					if ( !getWMPDisplayText( pWMPButtonL, szBtnText, 50 ) )
						return;
					setWMPDisplayText( pWMPButtonL, _T("") );
					//
					pWMPButtonR = dynamic_cast< hm_wmp_button* >( g_pHMRoot->m_pWMPFactory->getObj( 7 ) );
					if ( pWMPButtonR ) {
						setWMPDisplayText( pWMPButtonR, szBtnText );
					}
					::InvalidateRect( g_hBtnTest1, NULL, FALSE );
					::InvalidateRect( g_hWnd, NULL, FALSE );
				}
			}
			//
			if ( pKeelObj->getId() == 5 ) {
				pWMPButtonR = dynamic_cast< hm_wmp_button* >( pKeelObj->getWMP( 7 ) );
				if ( pWMPButtonR ) {
					if ( !getWMPDisplayText( pWMPButtonR, szBtnText, 50 ) )
						return;
					setWMPDisplayText( pWMPButtonR, _T("") );
					//
					pWMPButtonL = dynamic_cast< hm_wmp_button* >( g_pHMRoot->m_pWMPFactory->getObj( 4 ) );
					if ( pWMPButtonL ) {
						setWMPDisplayText( pWMPButtonL, szBtnText );
					}
					::InvalidateRect( g_hBtnTest1, NULL, FALSE );
					::InvalidateRect( g_hWnd, NULL, FALSE );
				}
			}

		}
			break;
		case HMSKIN_EVENT_TABBTN_CLICK:
		{
			hm_keel_obj* pKeelObj = NULL;
			unsigned int uIndexTab;
			pKeelObj = ( hm_keel_obj* )lParam1;
			uIndexTab = (unsigned int)(UINT_PTR)lParam2;
			if ( !pKeelObj )
				return;
			//
			if ( pKeelObj->getId() == 4 ) {
				int a = 0;
			}
		}
			break;
		default:
			break;
	}
}