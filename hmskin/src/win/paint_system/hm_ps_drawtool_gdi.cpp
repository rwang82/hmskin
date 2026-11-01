#include "stdafx.h"
#include "win\paint_system\hm_ps_drawtool_gdi.h"
#include "common\MemFuncPack.h"
#include <assert.h>
#include "win\resource_system\hm_rs_uires_text.h"
//
hm_ps_drawtool_gdi::hm_ps_drawtool_gdi() {}
//
hm_ps_drawtool_gdi::~hm_ps_drawtool_gdi() {}
//
bool hm_ps_drawtool_gdi::loadRes( hm_rs_rawres_pic* pRawResPic ) {
	if ( !pRawResPic ) return false;
	assert( false );
	//pRawResPic->addResRefCnt( hm_rs_rawres_pic::REFRES_HBITMAP );
	return true;
}
//
bool hm_ps_drawtool_gdi::unloadRes( hm_rs_rawres_pic* pRawResPic ) {
	if ( !pRawResPic ) return false;
	assert( false );
	//pRawResPic->releaseResRefCnt( hm_rs_rawres_pic::REFRES_HBITMAP );
	return true;
}
//
void hm_ps_drawtool_gdi::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_pic* pRawResPic, const RECT& rcRawResPart ) {
	if ( !pRawResPic ) return;
	
	assert( false );
	return;
	//HBITMAP hBitmap, hBitmapOld;
	//HDC hMemDC;
	//hBitmap = pRawResPic->getHBitmap();
	//hMemDC = ::CreateCompatibleDC( hDC );  // need more code here, check if need hMemDC here.
	//hBitmapOld = (HBITMAP)::SelectObject( hMemDC, hBitmap );
	//
	//if ( ( (rcCanvas.right - rcCanvas.left) == (rcRawResPart.right - rcRawResPart.left) )
	//	&& ( (rcCanvas.bottom - rcCanvas.top) == (rcRawResPart.bottom - rcRawResPart.top) ) ) {
	//		::BitBlt( hDC, rcCanvas.left, rcCanvas.top, (rcCanvas.right - rcCanvas.left), (rcCanvas.bottom - rcCanvas.top), hMemDC, rcRawResPart.left, rcRawResPart.top, SRCCOPY );
	//} else {
	//	::StretchBlt( hDC, rcCanvas.left, rcCanvas.top, (rcCanvas.right - rcCanvas.left), (rcCanvas.bottom - rcCanvas.top), hMemDC, rcRawResPart.left, rcRawResPart.top, 
	//		( rcRawResPart.right - rcRawResPart.left ), 
	//		( rcRawResPart.bottom - rcRawResPart.top ),
	//		SRCCOPY );	
	//}

	//::SelectObject( hMemDC, hBitmapOld );
	//::DeleteDC( hMemDC );
	//pRawResPic->freeHBitmap();
}
//
bool hm_ps_drawtool_gdi::loadRes( hm_rs_rawres_font* pRawResFont ) {
	if ( !pRawResFont ) return false;
	pRawResFont->addResRefCnt( hm_rs_rawres_font::REFRES_HFONT );
	return true;
}
//
bool hm_ps_drawtool_gdi::unloadRes( hm_rs_rawres_font* pRawResFont ) {
	if ( !pRawResFont ) return false;
	pRawResFont->releaseResRefCnt( hm_rs_rawres_font::REFRES_HFONT );
	return true;
}
//
bool hm_ps_drawtool_gdi::loadRes( hm_rs_rawres_color* pRawResColor ) {
	if ( !pRawResColor ) return false;
	pRawResColor->addResRefCnt( hm_rs_rawres_color::REFRES_COLORREF );
	pRawResColor->addResRefCnt( hm_rs_rawres_color::REFRES_HBRUSH );
	return true;
}
//
bool hm_ps_drawtool_gdi::unloadRes( hm_rs_rawres_color* pRawResColor ) {
	if ( !pRawResColor ) return false;
	pRawResColor->releaseResRefCnt( hm_rs_rawres_color::REFRES_COLORREF );
	pRawResColor->releaseResRefCnt( hm_rs_rawres_color::REFRES_HBRUSH );
	return true;
}
//
bool hm_ps_drawtool_gdi::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_color* pRawResColor ) {
	if ( !pRawResColor ) return false;
	HBRUSH hBrush = pRawResColor->getHBrush();
	CMemFuncPack mfpkFreeHBrush( pRawResColor, &hm_rs_rawres_color::freeHBrush );
	::FillRect( hDC, &rcCanvas, hBrush );
	return TRUE;
}
//
bool hm_ps_drawtool_gdi::draw( HDC hDC, const RECT& rcCanvas, hm_uires_text_data* pText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	if ( !pText || !pRawResFont || !pRawResColor ) return false;
	COLORREF clrTextOld;
	HFONT hFontOld;
	RECT rcText;
	UINT format;
	int nBkModeOld;
	
	if ( bMultiLine )
		format = DT_LEFT|DT_TOP|DT_WORDBREAK|DT_EDITCONTROL;
	else 
		format = DT_LEFT|DT_TOP|DT_SINGLELINE;
	rcText = rcCanvas;
	clrTextOld = ::SetTextColor( hDC, pRawResColor->getCOLORREF() );
	hFontOld = (HFONT)::SelectObject( hDC, pRawResFont->getHFont() );
	nBkModeOld = ::SetBkMode( hDC, TRANSPARENT );
	
	//
	::DrawText( hDC, pText->m_tstrData.c_str(), (int)_tcslen( pText->m_tstrData.c_str() ), &rcText, format );

	//
	::SetBkMode( hDC, nBkModeOld );
	::SelectObject( hDC, hFontOld );
	::SetTextColor( hDC, clrTextOld );
	return true;
}

bool hm_ps_drawtool_gdi::calcRect( HDC hDC, RECT& rcCanvas, const TCHAR* szText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	if ( !szText || !pRawResFont || !pRawResColor ) return false;
	COLORREF clrTextOld;
	HFONT hFontOld;
	UINT format;
	
	if ( bMultiLine )
		format = DT_LEFT|DT_TOP|DT_WORDBREAK|DT_EDITCONTROL;
	else 
		format = DT_LEFT|DT_TOP|DT_SINGLELINE;
	clrTextOld = ::SetTextColor( hDC, pRawResColor->getCOLORREF() );
	hFontOld = (HFONT)::SelectObject( hDC, pRawResFont->getHFont() );
	
	//
	LOGFONT lgFont;
	::GetObject( hFontOld, sizeof(LOGFONT), &lgFont );
	int a = 0;

	//
	::DrawText( hDC, szText, (int)_tcslen( szText ), &rcCanvas, format|DT_CALCRECT );
	//
	::SelectObject( hDC, hFontOld );
	::SetTextColor( hDC, clrTextOld );
	
	return true;
}

hm_ps_drawtool_gdi* hm_ps_drawtool_gdi::getInstance() {
	static hm_ps_drawtool_gdi s_instance;
	return &s_instance;
}