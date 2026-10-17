#include "stdafx.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "common\FuncPack.h"
#include <stdexcept>
#include <AtlConv.h>
#include <assert.h>

IMPLEMENT_FACTORY( hm_rs_rawres_pic )
HMLIB_API hm_rawres_pic_factory g_factoryRawPic;

//
hm_rs_rawres_pic::hm_rs_rawres_pic( const TCHAR* szPicPath ) 
: m_pGPImage( NULL )
, m_strPicPath( ( szPicPath ) ? szPicPath : _T("") ) {
	memset( &m_sizePic, 0, sizeof( SIZE ) );

	getGPImage();
}

//
hm_rs_rawres_pic::~hm_rs_rawres_pic(){
	freeGPImage();

	for ( unsigned int rrId = REFRES_BEGIN; rrId<REFRES_COUNT; ++rrId ) {
		doFinalRelase( rrId );
	}
};

//
void hm_rs_rawres_pic::doFirstRef( refres_id_type rrId ) {
	switch ( rrId )
	{
	case REFRES_IMAGE_GP:
		{
			if ( m_pGPImage ) {
				throw std::runtime_error("before call doFirstRef(), m_pImage is not NULL.");
			}
			m_pGPImage = new Gdiplus::Image( m_strPicPath.c_str() );
			m_sizePic.cx = m_pGPImage->GetWidth();
			m_sizePic.cy = m_pGPImage->GetHeight();
		}
		break;
	default:
		break;
	}
}

//
void hm_rs_rawres_pic::doFinalRelase( refres_id_type rrId ) {
	switch ( rrId )
	{
	case REFRES_IMAGE_GP:
		{
			if ( m_pGPImage ) {
				delete m_pGPImage;
				m_pGPImage = NULL;
			}
			memset( &m_sizePic, 0, sizeof( SIZE ) );
		}
		break;
	default:
		break;
	}
}

//
Gdiplus::Image* hm_rs_rawres_pic::getGPImage() {
	_addRefCount( REFRES_IMAGE_GP );
	if ( !m_pGPImage ) {
		throw std::runtime_error("m_pGPImage is NULL.");
	}
	return m_pGPImage;
}

//
void hm_rs_rawres_pic::freeGPImage() {
	_releaseRefCount( REFRES_IMAGE_GP );
}

void hm_rs_rawres_pic::fetchSize( SIZE& sizePic ) {
	if ( m_sizePic.cx || m_sizePic.cy ) {
		sizePic = m_sizePic;
		return; //
	}
	Gdiplus::Image* pImage = new Gdiplus::Image( m_strPicPath.c_str() );
	m_sizePic.cx = pImage->GetWidth();
	m_sizePic.cy = pImage->GetHeight();
	sizePic = m_sizePic;
	delete pImage;
}

////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_FACTORY( hm_rs_rawres_font )
HMLIB_API hm_rawres_font_factory g_factoryRawFont;
//
hm_rs_rawres_font::hm_rs_rawres_font( const hm_rs_font_param& fontParam )
: m_fontParam( fontParam )
, m_hFont( NULL )
, m_pGPFont( NULL ){
	
};

hm_rs_rawres_font::~hm_rs_rawres_font() {
	for ( unsigned int rrId = REFRES_BEGIN; rrId<REFRES_COUNT; ++rrId ) {
		doFinalRelase( rrId );
	}
};

HFONT hm_rs_rawres_font::getHFont() {
	_addRefCount( REFRES_HFONT );
	if ( !m_hFont ) {
		throw std::runtime_error("m_hFont is NULL.");
	}
	return m_hFont;
}

Gdiplus::Font* hm_rs_rawres_font::getGPFont() {
	_addRefCount( REFRES_FONT_GP );
	if ( !m_pGPFont ) {
		throw std::runtime_error("m_pFont is NULL");
	}
	return m_pGPFont;
}

void hm_rs_rawres_font::freeHFont() {
	_releaseRefCount( REFRES_HFONT );
}

void hm_rs_rawres_font::freeGPFont() {
	_releaseRefCount( REFRES_FONT_GP );
}

LONG hm_rs_rawres_font::FontStyle2Weight( unsigned int uFontStyle ) {
	LONG lfWeight;

	switch ( uFontStyle ) {
	case FONTSTYLE_REGULAR:
		{
			lfWeight = FW_NORMAL;
		}
		break;
	case FONTSTYLE_BOLD:
		{
			lfWeight = FW_EXTRABOLD;
		}
		break;
	default:
		{
			lfWeight = FW_NORMAL;
		}
		break;
	}

	return lfWeight;
}

BYTE hm_rs_rawres_font::FaceName2CharSet( const string_type& strFaceName ) {
	if ( 0 == _tcscmp( strFaceName.c_str(), _T("ºÚÌå") ) ) {
		return GB2312_CHARSET;
	} else if ( 0 == _tcscmp( strFaceName.c_str(), _T("Arial") ) ) {
		return GB2312_CHARSET;
	} else {
		return GB2312_CHARSET;
	}
}

