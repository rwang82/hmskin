#ifndef __HM_RS_RAWRES_BASE_H__
#define __HM_RS_RAWRES_BASE_H__
#include <map>
#include <string>
#include "common\hm_cmn_pattern_factory.h"
#include "common\hm_cmn_refcnt.h"
#include "win\hm_export_defs.h"
#include "tchar.h"
//for gdi+ begin
#include <unknwn.h>
#include <Rpc.h>
#include <gdiplus.h>
//for gdi+ end 
//////////////////////////////////////////////////////////////////////////
class hm_ps_drawtool_gdi;
class hm_ps_drawtool_gdiplus;
//////////////////////////////////////////////////////////////////////////////////////
class HMLIB_API hm_rs_rawres_pic: public hm_cmn_refcnt
{
//type define.
public:
	typedef std::basic_string< TCHAR > string_type;
	typedef enum _ENUMREFRES {
		REFRES_BEGIN = 0,
		REFRES_IMAGE_GP,
		REFRES_COUNT
	} ENUMREFRES;

public:
	hm_rs_rawres_pic( const TCHAR* szPicPath );
	virtual ~hm_rs_rawres_pic();

public:
	const TCHAR* getPicPath() { return m_strPicPath.c_str(); }
	Gdiplus::Image* getGPImage();
	void freeGPImage();
	void fetchSize( SIZE& sizePic );

	// interface of hm_cmn_refcnt
protected:
	virtual void doFirstRef( refres_id_type rrId );
	virtual void doFinalRelase( refres_id_type rrId );
	
protected:
#pragma warning(push)
#pragma warning(disable:4251)
	string_type m_strPicPath;
#pragma warning(pop)
	Gdiplus::Image* m_pGPImage;
	SIZE m_sizePic;
};
//
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( hm_rs_rawres_pic )
#pragma warning(pop)
#define hm_rawres_pic_factory FACTORY_TYPE( hm_rs_rawres_pic )
extern HMLIB_API hm_rawres_pic_factory g_factoryRawPic;
#define RAWPICFACTORY_INSTANCE (&g_factoryRawPic)

//////////////////////////////////////////////////////////////////////////////////////
#define FONTSTYLE_REGULAR ( 0x01 )
#define FONTSTYLE_BOLD ( 0x02 )
#define FONTSTYLE_ITALIC ( 0x04 )
//
#define FONTUNIT_POINT ( 0x00 )
#define FONTUNIT_PIXEL ( 0x01 )
//
struct HMLIB_API hm_rs_font_param {
public:
	typedef std::basic_string< TCHAR > string_type;
public:
#pragma warning(push)
#pragma warning(disable:4251)
	// face name, e.g. "SimSun", "Arial"
#pragma warning(pop)
	unsigned int m_uSize; // specifies the size of the font measured in units identified by the third argument.
	unsigned int m_uFlagSizeUnit; // FONTUNIT_PIXEL, FONTUNIT_POINT is Specifies that a unit is 1 point or 1/72 inch. 
	unsigned int m_uFlagStyle; // FONTSTYLE_REGULAR or FONTSTYLE_BOLD, FONTSTYLE_ITALIC, or combine two
};
//
class HMLIB_API hm_rs_rawres_font : public hm_cmn_refcnt
{
//type define.
public:
	typedef std::basic_string< TCHAR > string_type;	
	typedef enum _ENUMREFRES {
		REFRES_BEGIN = 0,
		REFRES_HFONT = 0,
		REFRES_FONT_GP,
		REFRES_COUNT
	} ENUMREFRES;

public:
	hm_rs_rawres_font( const hm_rs_font_param& fontParam );
	virtual ~hm_rs_rawres_font();
	
	// interface of hm_cmn_refcnt
protected:
	virtual void doFirstRef( refres_id_type rrId );
	virtual void doFinalRelase( refres_id_type rrId );

	// attribute
public:
	HFONT getHFont();
	void freeHFont();
	Gdiplus::Font* getGPFont();
	void freeGPFont();
	// raw 96-dpi font params ( face/size/unit/style ) for backends that build
	// their own font objects ( e.g. hm_ps_drawtool_d2d / DirectWrite )
	const hm_rs_font_param& getFontParam() const { return m_fontParam; }

	// helper
protected:
	// for m_hFont
	LONG FontStyle2Weight( unsigned int uFontStyle );
	BYTE FaceName2CharSet( const string_type& strFaceName );
	// for m_pGPFont
	Gdiplus::FontStyle FontStyle2GP( unsigned int uFontStyle );
	Gdiplus::Unit FontUnit2GP( unsigned int uFontUnit );;

protected:
	hm_rs_font_param m_fontParam;
	HFONT m_hFont;
	Gdiplus::Font* m_pGPFont;
};
//
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( hm_rs_rawres_font )
#pragma warning(pop)
#define hm_rawres_font_factory FACTORY_TYPE( hm_rs_rawres_font )
extern HMLIB_API hm_rawres_font_factory g_factoryRawFont;
#define RAWFONTFACTORY_INSTANCE ( &g_factoryRawFont )

//////////////////////////////////////////////////////////////////////////////////////
class HMLIB_API hm_rs_rawres_color : public hm_cmn_refcnt
{
public:
	typedef enum _ENUMREFRES {
		REFRES_BEGIN = 0,
		REFRES_COLORREF = 0,
		REFRES_COLOR_GP,
		REFRES_HBRUSH,
		REFRES_COUNT
	} ENUMREFRES;
public:
	hm_rs_rawres_color( const COLORREF& clrData );
	virtual ~hm_rs_rawres_color();

	// interface of hm_cmn_refcnt
protected:
	virtual void doFirstRef( refres_id_type rrId );
	virtual void doFinalRelase( refres_id_type rrId );

public:
	HBRUSH getHBrush();
	void freeHBrush();
	const COLORREF& getCOLORREF() { return m_clrData; }
	const Gdiplus::Color& getGPColor() { return m_gpclrData; }


private:
	COLORREF m_clrData;
#pragma warning(push)
#pragma warning(disable:4251)
	Gdiplus::Color m_gpclrData; 
#pragma warning(pop)
	HBRUSH m_hBrush;
};
//
#pragma warning(push)
#pragma warning(disable:4251)
DECLARE_FACTORY_EXPORTS( hm_rs_rawres_color )
#pragma warning(pop)
#define hm_rawres_color_factory FACTORY_TYPE( hm_rs_rawres_color )
extern HMLIB_API hm_rawres_color_factory g_factoryRawColor;
#define RAWCOLORFACTORY_INSTANCE ( &g_factoryRawColor )










#endif //__HM_RS_RAWRES_BASE_H__
