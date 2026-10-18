#ifndef __HM_PS_DRAWTOOL_D2D_H__
#define __HM_PS_DRAWTOOL_D2D_H__
#include "win\hm_export_defs.h"
#include "win\paint_system\hm_ps_drawtool_i.h"

///////////////////////////////////////////////////////////////
// hybrid backend, D2D-M1 step:
//   text draw          -> Direct2D DC render target + DirectWrite ( this class )
//   pic/color + calcRect -> forwarded to hm_ps_drawtool_gdiplus
//
// why the forwards: calcRect keeps GDI+ metrics so existing layouts don't
// shift a single pixel while we A/B the glyph quality; bitmap/color paths
// stay on the proven code until M2 migrates them. manifest switches a WMP
// to this backend with "DrawTool": "D2D".
//
// D2D/DWrite state lives behind m_pImpl ( defined in the .cpp ) so this
// header carries no COM names at all - include order stays a non-topic.
//
// perf note: text layouts and solid brushes are cached per key - the
// DrawTextW convenience call rebuilt + reshaped an IDWriteTextLayout on
// every draw. the cache isolates the remaining per-draw cost ( the DCRT
// BindDC/EndDraw round trip ) for the M2 batch design decision.
class HMLIB_API hm_ps_drawtool_d2d : public hm_ps_drawtool_i {
public:
	virtual ~hm_ps_drawtool_d2d();

public:
	static hm_ps_drawtool_d2d* getInstance();

public:
	// hm_rs_rawres_pic
	virtual bool loadRes( hm_rs_rawres_pic* pRawResPic );
	virtual bool unloadRes( hm_rs_rawres_pic* pRawResPic );
	virtual void draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_pic* pRawResPic, const RECT& rcRawResPart );
	// hm_rs_rawres_font
	virtual bool loadRes( hm_rs_rawres_font* pRawResFont );
	virtual bool unloadRes( hm_rs_rawres_font* pRawResFont );
	// hm_rs_rawres_color
	virtual bool loadRes( hm_rs_rawres_color* pRawResColor );
	virtual bool unloadRes( hm_rs_rawres_color* pRawResColor );
	virtual bool draw( HDC hDC, const RECT& rcCanvas, hm_rs_rawres_color* pRawResColor );
	//
	virtual bool draw( HDC hDC, const RECT& rcCanvas, hm_uires_text_data* pText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine );
	virtual bool calcRect( HDC hDC, RECT& rcCanvas, const TCHAR* szText, hm_rs_rawres_font* pRawResFont, hm_rs_rawres_color* pRawResColor, bool bMultiLine );

private:
	hm_ps_drawtool_d2d();

private:
	void* m_pImpl;   // struct _d2d_impl ( see .cpp ): factories, DCRT, text-format cache
};

#endif //__HM_PS_DRAWTOOL_D2D_H__
