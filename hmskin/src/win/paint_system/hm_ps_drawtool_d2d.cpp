#include "stdafx.h"
#include "win\paint_system\hm_ps_drawtool_d2d.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <map>

// forwarding target for everything but text draw ( hybrid M1 backend )
static hm_ps_drawtool_gdiplus* _gp() {
	return hm_ps_drawtool_gdiplus::getInstance();
}

// ---- private state ( hidden from the header via m_pImpl ) ----
struct _d2d_impl {
	ID2D1Factory* m_pD2DFactory;
	IDWriteFactory* m_pDWriteFactory;
	ID2D1DCRenderTarget* m_pDCRT;
	// bare text formats ( face/weight/style/size only ): alignment/wrapping
	// are set per-draw so LEFT and VCENTER consumers can't pollute each other
	std::map< hm_rs_rawres_font*, IDWriteTextFormat* > m_mapFont2TextFormat;

	_d2d_impl() : m_pD2DFactory( NULL ), m_pDWriteFactory( NULL ), m_pDCRT( NULL ) {}
	~_d2d_impl() {
		std::map< hm_rs_rawres_font*, IDWriteTextFormat* >::iterator it;
		for ( it = m_mapFont2TextFormat.begin(); it != m_mapFont2TextFormat.end(); ++it ) {
			if ( it->second )
				it->second->Release();
		}
		m_mapFont2TextFormat.clear();
		if ( m_pDCRT ) { m_pDCRT->Release(); m_pDCRT = NULL; }
		if ( m_pDWriteFactory ) { m_pDWriteFactory->Release(); m_pDWriteFactory = NULL; }
		if ( m_pD2DFactory ) { m_pD2DFactory->Release(); m_pD2DFactory = NULL; }
	}

	// lazy idempotent init of the two factories
	bool ensureFactories() {
		if ( m_pD2DFactory && m_pDWriteFactory )
			return true;
		if ( !m_pD2DFactory ) {
			if ( FAILED( D2D1CreateFactory( D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory ) ) )
				return false;
		}
		if ( !m_pDWriteFactory ) {
			if ( FAILED( DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ),
			                                  reinterpret_cast< IUnknown** >( &m_pDWriteFactory ) ) ) )
				return false;
		}
		return true;
	}

	// lazy (re)creation of the DC render target, then BindDC to hDC/rcBind
	bool ensureDCRT( HDC hDC, const RECT& rcBind ) {
		if ( !m_pDCRT ) {
			if ( !ensureFactories() )
				return false;
			// DPI stays at the default 96: engine coordinates are already
			// physical pixels ( loader throat-scaled ), so 1 DIP == 1 px here.
			// setting a DPI would double-scale ( the GDI+ point-font lesson's
			// sibling ).
			D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
			    D2D1_RENDER_TARGET_TYPE_DEFAULT,
			    D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED ) );
			if ( FAILED( m_pD2DFactory->CreateDCRenderTarget( &props, &m_pDCRT ) ) )
				return false;
		}
		if ( FAILED( m_pDCRT->BindDC( hDC, &rcBind ) ) )
			return false;
		return true;
	}

	// find-or-create the bare text format for pFont, sized with the DPI
	// probed from hDC ( same semantics as the HFONT path in
	// hm_rs_rawres_base.cpp: MulDiv( pt, LOGPIXELSY, 72 ) )
	IDWriteTextFormat* getTextFormat( HDC hDC, hm_rs_rawres_font* pRawResFont ) {
		std::map< hm_rs_rawres_font*, IDWriteTextFormat* >::iterator it =
		    m_mapFont2TextFormat.find( pRawResFont );
		if ( it != m_mapFont2TextFormat.end() )
			return it->second;
		if ( !ensureFactories() )
			return NULL;

		const hm_rs_font_param& param = pRawResFont->getFontParam();
		// font sizes are stored 96-dpi raw in the manifest ( the loader
		// deliberately doesn't scale them - GDI+ converts POINT natively
		// through the Graphics DPI ); DirectWrite has no such auto-conversion
		float fontSize = (float)param.m_uSize;
		if ( param.m_uFlagSizeUnit == FONTUNIT_POINT ) {
			int nDpiY = ::GetDeviceCaps( hDC, LOGPIXELSY );
			fontSize = (float)::MulDiv( (int)param.m_uSize, nDpiY, 72 );
		}
		DWRITE_FONT_WEIGHT weight = ( param.m_uFlagStyle & FONTSTYLE_BOLD )
		    ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
		DWRITE_FONT_STYLE style = ( param.m_uFlagStyle & FONTSTYLE_ITALIC )
		    ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;

		IDWriteTextFormat* pFormat = NULL;
		HRESULT hr = m_pDWriteFactory->CreateTextFormat(
		    param.m_strFaceName.c_str(), NULL, weight, style,
		    DWRITE_FONT_STRETCH_NORMAL, fontSize, L"", &pFormat );
		if ( FAILED( hr ) || !pFormat )
			return NULL;
		m_mapFont2TextFormat[ pRawResFont ] = pFormat;
		return pFormat;
	}
};

