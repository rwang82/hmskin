#include "stdafx.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "common\FuncPack.h"
#include "win\frame\hm_debug_utils.h"
#include "win\frame\hm_perf_stats.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include <stdexcept>
#include <AtlConv.h>
//
// RAII timing guard: destructor records one draw on EVERY return path
namespace {
struct _perf_text_guard {
	LARGE_INTEGER m_t0;
	int m_nTool;
	unsigned int m_uChars;
	_perf_text_guard( int nTool, unsigned int uChars ) : m_nTool( nTool ), m_uChars( uChars ) {
		::QueryPerformanceCounter( &m_t0 );
	}
	~_perf_text_guard() {
		LARGE_INTEGER t1, f;
		::QueryPerformanceCounter( &t1 );
		::QueryPerformanceFrequency( &f );
		double us = (double)( t1.QuadPart - m_t0.QuadPart ) * 1e6 / (double)f.QuadPart;
		hm_perf_text_draw( m_nTool, us, m_uChars );
	}
};
} // namespace
//
hm_ps_drawtool_gdiplus::hm_ps_drawtool_gdiplus() {}

//
hm_ps_drawtool_gdiplus::~hm_ps_drawtool_gdiplus() {}

//
bool hm_ps_drawtool_gdiplus::loadRes( hm_rs_rawres_pic* pRawResPic ) {
	if ( !pRawResPic ) return false;
	pRawResPic->addResRefCnt( hm_rs_rawres_pic::REFRES_IMAGE_GP );
	return true;
}

//
bool hm_ps_drawtool_gdiplus::unloadRes( hm_rs_rawres_pic* pRawResPic ) {
	if ( !pRawResPic ) return false;
	pRawResPic->releaseResRefCnt( hm_rs_rawres_pic::REFRES_IMAGE_GP );
	return true;
}

//
void hm_ps_drawtool_gdiplus::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_pic* pRawResPic, const RECT& rcRawResPart ) {
	if ( !pRawResPic ) return;
	Gdiplus::Image* pImage = NULL;
	//HDC hMemDC;
	Gdiplus::RectF rcDraw( (Gdiplus::REAL)rcCanvas.left, (Gdiplus::REAL)rcCanvas.top, (Gdiplus::REAL)(rcCanvas.right - rcCanvas.left), (Gdiplus::REAL)(rcCanvas.bottom - rcCanvas.top) );
	
	pImage = pRawResPic->getGPImage();
	//hMemDC = ::CreateCompatibleDC( hDC );
	
	Gdiplus::Graphics graph( hDC );
	// crisp stretched bitmaps: high quality interpolation + half pixel offset
	// ( default bilinear made upscaled 9-patch edges look blurry/jagged )
	graph.SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );
	graph.SetPixelOffsetMode( Gdiplus::PixelOffsetModeHalf );
	//graph.DrawImage( pImage, rcDraw, pUIResPic->m_uLeft, pUIResPic->m_uTop, pUIResPic->m_nWidth, pUIResPic->m_uHeight, Gdiplus::UnitPixel );
	graph.DrawImage( pImage, rcDraw, (Gdiplus::REAL)rcRawResPart.left, (Gdiplus::REAL)rcRawResPart.top,
		(Gdiplus::REAL)( rcRawResPart.right - rcRawResPart.left ),
		(Gdiplus::REAL)( rcRawResPart.bottom - rcRawResPart.top ),
		Gdiplus::UnitPixel );
	graph.ReleaseHDC( hDC );
	
	//::DeleteObject( hMemDC );
	pRawResPic->freeGPImage();
	//pRawResPic-
}

