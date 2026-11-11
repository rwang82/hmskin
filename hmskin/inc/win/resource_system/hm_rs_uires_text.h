#ifndef __HM_RS_UIRES_TEXT_H__
#define __HM_RS_UIRES_TEXT_H__
#include "win\resource_system\hm_rs_uires_i.h"
#include "win\resource_system\hm_rs_rawres_base.h"
#include "win\hm_export_defs.h"
#include "tchar.h"
#include <string>

struct HMLIB_API hm_uires_text_data {
public:
	typedef std::basic_string< TCHAR > tstring_type;
	typedef enum _ENUMTEXTFALG {
		TEXTFLAG_SINGLELINE = 0x00,
		TEXTFLAG_MULTILINE = 0x01,
		TEXTFLAG_ELLIPSIS = 0x02
	} ENUMTEXTFALG;
	typedef enum _ENUMALGINMENT {
		EALGINMENT_LEFT = 0,
		EALGINMENT_VCENTER,
		EALGINMENT_RIGHT
	} ENUMALGINMENT;
public:
	hm_uires_text_data();
	~hm_uires_text_data();

public:
	void setAlignment( int nAlignment );

public:
#pragma warning(push)
#pragma warning(disable:4251)
	tstring_type m_tstrData;
#pragma warning(pop)
	hm_rs_rawres_font_factory::obj_id_type m_idRawResFont;
	hm_rs_rawres_color_factory::obj_id_type m_idRawResColor;
	ENUMTEXTFALG m_eTextFlag;
	ENUMALGINMENT m_eAlginment;
	int m_nAlignment;
};

class HMLIB_API hm_rs_uires_text: public hm_rs_uires_i{
public:
	typedef std::basic_string< TCHAR > tstring_type;
public:
	hm_rs_uires_text( const hm_uires_text_data& data );
	virtual ~hm_rs_uires_text();

	// interface of hm_rs_uires_i
public:
	virtual void show( HDC hDC, const RECT& rcCanvas, hm_ps_drawtool_i* pDrawTool );
	//
	virtual bool loadRawRes( hm_ps_drawtool_i* pDrawTool );
	//
	virtual bool unloadRawRes( hm_ps_drawtool_i* pDrawTool );

public:
	const hm_uires_text_data* getData() { return &m_data; }
	void setFontId( unsigned int uFontId );
	void setColorId( unsigned int uColorId );
	void setText( const tstring_type& tstrText );

protected:
	hm_rs_rawres_font* getRawResFont();
	hm_rs_rawres_color* getRawResColor();

protected:
	hm_uires_text_data m_data;
	hm_rs_rawres_font* m_pRawResFont;
	hm_rs_rawres_color* m_pRawResColor;
};

#endif //__HM_RS_UIRES_TEXT_H__