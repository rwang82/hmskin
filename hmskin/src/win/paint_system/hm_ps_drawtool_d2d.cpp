#include "stdafx.h"
#include "win\paint_system\hm_ps_drawtool_d2d.h"
#include "win\paint_system\hm_ps_drawtool_gdiplus.h"
#include "win\resource_system\hm_rs_uires_text.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <map>
#include "win\frame\hm_perf_stats.h"

// forwarding target for everything but text draw ( hybrid M1 backend )
static hm_ps_drawtool_gdiplus* _gdiplus() {
	return hm_ps_drawtool_gdiplus::getInstance();
}

// ---- private state ( hidden from the header via m_pImpl ) ----
struct D2DImpl {
	ID2D1Factory* m_pD2DFactory;
	IDWriteFactory* m_pDWriteFactory;
	ID2D1DCRenderTarget* m_pDCRT;
	// bare text formats ( face/weight/style/size only ): alignment/wrapping
	// are set per-draw so LEFT and VCENTER consumers can't pollute each other
	std::map< hm_rs_rawres_font*, IDWriteTextFormat* > m_mapFont2TextFormat;

	// shaped layouts, keyed by the full draw request ( font, rect, style,
	// text ): DrawTextW rebuilt + reshaped one of these on EVERY call -
	// caching them takes layout/shaping off the hot path so what remains
	// per draw isolates the DCRT BindDC/EndDraw round trip ( the number
	// the M2 batch design needs ). alignment/wrapping are snapshotted into
	// the layout at CreateTextLayout time, so per-key state can't leak.
	struct LayoutKey {
		hm_rs_rawres_font* pFont;
		float w, h;
		int nAlign;          // hm_uires_text_data::EALGINMENT_*
		BOOL bWrap;
		std::wstring strText;
		bool operator<( const LayoutKey& r ) const {
			if ( pFont != r.pFont ) return pFont < r.pFont;
			if ( w != r.w ) return w < r.w;
			if ( h != r.h ) return h < r.h;
			if ( nAlign != r.nAlign ) return nAlign < r.nAlign;
			if ( bWrap != r.bWrap ) return bWrap < r.bWrap;
			return strText < r.strText;
		}
	};
	std::map< LayoutKey, IDWriteTextLayout* > m_mapTextLayout;
	// solid brushes keyed by color, owned by the DCRT ( dropped with it )
	std::map< COLORREF, ID2D1SolidColorBrush* > m_mapBrush;

	D2DImpl() : m_pD2DFactory( NULL ), m_pDWriteFactory( NULL ), m_pDCRT( NULL ) {}
	// INTENTIONALLY no Release() here: this struct lives in a process-wide
	// singleton whose destructor runs during static destruction / DLL unload
	// ( loader-lock context ) - releasing D2D/DWrite objects there crashes
	// ( observed abort at process exit ). the singleton never dies mid-run
	// ( only the process exit tears it down ), so the "leak" is reclaimed by
	// the OS and harmless. standard practice for D2D/DXGI in statics.
	~D2DImpl() {}

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
		std::map< hm_rs_rawres_font*, IDWriteTextFormat* >::iterator itFormat =
		    m_mapFont2TextFormat.find( pRawResFont );
		if ( itFormat != m_mapFont2TextFormat.end() )
			return itFormat->second;
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

	// find-or-create the shaped layout for one exact draw request.
	// *pnCacheHit reports the outcome for perf tagging ( 1 hit / 0 miss )
	IDWriteTextLayout* getTextLayout( HDC hDC, hm_uires_text_data* pText,
	                                  hm_rs_rawres_font* pRawResFont,
	                                  const D2D1_RECT_F& rcLayout, bool bMultiLine,
	                                  DWRITE_TEXT_ALIGNMENT eTextAlign,
	                                  DWRITE_PARAGRAPH_ALIGNMENT eParaAlign,
	                                  int* pnCacheHit ) {
		LayoutKey key;
		key.pFont = pRawResFont;
		key.w = rcLayout.right;
		key.h = rcLayout.bottom;
		key.nAlign = (int)pText->m_eAlginment;
		key.bWrap = bMultiLine ? TRUE : FALSE;
		key.strText = pText->m_tstrData;
		std::map< LayoutKey, IDWriteTextLayout* >::iterator it = m_mapTextLayout.find( key );
		if ( it != m_mapTextLayout.end() ) {
			*pnCacheHit = 1;
			return it->second;
		}
		*pnCacheHit = 0;   // served by on-the-fly construction below
		// crude cap: beyond 256 live keys the working set has clearly moved
		// ( scrolling text, event log ) - drop everything, they rebuild cheap
		if ( m_mapTextLayout.size() >= 256 ) {
			for ( std::map< LayoutKey, IDWriteTextLayout* >::iterator i = m_mapTextLayout.begin();
			      i != m_mapTextLayout.end(); ++i ) {
				if ( i->second ) i->second->Release();
			}
			m_mapTextLayout.clear();
		}
		IDWriteTextFormat* pFormat = getTextFormat( hDC, pRawResFont );
		if ( !pFormat )
			return NULL;
		pFormat->SetTextAlignment( eTextAlign );
		pFormat->SetParagraphAlignment( eParaAlign );
		pFormat->SetWordWrapping( bMultiLine ? DWRITE_WORD_WRAPPING_WRAP
		                                     : DWRITE_WORD_WRAPPING_NO_WRAP );
		IDWriteTextLayout* pLayout = NULL;
		HRESULT hr = m_pDWriteFactory->CreateTextLayout(
		    key.strText.c_str(), (UINT32)key.strText.size(), pFormat,
		    key.w, key.h, &pLayout );
		if ( FAILED( hr ) || !pLayout )
			return NULL;
		m_mapTextLayout[ key ] = pLayout;
		return pLayout;
	}