//
bool hm_ps_drawtool_gdiplus::loadRes( hm_rs_rawres_font* pRawResFont ) {
	if ( !pRawResFont ) return false;
	pRawResFont->addResRefCnt( hm_rs_rawres_font::REFRES_FONT_GP );
	return true;
}
//
bool hm_ps_drawtool_gdiplus::unloadRes( hm_rs_rawres_font* pRawResFont ) {
	if ( !pRawResFont ) return false;
	pRawResFont->releaseResRefCnt( hm_rs_rawres_font::REFRES_FONT_GP );
	return true;
}
//
bool hm_ps_drawtool_gdiplus::loadRes( hm_rs_rawres_color* pRawResColor ) {
	if ( !pRawResColor ) return false;
	pRawResColor->addResRefCnt( hm_rs_rawres_color::REFRES_COLOR_GP );
	return true;
}
//
bool hm_ps_drawtool_gdiplus::unloadRes( hm_rs_rawres_color* pRawResColor ) {
	if ( !pRawResColor ) return false;
	pRawResColor->releaseResRefCnt( hm_rs_rawres_color::REFRES_COLOR_GP );
	return true;
}
//
bool hm_ps_drawtool_gdiplus::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_color* pRawResColor ) {
	if ( !pRawResColor ) return false;
	Gdiplus::Graphics graph( hDC );
	Gdiplus::SolidBrush brush( pRawResColor->getGPColor() );

	graph.FillRectangle( &brush, rcCanvas.left, rcCanvas.top, rcCanvas.right - rcCanvas.left, rcCanvas.bottom - rcCanvas.top );
	return true;
}
//
bool hm_ps_drawtool_gdiplus::draw( HDC hDC, const RECT& rcCanvas, hm_uires_text_data* pText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	if ( !pText || !pRawResFont || !pRawResColor ) return false;
	_perf_text_guard perf( HM_PERF_TOOL_GP, (unsigned int)pText->m_tstrData.size() );
	TCHAR* pTextData = NULL;
	int sizeTextData = 0;
	Gdiplus::Graphics graph( hDC );
	Gdiplus::RectF rcDraw( (Gdiplus::REAL)rcCanvas.left, (Gdiplus::REAL)rcCanvas.top, (Gdiplus::REAL)(rcCanvas.right - rcCanvas.left), (Gdiplus::REAL)(rcCanvas.bottom - rcCanvas.top) );
	
	sizeTextData = _tcslen( pText->m_tstrData.c_str() ) + 1;
	pTextData = new TCHAR[ sizeTextData ];
	CFuncPack fpkDelBuf( ::gfnDelBuf< TCHAR >, pTextData );
	_tcscpy_s( pTextData, sizeTextData, pText->m_tstrData.c_str() );
	// single-line labels: NoWrap ( never wrap ) and NO LineLimit - a line slightly taller
	// than a knife-edge rect must still render ( bottom-clipped a pixel );
	// LineLimit made whole lines vanish at fractional DPI scales.
	Gdiplus::StringFormat strFormat( bMultiLine ? (Gdiplus::StringFormatFlags)0 : Gdiplus::StringFormatFlagsNoWrap, LANG_NEUTRAL );
	Gdiplus::SolidBrush brushTextClr( pRawResColor->getGPColor() );
	//
	if ( pText->m_eAlginment == hm_uires_text_data::EALGINMENT_RIGHT ) {
		strFormat.SetAlignment( Gdiplus::StringAlignmentFar );
	} else if ( pText->m_eAlginment == hm_uires_text_data::EALGINMENT_VCENTER ) {
		strFormat.SetAlignment( Gdiplus::StringAlignmentCenter );
		// vertical centering needs line alignment; SetAlignment only centers horizontally
		// ( text used to hug the top of its district in buttons etc. )
		strFormat.SetLineAlignment( Gdiplus::StringAlignmentCenter );
	} else {
		strFormat.SetAlignment( Gdiplus::StringAlignmentNear );
	}
//typedef enum {
//    StringFormatFlagsDirectionRightToLeft = 0x00000001,	�����������˳����ʾ�ı�.
//    StringFormatFlagsDirectionVertical = 0x00000002,		�ı���ֱ����.
//    StringFormatFlagsNoFitBlackBox = 0x00000004,			���������ַ�������ַ����Ĳ��־���.Ĭ������£������¶�λ�ַ��Ա����κ�����.
//    StringFormatFlagsDisplayFormatControl = 0x00000020,	�����ַ�(������ұ��)����д����Եı�־����һ����ʾ�������.
//    StringFormatFlagsNoFontFallback = 0x00000400,
//    StringFormatFlagsMeasureTrailingSpaces = 0x00000800,
//    StringFormatFlagsNoWrap = 0x00001000,					�ھ��������ø�ʽʱ�������ı����й���.
//    StringFormatFlagsLineLimit = 0x00002000,				
//    StringFormatFlagsNoClip = 0x00004000					������ʾ��־���ŵ�������ֺ����쵽�߿����δ�����ı�.
//} StringFormatFlags;

	USES_CONVERSION;
	graph.DrawString( T2W( pTextData ), _tcslen( pTextData ), pRawResFont->getGPFont(), rcDraw, &strFormat, &brushTextClr );
    //

	return true;
}