hm_ps_drawtool_d2d::hm_ps_drawtool_d2d()
	: m_pImpl( new _d2d_impl() ) {
}

hm_ps_drawtool_d2d::~hm_ps_drawtool_d2d() {
	delete ( _d2d_impl* )m_pImpl;
	m_pImpl = NULL;
}

hm_ps_drawtool_d2d* hm_ps_drawtool_d2d::getInstance() {
	static hm_ps_drawtool_d2d s_instance;
	return &s_instance;
}

// ---- forwarded: bitmap / color / measurement ( hybrid M1 ) ----

bool hm_ps_drawtool_d2d::loadRes( hm_rs_rawres_pic* pRawResPic ) {
	return _gp()->loadRes( pRawResPic );
}
bool hm_ps_drawtool_d2d::unloadRes( hm_rs_rawres_pic* pRawResPic ) {
	return _gp()->unloadRes( pRawResPic );
}
void hm_ps_drawtool_d2d::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_pic* pRawResPic, const RECT& rcRawResPart ) {
	_gp()->draw( hDC, rcCanvas, pRawResPic, rcRawResPart );
}
bool hm_ps_drawtool_d2d::loadRes( hm_rs_rawres_color* pRawResColor ) {
	return _gp()->loadRes( pRawResColor );
}
bool hm_ps_drawtool_d2d::unloadRes( hm_rs_rawres_color* pRawResColor ) {
	return _gp()->unloadRes( pRawResColor );
}
bool hm_ps_drawtool_d2d::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_color* pRawResColor ) {
	return _gp()->draw( hDC, rcCanvas, pRawResColor );
}
bool hm_ps_drawtool_d2d::calcRect( HDC hDC, RECT& rcCanvas, const TCHAR* szText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	// GDI+ metrics on purpose: layouts built on them must not shift while the
	// draw side A/Bs glyph quality ( see header comment )
	return _gp()->calcRect( hDC, rcCanvas, szText, pRawResFont, pRawResColor, bMultiLine );
}

// ---- font lifecycle: forward for the GDI+ object, own the DWrite format ----

bool hm_ps_drawtool_d2d::loadRes( hm_rs_rawres_font* pRawResFont ) {
	// keep the GDI+ font object/refcount alive ( forwarded paths may still
	// measure with it ); the DirectWrite format is created lazily on first
	// draw, when an hDC is available for the DPI probe
	return _gp()->loadRes( pRawResFont );
}
bool hm_ps_drawtool_d2d::unloadRes( hm_rs_rawres_font* pRawResFont ) {
	_d2d_impl* p = ( _d2d_impl* )m_pImpl;
	std::map< hm_rs_rawres_font*, IDWriteTextFormat* >::iterator it =
	    p->m_mapFont2TextFormat.find( pRawResFont );
	if ( it != p->m_mapFont2TextFormat.end() ) {
		if ( it->second )
			it->second->Release();
		p->m_mapFont2TextFormat.erase( it );
	}
	return _gp()->unloadRes( pRawResFont );
}