	// find-or-create a solid brush ( usable while the creating DCRT lives )
	ID2D1SolidColorBrush* getBrush( COLORREF clr ) {
		std::map< COLORREF, ID2D1SolidColorBrush* >::iterator it = m_mapBrush.find( clr );
		if ( it != m_mapBrush.end() )
			return it->second;
		if ( !m_pDCRT )
			return NULL;
		D2D1_COLOR_F clrF = D2D1::ColorF( GetRValue( clr ) / 255.0f,
		                                  GetGValue( clr ) / 255.0f,
		                                  GetBValue( clr ) / 255.0f );
		ID2D1SolidColorBrush* pBrush = NULL;
		if ( FAILED( m_pDCRT->CreateSolidColorBrush( clrF, &pBrush ) ) )
			return NULL;
		m_mapBrush[ clr ] = pBrush;
		return pBrush;
	}

	// brushes belong to the DCRT: when the target dies, so do they
	void dropBrushCache() {
		for ( std::map< COLORREF, ID2D1SolidColorBrush* >::iterator i = m_mapBrush.begin();
		      i != m_mapBrush.end(); ++i ) {
			if ( i->second ) i->second->Release();
		}
		m_mapBrush.clear();
	}
};

hm_ps_drawtool_d2d::hm_ps_drawtool_d2d()
	: m_pImpl( new D2DImpl() ) {
}

hm_ps_drawtool_d2d::~hm_ps_drawtool_d2d() {
	delete ( D2DImpl* )m_pImpl;
	m_pImpl = NULL;
}

hm_ps_drawtool_d2d* hm_ps_drawtool_d2d::getInstance() {
	static hm_ps_drawtool_d2d s_instance;
	return &s_instance;
}

// ---- forwarded: bitmap / color / measurement ( hybrid M1 ) ----

bool hm_ps_drawtool_d2d::loadRes( hm_rs_rawres_pic* pRawResPic ) {
	return _gdiplus()->loadRes( pRawResPic );
}
bool hm_ps_drawtool_d2d::unloadRes( hm_rs_rawres_pic* pRawResPic ) {
	return _gdiplus()->unloadRes( pRawResPic );
}
void hm_ps_drawtool_d2d::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_pic* pRawResPic, const RECT& rcRawResPart ) {
	_gdiplus()->draw( hDC, rcCanvas, pRawResPic, rcRawResPart );
}
bool hm_ps_drawtool_d2d::loadRes( hm_rs_rawres_color* pRawResColor ) {
	return _gdiplus()->loadRes( pRawResColor );
}
bool hm_ps_drawtool_d2d::unloadRes( hm_rs_rawres_color* pRawResColor ) {
	return _gdiplus()->unloadRes( pRawResColor );
}
bool hm_ps_drawtool_d2d::draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_color* pRawResColor ) {
	return _gdiplus()->draw( hDC, rcCanvas, pRawResColor );
}
bool hm_ps_drawtool_d2d::calcRect( HDC hDC, RECT& rcCanvas, const TCHAR* szText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	// GDI+ metrics on purpose: layouts built on them must not shift while the
	// draw side A/Bs glyph quality ( see header comment )
	return _gdiplus()->calcRect( hDC, rcCanvas, szText, pRawResFont, pRawResColor, bMultiLine );
}

// ---- font lifecycle: forward for the GDI+ object, own the DWrite format ----