bool hm_ps_drawtool_gdiplus::calcRect( HDC hDC, RECT& rcCanvas, const TCHAR* szText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	TCHAR* pTextData = NULL;
	int sizeTextData = 0;
	Gdiplus::Graphics graphic( hDC );
	//Gdiplus::StringFormat strFormat( bMultiLine ? 0 : Gdiplus::StringFormatFlagsLineLimit, LANG_NEUTRAL );
	Gdiplus::StringFormat strFormat;
	Gdiplus::RectF rcDraw( (Gdiplus::REAL)rcCanvas.left, (Gdiplus::REAL)rcCanvas.top, (Gdiplus::REAL)(rcCanvas.right - rcCanvas.left), (Gdiplus::REAL)(rcCanvas.bottom - rcCanvas.top) );
	Gdiplus::RectF boundingBox;
	Gdiplus::Status status;
	int codepointsFitted;
	int linesFilled;
	
	strFormat.SetAlignment( Gdiplus::StringAlignmentFar );
	sizeTextData = _tcslen( szText ) + 1;
	pTextData = new TCHAR[ sizeTextData ];
	CFuncPack fpkDelBuf( ::gfnDelBuf< TCHAR >, pTextData );
	_tcscpy_s( pTextData, sizeTextData, szText );
	
	if ( bMultiLine ) {
		codepointsFitted = 0;
		while( 1 ) {
			USES_CONVERSION;
			graphic.MeasureString( T2W( pTextData ), _tcslen( pTextData ), pRawResFont->getGPFont(), rcDraw, &strFormat, &boundingBox, &codepointsFitted, &linesFilled );
			if ( codepointsFitted != _tcslen( pTextData ) ) {
				rcDraw.Height += 1;
				continue;
			}
			break;
		}
	} else {
		codepointsFitted = 0;
		linesFilled = 0;
		while ( 1 ) {
			USES_CONVERSION;
			status = graphic.MeasureString( T2W( pTextData ), _tcslen( pTextData ), pRawResFont->getGPFont(), rcDraw, &strFormat, &boundingBox, &codepointsFitted, &linesFilled );
			if ( linesFilled != 1 
			|| codepointsFitted != _tcslen( pTextData ) ) {
				rcDraw.Width += 1;
				continue;
			}
			break;
		}
	}
	
	//
    RECT rcBoundBox;
	rcBoundBox.left = ( long )boundingBox.GetLeft();
	rcBoundBox.right = ( long )boundingBox.GetRight();
	rcBoundBox.top = ( long )boundingBox.GetTop();
	rcBoundBox.bottom = ( long )boundingBox.GetBottom();
	hm_util_draw_rect( hDC, rcBoundBox, RGB(255,0,0) );
	//

	
	rcCanvas.left = ( long )rcDraw.GetLeft();
	rcCanvas.right = ( long )rcDraw.GetRight();
	rcCanvas.top = ( long )rcDraw.GetTop();
	rcCanvas.bottom = ( long )rcDraw.GetBottom();

	// not the right way.
	//rcCanvas.left = boundingBox.GetLeft();
	//rcCanvas.right = boundingBox.GetRight();
	//rcCanvas.top = boundingBox.GetTop();
	//rcCanvas.bottom = boundingBox.GetBottom();
	
	return true;
}

hm_ps_drawtool_gdiplus* hm_ps_drawtool_gdiplus::getInstance() {
	static hm_ps_drawtool_gdiplus s_instance;
	return &s_instance;
}