Gdiplus::FontStyle hm_rs_rawres_font::FontStyle2GP( unsigned int uFontStyle ) {
	if ( uFontStyle & FONTSTYLE_REGULAR ) {
		if ( uFontStyle & FONTSTYLE_ITALIC )
			return Gdiplus::FontStyleItalic;
		else
		    return Gdiplus::FontStyleRegular;
	} else if ( uFontStyle & FONTSTYLE_BOLD ) {
		if ( uFontStyle & FONTSTYLE_ITALIC )
			return Gdiplus::FontStyleBoldItalic;
		else 
			return Gdiplus::FontStyleBold;
	}

	return Gdiplus::FontStyleRegular;
}


Gdiplus::Unit hm_rs_rawres_font::FontUnit2GP( unsigned int uFontUnit ) {
	switch ( uFontUnit ) 
	{
	case FONTUNIT_POINT:
		{ 
			return Gdiplus::UnitPoint;
		}
		break;
	case FONTUNIT_PIXEL:
		{
			return Gdiplus::UnitPixel;
		}
		break;
	default:
		return Gdiplus::UnitPoint;
		break;
	} 
	return Gdiplus::UnitPoint;
}

void hm_rs_rawres_font::doFirstRef( refres_id_type rrId ) {
	switch ( rrId ) 
	{
	case REFRES_HFONT:
		{
			if ( m_hFont ) {
				throw std::runtime_error("before call doFirstRef(), m_hFont is not NULL.");
			}
			LOGFONT logFont;
			HDC hFontDC;

			hFontDC = ::GetDC( NULL );
			CFuncPack fpkReleaseDC( ::ReleaseDC, (HWND)NULL, hFontDC );
			memset( &logFont, 0, sizeof(LOGFONT) );
			logFont.lfHeight = MulDiv( m_fontParam.m_uSize, -::GetDeviceCaps( hFontDC,  LOGPIXELSY ), 72 );// pt -> px through the DC real DPI ( negative = char height )
			logFont.lfWeight = FontStyle2Weight( m_fontParam.m_uFlagStyle );
			logFont.lfCharSet = FaceName2CharSet( m_fontParam.m_strFaceName );
			logFont.lfItalic = ( m_fontParam.m_uFlagStyle & FONTSTYLE_ITALIC );
			_tcscpy_s( logFont.lfFaceName, m_fontParam.m_strFaceName.c_str() );//
			m_hFont = ::CreateFontIndirect( &logFont );
			
		}
		break;
	case REFRES_FONT_GP:
		{
			HDC hFontDC;
			TCHAR szFaceName[ LF_FACESIZE ];

			_tcscpy_s( szFaceName, m_fontParam.m_strFaceName.c_str() );
			hFontDC = ::GetDC( NULL );
			CFuncPack fpkReleaseDC( ::ReleaseDC, (HWND)NULL, hFontDC );
			USES_CONVERSION;
			m_pGPFont = new Gdiplus::Font( T2W( szFaceName ), (Gdiplus::REAL)m_fontParam.m_uSize, FontStyle2GP( m_fontParam.m_uFlagStyle ), FontUnit2GP( m_fontParam.m_uFlagSizeUnit ) );

		}
		break;
	default:
		break;
	}
}

void hm_rs_rawres_font::doFinalRelase( refres_id_type rrId ) {
	switch ( rrId ) 
	{
	case REFRES_HFONT:
		{
			if ( m_hFont )
				DeleteObject( m_hFont );
			m_hFont = NULL;
		}
		break;
	case REFRES_FONT_GP:
		{
			if ( m_pGPFont )
				delete m_pGPFont;
			m_pGPFont = NULL;
		}
		break;
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_FACTORY( hm_rs_rawres_color )
HMLIB_API hm_rawres_color_factory g_factoryRawColor;
//
hm_rs_rawres_color::hm_rs_rawres_color( const COLORREF& clrData ) 
: m_clrData( clrData )
, m_hBrush( NULL )
{
	m_gpclrData.SetFromCOLORREF( clrData );
}
//
hm_rs_rawres_color::~hm_rs_rawres_color()
{
	//
	for ( unsigned int rrId = REFRES_BEGIN; rrId<REFRES_COUNT; ++rrId ) {
		doFinalRelase( rrId );
	}
}
//
void hm_rs_rawres_color::doFirstRef( refres_id_type rrId )
{
	switch ( rrId )
	{
	case REFRES_HBRUSH:
		{
			m_hBrush = ::CreateSolidBrush( m_clrData );
		}
		break;
	default:
		break;
	}
	
}
//
void hm_rs_rawres_color::doFinalRelase( refres_id_type rrId )
{
	switch ( rrId )
	{
	case REFRES_HBRUSH:
		{
			if ( !m_hBrush ) {
				return;
			}
			::DeleteObject( m_hBrush );
			m_hBrush = NULL;
		}
		break;
	default:
		break;
	}

}
//
HBRUSH hm_rs_rawres_color::getHBrush() {
	_addRefCount( REFRES_HBRUSH );
	if ( !m_hBrush ) {
		throw std::runtime_error("m_hBrush is NULL.");
	}
	return m_hBrush;
}
//
void hm_rs_rawres_color::freeHBrush() {
	_releaseRefCount( REFRES_HBRUSH );
}