bool hm_ps_drawtool_d2d::loadRes( hm_rs_rawres_font* pRawResFont ) {
	// keep the GDI+ font object/refcount alive ( forwarded paths may still
	// measure with it ); the DirectWrite format is created lazily on first
	// draw, when an hDC is available for the DPI probe
	return _gdiplus()->loadRes( pRawResFont );
}
bool hm_ps_drawtool_d2d::unloadRes( hm_rs_rawres_font* pRawResFont ) {
	D2DImpl* pImpl = ( D2DImpl* )m_pImpl;
	std::map< hm_rs_rawres_font*, IDWriteTextFormat* >::iterator itFormat =
	    pImpl->m_mapFont2TextFormat.find( pRawResFont );
	if ( itFormat != pImpl->m_mapFont2TextFormat.end() ) {
		if ( itFormat->second )
			itFormat->second->Release();
		pImpl->m_mapFont2TextFormat.erase( itFormat );
	}
	// layouts keyed on this font pointer: drop them all so a reused address
	// can never serve a stale typeface ( unload is rare, rebuild is cheap )
	for ( std::map< D2DImpl::LayoutKey, IDWriteTextLayout* >::iterator i =
	          pImpl->m_mapTextLayout.begin(); i != pImpl->m_mapTextLayout.end(); ++i ) {
		if ( i->second )
			i->second->Release();
	}
	pImpl->m_mapTextLayout.clear();
	return _gdiplus()->unloadRes( pRawResFont );
}

// ---- text draw: the point of this backend ----

bool hm_ps_drawtool_d2d::draw( HDC hDC, const RECT& rcCanvas, hm_uires_text_data* pText,
                               hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine ) {
	if ( !pText || !pRawResFont || !pRawResColor )
		return false;
	hm_perf_text_guard perf( HM_PERF_TOOL_D2D, (unsigned int)pText->m_tstrData.size(),
	                         pText->m_tstrData.c_str() );
	D2DImpl* pImpl = ( D2DImpl* )m_pImpl;
	if ( !pImpl->ensureDCRT( hDC, rcCanvas ) )
		return false;

	// alignment mapping mirrors hm_ps_drawtool_gdiplus::draw exactly,
	// including its default: vertical TOP unless VCENTER asks for center
	// ( GDI+ LineAlignment defaults to Near - centering every single-line
	// label here would shift the whole skin's text baseline ).
	// single line: NO_WRAP, never a line-limit flag ( the LineLimit lesson:
	// fractional-DPI lines must render bottom-clipped, not vanish ).
	// ELLIPSIS is ignored, mirroring the GDI+ backend ( never consumed ).
	DWRITE_TEXT_ALIGNMENT eTextAlign = DWRITE_TEXT_ALIGNMENT_LEADING;
	DWRITE_PARAGRAPH_ALIGNMENT eParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
	if ( pText->m_eAlginment == hm_uires_text_data::EALGINMENT_RIGHT ) {
		eTextAlign = DWRITE_TEXT_ALIGNMENT_TRAILING;
	} else if ( pText->m_eAlginment == hm_uires_text_data::EALGINMENT_VCENTER ) {
		eTextAlign = DWRITE_TEXT_ALIGNMENT_CENTER;
		eParaAlign = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
	}

	// layout rect is relative to the BindDC rect origin; the layout itself
	// comes from the cache ( key includes rect/style/text, state snapshotted )
	D2D1_RECT_F rcLayout = D2D1::RectF( 0.0f, 0.0f,
	    (float)( rcCanvas.right - rcCanvas.left ),
	    (float)( rcCanvas.bottom - rcCanvas.top ) );
	int nCacheHit = -1;
	IDWriteTextLayout* pLayout = pImpl->getTextLayout( hDC, pText, pRawResFont,
	                                                   rcLayout, bMultiLine,
	                                                   eTextAlign, eParaAlign,
	                                                   &nCacheHit );
	perf.setCacheHit( nCacheHit );   // tag the sample: hit vs miss population
	if ( !pLayout )
		return false;
	ID2D1SolidColorBrush* pBrush = pImpl->getBrush( pRawResColor->getCOLORREF() );
	if ( !pBrush )
		return false;

	pImpl->m_pDCRT->BeginDraw();
	pImpl->m_pDCRT->DrawTextLayout( D2D1::Point2F( 0.0f, 0.0f ), pLayout, pBrush );
	HRESULT hr = pImpl->m_pDCRT->EndDraw();
	if ( hr == D2DERR_RECREATE_TARGET ) {
		// fresh target, mandatory rebind; layouts/formats survive ( they
		// belong to the DWrite factory, not the render target ), brushes
		// do not - drop the cache and rebuild on demand. one retry.
		pImpl->m_pDCRT->Release();
		pImpl->m_pDCRT = NULL;
		pImpl->dropBrushCache();
		if ( pImpl->ensureDCRT( hDC, rcCanvas ) ) {
			ID2D1SolidColorBrush* pBrush2 = pImpl->getBrush( pRawResColor->getCOLORREF() );
			if ( pBrush2 ) {
				pImpl->m_pDCRT->BeginDraw();
				pImpl->m_pDCRT->DrawTextLayout( D2D1::Point2F( 0.0f, 0.0f ), pLayout, pBrush2 );
				hr = pImpl->m_pDCRT->EndDraw();
			} else {
				hr = E_FAIL;
			}
		} else {
			hr = E_FAIL;
		}
	}
	return SUCCEEDED( hr );
}