// ---- text draw: the point of this backend ----

bool hm_ps_drawtool_d2d::draw( HDC hDC, const RECT& rcCanvas, hm_uires_text_data* pText,
                               hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	if ( !pText || !pRawResFont || !pRawResColor )
		return false;
	_d2d_impl* p = ( _d2d_impl* )m_pImpl;
	IDWriteTextFormat* pFormat = p->getTextFormat( hDC, pRawResFont );
	if ( !pFormat )
		return false;
	if ( !p->ensureDCRT( hDC, rcCanvas ) )
		return false;

	// alignment mapping mirrors hm_ps_drawtool_gdiplus::draw exactly,
	// including its default: vertical TOP unless VCENTER asks for center
	// ( GDI+ LineAlignment defaults to Near - centering every single-line
	// label here would shift the whole skin's text baseline )
	DWRITE_TEXT_ALIGNMENT eTextAlign = DWRITE_TEXT_ALIGNMENT_LEADING;
	DWRITE_PARAGRAPH_ALIGNMENT eParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
	if ( pText->m_eAlginment == hm_uires_text_data::EALGINMENT_RIGHT ) {
		eTextAlign = DWRITE_TEXT_ALIGNMENT_TRAILING;
	} else if ( pText->m_eAlginment == hm_uires_text_data::EALGINMENT_VCENTER ) {
		eTextAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
		eParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	}
	pFormat->SetTextAlignment( eTextAlign );
	pFormat->SetParagraphAlignment( eParaAlign );
	// single line: NO_WRAP, never a line-limit flag ( the LineLimit lesson:
	// fractional-DPI lines must render bottom-clipped, not vanish ).
	// ELLIPSIS is ignored, mirroring the GDI+ backend ( never consumed ).
	pFormat->SetWordWrapping( bMultiLine ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP );

	const COLORREF& clr = pRawResColor->getCOLORREF();
	D2D1_COLOR_F clrF = D2D1::ColorF( GetRValue( clr ) / 255.0f,
	                                  GetGValue( clr ) / 255.0f,
	                                  GetBValue( clr ) / 255.0f );

	// layout rect is relative to the BindDC rect origin
	D2D1_RECT_F rcLayout = D2D1::RectF( 0.0f, 0.0f,
	    (float)( rcCanvas.right - rcCanvas.left ),
	    (float)( rcCanvas.bottom - rcCanvas.top ) );
	const TCHAR* szText = pText->m_tstrData.c_str();
	UINT32 uTextLen = (UINT32)_tcslen( szText );

	HRESULT hr = S_OK;
	ID2D1SolidColorBrush* pBrush = NULL;
	if ( FAILED( p->m_pDCRT->CreateSolidColorBrush( clrF, &pBrush ) ) )
		return false;
	p->m_pDCRT->BeginDraw();
	p->m_pDCRT->DrawTextW( szText, uTextLen, pFormat, rcLayout, pBrush );
	hr = p->m_pDCRT->EndDraw();
	pBrush->Release();
	if ( hr == D2DERR_RECREATE_TARGET ) {
		// fresh target, mandatory rebind; text formats survive ( they belong
		// to the DWrite factory, not the render target ). one retry.
		p->m_pDCRT->Release();
		p->m_pDCRT = NULL;
		if ( p->ensureDCRT( hDC, rcCanvas ) ) {
			ID2D1SolidColorBrush* pBrush2 = NULL;
			if ( SUCCEEDED( p->m_pDCRT->CreateSolidColorBrush( clrF, &pBrush2 ) ) ) {
				p->m_pDCRT->BeginDraw();
				p->m_pDCRT->DrawTextW( szText, uTextLen, pFormat, rcLayout, pBrush2 );
				hr = p->m_pDCRT->EndDraw();
				pBrush2->Release();
			} else {
				hr = E_FAIL;
			}
		} else {
			hr = E_FAIL;
		}
	}
	return SUCCEEDED( hr );
}
